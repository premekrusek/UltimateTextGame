# UltimateTextGame

Jednoduchá hra v C++ využívající knihovnu **SFML**.

## Požadavky

* C++ kompilátor (g++)
* SFML (nainstalované přes Homebrew)

## Instalace SFML (macOS)

```bash
brew install sfml
```

## Kompilace

Ve složce `src` spusť:

```bash
g++ main.cpp -o hra -std=c++17 \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lsfml-graphics -lsfml-window -lsfml-system
```

## Spuštění

```bash
./hra
```

## Poznámky

* Projekt je určen pro macOS
* SFML se instaluje globálně (není součástí projektu)
