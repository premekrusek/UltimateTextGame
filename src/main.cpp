#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

using namespace ftxui;

int main() {
    auto document = vbox({
        text("Moje hra") | bold,
        text("HP: 100") | color(Color::Green),
    });

    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    screen.Print();

    return 0;
}
