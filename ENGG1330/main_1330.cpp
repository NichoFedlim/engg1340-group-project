#include "1330.h"
#include "text_animator.h"
#include <ncursesw/ncurses.h>

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Set up colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_CYAN, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);
    }
    
    // Show game introduction
    showGameIntroduction();
    nodelay(stdscr, TRUE); // Reset nodelay after animation
    
    // Main game loop
    bool gameWon = true;
    int finalRound = 0;
    int currentHP = 10; // Track health between rounds
    
    for (int round = 1; round <= 7; round++) {
        finalRound = round;
        
        // Show dialogue
        showRoundInfo(round);
        nodelay(stdscr, TRUE); // Reset nodelay after animation
        
        clear();
        refresh();
        
        // Run the round with the current HP
        bool roundSuccess = runRound(round, currentHP);
        
        // Get the updated HP from the runRound function (this will be added later)
        currentHP = getLastHP(); // Get the updated HP after the round
        
        if (roundSuccess) {
            showRoundComplete(round, true);
            nodelay(stdscr, TRUE); // Reset nodelay after animation
        } else {
            gameWon = false;
            break;
        }
    }
    
    // Show ending
    gameOver(gameWon, finalRound);
    
    // Clean up
    endwin();
    return 0;
}