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

bool story_mode_active = false;
int current_encounter_index = 0;

enum class EncounterType { Village, Monsters, MiniBoss, FinalBoss };

struct Encounter {
private:
  EncounterType type;
  int monster_count;

public:
  Encounter(EncounterType type, int monster_count) {
    this->type = type;
    this->monster_count = monster_count;
  }

  EncounterType getType() { return type; }

  int getMonsterCount() { return monster_count; }
};

vector<Encounter> CreateGamePath() {
  vector<Encounter> path;

  path.push_back(Encounter(EncounterType::Village, 0));
  path.push_back(Encounter(EncounterType::Monsters, 1));
  path.push_back(Encounter(EncounterType::Monsters, 1));
  path.push_back(Encounter(EncounterType::Monsters, 2));
  path.push_back(Encounter(EncounterType::MiniBoss, 1));
  path.push_back(Encounter(EncounterType::Village, 0));
  path.push_back(Encounter(EncounterType::Monsters, 1));
  path.push_back(Encounter(EncounterType::Monsters, 2));
  path.push_back(Encounter(EncounterType::Monsters, 2));
  path.push_back(Encounter(EncounterType::MiniBoss, 1));
  path.push_back(Encounter(EncounterType::Village, 0));
  path.push_back(Encounter(EncounterType::Monsters, 2));
  path.push_back(Encounter(EncounterType::Monsters, 2));
  path.push_back(Encounter(EncounterType::Monsters, 3));
  path.push_back(Encounter(EncounterType::Village, 0));
  path.push_back(Encounter(EncounterType::FinalBoss, 1));

  return path;
}

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

Monster CreateMonster(int monster_number) {
  if (monster_number == 1) {
    return Monster("Sliz", 12, 2, 5, 3, false);
  }

  if (monster_number == 2) {
    return Monster("Goblin", 16, 3, 8, 5, false);
  }

  return Monster("Kostlivec", 20, 4, 10, 6, false);
}

Monster CreateMiniBoss() {
  return Monster("Mini boss", 35, 6, 20, 15, true);
}

Monster CreateFinalBoss() {
  return Monster("Hlavni boss", 60, 9, 50, 30, true);
}

