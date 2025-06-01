#include "1310.h"
#include "text_animator.h"
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>

Heart::Heart(int startX, int startY) : 
    x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
    lastDrawnX(startX), lastDrawnY(startY),
    directionX(0.0f), directionY(0.0f),
    baseSpeed(0.3f), aspectRatio(2.0f), // Assume character cells are about half as wide as they are tall
    moving(false), symbol(ACS_DIAMOND) {}

void Heart::update() {
    if (moving) {
        // Move in the current direction with aspect ratio compensation
        // Horizontal movement is sped up by the aspect ratio
        x += directionX * baseSpeed * aspectRatio;
        // Vertical movement stays at base speed
        y += directionY * baseSpeed;
    }
}

void Heart::setDirection(float dx, float dy) {
    // Set a new direction vector
    if (dx != 0.0f || dy != 0.0f) {
        // Normalize the direction vector
        float length = sqrt(dx * dx + dy * dy);
        directionX = dx / length;
        directionY = dy / length;
        moving = true;  // Start moving when a direction is set
    }
}

void Heart::setAspectRatio(float ratio) {
    aspectRatio = ratio;
}

void Heart::setSpeed(float speed) {
    baseSpeed = speed;
}

void Heart::stop() {
    moving = false;
}

void Heart::start() {
    moving = true;
}

bool Heart::isMoving() const {
    return moving;
}

void Heart::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Heart::clearPrevious() {
    // Clear the previous position
    mvaddch(lastDrawnY, lastDrawnX, ' ');
}

void Heart::draw() {
    int currentX = static_cast<int>(round(x));
    int currentY = static_cast<int>(round(y));
    
    // Only redraw if position has changed
    if (currentX != lastDrawnX || currentY != lastDrawnY) {
        // Clear previous position if it's different
        clearPrevious();
        
        // Draw at new position
        attron(COLOR_PAIR(1)); // Red heart color
        mvaddch(currentY, currentX, symbol);
        attroff(COLOR_PAIR(1));
        
        // Update last drawn position
        lastDrawnX = currentX;
        lastDrawnY = currentY;
    } else {
        // Redraw at same position (in case it was overwritten)
        attron(COLOR_PAIR(1)); // Red heart color
        mvaddch(currentY, currentX, symbol);
        attroff(COLOR_PAIR(1));
    }
}

float Heart::getX() const { return x; }
float Heart::getY() const { return y; }
float Heart::getDirectionX() const { return directionX; }
float Heart::getDirectionY() const { return directionY; }
float Heart::getAspectRatio() const { return aspectRatio; }
float Heart::getSpeed() const { return baseSpeed; }

GameBoard::GameBoard(int startX, int startY, int w, int h, int tw, int th) :
    x(startX), y(startY), width(w), height(h), tileWidth(tw), tileHeight(th) {}

void GameBoard::draw(bool showSafeTiles, bool redBorder) {
    // Draw the game board grid
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Choose border color
            if (redBorder) {
                attron(COLOR_PAIR(1)); // Red borders
            } else {
                attron(COLOR_PAIR(5)); // White borders
            }
            
            // Draw horizontal line at the top of each tile
            for (int i = 0; i < tileWidth; i++) {
                mvaddch(y + row * tileHeight, x + col * tileWidth + i, '-');
            }
            
            // Draw vertical line at the left of each tile
            for (int i = 0; i < tileHeight; i++) {
                mvaddch(y + row * tileHeight + i, x + col * tileWidth, '|');
            }
            
            // Draw corner
            mvaddch(y + row * tileHeight, x + col * tileWidth, '+');
            
            if (redBorder) {
                attroff(COLOR_PAIR(1));
            } else {
                attroff(COLOR_PAIR(5));
            }
        }
    }
    
    // Draw the right border (only for rows 0 to height-1)
    for (int row = 0; row < height; row++) {
        if (redBorder) {
            attron(COLOR_PAIR(1)); // Red borders
        } else {
            attron(COLOR_PAIR(5)); // White borders
        }
        
        for (int i = 0; i < tileHeight; i++) {
            mvaddch(y + row * tileHeight + i, x + width * tileWidth, '|');
        }
        // Draw the right corners
        mvaddch(y + row * tileHeight, x + width * tileWidth, '+');
        
        if (redBorder) {
            attroff(COLOR_PAIR(1));
        } else {
            attroff(COLOR_PAIR(5));
        }
    }
    
    // Draw the bottom border (only for columns 0 to width-1)
    for (int col = 0; col < width; col++) {
        if (redBorder) {
            attron(COLOR_PAIR(1)); // Red borders
        } else {
            attron(COLOR_PAIR(5)); // White borders
        }
        
        for (int i = 0; i < tileWidth; i++) {
            mvaddch(y + height * tileHeight, x + col * tileWidth + i, '-');
        }
        // Draw the bottom corners
        mvaddch(y + height * tileHeight, x + col * tileWidth, '+');
        
        if (redBorder) {
            attroff(COLOR_PAIR(1));
        } else {
            attroff(COLOR_PAIR(5));
        }
    }
    
    // Draw the bottom-right corner
    if (redBorder) {
        attron(COLOR_PAIR(1));
    } else {
        attron(COLOR_PAIR(5));
    }
    mvaddch(y + height * tileHeight, x + width * tileWidth, '+');
    if (redBorder) {
        attroff(COLOR_PAIR(1));
    } else {
        attroff(COLOR_PAIR(5));
    }
    
    // Draw safe tiles if requested
    if (showSafeTiles) {
        for (const auto& tile : safeTiles) {
            int tileX = tile.first;
            int tileY = tile.second;
            
            // Fill in the tile with a distinct character
            attron(COLOR_PAIR(2)); // Green for safe tiles
            for (int i = 1; i < tileHeight; i++) {
                for (int j = 1; j < tileWidth; j++) {
                    mvaddch(y + tileY * tileHeight + i, x + tileX * tileWidth + j, ACS_CKBOARD);
                }
            }
            attroff(COLOR_PAIR(2));
        }
    }
}

