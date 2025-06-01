#include "1320.h"
#include "text_animator.h"
#include <ncurses.h>

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);  // Non-blocking input
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);     // Player
        init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Arena
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Regular coins & dialogue
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Special coins & dialogue box
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Trap coins
        init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Messages
    }

    // Show game introduction
    showGameIntroduction();

    // Track total score (based on completed rounds and coins collected)
    int totalScore = 0;
    
    // Run all five rounds in sequence
    for (int round = 1; round <= MAX_LEVEL; round++) {
        // Show information about this round
        clear();
        showRoundInfo(round);
        
        // Run the round
        int result = runRound(round);
        
        if (result > 0) {
            // Successfully completed the round
            totalScore += round * 100;  // Score based on round number
            showRoundComplete(round, totalScore);
        } else {
            // Failed the round
            gameOver(false, round, totalScore);
            endwin();
            return 0;
        }
    }
    
    // Player completed all rounds
    gameOver(true, MAX_LEVEL, totalScore);

    // Clean up
    endwin();
    return 0;
}