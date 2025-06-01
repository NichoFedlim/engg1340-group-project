#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>

// Forward declarations for the games
int run_engg1300_game(); // Breakout game
int run_engg1310_game(); // SafeTiles game
int run_engg1320_game(); // Coin Chase game
int run_engg1330_game(); // Laser Battle game
int run_engg1340_game(); // Final game

// Structure to represent a level
struct Level {
    std::string name;
    int (*run_function)(); // Function pointer to run the game
};

// Structure to track a cloud's position
struct Cloud {
    int x;
    int y;
    char symbol;
    int speed; // 1 = slow, 2 = medium, 3 = fast
};

// Helper function to draw a single menu item (highlighted or not)
void drawMenuItem(int row, int col, const std::string& text, bool isHighlighted) {
    if (isHighlighted) {
        attron(A_REVERSE);
        mvprintw(row, col, "%s", text.c_str());
        attroff(A_REVERSE);
    } else {
        mvprintw(row, col, "%s", text.c_str());
    }
}

// Function to draw a simple box around text
void drawTextBox(int y, int x, int height, int width) {
    // Basic ASCII box characters for better terminal compatibility
    mvaddch(y, x, '+');
    mvaddch(y, x + width - 1, '+');
    mvaddch(y + height - 1, x, '+');
    mvaddch(y + height - 1, x + width - 1, '+');
    
    for (int i = 1; i < width - 1; i++) {
        mvaddch(y, x + i, '-');
        mvaddch(y + height - 1, x + i, '-');
    }
    
    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, '|');
        mvaddch(y + i, x + width - 1, '|');
    }
}

// Function to draw a fancy title with a border
void drawFancyTitle(int y, int x, const std::string& title) {
    int width = title.length() + 8;
    
    // Draw box using simple ASCII characters
    drawTextBox(y, x, 3, width);
    
    // Add title
    mvprintw(y + 1, x + 4, "%s", title.c_str());
}

// Function to initialize clouds
std::vector<Cloud> initClouds(int maxY, int maxX, int count) {
    std::vector<Cloud> clouds;
    for (int i = 0; i < count; i++) {
        Cloud cloud;
        cloud.x = rand() % maxX; // Start at random x positions
        cloud.y = 1 + (rand() % (maxY - 2)); // Random y position
        cloud.symbol = '.';
        cloud.speed = (rand() % 3) + 1; // Random speed 1-3
        clouds.push_back(cloud);
    }
    return clouds;
}

// Function to update and draw clouds
void updateAndDrawClouds(std::vector<Cloud>& clouds, int maxY, int maxX) {
    // First clear old positions
    for (size_t i = 0; i < clouds.size(); i++) {
        // Only clear if there's a cloud at this position (don't clear other text)
        chtype ch = mvinch(clouds[i].y, clouds[i].x);
        if ((ch & A_CHARTEXT) == '.') {
            mvaddch(clouds[i].y, clouds[i].x, ' ');
        }
    }
    
    // Update positions
    for (size_t i = 0; i < clouds.size(); i++) {
        // Move cloud to the right based on speed
        clouds[i].x += clouds[i].speed;
        
        // If cloud moves off screen, wrap around to the left
        if (clouds[i].x >= maxX) {
            clouds[i].x = 0;
            clouds[i].y = 1 + (rand() % (maxY - 2)); // New random y position
        }
        
        // Draw at new position
        // Check if position already has text
        chtype ch = mvinch(clouds[i].y, clouds[i].x);
        if ((ch & A_CHARTEXT) == ' ') {
            // Only draw cloud if space is empty
            mvaddch(clouds[i].y, clouds[i].x, clouds[i].symbol);
        }
    }
}

// Function to draw a centered box with a message
void drawLevelBox(int y, int x, int height, int width, const std::string& title, bool isHighlighted) {
    // Draw the box border with simple ASCII characters
    if (isHighlighted) {
        attron(A_BOLD);
    }
    
    drawTextBox(y, x, height, width);
    
    // Fill the box interior
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            mvaddch(y + i, x + j, ' ');
        }
    }
    
    // Draw title
    int titleY = y + height / 2;
    int titleX = x + (width - title.length()) / 2;
    mvprintw(titleY, titleX, "%s", title.c_str());
    
    if (isHighlighted) {
        attroff(A_BOLD);
    }
}

