#include <atomic> // atomic<bool> timer_running = false;
#include <chrono> // chrono::milliseconds(delay)
#include <cstdlib> // rand(), system("clear")
#include <ctime> // time(nullptr)
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace ftxui;

int m = 100;  // 16
int mB = 100; // 2
int hB = 100; // 1
int village = 2;
int level = 1;

enum class AppState { Menu, Village, Combat, Exit };
AppState result = AppState::Menu;
int selected_class = 1;

enum class CombatMode { Action, TurnBased };
CombatMode selected_combat_mode = CombatMode::Action;

struct Maps {
private:
  vector<string> combat_map = {
      "+----------------------------+",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "|                            |",
      "+----------------------------+",
  };

public:
  int height() { return combat_map.size(); }

  int width() { return combat_map[0].size(); }

  string getLine(int row) { return combat_map[row]; }

  void setChar(int y, int x, char ch) {
    if (y >= 0 && y < height() && x >= 0 && x < width()) {
      combat_map[y][x] = ch;
    }
  }
};

struct Bullet {
  int x;
  int y;
};

struct Enemy {
  int x = 20;
  int y = 2;

  void move(int dx) {
    x += dx;
    if (x < 2)
      x = 2;
    if (x > 27)
      x = 27;
  }
};

struct Monster {
  string name;
  int maxHp;
  int hp;
  int attack;
  int xpReward;
  int goldReward;
  bool alwaysDropsGold;

  Monster(string name, int maxHp, int attack, int xpReward, int goldReward,
          bool alwaysDropsGold) {
    this->name = name;
    this->maxHp = maxHp;
    this->hp = maxHp;
    this->attack = attack;
    this->xpReward = xpReward;
    this->goldReward = goldReward;
    this->alwaysDropsGold = alwaysDropsGold;
  }
};

struct CombatTimerConfig {
  int enemy_tick_ms = 400;
  int enemy_shot_chance = 5;
};

struct player {
  string name;
  int maxHp;
  int hp;
  int maxEnergy;
  int energy;
  int gold = 0;
  int level = 0;
  int xp = 0;
  int attack_dmg;

  int figure;

  int x = 5;
  int y = 5;

  player(int f) {
    figure = f;
    switch (f) {
    case 1:
      name = "Paladin";
      maxHp = hp = 100;
      maxEnergy = energy = 100;
      attack_dmg = 3;
      break;
    case 2:
      name = "Lovec";
      maxHp = hp = 100;
      maxEnergy = energy = 100;
      attack_dmg = 4;
      break;
    case 3:
      name = "Mag";
      maxHp = hp = 100;
      maxEnergy = energy = 100;
      attack_dmg = 2;
      break;
    case 4:
      name = "Warlock";
      maxHp = hp = 100;
      maxEnergy = energy = 100;
      attack_dmg = 3;
      break;
    default:
      name = "Neznama classa";
      maxHp = hp = 100;
      maxEnergy = energy = 100;
      attack_dmg = 1;
      break;
    }
  }

  void move(int dx) {
    x += dx;
    if (x < 1)
      x = 1;
    if (x > 26)
      x = 26;
  }
};

struct ActionCombatController {
private:
  ScreenInteractive &screen;
  player &p;
  CombatTimerConfig timer_config;
  Enemy enemy;
  vector<Bullet> bullets;
  atomic<bool> timer_running = false; // atomic<bool> říká: Tahle hodnota se může bezpečně číst a zapisovat z více vláken.
  thread timer_thread; // přidání vlákna

  void TimerLoop() {
    while (timer_running) {
      int delay = timer_config.enemy_tick_ms;
      if (delay < 1) {
        delay = 1;
      }

      this_thread::sleep_for(chrono::milliseconds(delay));

      if (timer_running) {
        screen.PostEvent(Event::Custom); // pošli signál do OnEvent přes CatchEvent, že se má něco stát
      }
    }
  }

  void EnemyTurn() {
    MoveEnemy();
    MoveBullets();
    RemoveOldBullets();
  }

