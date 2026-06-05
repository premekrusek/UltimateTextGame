#include <iostream>
// #include <string>
// #include <vector>
#include <cstdlib>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

using namespace std;
using namespace ftxui;

int m = 100; // 16
/*
Min. jedenkrát musí hráč bojovat proti: 1 monstru v jednom setkání.
Min. jedenkrát musí hráč bojovat proti: 2 monstrům v jednom setkání.
Min. jedenkrát musí hráč bojovat proti: 3 monstrům v jednom setkání.
Monstra mají 50% šanci mít u sebe peníze/zlato.
*/
int mB = 100; // 2
/*
Mini-boss má peníze/zlato vždy.
*/
int hB = 100; // 1
int village = 2;
int level = 1;

// |V| > |M| > |M|> |2xM| > |MB| > |V| > |M| > |2xM| > |2xM| > |MB| > |V| >
// |2xM| > |2xM| > |3xM| > |V| > |HB|

enum class AppState { Menu, Village, Combat, Exit };
AppState result = AppState::Menu;

struct player {
  // Hráč si na začátku hry zvolí classu. Hráč musí potvrdit, že chce dannou
  // classu.
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

  // Pozice hráče na obrazovce / mapě
  int x = 5;
  int y = 5;

  // Vizuální podoba hráče
  vector<string> sprite = {
      " O ",
      "/|\\", // Dvě zpětné lomítka, aby to rozpoznalo, že to je zpětné lomítko
      "/ \\"};

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

  // Pohyb hráče
  void move(int dx, int dy) {
    x += dx;
    y += dy;

    if (x < 0)
      x = 0;

    if (y < 0)
      y = 0;

    if (x > 37)
      x = 37;

    if (y > 17)
      y = 17;
  }

  // Vykreslení hráče jako FTXUI Element
  Element render() {
    
    vector<Element> rows;
    
    // vytvoření mapy
    vector<string> map =
    {
      "+--------------------------------------+",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "|                                      |",
      "+--------------------------------------+",
    };

    const int HEIGHT = map.size();
    const int WIDTH = map[0].size();

    // vykreslení hráče
    map[y][x + 1] = 'O';

    map[y + 1][x] = '/';
    map[y + 1][x + 1] = '|';
    map[y + 1][x + 2] = '\\';

    map[y + 2][x] = '/';
    map[y + 2][x + 2] = '\\';

    // převod mapy na FTXUI elementy
    for (int i = 0; i < HEIGHT; i++) {
      rows.push_back(text(map[i]));
    }

    return vbox(std::move(rows)) | color(Color::Green);
  }

  void attack() {}

  void ability1() {
    switch (figure) {
    case 1:
      cout << "Paladin pouzil svaty uder!\n";
      break;
    case 2:
      cout << "Lovec vystrelil ohnivy sip!\n";
      break;
    case 3:
      cout << "Mag seslal fireball!\n";
      break;
    case 4:
      cout << "Warlock seslal temnou kletbu!\n";
      break;
    }
  }

  void ability2() {
    switch (figure) {
    case 1:
      cout << "Paladin se vylecil!\n";
      break;
    case 2:
      cout << "Lovec vystrelil rychlou strelu!\n";
      break;
    case 3:
      cout << "Mag se teleportoval!\n";
      break;
    case 4:
      cout << "Warlock vysal zivot!\n";
      break;
    }
  }
};