vector<Monster> CreateEnemiesForEncounter(Encounter encounter) {
  vector<Monster> enemies;

  if (encounter.getType() == EncounterType::Monsters) {
    for (int i = 0; i < encounter.getMonsterCount(); i++) {
      enemies.push_back(CreateMonster(i + 1));
    }
    return enemies;
  }

  if (encounter.getType() == EncounterType::MiniBoss) {
    enemies.push_back(CreateMiniBoss());
    return enemies;
  }

  if (encounter.getType() == EncounterType::FinalBoss) {
    enemies.push_back(CreateFinalBoss());
    return enemies;
  }

  return enemies;
}

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

  void takeDamage(int damage) {
    hp -= damage;
    if (hp < 0) {
      hp = 0;
    }
  }

  bool isAlive() { return hp > 0; }

  int xpForNextLevel() { return (level + 1) * 20; }

  string addXp(int amount) {
    xp += amount;

    if (xp < xpForNextLevel()) {
      return "";
    }

    int oldMaxHp = maxHp;
    int oldAttack = attack_dmg;
    int oldMaxEnergy = maxEnergy;

    xp -= xpForNextLevel();
    level++;
    maxHp += 5;
    hp = maxHp;
    maxEnergy += 2;
    energy = maxEnergy;
    attack_dmg++;

    return " Novy level! Max HP: " + to_string(oldMaxHp) + " => " +
           to_string(maxHp) + ", utok: " + to_string(oldAttack) + " => " +
           to_string(attack_dmg) + ", energie: " + to_string(oldMaxEnergy) +
           " => " + to_string(maxEnergy) + ".";
  }

  void addGold(int amount) {
    gold += amount;
    if (gold < 0) {
      gold = 0;
    }
  }

  bool spendGold(int amount) {
    if (gold < amount) {
      return false;
    }

    gold -= amount;
    return true;
  }

  void restoreHealthAndEnergy() {
    hp = maxHp;
    energy = maxEnergy;
  }

  void upgradeMaxHp(int amount) {
    maxHp += amount;
    hp = maxHp;
  }

  void upgradeMaxEnergy(int amount) {
    maxEnergy += amount;
    energy = maxEnergy;
  }

  void upgradeAttack(int amount) { attack_dmg += amount; }
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
  bool battle_finished = false;
  bool rewards_given = false;
  bool final_boss_encounter = false;
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

  bool PlayerAttack() {
    if (!HasLivingEnemies()) {
      message = "Vsichni nepratele uz byli porazeni.";
      battle_finished = true;
      return false;
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

    return true;
  }

  bool ExecuteSelectedAction() {
    if (battle_finished) {
      return false;
    }

    if (selected_action == 0) {
      return PlayerAttack();
    }

    message = "Tato schopnost zatim neni implementovana.";
    return false;
  }

  void GiveRewards() {
    if (rewards_given) {
      return;
    }

    rewards_given = true;
    int total_xp = 0;
    int total_gold = 0;

    for (int i = 0; i < enemies.size(); i++) {
      total_xp += enemies[i].xpReward;

      if (enemies[i].alwaysDropsGold || rand() % 2 == 0) {
        total_gold += enemies[i].goldReward;
      }
    }

    p.addGold(total_gold);
    string level_message = p.addXp(total_xp);

    message = message + " Ziskavas " + to_string(total_xp) + " XP a " +
              to_string(total_gold) + " zlata." + level_message;
  }

  void EnemyTurn() {
    if (!HasLivingEnemies()) {
      if (final_boss_encounter) {
        message = message + " Vyhral jsi celou hru. ESC = zpet do menu.";
      } else {
        message = message + " Vyhral jsi souboj.";
      }
      GiveRewards();
      battle_finished = true;
      return;
    }

    int total_damage = 0;
    for (int i = 0; i < enemies.size(); i++) {
      if (IsEnemyAlive(i)) {
        total_damage += enemies[i].attack;
      }
    }

    p.takeDamage(total_damage);
    message = message + " Nepratele utoci za " + to_string(total_damage) +
              " poskozeni.";

    if (!p.isAlive()) {
      message = message + " Prohral jsi. ESC = zpet do menu.";
      battle_finished = true;
    }
  }

public:
  TurnBasedCombatController(ScreenInteractive &screen, player &p,
                            vector<Monster> enemies, bool final_boss_encounter)
      : screen(screen), p(p), enemies(enemies),
        final_boss_encounter(final_boss_encounter) {}

  Element Render() {
    return vbox({hbox({RenderMap(), separator(), RenderSidePanel()}),
                 text(message) | border});
  }

  bool OnEvent(Event event) {
    if (event == Event::Escape) {
      if (battle_finished && (final_boss_encounter || !p.isAlive())) {
        story_mode_active = false;
      }
      result = AppState::Menu;
      screen.Exit();
      return true;
    }

    if (battle_finished) {
      if (final_boss_encounter || !p.isAlive()) {
        return false;
      }

      if (event == Event::Return) {
        if (p.isAlive()) {
          if (!story_mode_active) {
            result = AppState::Menu;
          }
          screen.Exit();
          return true;
        }

        result = AppState::Exit;
        screen.Exit();
        return true;
      }

      return false;
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
      bool action_was_done = ExecuteSelectedAction();
      if (action_was_done) {
        EnemyTurn();
      }
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

  vector<string> main_entries = {"Pokracovat pribehem", "Akcni souboj",
                                 "Tahovy souboj", "Vesnice", "Postavy",
                                 "Konec"};
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
      story_mode_active = true;
      current_encounter_index = 0;
      selected_combat_mode = CombatMode::Action;
      result = AppState::Village;
      screen.Exit();
      return true;
    }

    if (main_selected == 1) {
      story_mode_active = false;
      selected_combat_mode = CombatMode::Action;
      result = AppState::Combat;
      screen.Exit();
      return true;
    }

    if (main_selected == 2) {
      story_mode_active = false;
      selected_combat_mode = CombatMode::TurnBased;
      result = AppState::Combat;
      screen.Exit();
      return true;
    }

    if (main_selected == 3) {
      story_mode_active = false;
      result = AppState::Village;
      screen.Exit();
      return true;
    }

    if (main_selected == 4) {
      state = MenuState::Characters;
      return true;
    }

    if (main_selected == 5) {
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

struct VillageController {
private:
  ScreenInteractive &screen;
  player &p;

  vector<string> entries = {"Pokracovat dal",
                            "Doplnit zivoty a energii - 5 zlata",
                            "Vylepsit max zivoty - 10 zlata",
                            "Vylepsit max energii - 10 zlata",
                            "Vylepsit utok - 15 zlata"};
  int selected = 0;
  Component menu;
  string message = "Vesnice: vyber akci a potvrd ENTER.";
  bool continue_selected = false;

  Element RenderPlayerStats() {
    return vbox({text("Hrac: " + p.name) | bold,
                 text("HP: " + to_string(p.hp) + "/" + to_string(p.maxHp)),
                 text("Energie: " + to_string(p.energy) + "/" +
                      to_string(p.maxEnergy)),
                 text("Utok: " + to_string(p.attack_dmg)),
                 text("Level: " + to_string(p.level)),
                 text("XP: " + to_string(p.xp) + "/" +
                      to_string(p.xpForNextLevel())),
                 text("Zlato: " + to_string(p.gold))});
  }

  bool TrySpendGold(int price) {
    if (p.spendGold(price)) {
      return true;
    }

    message = "Nemas dost zlata.";
    return false;
  }

  void RestorePlayer() {
    if (!TrySpendGold(5)) {
      return;
    }

    p.restoreHealthAndEnergy();
    message = "Doplnil sis zivoty a energii.";
  }

  void UpgradeMaxHp() {
    if (!TrySpendGold(10)) {
      return;
    }

    p.upgradeMaxHp(5);
    message = "Max zivoty zvyseny o 5.";
  }

  void UpgradeMaxEnergy() {
    if (!TrySpendGold(10)) {
      return;
    }

    p.upgradeMaxEnergy(2);
    message = "Max energie zvysena o 2.";
  }

  void UpgradeAttack() {
    if (!TrySpendGold(15)) {
      return;
    }

    p.upgradeAttack(1);
    message = "Utok zvysen o 1.";
  }

  void ContinueGame() {
    continue_selected = true;
    if (!story_mode_active) {
      result = AppState::Menu;
    }
    screen.Exit();
  }

  void ExecuteSelectedAction() {
    if (selected == 0) {
      ContinueGame();
      return;
    }

    if (selected == 1) {
      RestorePlayer();
      return;
    }

    if (selected == 2) {
      UpgradeMaxHp();
      return;
    }

    if (selected == 3) {
      UpgradeMaxEnergy();
      return;
    }

    if (selected == 4) {
      UpgradeAttack();
    }
  }

public:
  VillageController(ScreenInteractive &screen, player &p)
      : screen(screen), p(p) {
    menu = Menu(&entries, &selected);
  }

  Component GetContainer() { return menu; }

  bool ShouldContinueStory() { return continue_selected; }

  Element Render() {
    return vbox({text("=== VESNICE ===") | bold,
                 hbox({RenderPlayerStats() | border, menu->Render() | border}),
                 text(message) | border,
                 text("ESC = zpet do menu")}) |
           border;
  }

  bool OnEvent(Event event) {
    if (event == Event::Escape) {
      result = AppState::Menu;
      screen.Exit();
      return true;
    }

    if (event == Event::Return) {
      ExecuteSelectedAction();
      return true;
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

void AdvanceStoryEncounter(vector<Encounter> &game_path) { // posune hráče na další část příběhové cesty.
  if (!story_mode_active) {
    return;
  }

  current_encounter_index++;
  if (current_encounter_index >= game_path.size()) {
    story_mode_active = false;
    result = AppState::Menu;
    return;
  }

  Encounter next_encounter = game_path[current_encounter_index];
  if (next_encounter.getType() == EncounterType::Village) {
    result = AppState::Village;
    return;
  }

  selected_combat_mode = CombatMode::TurnBased;
  result = AppState::Combat;
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

vector<Monster> CreateCurrentTurnBasedEnemies(vector<Encounter> &game_path) {
  if (story_mode_active && current_encounter_index < game_path.size()) {
    return CreateEnemiesForEncounter(game_path[current_encounter_index]);
  }

  Encounter test_encounter(EncounterType::Monsters, 2);
  return CreateEnemiesForEncounter(test_encounter);
}

bool IsCurrentFinalBossEncounter(vector<Encounter> &game_path) {
  return story_mode_active && current_encounter_index < game_path.size() &&
         game_path[current_encounter_index].getType() == EncounterType::FinalBoss;
}

void TurnBasedCombat(ScreenInteractive &screen, player &p,
                     vector<Encounter> &game_path) {
  vector<Monster> enemies = CreateCurrentTurnBasedEnemies(game_path);
  bool final_boss_encounter = IsCurrentFinalBossEncounter(game_path);
  TurnBasedCombatController combat(screen, p, enemies, final_boss_encounter);

  Component empty = Container::Vertical({});
  Component renderer = Renderer(empty, [&] { return combat.Render(); });

  Component component =
      CatchEvent(renderer, [&](Event event) { return combat.OnEvent(event); });

  screen.Loop(component); // Dokud běží screen.Loop(component), hráč je uvnitř obrazovky souboje

  if (story_mode_active && p.isAlive() && result == AppState::Combat) {
    AdvanceStoryEncounter(game_path);
  }
}

void Combat(ScreenInteractive &screen, player &p, vector<Encounter> &game_path) {
  if (selected_combat_mode == CombatMode::TurnBased) {
    TurnBasedCombat(screen, p, game_path);
    return;
  }

  ActionCombat(screen, p);
}

void Village(ScreenInteractive &screen, player &p, vector<Encounter> &game_path) {
  VillageController village(screen, p);

  Component renderer =
      Renderer(village.GetContainer(), [&] { return village.Render(); });

  Component component = CatchEvent(
      renderer, [&](Event event) { return village.OnEvent(event); });

  screen.Loop(component);

  if (story_mode_active && village.ShouldContinueStory()) {
    AdvanceStoryEncounter(game_path);
  }
}

int main() {
  ScreenInteractive screen = ScreenInteractive::TerminalOutput();
  srand(time(nullptr));

  vector<Encounter> game_path = CreateGamePath();
  player Player(selected_class);
  int active_player_class = selected_class;

  while (result != AppState::Exit) {
    if (active_player_class != selected_class) {
      Player = player(selected_class); // Vytvoř nový objekt typu player podle vybrané classy.
      active_player_class = selected_class;
    }

    switch (result) {
    case AppState::Menu:
      system("clear");
      MainMenu(screen);
      break;
    case AppState::Combat: {
      system("clear");
      Combat(screen, Player, game_path);
      break;
    }
    case AppState::Village: {
      system("clear");
      Village(screen, Player, game_path);
      break;
    }
    default:
      break;
    }
  }

  return 0;
}