void GameBoard::generateSafeTiles(int numTiles) {
    safeTiles.clear();
    
    // Create a list of all possible tile positions
    std::vector<std::pair<int, int>> allTiles;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            allTiles.push_back(std::make_pair(col, row));
        }
    }
    
    // Shuffle the list
    std::random_shuffle(allTiles.begin(), allTiles.end());
    
    // Select the first numTiles as safe tiles
    for (size_t i = 0; i < static_cast<size_t>(numTiles) && i < allTiles.size(); i++) {
        safeTiles.push_back(allTiles[i]);
    }
}

bool GameBoard::isSafeTile(int tileX, int tileY) const {
    return std::find(safeTiles.begin(), safeTiles.end(), 
                    std::make_pair(tileX, tileY)) != safeTiles.end();
}

std::pair<float, float> GameBoard::boardToScreen(int tileX, int tileY) const {
    return std::make_pair(
        x + tileX * tileWidth + tileWidth / 2.0f,
        y + tileY * tileHeight + tileHeight / 2.0f
    );
}

std::pair<int, int> GameBoard::screenToBoard(float screenX, float screenY) const {
    int boardX = static_cast<int>(floor((screenX - x) / tileWidth));
    int boardY = static_cast<int>(floor((screenY - y) / tileHeight));
    
    // Clamp to valid board coordinates
    boardX = std::max(0, std::min(width - 1, boardX));
    boardY = std::max(0, std::min(height - 1, boardY));
    
    return std::make_pair(boardX, boardY);
}

bool GameBoard::isOnBoard(float screenX, float screenY) const {
    // Check if coordinates are within the board area
    return screenX >= x && screenX < x + width * tileWidth &&
            screenY >= y && screenY < y + height * tileHeight;
}

bool GameBoard::isOnTile(float screenX, float screenY) const {
    // Check if coordinates are on a tile (not on grid lines)
    int boardX = static_cast<int>(floor((screenX - x) / tileWidth));
    int boardY = static_cast<int>(floor((screenY - y) / tileHeight));
    
    if (boardX < 0 || boardX >= width || boardY < 0 || boardY >= height) {
        return false;
    }
    
    float relativeX = (screenX - x) - boardX * tileWidth;
    float relativeY = (screenY - y) - boardY * tileHeight;
    
    return relativeX > 0 && relativeX < tileWidth &&
            relativeY > 0 && relativeY < tileHeight;
}

int GameBoard::getX() const { return x; }
int GameBoard::getY() const { return y; }
int GameBoard::getWidth() const { return width; }
int GameBoard::getHeight() const { return height; }
int GameBoard::getTileWidth() const { return tileWidth; }
int GameBoard::getTileHeight() const { return tileHeight; }