  void DrawPlayer(Maps &map) {
    map.setChar(p.y, p.x + 1, 'O');
    map.setChar(p.y + 1, p.x, '/');
    map.setChar(p.y + 1, p.x + 1, '|');
    map.setChar(p.y + 1, p.x + 2, '\\');
    map.setChar(p.y + 2, p.x, '/');
    map.setChar(p.y + 2, p.x + 2, '\\');
  }

  void DrawEnemy(Maps &map) {
    map.setChar(enemy.y, enemy.x, 'O');
    map.setChar(enemy.y + 1, enemy.x, '|');
    map.setChar(enemy.y + 2, enemy.x - 1, '/');
    map.setChar(enemy.y + 2, enemy.x + 1, '\\');
  }

  void DrawBullets(Maps &map) {
    for (int i = 0; i < bullets.size(); i++) {
      map.setChar(bullets[i].y, bullets[i].x, 'I');
    }
  }

  void MoveEnemy() {
    if (rand() % 2)
      enemy.move(2);
    else
      enemy.move(-2);
  }

  void MoveBullets() {
    if (timer_config.enemy_shot_chance > 0 &&
        rand() % timer_config.enemy_shot_chance == 0) {
      Bullet bullet;
      bullet.x = enemy.x;
      bullet.y = enemy.y + 3;
      bullets.push_back(bullet);
    }

    for (int i = 0; i < bullets.size(); i++) {
      bullets[i].y++;
    }
  }

  void RemoveOldBullets() {
    Maps map;
    vector<Bullet> active_bullets;

    for (int i = 0; i < bullets.size(); i++) {
      if (bullets[i].y < map.height()) {
        active_bullets.push_back(bullets[i]);
      }
    }

    bullets = active_bullets;
  }

public:
  ActionCombatController(ScreenInteractive &screen, player &p,
                         CombatTimerConfig timer_config)
      : screen(screen), p(p), timer_config(timer_config) {}

  ~ActionCombatController() { StopTimer(); }

  void StartTimer() {
    timer_running = true;
    timer_thread = thread(&ActionCombatController::TimerLoop, this); // Do timer_thread vytvoř v aktuálním objektu nové vlákno a v něm spusť metodu TimerLoop() z objektu ActionCombatController
    // & předá adresu metody ActionCombatController::TimerLoop
  }

  void StopTimer() {
    timer_running = false;

    if (timer_thread.joinable()) { // existuje běžící vlákno?
      timer_thread.join(); // Hlavní program počká, až timer vlákno skončí.
    }
  }

  Element Render() {
    Maps visible_map;
    DrawPlayer(visible_map);
    DrawEnemy(visible_map);
    DrawBullets(visible_map);

    vector<Element> rows;
    for (int i = 0; i < visible_map.height(); i++) {
      rows.push_back(text(visible_map.getLine(i)));
    }

    return vbox(rows) | color(Color::Green);
  }

  bool OnEvent(Event event) {
    if (event == Event::Custom) {
      EnemyTurn();
      return true;
    }

    if (event == Event::ArrowLeft) {
      p.move(-2);
      return true;
    }

    if (event == Event::ArrowRight) {
      p.move(2);
      return true;
    }

    if (event == Event::Escape) {
      result = AppState::Menu;
      screen.Exit();
      return true;
    }

    return false;
  }
};

struct TurnBasedCombatController {
private:
  ScreenInteractive &screen;
  player &p;
  vector<Monster> enemies;
  int selected_enemy = 0;
  int selected_action = 0;
  string message = "Tahovy boj pripraven. ESC = navrat do menu.";

  vector<string> actions = {"Utok", "Schopnost 1", "Schopnost 2"};

  void DrawPlayer(Maps &map) {
    int x = 13;
    int y = 12;

    map.setChar(y, x + 1, 'O');
    map.setChar(y + 1, x, '/');
    map.setChar(y + 1, x + 1, '|');
    map.setChar(y + 1, x + 2, '\\');
    map.setChar(y + 2, x, '/');
    map.setChar(y + 2, x + 2, '\\');
  }

