cmake_minimum_required(VERSION 3.10)
project(PF_Minesweeper)

set(CMAKE_CXX_STANDARD 17)

# Point to your SFML 2.6.1 installation
set(SFML_DIR "C:/msys64/ucrt64/lib/cmake/SFML")

find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

add_executable(Minesweeper main.cpp)

# Use the old lowercase linking style
target_link_libraries(Minesweeper sfml-graphics sfml-window sfml-system sfml-audio)