void countdown(int seconds, GameBoard& board, Heart& heart, int round, int totalRounds, int hp) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Calculate position above the board
    int countdownY = board.getY() - 3; // 3 lines above the board
    int countdownX = maxX / 2;
    
    // First display "Ready?"
    erase();
    box(stdscr, 0, 0);
    board.draw(false, false); // White borders during countdown
    heart.draw();
    
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(countdownY, countdownX - 3, "Ready?");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    drawHealthBar(NULL, 5, maxY - 2, 10, hp); // Assuming 10 is the maximum HP
    mvprintw(maxY - 2, maxX - 25, "Round: %d/%d", round, totalRounds);
    
    refresh();
    napms(1000); // Use napms instead of sleep for better compatibility
    
    // Then display the countdown numbers
    for (int i = seconds; i > 0; i--) {
        erase();
        box(stdscr, 0, 0);
        board.draw(false, false); // White borders during countdown
        heart.draw();
        
        // Display just the number (perfectly centered)
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(countdownY, countdownX, "%d", i);
        attroff(COLOR_PAIR(3) | A_BOLD);
        
        drawHealthBar(NULL, 5, maxY - 2, 10, hp); // Assuming 10 is the maximum HP
        mvprintw(maxY - 2, maxX - 25, "Round: %d/%d", round, totalRounds);
        
        refresh();
        napms(1000); // Use napms instead of sleep
    }
    
    // After countdown ends, draw the board with red borders and display "GO!"
    erase();
    box(stdscr, 0, 0);
    board.draw(false, true); // Red borders after countdown
    heart.draw();
    
    // Display "GO!" message perfectly centered (GO! is 3 characters)
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(countdownY, countdownX - 1, "GO!");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    drawHealthBar(NULL, 5, maxY - 2, 10, hp); // Assuming 10 is the maximum HP
    mvprintw(maxY - 2, maxX - 25, "Round: %d/%d", round, totalRounds);
    refresh();
    napms(1000); // Use napms instead of sleep
    
    erase();
}

void drawHealthBar(WINDOW* win, int x, int y, int maxHP, int currentHP) {
    // If win is NULL, draw to the standard screen
    if (win == NULL) {
        mvprintw(y, x, "HP: %d/%d [", currentHP, maxHP);
        
        int barWidth = 20; // Width of the health bar
        int filledWidth = static_cast<int>((static_cast<float>(currentHP) / maxHP) * barWidth);
        
        for (int i = 0; i < barWidth; i++) {
            if (i < filledWidth) {
                attron(COLOR_PAIR(1)); // Red for filled health
                addch('=');
                attroff(COLOR_PAIR(1));
            } else {
                addch('-');
            }
        }
        addch(']');
    } else {
        // Draw to the provided window
        mvwprintw(win, y, x, "HP: %d/%d [", currentHP, maxHP);
        
        int barWidth = 20; // Width of the health bar
        int filledWidth = static_cast<int>((static_cast<float>(currentHP) / maxHP) * barWidth);
        
        for (int i = 0; i < barWidth; i++) {
            if (i < filledWidth) {
                wattron(win, COLOR_PAIR(1)); // Red for filled health
                waddch(win, '=');
                wattroff(win, COLOR_PAIR(1));
            } else {
                waddch(win, '-');
            }
        }
        waddch(win, ']');
    }
}

