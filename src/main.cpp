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


int main() {
    ScreenInteractive screen = ScreenInteractive::TerminalOutput();

    // položky menu
    std::vector<std::string> entries = {
        "Start hry",
        "Nastavení",
        "Konec"
    };

    int selected = 0;

    // vytvoření menu
    Component menu = Menu(&entries, &selected);

    // co se vykreslí
    Component renderer = Renderer(menu, [&] {
        return vbox({
            text("=== MENU ===") | bold,
            menu->Render()
        }) | border;
    });

    screen.Loop(renderer);

    // po ukončení
    cout << "Vybral jsi: " << entries[selected] << "\n";
}