// Function to display the main menu and handle navigation
int displayMainMenu(std::vector<std::string>& menuItems) {
    int highlight = 0;    // Which menu item is highlighted
    int choice = -1;      // User's final choice
    int input;

    // Get terminal dimensions
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Calculate start positions for various items
    int menuHeight = menuItems.size();
    int startY = (termHeight - menuHeight) / 2;
    int startX = termWidth / 2;
    
    // A simple banner to display above the menu
    std::vector<std::string> banner;
    banner.push_back("The Year 1 Engineering");
    banner.push_back("Experience");

    // Initialize moving clouds
    std::vector<Cloud> clouds = initClouds(termHeight, termWidth, 10);
    
    // 1) Draw the framework (box, banner, instructions)
    clear();
    
    // Draw outer box
    drawTextBox(0, 0, termHeight, termWidth);
    
    // Display banner
    for (int i = 0; i < (int)banner.size(); i++) {
        int bannerRow = startY - 5 + i * 2;
        int bannerCol = startX - (int)banner[i].size() / 2;
        
        // Draw a fancy box around the title
        if (i == 0) {
            int boxWidth = banner[i].size() + 6;
            int boxStart = bannerCol - 3;
            
            drawTextBox(bannerRow - 1, boxStart, 3, boxWidth);
        }
        
        // Draw the title text
        attron(A_BOLD);
        mvprintw(bannerRow, bannerCol, "%s", banner[i].c_str());
        attroff(A_BOLD);
    }

    // Display instructions
    std::string instruction1 = "Use UP/DOWN Arrow Keys to Navigate";
    std::string instruction2 = "Press ENTER to Select";
    mvprintw(termHeight - 3, (termWidth - instruction1.length()) / 2, "%s", instruction1.c_str());
    mvprintw(termHeight - 2, (termWidth - instruction2.length()) / 2, "%s", instruction2.c_str());

    // 2) Draw all menu items in normal text with decorative elements
    for (int i = 0; i < (int)menuItems.size(); i++) {
        int row = 3 + startY + i * 2;
        int col = startX - (int)menuItems[i].size() / 2;
        
        // Draw decorative arrows around the selected item
        if (i == highlight) {
            attron(A_BOLD);
            mvprintw(row, col - 3, ">>");
            mvprintw(row, col + menuItems[i].size() + 1, "<<");
            attroff(A_BOLD);
        }
        
        drawMenuItem(row, col, menuItems[i], (i == highlight));
    }
    
    refresh(); // Show everything

    // 3) Main input loop: highlight changes or user presses Enter
    int animationCounter = 0;
    timeout(150); // Set timeout for getch to enable animation (slightly slower to reduce lag)
    
    while (choice == -1) {
        input = getch();
        
        // Update and draw the moving clouds
        updateAndDrawClouds(clouds, termHeight, termWidth);
        
        // If timeout occurred, just refresh and continue
        if (input == ERR) {
            refresh();
            animationCounter++;
            continue;
        }

        switch (input) {
            case KEY_UP: {
                // Un-highlight the old item
                int oldRow = 3 + startY + highlight * 2;
                int oldCol = startX - (int)menuItems[highlight].size() / 2;
                drawMenuItem(oldRow, oldCol, menuItems[highlight], false);
                
                // Clear decorative arrows
                mvprintw(oldRow, oldCol - 3, "  ");
                mvprintw(oldRow, oldCol + menuItems[highlight].size() + 1, "  ");

                // Move highlight up
                highlight--;
                if (highlight < 0) highlight = (int)menuItems.size() - 1;

                // Highlight the new item
                int newRow = 3 + startY + highlight * 2;
                int newCol = startX - (int)menuItems[highlight].size() / 2;
                drawMenuItem(newRow, newCol, menuItems[highlight], true);
                
                // Draw new decorative arrows
                attron(A_BOLD);
                mvprintw(newRow, newCol - 3, ">>");
                mvprintw(newRow, newCol + menuItems[highlight].size() + 1, "<<");
                attroff(A_BOLD);
                
                refresh();
                break;
            }
            case KEY_DOWN: {
                // Un-highlight the old item
                int oldRow = 3 + startY + highlight * 2;
                int oldCol = startX - (int)menuItems[highlight].size() / 2;
                drawMenuItem(oldRow, oldCol, menuItems[highlight], false);
                
                // Clear decorative arrows
                mvprintw(oldRow, oldCol - 3, "  ");
                mvprintw(oldRow, oldCol + menuItems[highlight].size() + 1, "  ");

                // Move highlight down
                highlight++;
                if (highlight >= (int)menuItems.size()) highlight = 0;

                // Highlight the new item
                int newRow = 3 + startY + highlight * 2;
                int newCol = startX - (int)menuItems[highlight].size() / 2;
                drawMenuItem(newRow, newCol, menuItems[highlight], true);
                
                // Draw new decorative arrows
                attron(A_BOLD);
                mvprintw(newRow, newCol - 3, ">>");
                mvprintw(newRow, newCol + menuItems[highlight].size() + 1, "<<");
                attroff(A_BOLD);
                
                refresh();
                break;
            }
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                // Ignore other keys
                break;
        }
    }
    
    timeout(-1); // Reset timeout to blocking mode
    return choice;
}

