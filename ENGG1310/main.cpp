#include <ncursesw/ncurses.h>
#include <cstdlib>
#include <ctime>
#include "text_animator.h"
#include "1310.h"

int main() {
    // Initialize random seed
    srand(time(nullptr));
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);     // Red heart/danger
        init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Safe tiles
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Messages/names
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // UI elements
        init_pair(5, COLOR_WHITE, COLOR_BLACK);   // Normal borders
    }

    // Show game introduction
    showGameIntroduction();

    // Run the game
    runSafeTilesGame();
    
    // Clean up and exit
    endwin();
    return 0;
}