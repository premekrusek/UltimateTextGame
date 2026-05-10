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
    CharacterDetail
};

int main() {
    ScreenInteractive screen = ScreenInteractive::TerminalOutput();

    ScreenState state = ScreenState::MainMenu;

    // Hlavni menu
    std::vector<std::string> main_entries = {
        "Start hry",
        "Postavy",
        "Konec"
    };

    int main_selected = 0;
    Component main_menu = Menu(&main_entries, &main_selected);

    // Postavy
    std::vector<std::string> characters = {
        "Paladin",
        "Lovec",
        "Mag",
        "Warlock"
    };
    
    int char_selected = 0;
    Component char_menu = Menu(&characters, &char_selected);

    // Popisy postav
    std::vector<std::string> descriptions = {
        "Paladin\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- Svaty uder\n- Leceni",

        "Lovec\nHP: 100\nEnergy: 100\nDMG: 4\n\nSchopnosti:\n- Ohnivy sip\n- Rychla strela",

        "Mag\nHP: 100\nEnergy: 100\nDMG: 2\n\nSchopnosti:\n- Fireball\n- Teleport",
        
        "Warlock\nHP: 100\nEnergy: 100\nDMG: 3\n\nSchopnosti:\n- (zatim nedefinovano)\n- (zatim nedefinovano)"
    };

    // Rendrovani
    Component container = Container::Vertical({
        main_menu,
        char_menu
    });

    Component renderer = Renderer(container, [&] {
        if (state == ScreenState::MainMenu) {
            return vbox({
                text("=== HLAVNÍ MENU ===") | bold,
                main_menu->Render(),
                text("ENTER = vybrat")
            }) | border;
        }

        if (state == ScreenState::Characters) {
            return vbox({
                text("=== POSTAVY ===") | bold,
                char_menu->Render(),
                text("ENTER = vybrat | ESC = zpět")
            }) | border;
        }

        if (state == ScreenState::CharacterDetail) {
            return vbox({
                text("=== DETAIL POSTAVY ===") | bold,
                text(descriptions[char_selected]),
                text("ESC = zpět")
            }) | border;
        }

        return text("Chyba");
    });

    // Eventy
    Component component = CatchEvent(renderer, [&](Event event) {
        // Hlavni menu
        if (state == ScreenState::MainMenu) {
            if (event == Event::Return) {
                if (main_selected == 1) {
                    state = ScreenState::Characters;
                    return true;
                }
                if (main_selected == 2) {
                    screen.Exit();
                    return true;
                }
            }
        }
        // Postavy
        if (state == ScreenState::Characters) {
            if (event == Event::Return) {
                state = ScreenState::CharacterDetail;
                return true;
            }
            if (event == Event::Escape) {
                state = ScreenState::MainMenu;
                return true;
            }
        }
        // Detail postavy
        if (state == ScreenState::CharacterDetail) {
            if (event == Event::Escape) {
                state = ScreenState::Characters;
                return true;
            }
        }
        return false;
    });

    screen.Loop(component);

    return 0;
}