// Function to display the level selection menu with horizontal navigation
int displayLevelSelectionMenu(std::vector<Level>& levels) {
    int currentLevel = 0;  // Currently displayed level
    int choice = -1;       // User's final choice
    int input;
    int animCounter = 0;   // Counter for animation

    // Get terminal dimensions
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Box dimensions
    int boxWidth = 20;
    int boxHeight = 7;
    
    // Initialize moving clouds
    std::vector<Cloud> clouds = initClouds(termHeight, termWidth, 10);
    
    // Initial screen setup
    clear();
    drawTextBox(0, 0, termHeight, termWidth);
    
    // Display fixed elements that don't change during navigation
    std::string title = "Select a Level";
    int titleX = (termWidth - title.length() - 8) / 2;
    drawFancyTitle(2, titleX, title);
    
    // Draw a horizontal line below the title
    for (int i = 1; i < termWidth - 1; i++) {
        mvaddch(6, i, '-');
    }
    mvaddch(6, 0, '+');
    mvaddch(6, termWidth - 1, '+');
    
    // Display navigation instructions in a box at the bottom
    int instructY = termHeight - 5;
    for (int i = 1; i < termWidth - 1; i++) {
        mvaddch(instructY, i, '-');
    }
    mvaddch(instructY, 0, '+');
    mvaddch(instructY, termWidth - 1, '+');
    
    mvprintw(instructY + 1, 2, "LEFT/RIGHT: Navigate between levels");
    mvprintw(instructY + 2, 2, "ENTER: Select level");
    mvprintw(instructY + 3, 2, "ESC: Return to main menu");
    
    // Draw a decorative footer
    std::string footerText = "Year 1 Engineering Experience";
    mvprintw(termHeight - 1, (termWidth - footerText.length()) / 2, "%s", footerText.c_str());
    
    // Variable to store previous level to avoid redrawing unnecessarily
    int previousLevel = -1;
    
    // Set timeout for getch to enable animation (slightly slower)
    timeout(150);
    
    while (choice == -1) {
        // Update and draw the moving clouds
        updateAndDrawClouds(clouds, termHeight, termWidth);
        
        // Update animation counter
        animCounter++;
        
        // Only redraw what changed
        if (currentLevel != previousLevel || animCounter % 5 == 0) {
            // Store current level as previous for next iteration
            previousLevel = currentLevel;
            
            // Display current level - calculate positions
            int boxY = (termHeight - boxHeight) / 2;
            int boxX = (termWidth - boxWidth) / 2;
            
            // Clear the area where level boxes are displayed
            for (int y = boxY - 1; y < boxY + boxHeight + 3; y++) {
                for (int x = boxX - boxWidth - 10; x < boxX + boxWidth + 10; x++) {
                    if (x > 0 && x < termWidth - 1 && y > 6 && y < instructY) {
                        // Don't clear the clouds
                        chtype ch = mvinch(y, x) & A_CHARTEXT;
                        if (ch != '.') {
                            mvaddch(y, x, ' ');
                        }
                    }
                }
            }
            
            // Clear navigation areas
            // Left side for PREV
            for (int y = boxY; y < boxY + boxHeight; y++) {
                for (int x = 2; x < boxX - boxWidth - 10; x++) {
                    if (y > 6 && y < instructY) {
                        chtype ch = mvinch(y, x) & A_CHARTEXT;
                        if (ch != '.') {
                            mvaddch(y, x, ' ');
                        }
                    }
                }
            }
            
            // Right side for NEXT
            for (int y = boxY; y < boxY + boxHeight; y++) {
                for (int x = boxX + boxWidth + 10; x < termWidth - 2; x++) {
                    if (y > 6 && y < instructY) {
                        chtype ch = mvinch(y, x) & A_CHARTEXT;
                        if (ch != '.') {
                            mvaddch(y, x, ' ');
                        }
                    }
                }
            }
            
            // Draw navigation arrows with animation - positioned far away from level boxes
            if (currentLevel > 0) {
                std::string leftArrow = "<<< PREV";
                // Position far left to avoid overlap
                int leftPosX = 4;
                int leftPosY = boxY + boxHeight / 2;
                
                // Animation shifts position slightly
                if (animCounter % 6 < 3) {
                    mvprintw(leftPosY, leftPosX, "%s", leftArrow.c_str());
                } else {
                    mvprintw(leftPosY, leftPosX + 1, "%s", leftArrow.c_str());
                }
            }
            
            if (currentLevel < (int)levels.size() - 1) {
                std::string rightArrow = "NEXT >>>";
                // Position far right to avoid overlap
                int rightPosX = termWidth - 12;
                int rightPosY = boxY + boxHeight / 2;
                
                // Animation shifts position slightly
                if (animCounter % 6 < 3) {
                    mvprintw(rightPosY, rightPosX, "%s", rightArrow.c_str());
                } else {
                    mvprintw(rightPosY, rightPosX + 1, "%s", rightArrow.c_str());
                }
            }
            
            // Draw level indicators (small dots)
            int indicatorY = boxY + boxHeight + 2;
            int indicatorX = (termWidth - levels.size() * 2) / 2;
            
            // Draw level indicator dots
            for (int i = 0; i < (int)levels.size(); i++) {
                if (i == currentLevel) {
                    mvaddch(indicatorY, indicatorX + i * 2, 'O');
                } else {
                    mvaddch(indicatorY, indicatorX + i * 2, '.');
                }
            }
            
            // Draw the active level box in the center
            drawLevelBox(boxY, boxX, boxHeight, boxWidth, levels[currentLevel].name, true);
            
            // Draw previous level (if any)
            if (currentLevel > 0) {
                drawLevelBox(boxY + 2, boxX - boxWidth - 5, boxHeight - 2, boxWidth - 4,
                           levels[currentLevel - 1].name, false);
            }
            
            // Draw next level (if any) - but only if it's not the last level
            if (currentLevel < (int)levels.size() - 1) {
                drawLevelBox(boxY + 2, boxX + boxWidth + 5, boxHeight - 2, boxWidth - 4,
                           levels[currentLevel + 1].name, false);
            }
            
            // Draw level progress information
            std::string progressText = "Level " + std::to_string(currentLevel + 1) + " of " + std::to_string(levels.size());
            
            // Clear the area where progress text is displayed
            for (int x = (termWidth - progressText.length()) / 2 - 5; 
                x < static_cast<int>((termWidth + progressText.length()) / 2 + 5); x++) {
                if (x > 0 && x < termWidth - 1) {
                    mvaddch(boxY - 2, x, ' ');
                }
            }
            
            mvprintw(boxY - 2, (termWidth - progressText.length()) / 2, "%s", progressText.c_str());
            
            // Refresh to show changes
            refresh();
        } else {
            // Just refresh for animation
            refresh();
        }
        
        // Get user input
        input = getch();
        
        // If timeout occurred, just continue the loop
        if (input == ERR) {
            continue;
        }
        
        switch (input) {
            case KEY_LEFT:
                if (currentLevel > 0) {
                    currentLevel--;
                }
                break;
                
            case KEY_RIGHT:
                if (currentLevel < (int)levels.size() - 1) {
                    currentLevel++;
                }
                break;
            
            case 10: // Enter key
                choice = currentLevel;
                break;
                
            case 27: // ESC key
                timeout(-1); // Reset timeout to blocking mode
                return -1; // Return to main menu
                
            default:
                // Ignore other keys
                break;
        }
    }
    
    timeout(-1); // Reset timeout to blocking mode
    return choice;
}

