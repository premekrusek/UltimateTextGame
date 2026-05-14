#include <iostream>
//#include <string>
//#include <vector>
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

// |V| > |M| > |M|> |2xM| > |MB| > |V| > |M| > |2xM| > |2xM| > |MB| > |V| > |2xM| > |2xM| > |3xM| > |V| > |HB|


struct player{
    // Hráč si na začátku hry zvolí classu. Hráč musí potvrdit, že chce dannou classu. Pokud nechce může si vybrat jinou a to bez omezení počtu vybírání.
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
    player(int f){
        figure = f;
        switch(f){
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
                break;
        }
    }


    void attack(){

    }

    void ability1(){
        switch (figure){
            case 1:
                cout << "Paladin pouzil svaty uder!\n";
                break;
            case 2:
                cout << "Lovec vystrelil ohnivy sip!\n";
                break;
            case 3:
                cout << "Mag seslal fireball!\n";
                break;
        }
    }

    void ability2(){
        switch (figure){
            case 1:
                cout << "Paladin se vylecil!\n";
                break;
            case 2:
                cout << "Lovec vystrelil rychlou strelu!\n";
                break;
            case 3:
                cout << "Mag se teleportoval!\n";
                break;
        }
    }
};


enum class ScreenState {
    MainMenu,
    Characters,
    CharacterDetail,
    GameStart
};

void MainMenu(ScreenInteractive& screen){ // funkce pro celé menu
    ScreenState state = ScreenState::MainMenu; // nastaví aktuální stav aplikace (začínáme v hlavním menu)
    
    // Hlavni menu
    std::vector<std::string> main_entries = { // seznam položek hlavního menu
        "Start hry", // položka 0
        "Postavy",   // položka 1
        "Konec"      // položka 2
    };
    
    int main_selected = 0; // index aktuálně vybrané položky (defaultně první)
    Component main_menu = Menu(&main_entries, &main_selected); // vytvoří interaktivní menu (napojené na data a výběr)
    
    // Postavy
    std::vector<std::string> characters = { // seznam postav
        "Paladin",
        "Lovec",
        "Mag",
        "Warlock"
    };
    
    int char_selected = 0; // index vybrané postavy
    Component char_menu = Menu(&characters, &char_selected); // menu pro výběr postavy
    
    // Popisy postav
    std::vector<std::string> descriptions = { // textové popisy jednotlivých postav
        "Paladin\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- Svaty uder\n- Leceni",
    
        "Lovec\nHP: 100\nEnergy: 100\nDMG: 4\n\nSchopnosti:\n- Ohnivy sip\n- Rychla strela",
    
        "Mag\nHP: 100\nEnergy: 100\nDMG: 2\n\nSchopnosti:\n- Fireball\n- Teleport",
        
        "Warlock\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- (zatim nedefinovano)\n- (zatim nedefinovano)"
    };
    
    // Rendrovani
    Component container = Container::Vertical({ // kontejner, který drží komponenty (menu)
        main_menu, // hlavní menu
        char_menu  // menu postav
    });
    
    Component renderer = Renderer(container, [&] { // renderer říká jak vykreslit UI podle stavu
        if (state == ScreenState::MainMenu) { // pokud jsme v hlavním menu
            return vbox({ // vykreslí vertikální sloupec
                text("=== HLAVNÍ MENU ===") | bold, // nadpis (tučný)
                main_menu->Render(), // vykreslí samotné menu
                text("ENTER = vybrat") // nápověda
            }) | border; // přidá rámeček
        }
    
        if (state == ScreenState::Characters) { // pokud jsme v menu postav
            return vbox({
                text("=== POSTAVY ===") | bold,
                char_menu->Render(), // vykreslí menu postav
                text("ENTER = vybrat | ESC = zpět")
            }) | border;
        }
    
        if (state == ScreenState::CharacterDetail) { // pokud jsme v detailu postavy
            return vbox({
                text("=== DETAIL POSTAVY ===") | bold,
                text(descriptions[char_selected]), // zobrazí popis vybrané postavy
                text("ESC = zpět")
            }) | border;
        }

        if (state == ScreenState::GameStart) { // pokud jsme ve hře
            return vbox({
                text("=== START HRY ===") | bold,
                text("Tady zacne hra."),
                text("ESC = zpět")
            }) | border;
        }
    
        return text("Chyba"); // fallback (když by byl neplatný stav)
    });
    
    // Eventy
    Component component = CatchEvent(renderer, [&](Event event) { // zachytává klávesy / vstupy
        // Hlavni menu
        if (state == ScreenState::MainMenu) { // pokud jsme v hlavním menu
            if (event == Event::Return) { // pokud uživatel stiskne ENTER
                if (main_selected == 0) { // pokud je vybráno "Start hry"
                    state = ScreenState::GameStart;
                    return true; // event byl zpracován
                }
                if (main_selected == 1) { // pokud je vybráno "Postavy"
                    state = ScreenState::Characters; // přepne do menu postav
                    return true; // event byl zpracován
                }
                if (main_selected == 2) { // pokud je vybráno "Konec"
                    screen.Exit(); // ukončí aplikaci
                    return true;
                }
            }
        }
    
        // Postavy
        if (state == ScreenState::Characters) { // pokud jsme v menu postav
            if (event == Event::Return) { // ENTER
                state = ScreenState::CharacterDetail; // otevře detail postavy
                return true;
            }
            if (event == Event::Escape) { // ESC
                state = ScreenState::MainMenu; // návrat do hlavního menu
                return true;
            }
        }
    
        // Detail postavy
        if (state == ScreenState::CharacterDetail) { // pokud jsme v detailu
            if (event == Event::Escape) { // ESC
                state = ScreenState::Characters; // návrat do seznamu postav
                return true;
            }
        }

        // Hra
        if (state == ScreenState::GameStart) {
            if (event == Event::Escape) {
                state = ScreenState::MainMenu;
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

int main() { // vstupní bod programu
    ScreenInteractive screen = ScreenInteractive::TerminalOutput(); // vytvoří interaktivní terminálové okno

    MainMenu(screen); // zavolá menu

    return 0; // konec programu
}