  void DrawEnemy(Maps &map, int enemy_index) {
    if (!IsEnemyAlive(enemy_index)) {
      return;
    }

    int x = 8 + enemy_index * 7;
    int y = 2;

    map.setChar(y, x, 'O');
    map.setChar(y + 1, x, '|');
    map.setChar(y + 2, x - 1, '/');
    map.setChar(y + 2, x + 1, '\\');
  }

  Element RenderMap() {
    Maps visible_map;

    for (int i = 0; i < enemies.size(); i++) {
      DrawEnemy(visible_map, i);
    }

    DrawPlayer(visible_map);

    vector<Element> rows;
    for (int i = 0; i < visible_map.height(); i++) {
      rows.push_back(text(visible_map.getLine(i)));
    }

    return vbox(rows) | color(Color::Green);
  }

  Element RenderPlayerStats() {
    return vbox({text("Hrac: " + p.name) | bold,
                 text("HP: " + to_string(p.hp) + "/" + to_string(p.maxHp)),
                 text("Energie: " + to_string(p.energy) + "/" +
                      to_string(p.maxEnergy)),
                 text("Utok: " + to_string(p.attack_dmg)),
                 text("Level: " + to_string(p.level)),
                 text("Zlato: " + to_string(p.gold))});
  }

  Element RenderTargetList() {
    vector<Element> rows;
    rows.push_back(text("Cil utoku:") | bold);

    for (int i = 0; i < enemies.size(); i++) {
      string prefix = "  ";
      if (i == selected_enemy) {
        prefix = "> ";
      }

      string enemy_text = to_string(i + 1) + ") " + enemies[i].name + " HP: " +
                          to_string(enemies[i].hp) + "/" +
                          to_string(enemies[i].maxHp);
      if (!IsEnemyAlive(i)) {
        enemy_text = enemy_text + " (porazen)";
      }
      rows.push_back(text(prefix + enemy_text));
    }

    return vbox(rows);
  }

  Element RenderActionList() {
    vector<Element> rows;
    rows.push_back(text("Akce:") | bold);

    for (int i = 0; i < actions.size(); i++) {
      string prefix = "  ";
      if (i == selected_action) {
        prefix = "> ";
      }

      rows.push_back(text(prefix + actions[i]));
    }

    return vbox(rows);
  }

  Element RenderSidePanel() {
    return vbox({RenderPlayerStats(), separator(), RenderTargetList(),
                 separator(), RenderActionList()}) |
           border;
  }

  bool IsEnemyAlive(int enemy_index) {
    return enemy_index >= 0 && enemy_index < enemies.size() &&
           enemies[enemy_index].hp > 0;
  }

  bool HasLivingEnemies() {
    for (int i = 0; i < enemies.size(); i++) {
      if (IsEnemyAlive(i)) {
        return true;
      }
    }

    return false;
  }

  void SelectNextLivingEnemy() {
    if (!HasLivingEnemies()) {
      return;
    }

    if (IsEnemyAlive(selected_enemy)) {
      return;
    }

    for (int i = 0; i < enemies.size(); i++) {
      if (IsEnemyAlive(i)) {
        selected_enemy = i;
        return;
      }
    }
  }

  void SelectPreviousAction() {
    selected_action--;
    if (selected_action < 0) {
      selected_action = actions.size() - 1;
    }
  }

  void SelectNextAction() {
    selected_action++;
    if (selected_action >= actions.size()) {
      selected_action = 0;
    }
  }

  void SelectPreviousEnemy() {
    if (enemies.size() == 0) {
      return;
    }

    for (int i = 0; i < enemies.size(); i++) {
      selected_enemy--;
      if (selected_enemy < 0) {
        selected_enemy = enemies.size() - 1;
      }

      if (IsEnemyAlive(selected_enemy)) {
        return;
      }
    }
  }