void MainMenu(ScreenInteractive &screen) { // funkce pro celé menu
  enum class MenuState { MainMenu, Characters, CharacterDetail, GameStart };

  MenuState state = MenuState::MainMenu; // nastaví aktuální stav aplikace
                                         // (začínáme v hlavním menu)

  // Hlavni menu
  std::vector<std::string> main_entries = {
      // seznam položek hlavního menu
      "Start hry", // položka 0
      "Postavy",   // položka 1
      "Konec"      // položka 2
  };

  int main_selected = 0; // index aktuálně vybrané položky (defaultně první)
  Component main_menu = Menu(
      &main_entries,
      &main_selected); // vytvoří interaktivní menu (napojené na data a výběr)

  // Postavy
  std::vector<std::string> characters = {// seznam postav
                                         "Paladin", "Lovec", "Mag", "Warlock"};

  int char_selected = 0; // index vybrané postavy
  Component char_menu =
      Menu(&characters, &char_selected); // menu pro výběr postavy

  // Popisy postav
  std::vector<std::string> descriptions = {
      // textové popisy jednotlivých postav
      "Paladin\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- Svaty uder\n- "
      "Leceni",

      "Lovec\nHP: 100\nEnergy: 100\nDMG: 4\n\nSchopnosti:\n- Ohnivy sip\n- "
      "Rychla strela",

      "Mag\nHP: 100\nEnergy: 100\nDMG: 2\n\nSchopnosti:\n- Fireball\n- "
      "Teleport",

      "Warlock\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- (zatim "
      "nedefinovano)\n- (zatim nedefinovano)"};

  // Rendrovani
  Component container = Container::Vertical({
      // kontejner, který drží komponenty (menu)
      main_menu, // hlavní menu
      char_menu  // menu postav
  });

  Component renderer =
      Renderer(container, [&] { // renderer říká jak vykreslit UI podle stavu
        if (state == MenuState::MainMenu) { // pokud jsme v hlavním menu
          return vbox({
                     // vykreslí vertikální sloupec
                     text("=== HLAVNÍ MENU ===") | bold, // nadpis (tučný)
                     main_menu->Render(),   // vykreslí samotné menu
                     text("ENTER = vybrat") // nápověda
                 }) |
                 border;                             // přidá rámeček
        } else if (state == MenuState::Characters) { // pokud jsme v menu postav
          return vbox({text("=== POSTAVY ===") | bold,
                       char_menu->Render(), // vykreslí menu postav
                       text("ENTER = vybrat | ESC = zpět")}) |
                 border;
        } else if (state ==
                   MenuState::CharacterDetail) { // pokud jsme v detailu postavy
          return vbox({text("=== DETAIL POSTAVY ===") | bold,
                       text(descriptions[char_selected]), // zobrazí popis
                                                          // vybrané postavy
                       text("ESC = zpět")}) |
                 border;
        } else if (state == MenuState::GameStart) { // pokud jsme ve hře
          result = AppState::Combat;
          return vbox({text("UltimateTextGame") | bold, text("")});
        }

        return text("Chyba v menu"); // fallback (když by byl neplatný stav)
      });

  // Eventy
  Component component =
      CatchEvent(renderer, [&](Event event) { // zachytává klávesy / vstupy
        // Hlavni menu
        if (state == MenuState::MainMenu) { // pokud jsme v hlavním menu
          if (event == Event::Return) {     // pokud uživatel stiskne ENTER
            if (main_selected == 0) {       // pokud je vybráno "Start hry"
              state = MenuState::GameStart;
              result = AppState::Combat; // Village
              screen.Exit();
              return true; // event byl zpracován
            }
            if (main_selected == 1) {        // pokud je vybráno "Postavy"
              state = MenuState::Characters; // přepne do menu postav
              return true;                   // event byl zpracován
            }
            if (main_selected == 2) { // pokud je vybráno "Konec"
              result = AppState::Exit;
              screen.Exit(); // ukončí aplikaci
              return true;
            }
          }
        }

        // Postavy
        if (state == MenuState::Characters) {   // pokud jsme v menu postav
          if (event == Event::Return) {         // ENTER
            state = MenuState::CharacterDetail; // otevře detail postavy
            return true;
          }
          if (event == Event::Escape) {  // ESC
            state = MenuState::MainMenu; // návrat do hlavního menu
            return true;
          }
        }

        // Detail postavy
        if (state == MenuState::CharacterDetail) { // pokud jsme v detailu
          if (event == Event::Escape) {            // ESC
            state = MenuState::Characters;         // návrat do seznamu postav
            return true;
          }
        }

        return false; // event nebyl zpracován (pošle se dál)
      });

  screen.Loop(component); // spustí hlavní smyčku
                          /*
                          while (běží program) {
                              načti vstup (klávesy)
                              pošli ho do component (OnEvent)
                              překresli obrazovku (Render)
                          }*/
}

void Combat(ScreenInteractive &screen, player &p) {
  Component empty = Container::Vertical({});
  Component renderer = Renderer(empty, [&] { return p.render(); });

  Component component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::ArrowLeft) {
      p.move(-2, 0);
      return true;
    }

    if (event == Event::ArrowRight) {
      p.move(2, 0);
      return true;
    }

    if (event == Event::Escape) {
      result = AppState::Menu;
      screen.Exit();
      return true;
    }

    return false;
  });

  screen.Loop(component);
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
  player Player(1);

  while (result != AppState::Exit) {
    switch (result) {
    case AppState::Menu:
      system("clear");
      MainMenu(screen);
      break;

    case AppState::Combat:
      system("clear");
      Combat(screen, Player);
      break;

    case AppState::Village:
      system("clear");
      Village(screen, Player);
      break;

    default:
      break;
    }
  }

  return 0;
}