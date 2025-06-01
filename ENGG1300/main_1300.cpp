#include "1300.h"
#include "text_animator.h"
#include <ncursesw/ncurses.h>

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
        init_pair(1, COLOR_RED, COLOR_BLACK);    // Paddle color
        init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Ball color
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Block color 1
        init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Block color 2
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);// Block color 3
        init_pair(6, COLOR_BLUE, COLOR_BLACK);   // Block color 4
        init_pair(7, COLOR_WHITE, COLOR_BLACK);  // Block color 5
    }

    // Show game introduction
    showGameIntroduction();

    // Track total score (based on completed rounds)
    int totalScore = 0;
    
    // Run all three rounds in sequence
    showRoundInfo(1);
    int result1 = runRound(1);
    if (result1 > 0) {
        totalScore += 100;
        showRoundComplete(1, totalScore);
        
        clear();
        showRoundInfo(2);
        int result2 = runRound(2);
        if (result2 > 0) {
            totalScore += 200;
            showRoundComplete(2, totalScore);
            
            clear();
            showRoundInfo(3);
            int result3 = runRound(3);
            if (result3 > 0) {
                totalScore += 300;
                // Player completed all rounds
                gameOver(true, 3, totalScore);
            } else {
                // Failed on round 3
                gameOver(false, 3, totalScore);
            }
        } else {
            // Failed on round 2
            gameOver(false, 2, totalScore);
        }
    } else {
        // Failed on round 1
        gameOver(false, 1, totalScore);
    }

    // Clean up
    endwin();
    return 0;
}