  void SelectNextEnemy() {
    if (enemies.size() == 0) {
      return;
    }

    for (int i = 0; i < enemies.size(); i++) {
      selected_enemy++;
      if (selected_enemy >= enemies.size()) {
        selected_enemy = 0;
      }

      if (IsEnemyAlive(selected_enemy)) {
        return;
      }
    }
  }

  void PlayerAttack() {
    if (!HasLivingEnemies()) {
      message = "Vsichni nepratele uz byli porazeni.";
      return;
    }

    SelectNextLivingEnemy();

    Monster &target = enemies[selected_enemy];
    target.hp -= p.attack_dmg;
    if (target.hp < 0) {
      target.hp = 0;
    }

    message = p.name + " utoci na " + target.name + " za " +
              to_string(p.attack_dmg) + " poskozeni.";

    if (target.hp == 0) {
      message = message + " " + target.name + " byl porazen.";
      SelectNextLivingEnemy();
    }
  }

  void ExecuteSelectedAction() {
    if (selected_action == 0) {
      PlayerAttack();
      return;
    }

    message = "Tato schopnost zatim neni implementovana.";
  }

public:
  TurnBasedCombatController(ScreenInteractive &screen, player &p)
      : screen(screen), p(p) {
    enemies.push_back(Monster("Sliz", 12, 2, 5, 3, false));
    enemies.push_back(Monster("Goblin", 16, 3, 8, 5, false));
  }

  Element Render() {
    return vbox({hbox({RenderMap(), separator(), RenderSidePanel()}),
                 text(message) | border});
  }

  bool OnEvent(Event event) {
    if (event == Event::Escape) {
      result = AppState::Menu;
      screen.Exit();
      return true;
    }

    if (event == Event::ArrowUp) {
      SelectPreviousAction();
      return true;
    }

    if (event == Event::ArrowDown) {
      SelectNextAction();
      return true;
    }

    if (event == Event::ArrowLeft) {
      SelectPreviousEnemy();
      return true;
    }

    if (event == Event::ArrowRight) {
      SelectNextEnemy();
      return true;
    }

    if (event == Event::Return) {
      ExecuteSelectedAction();
      return true;
    }

    return false;
  }
};

struct MainMenuController {
private:
  enum class MenuState { MainMenu, Characters, CharacterDetail };

  ScreenInteractive &screen;
  MenuState state = MenuState::MainMenu;

  vector<string> main_entries = {"Start hry", "Postavy", "Konec"};
  int main_selected = 0;
  Component main_menu;

  vector<string> characters = {"Paladin", "Lovec", "Mag", "Warlock"};
  int char_selected = 0;
  Component char_menu;

  vector<string> descriptions = {
      "Paladin\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- Svaty uder\n- "
      "Leceni",
      "Lovec\nHP: 100\nEnergy: 100\nDMG: 4\n\nSchopnosti:\n- Ohnivy sip\n- "
      "Rychla strela",
      "Mag\nHP: 100\nEnergy: 100\nDMG: 2\n\nSchopnosti:\n- Fireball\n- "
      "Teleport",
      "Warlock\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- (zatim "
      "nedefinovano)\n- (zatim nedefinovano)"};

  Component container;

  bool HandleMainMenu(Event event) {
    if (event != Event::Return) {
      return false;
    }

    if (main_selected == 0) {
      result = AppState::Combat;
      selected_class = char_selected + 1;
      screen.Exit();
      return true;
    }

    if (main_selected == 1) {
      state = MenuState::Characters;
      return true;
    }

    if (main_selected == 2) {
      result = AppState::Exit;
      screen.Exit();
      return true;
    }

    return false;
  }

  bool HandleCharacters(Event event) {
    if (event == Event::Return) {
      state = MenuState::CharacterDetail;
      return true;
    }

    if (event == Event::Escape) {
      state = MenuState::MainMenu;
      return true;
    }

    return false;
  }

