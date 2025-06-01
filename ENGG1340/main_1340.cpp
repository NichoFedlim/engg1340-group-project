#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <vector>
#include "text_animator.h"
#include "1340.h"

int main() {
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    
    // Enable function keys, arrow keys, etc.
    keypad(stdscr, TRUE);
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);     // Red heart
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Yellow for invincibility
        init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Cyan for laser/name
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Blue for spaceships/dialogue box
        init_pair(5, COLOR_GREEN, COLOR_BLACK);   // Green for projectiles
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Magenta for bombs
        init_pair(7, COLOR_WHITE, COLOR_BLACK);   // White for text
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Show game introduction
    showGameIntroduction();
    
    // Create a heart object for passing to setUpRound
    // Note: This is just a temporary heart for setup, the real one is created in runRound
    Heart tempHeart(maxX/2, maxY/2);
    
    // Main game loop for all 3 rounds
    bool gameCompleted = false;
    int finalRound = 1;
    int playerHealth = INITIAL_PLAYER_HEALTH; // Start with initial health
    int playerScore = 0;
    
    for (int currentRound = 1; currentRound <= 3; currentRound++) {
        // Important: Set nodelay mode for non-blocking input during gameplay
        nodelay(stdscr, TRUE);
        
        // Show round info dialogue
        clear();
        showRoundInfo(currentRound);
        
        // Clear screen before starting the round
        clear();
        refresh();
        
        // Run the round with current player stats
        std::vector<int> result = runRound(currentRound, playerHealth);
        
        // Switch back to blocking input for dialogues
        nodelay(stdscr, FALSE);
        
        // Process round result
        // result[0] is status: 1 = won, -1 = lost
        // result[1] is player health
        // result[2] is player score
        
        if (result[0] == -1) {
            // Player died
            finalRound = currentRound;
            playerHealth = 0;
            playerScore = result[2]; // Store final score
            gameCompleted = false;
            break;
        } else if (result[0] == 1) {
            // Player completed the round
            playerHealth = result[1];
            playerScore = result[2];
            
            if (currentRound == 3) {
                // Completed all rounds
                finalRound = 3;
                gameCompleted = true;
                break;
            }
        }
    }
    
    // Game end dialogue
    gameOver(gameCompleted, finalRound, playerScore);
    
    // Clean up
    endwin();
    return 0;
}