void runSafeTilesGame() {
    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Game settings
    const int BOARD_WIDTH = 10;
    const int BOARD_HEIGHT = 8;
    const int TILE_WIDTH = 5;
    const int TILE_HEIGHT = 3;
    const int TOTAL_ROUNDS = 5;
    const int ROUND_TIME = 5; // 5 seconds for movement after flash
    int playerHP = 10;
    
    // Round configurations: {num_safe_tiles, flash_time_in_seconds}
    const std::vector<std::pair<int, float>> roundConfigs = {
        {5, 0.5f},  // Round 1: 5 tiles, 0.5 seconds
        {5, 0.3f},  // Round 2: 5 tiles, 0.3 seconds
        {3, 0.3f},  // Round 3: 3 tiles, 0.3 seconds
        {1, 0.3f},  // Round 4: 1 tile, 0.3 seconds
        {1, 0.1f}   // Round 5: 1 tile, 0.1 seconds
    };
    
    // Calculate board position to center it
    int boardX = (maxX - BOARD_WIDTH * TILE_WIDTH) / 2;
    int boardY = (maxY - BOARD_HEIGHT * TILE_HEIGHT) / 2;
    
    // Create game board and heart (player)
    GameBoard gameBoard(boardX, boardY, BOARD_WIDTH, BOARD_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    Heart heart(maxX / 2, maxY / 2);
    
    // Main game loop for rounds
    int currentRound = 1;
    bool gameWon = false;
    
    while (currentRound <= TOTAL_ROUNDS && playerHP > 0) {
        // Get configuration for current round
        int numSafeTiles = roundConfigs[currentRound - 1].first;
        float flashTime = roundConfigs[currentRound - 1].second;
        
        // Show round information
        showRoundInfo(currentRound, numSafeTiles, flashTime, playerHP);
        
        // Generate safe tiles for this round
        gameBoard.generateSafeTiles(numSafeTiles);
        
        // Set initial heart position to the center of a random tile
        auto centerTile = gameBoard.boardToScreen(BOARD_WIDTH / 2, BOARD_HEIGHT / 2);
        heart.setPosition(centerTile.first, centerTile.second);
        
        // Draw empty board with player
        erase(); // Use erase instead of clear
        box(stdscr, 0, 0); // Add border around the screen
        gameBoard.draw(false, false); // White border initially
        heart.draw();
        refresh();
        
        // Countdown before showing safe tiles (board and player remain visible)
        countdown(3, gameBoard, heart, currentRound, TOTAL_ROUNDS, playerHP);
        
        // Show safe tiles
        erase(); // Use erase instead of clear
        box(stdscr, 0, 0); // Add border around the screen
        gameBoard.draw(true, true); // Show safe tiles with red border
        heart.draw();
        refresh();
        usleep(static_cast<useconds_t>(flashTime * 1000000));
        
        // Hide safe tiles and begin gameplay
        erase(); // Use erase instead of clear
        box(stdscr, 0, 0); // Add border around the screen
        gameBoard.draw(false, true); // Red border during gameplay
        heart.draw();
        
        // Display HP and round info
        drawHealthBar(NULL, 5, maxY - 2, 10, playerHP);
        mvprintw(maxY - 2, maxX - 25, "Round: %d/%d", currentRound, TOTAL_ROUNDS);
        refresh();
        
        // Enable non-blocking input
        nodelay(stdscr, TRUE);
        
        // Start the movement phase
        time_t startTime = time(nullptr);
        bool timeUp = false;
        
        // Create a buffer for the screen to prevent flickering
        WINDOW* buffer = newpad(maxY, maxX);
        
        while (!timeUp) {
            // Check if time is up (5 seconds per round)
            time_t currentTime = time(nullptr);
            int timeLeft = ROUND_TIME - (currentTime - startTime);
            
            if (timeLeft <= 0) {
                timeUp = true;
                continue;
            }
            
            // Process input
            int ch = getch();
            if (ch != ERR) {
                if (ch == 'q' || ch == 'Q') {
                    // Quit the game
                    delwin(buffer);
                    endwin();
                    return;
                } else if (ch == KEY_UP) {
                    heart.setDirection(0.0f, -1.0f);  // Up
                } else if (ch == KEY_DOWN) {
                    heart.setDirection(0.0f, 1.0f);   // Down
                } else if (ch == KEY_LEFT) {
                    heart.setDirection(-1.0f, 0.0f);  // Left
                } else if (ch == KEY_RIGHT) {
                    heart.setDirection(1.0f, 0.0f);   // Right
                } else if (ch == ' ') {
                    // Space toggles movement
                    if (heart.isMoving()) {
                        heart.stop();
                    } else {
                        heart.start();
                    }
                }
            }
            
            // Update heart position
            heart.update();
            
            // Boundary checking to keep heart inside the board
            float heartX = heart.getX();
            float heartY = heart.getY();
            
            if (!gameBoard.isOnBoard(heartX, heartY)) {
                // Push player back onto the board if they go outside
                float newX = std::max(static_cast<float>(gameBoard.getX() + 1), 
                                    std::min(heartX, static_cast<float>(gameBoard.getX() + gameBoard.getWidth() * gameBoard.getTileWidth() - 1)));
                float newY = std::max(static_cast<float>(gameBoard.getY() + 1), 
                                    std::min(heartY, static_cast<float>(gameBoard.getY() + gameBoard.getHeight() * gameBoard.getTileHeight() - 1)));
                heart.setPosition(newX, newY);
            }
            
            // Draw to buffer instead of screen directly to prevent flickering
            werase(buffer);
            box(buffer, 0, 0); // Add border around the screen
            
            // Draw the game board to buffer with red borders during gameplay
            // Simulate the draw functionality but on the buffer window
            for (int row = 0; row < gameBoard.getHeight(); row++) {
                for (int col = 0; col < gameBoard.getWidth(); col++) {
                    // Use red borders during gameplay
                    wattron(buffer, COLOR_PAIR(1));
                    
                    // Draw horizontal line at the top of each tile
                    for (int i = 0; i < gameBoard.getTileWidth(); i++) {
                        mvwaddch(buffer, gameBoard.getY() + row * gameBoard.getTileHeight(), 
                                gameBoard.getX() + col * gameBoard.getTileWidth() + i, '-');
                    }
                    
                    // Draw vertical line at the left of each tile
                    for (int i = 0; i < gameBoard.getTileHeight(); i++) {
                        mvwaddch(buffer, gameBoard.getY() + row * gameBoard.getTileHeight() + i, 
                                gameBoard.getX() + col * gameBoard.getTileWidth(), '|');
                    }
                    
                    // Draw corner
                    mvwaddch(buffer, gameBoard.getY() + row * gameBoard.getTileHeight(), 
                            gameBoard.getX() + col * gameBoard.getTileWidth(), '+');
                    
                    wattroff(buffer, COLOR_PAIR(1));
                }
            }
            
            // Draw the right border (only for rows 0 to height-1)
            for (int row = 0; row < gameBoard.getHeight(); row++) {
                wattron(buffer, COLOR_PAIR(1));
                
                for (int i = 0; i < gameBoard.getTileHeight(); i++) {
                    mvwaddch(buffer, gameBoard.getY() + row * gameBoard.getTileHeight() + i, 
                            gameBoard.getX() + gameBoard.getWidth() * gameBoard.getTileWidth(), '|');
                }
                // Draw the right corners
                mvwaddch(buffer, gameBoard.getY() + row * gameBoard.getTileHeight(), 
                        gameBoard.getX() + gameBoard.getWidth() * gameBoard.getTileWidth(), '+');
                
                wattroff(buffer, COLOR_PAIR(1));
            }
            
            // Draw the bottom border (only for columns 0 to width-1)
            for (int col = 0; col < gameBoard.getWidth(); col++) {
                wattron(buffer, COLOR_PAIR(1));
                
                for (int i = 0; i < gameBoard.getTileWidth(); i++) {
                    mvwaddch(buffer, gameBoard.getY() + gameBoard.getHeight() * gameBoard.getTileHeight(), 
                            gameBoard.getX() + col * gameBoard.getTileWidth() + i, '-');
                }
                // Draw the bottom corners
                mvwaddch(buffer, gameBoard.getY() + gameBoard.getHeight() * gameBoard.getTileHeight(), 
                        gameBoard.getX() + col * gameBoard.getTileWidth(), '+');
                
                wattroff(buffer, COLOR_PAIR(1));
            }
            
            // Draw the bottom-right corner
            wattron(buffer, COLOR_PAIR(1));
            mvwaddch(buffer, gameBoard.getY() + gameBoard.getHeight() * gameBoard.getTileHeight(), 
                    gameBoard.getX() + gameBoard.getWidth() * gameBoard.getTileWidth(), '+');
            wattroff(buffer, COLOR_PAIR(1));
            
            // Draw the heart to buffer
            wattron(buffer, COLOR_PAIR(1));
            mvwaddch(buffer, static_cast<int>(round(heart.getY())), 
                    static_cast<int>(round(heart.getX())), ACS_DIAMOND);
            wattroff(buffer, COLOR_PAIR(1));
            
            // Display HP and time remaining on buffer
            drawHealthBar(buffer, 5, maxY - 2, 10, playerHP);
            mvwprintw(buffer, maxY - 2, maxX / 2 - 10, "Time: %d seconds", timeLeft);
            mvwprintw(buffer, maxY - 2, maxX - 25, "Round: %d/%d", currentRound, TOTAL_ROUNDS);
            
            // Copy the buffer to the screen
            pnoutrefresh(buffer, 0, 0, 0, 0, maxY-1, maxX-1);
            
            // Update the screen in one go
            doupdate();
            
            usleep(16667);  // ~60 FPS
        }
        
        // Clean up buffer
        delwin(buffer);
        
        // Disable non-blocking input
        nodelay(stdscr, FALSE);
        
        // Time's up - evaluate player position
        float heartX = heart.getX();
        float heartY = heart.getY();
        
        bool onSafeTile = false;
        
        // Check if the player is on a tile (not on grid lines)
        if (gameBoard.isOnTile(heartX, heartY)) {
            // Check if the tile is a safe tile
            auto boardPos = gameBoard.screenToBoard(heartX, heartY);
            onSafeTile = gameBoard.isSafeTile(boardPos.first, boardPos.second);
        }
        
        // Apply damage if not on a safe tile
        if (!onSafeTile) {
            playerHP -= 5;
        }
        
        // Show the result to the player
        showResult(onSafeTile, playerHP);
        
        // Check if player has lost
        if (playerHP <= 0) {
            break;
        }
        
        // Move to next round
        currentRound++;
    }
    
    // Game over
    gameWon = (playerHP > 0);
    gameOver(gameWon, currentRound - 1);
}