  bool HandleCharacterDetail(Event event) {
    if (event == Event::Return) {
      selected_class = char_selected + 1;
      state = MenuState::MainMenu;
      return true;
    }

    if (event == Event::Escape) {
      state = MenuState::Characters;
      return true;
    }

    return false;
  }

public:
  MainMenuController(ScreenInteractive &screen) : screen(screen) {
    main_menu = Menu(&main_entries, &main_selected);
    char_menu = Menu(&characters, &char_selected);
    container = Container::Vertical({main_menu, char_menu});
  }

  Component GetContainer() { return container; }

  Element Render() {
    if (state == MenuState::MainMenu) {
      return vbox({text("=== HLAVNÍ MENU ===") | bold, main_menu->Render(),
                   text("ENTER = vybrat")}) |
             border;
    }

    if (state == MenuState::Characters) {
      return vbox({text("=== POSTAVY ===") | bold, char_menu->Render(),
                   text("ENTER = vybrat | ESC = zpět")}) |
             border;
    }

    if (state == MenuState::CharacterDetail) {
      return vbox({text("=== DETAIL POSTAVY ===") | bold,
                   text(descriptions[char_selected]),
                   text("ESC = zpět | ENTER = vybrat tuto postavu")}) |
             border;
    }

    return text("Chyba v menu");
  }

  bool OnEvent(Event event) {
    if (state == MenuState::MainMenu) {
      return HandleMainMenu(event);
    }

    if (state == MenuState::Characters) {
      return HandleCharacters(event);
    }

    if (state == MenuState::CharacterDetail) {
      return HandleCharacterDetail(event);
    }

    return false;
  }
};

void MainMenu(ScreenInteractive &screen) {
  MainMenuController menu(screen);

  Component renderer =
      Renderer(menu.GetContainer(), [&] { return menu.Render(); });

  Component component =
      CatchEvent(renderer, [&](Event event) { return menu.OnEvent(event); });

  screen.Loop(component);
}

void ActionCombat(ScreenInteractive &screen, player &p) {
  CombatTimerConfig timer_config;
  timer_config.enemy_tick_ms = 200;
  timer_config.enemy_shot_chance = 5;

  ActionCombatController combat(screen, p, timer_config);

  Component empty = Container::Vertical({});
  Component renderer = Renderer(empty, [&] { return combat.Render(); });

  Component component =
      CatchEvent(renderer, [&](Event event) { return combat.OnEvent(event); });

  combat.StartTimer();
  screen.Loop(component);
  combat.StopTimer();
}

void TurnBasedCombat(ScreenInteractive &screen, player &p) {
  TurnBasedCombatController combat(screen, p);

  Component empty = Container::Vertical({});
  Component renderer = Renderer(empty, [&] { return combat.Render(); });

  Component component =
      CatchEvent(renderer, [&](Event event) { return combat.OnEvent(event); });

  screen.Loop(component);
}

void Combat(ScreenInteractive &screen, player &p) {
  if (selected_combat_mode == CombatMode::TurnBased) {
    TurnBasedCombat(screen, p);
    return;
  }

  ActionCombat(screen, p);
}

void Village(ScreenInteractive &screen, player &p) {
  Component empty = Container::Vertical({});
  Component renderer = Renderer(empty, [&] { return text("Village"); });
  
  Component component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Escape) {
      result = AppState::Menu;
      screen.Exit();
      return true;
    }
    return false;
  });
  screen.Loop(component);
}

int main() {
  ScreenInteractive screen = ScreenInteractive::TerminalOutput();
  srand(time(nullptr));

  while (result != AppState::Exit) {
    switch (result) {
    case AppState::Menu:
      system("clear");
      MainMenu(screen);
      break;
    case AppState::Combat: {
      system("clear");
      player Player(selected_class);
      Combat(screen, Player);
      break;
    }
    case AppState::Village: {
      system("clear");
      player Player(selected_class);
      Village(screen, Player);
      break;
    }
    default:
      break;
    }
  }

  return 0;
}
