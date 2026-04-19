cmake_minimum_required(VERSION 3.11)
project(UltimateTextGame)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(FTXUI)

add_executable(UltimateTextGame src/main.cpp)

target_link_libraries(UltimateTextGame
  PRIVATE ftxui::screen
          ftxui::dom
          ftxui::component
)