// Main game loop
int main() {
    // Initialize ncurses ONCE here
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Initialize random number generator
    srand(time(NULL));

    // Main menu items - simplified to just Play and Exit
    std::vector<std::string> mainMenuItems = {
        "Play",
        "Exit"
    };

    // Initialize all levels - all are playable
    std::vector<Level> levels = {
        {"ENGG1300", run_engg1300_game}, // Breakout
        {"ENGG1310", run_engg1310_game}, // SafeTiles
        {"ENGG1320", run_engg1320_game}, // Coin Chase
        {"ENGG1330", run_engg1330_game}, // Laser Battle
        {"ENGG1340", run_engg1340_game}  // Final game
    };
    
    bool running = true;
    while (running) {
        // Show the main menu and get the user's choice
        int mainChoice = displayMainMenu(mainMenuItems);

        // Handle the user's choice
        switch (mainChoice) {
            case 0: { // "Play"
                // Show level selection menu
                while (true) {
                    int levelChoice = displayLevelSelectionMenu(levels);
                    
                    if (levelChoice == -1) {
                        // User pressed ESC, go back to main menu
                        break;
                    } else if (levelChoice >= 0 && levelChoice < (int)levels.size()) {
                        // Run the selected level
                        levels[levelChoice].run_function();
                        // No need to track results or save progress
                    }
                }
                break;
            }
            
            case 1: // "Exit"
                running = false;
                break;
                
            default:
                // Should never happen
                break;
        }
    }

    // End ncurses once at the very end
    endwin();

    return 0;
}

// Game launcher functions with better error handling

// Run the ENGG1300 game from the ENGG1300 directory
int run_engg1300_game() {
    // Clear the screen and reset terminal settings
    endwin();
    
    // Call the actual game - using cd to switch to the correct directory first
    // Using lowercase makefile as seen in your directory structure
    int result = system("cd ENGG1300 && make -f makefile run");
    
    // Re-initialize ncurses for our menu
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // For debugging
    if (result != 0) {
        clear();
        printw("Error running ENGG1300 game. Result code: %d\n", result);
        printw("Press any key to continue...");
        refresh();
        getch();
    }
    
    // Return 1 for success (level completed), 0 for failure
    return (result == 0) ? 1 : 0;
}

// Run the ENGG1310 game from the ENGG1310 directory
int run_engg1310_game() {
    // Clear the screen and reset terminal settings
    endwin();
    
    // Call the actual game - using cd to switch to the correct directory first
    // Using lowercase makefile as seen in your directory structure
    int result = system("cd ENGG1310 && make -f makefile run");
    
    // Re-initialize ncurses for our menu
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // For debugging
    if (result != 0) {
        clear();
        printw("Error running ENGG1310 game. Result code: %d\n", result);
        printw("Press any key to continue...");
        refresh();
        getch();
    }
    
    // Return 1 for success (level completed), 0 for failure
    return (result == 0) ? 1 : 0;
}

// Run the ENGG1320 game from the ENGG1320 directory
int run_engg1320_game() {
    // Clear the screen and reset terminal settings
    endwin();
    
    // Call the actual game - using cd to switch to the correct directory first
    // Using lowercase makefile as seen in your directory structure
    int result = system("cd ENGG1320 && make -f makefile run");
    
    // Re-initialize ncurses for our menu
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // For debugging
    if (result != 0) {
        clear();
        printw("Error running ENGG1320 game. Result code: %d\n", result);
        printw("Press any key to continue...");
        refresh();
        getch();
    }
    
    // Return 1 for success (level completed), 0 for failure
    return (result == 0) ? 1 : 0;
}

// Run the ENGG1330 game from the ENGG1330 directory
int run_engg1330_game() {
    // Clear the screen and reset terminal settings
    endwin();
    
    // Call the actual game - using cd to switch to the correct directory first
    // Using lowercase makefile as seen in your directory structure
    int result = system("cd ENGG1330 && make -f makefile run");
    
    // Re-initialize ncurses for our menu
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // For debugging
    if (result != 0) {
        clear();
        printw("Error running ENGG1330 game. Result code: %d\n", result);
        printw("Press any key to continue...");
        refresh();
        getch();
    }
    
    // Return 1 for success (level completed), 0 for failure
    return (result == 0) ? 1 : 0;
}

// Run the ENGG1340 game from the ENGG1340 directory
int run_engg1340_game() {
    // Clear the screen and reset terminal settings
    endwin();
    
    // Call the actual game - using cd to switch to the correct directory first
    // Using lowercase makefile as seen in your directory structure
    int result = system("cd ENGG1340 && make -f makefile run");
    
    // Re-initialize ncurses for our menu
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // For debugging
    if (result != 0) {
        clear();
        printw("Error running ENGG1340 game. Result code: %d\n", result);
        printw("Press any key to continue...");
        refresh();
        getch();
    }
    
    // Return 1 for success (level completed), 0 for failure
    return (result == 0) ? 1 : 0;
}