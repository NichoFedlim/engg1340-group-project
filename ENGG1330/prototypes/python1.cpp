#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>
#include "python.h"

// Forward declarations
class Laser;
class BattleBox;
class Snake;
class Heart;


// Snake class that follows the heart
class Snake {
private:
    std::deque<std::pair<int, int>> segments; // Snake segments positions
    int length;                               // Length of the snake
    int symbol;                               // Symbol to represent the snake
    int colorPair;                            // Color of the snake
    int updateDelay;                          // Movement delay counter
    int updateDelayThreshold;                 // How many frames to wait before updating
    int collisionCooldown;                    // Cooldown after collision
    bool active;                              // Whether the snake is active in the game

public:
    Snake(int startX, int startY, int len) : 
        length(len), symbol(ACS_BLOCK), colorPair(5), updateDelay(0), 
        updateDelayThreshold(6), collisionCooldown(0), active(false) { // Initially inactive, set active after 5th round
        // Initialize all segments at the same starting position
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
    }

    void update(int heartX, int heartY) {
        if (!active) return;  // Skip update if snake is not active
        
        updateDelay++;
        
        // Update cooldown
        if (collisionCooldown > 0) {
            collisionCooldown--;
        }
        
        // Only update every few frames to create a delay effect
        if (updateDelay >= updateDelayThreshold) { // Increased threshold for slower movement
            updateDelay = 0;
            
            // Move head towards heart
            int headX = segments.front().first;
            int headY = segments.front().second;
            
            // Calculate direction
            int dx = 0, dy = 0;
            
            if (heartX > headX) dx = 1;
            else if (heartX < headX) dx = -1;
            
            if (heartY > headY) dy = 1;
            else if (heartY < headY) dy = -1;
            
            // Slow down diagonal movement by only moving in one direction at a time
            // when moving diagonally (alternating between horizontal and vertical)
            if (dx != 0 && dy != 0) {
                // Choose either horizontal or vertical movement based on frame count
                if (rand() % 2 == 0) {
                    dy = 0; // Only move horizontally this frame
                } else {
                    dx = 0; // Only move vertically this frame
                }
            }
            
            // Add new head position
            segments.push_front(std::make_pair(headX + dx, headY + dy));
            
            // Remove tail if snake is too long
            if (segments.size() > length) {
                segments.pop_back();
            }
        }
    }

    void draw() {
        if (!active) return;  // Skip drawing if snake is not active
        
        for (size_t i = 0; i < segments.size(); i++) {
            attron(COLOR_PAIR(colorPair));
            // Use different character for head
            char segmentChar = (i == 0) ? 'O' : symbol;
            mvaddch(segments[i].second, segments[i].first, segmentChar);
            attroff(COLOR_PAIR(colorPair));
        }
    }

    void clear() {
        if (!active) return;  // Skip clearing if snake is not active
        
        for (auto& segment : segments) {
            mvaddch(segment.second, segment.first, ' ');
        }
    }

    bool checkCollision(const Heart& heart) {
        if (!active || collisionCooldown > 0) return false;
        
        int heartX = heart.getIntX();
        int heartY = heart.getIntY();
        
        // Check if heart collides with any snake segment
        for (auto& segment : segments) {
            if (segment.first == heartX && segment.second == heartY) {
                collisionCooldown = 60; // 1 second cooldown
                return true;
            }
        }
        
        return false;
    }

    void reset(int startX, int startY) {
        clear(); // Clear the snake from the screen
        segments.clear();
        
        // Initialize all segments at the same starting position
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
        
        updateDelay = 0;
        collisionCooldown = 0;
    }
    
    // Set whether the snake is active
    void setActive(bool isActive) {
        active = isActive;
    }
    
    // Check if snake is active
    bool isActive() const {
        return active;
    }
};
// Laser class for the attack patterns
        // Direction enum for laser orientation
enum class LaserDirection {
    HORIZONTAL,
    VERTICAL,
    DIAGONAL_DOWN, // Top-left to bottom-right
    DIAGONAL_UP    // Bottom-left to top-right
};
class Laser {
private:
    int startX, startY;       // Starting point of the laser
    int endX, endY;           // End point of the laser
    int warningTimer;         // Timer for warning animation before the laser fires
    int activeTimer;          // Timer for how long the laser is active
    int colorPair;            // Color of the laser
    bool active;              // Whether the laser is currently active
    bool warning;             // Whether the laser is in warning phase
    LaserDirection direction; // Direction of the laser
    char symbol;              // Symbol to draw the laser with
    
    // Variables for projectile behavior
    float projectilePos;      // Position of the projectile (0.0 to 1.0)
    float projectileSpeed;    // Speed of the projectile
    std::vector<std::pair<int, int>> trail; // Trail left behind by projectile
    std::vector<int> trailTimers; // Timers for each trail segment
    int maxTrailLength;       // Maximum length of the trail

public:
    Laser(int sX, int sY, int eX, int eY, LaserDirection dir) :
        startX(sX), startY(sY), endX(eX), endY(eY),
        warningTimer(30), activeTimer(150), // ~0.5s warning, ~2.5s active
        colorPair(3), active(false), warning(false),
        direction(dir), symbol('*'),
        projectilePos(0.0f), projectileSpeed(0.02f), // Move 2% of the path per frame
        maxTrailLength(60) {} // ~1 second at 60 FPS

        void activate() {
            warning = true;
            warningTimer = 30; // ~0.5s warning at 60 FPS
            trail.clear();
            trailTimers.clear(); // Clear trail timers too
            projectilePos = 0.0f;
        }

        void update() {
            if (warning) {
                warningTimer--;
                if (warningTimer <= 0) {
                    warning = false;
                    active = true;
                    activeTimer = 150; // ~2.5s active at 60 FPS
                }
            } else if (active) {
                activeTimer--;
                
                // Update projectile position
                projectilePos += projectileSpeed;
                
                // Add current projectile position to trail
                if (projectilePos <= 1.0f) {
                    int x, y;
                    getPointOnPath(projectilePos, x, y);
                    
                    // Only add if it's a new position
                    if (trail.empty() || trail.back().first != x || trail.back().second != y) {
                        trail.push_back(std::make_pair(x, y));
                        trailTimers.push_back(60); // Each trail segment lasts 1 second (60 frames)
                    }
                }
                
                // Update timers for each trail segment and remove expired ones
                for (int i = 0; i < trailTimers.size(); i++) {
                    trailTimers[i]--;
                    
                    // Remove expired trail segments
                    if (trailTimers[i] <= 0) {
                        // Clear this segment from the screen
                        mvaddch(trail[i].second, trail[i].first, ' ');
                        
                        // Remove from vectors
                        trail.erase(trail.begin() + i);
                        trailTimers.erase(trailTimers.begin() + i);
                        i--; // Adjust index since we removed an element
                    }
                }
                
                if (activeTimer <= 0) {
                    active = false;
                    trail.clear();
                    trailTimers.clear();
                }
            }
        }
    

    void clear() {
        // Clear the laser from the screen
        if (!trail.empty()) {
            for (const auto& point : trail) {
                mvaddch(point.second, point.first, ' ');
            }
        }
        
        // Also clear the warning if it exists
        if (warning) {
            drawWarning(' ', 0);
        }
    }

    void draw() {
        if (warning) {
            // Draw warning path
            drawWarning('~', 4); // Use ~ and warning color (dim yellow)
        } else if (active) {
            // Draw the trail
            for (size_t i = 0; i < trail.size(); i++) {
                // Gradient effect - newer trail segments are brighter
                int colorIntensity = 3; // Default to bright cyan
                if (i < trail.size() / 2) {
                    colorIntensity = 6; // Dimmer color for older trail segments
                }
                
                attron(COLOR_PAIR(colorIntensity));
                mvaddch(trail[i].second, trail[i].first, '*');
                attroff(COLOR_PAIR(colorIntensity));
            }
            
            // Draw the projectile head
            if (projectilePos <= 1.0f) {
                int x, y;
                getPointOnPath(projectilePos, x, y);
                
                attron(COLOR_PAIR(7)); // Bright white for projectile head
                mvaddch(y, x, 'O');
                attroff(COLOR_PAIR(7));
            }
        }
    }

    bool checkCollision(const Heart& heart) {
        if (!active) return false;

        int heartX = heart.getIntX();
        int heartY = heart.getIntY();

        // Check if heart collides with any point in the trail
        for (const auto& point : trail) {
            if (point.first == heartX && point.second == heartY) {
                return true;
            }
        }
        
        // Also check collision with projectile head
        if (projectilePos <= 1.0f) {
            int x, y;
            getPointOnPath(projectilePos, x, y);
            
            if (x == heartX && y == heartY) {
                return true;
            }
        }
        
        return false;
    }

    bool isActive() const { return active; }
    bool isWarning() const { return warning; }
    bool isComplete() const { return !active && !warning; }

private:
    void getPointOnPath(float t, int& x, int& y) {
        // Calculate point at position t (0.0 to 1.0) along the path
        x = static_cast<int>(round(startX + t * (endX - startX)));
        y = static_cast<int>(round(startY + t * (endY - startY)));
    }

    void drawWarning(char drawSymbol, int drawColor) {
        attron(COLOR_PAIR(drawColor));
        
        switch (direction) {
            case LaserDirection::HORIZONTAL:
                for (int x = std::min(startX, endX); x <= std::max(startX, endX); x++) {
                    mvaddch(startY, x, drawSymbol);
                }
                break;
                
            case LaserDirection::VERTICAL:
                for (int y = std::min(startY, endY); y <= std::max(startY, endY); y++) {
                    mvaddch(y, startX, drawSymbol);
                }
                break;
                
            case LaserDirection::DIAGONAL_DOWN:
            case LaserDirection::DIAGONAL_UP:
                drawDiagonalLine(startX, startY, endX, endY, drawSymbol);
                break;
        }
        
        attroff(COLOR_PAIR(drawColor));
    }

    void drawDiagonalLine(int x1, int y1, int x2, int y2, char drawSymbol) {
        // Bresenham's line algorithm for drawing diagonal lines
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            mvaddch(y1, x1, drawSymbol);
            
            if (x1 == x2 && y1 == y2) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }
};

// Function to draw health bar
void drawHealthBar(int x, int y, int maxHP, int currentHP) {
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
}

// Function to create a tic-tac-toe grid of lasers
std::vector<Laser> createTicTacToeAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int boxWidth = box.getWidth();
    int boxHeight = box.getHeight();
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    
    // Calculate positions for the grid lines
    int thirdWidth = boxWidth / 3;
    int thirdHeight = boxHeight / 3;
    
    // Horizontal lines - make them wide to make dodging harder
    int line1Y = innerMinY + thirdHeight;
    int line2Y = innerMinY + 2 * thirdHeight;
    
    // Create horizontal lasers (----)
    lasers.emplace_back(
        innerMinX, line1Y,
        innerMinX + boxWidth - 1, line1Y,
        LaserDirection::HORIZONTAL
    );
    
    lasers.emplace_back(
        innerMinX, line2Y,
        innerMinX + boxWidth - 1, line2Y,
        LaserDirection::HORIZONTAL
    );
    
    // Create vertical lasers (|)
    int line1X = innerMinX + thirdWidth;
    int line2X = innerMinX + 2 * thirdWidth;
    
    lasers.emplace_back(
        line1X, innerMinY,
        line1X, innerMinY + boxHeight - 1,
        LaserDirection::VERTICAL
    );
    
    lasers.emplace_back(
        line2X, innerMinY,
        line2X, innerMinY + boxHeight - 1,
        LaserDirection::VERTICAL
    );
    
    return lasers;
}

// Function to create an X cross of lasers
std::vector<Laser> createXCrossAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    // Create diagonal lasers (\)
    lasers.emplace_back(
        innerMinX, innerMinY,
        innerMaxX, innerMaxY,
        LaserDirection::DIAGONAL_DOWN
    );
    
    // Create diagonal lasers (/)
    lasers.emplace_back(
        innerMaxX, innerMinY,
        innerMinX, innerMaxY,
        LaserDirection::DIAGONAL_UP
    );
    
    return lasers;
}

// Function to create a laser split attack (two horizontal then one that splits)
std::vector<Laser> createSplitLaserAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    int centerX = box.getCenterX();
    int centerY = box.getCenterY();
    
    // Left to center laser
    lasers.emplace_back(
        innerMinX, centerY,
        centerX - 5, centerY, // Stop short of center
        LaserDirection::HORIZONTAL
    );
    
    // Right to center laser
    lasers.emplace_back(
        innerMaxX, centerY,
        centerX + 5, centerY, // Stop short of center
        LaserDirection::HORIZONTAL
    );
    
    // Center vertical laser (splits up and down)
    lasers.emplace_back(
        centerX, innerMinY,
        centerX, innerMaxY,
        LaserDirection::VERTICAL
    );
    
    return lasers;
}

// Function to create a spiral attack pattern
std::vector<Laser> createSpiralAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int centerX = box.getCenterX();
    int centerY = box.getCenterY();
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    // Create spiral by making 4 lasers from center to each corner
    lasers.emplace_back(
        centerX, centerY,
        innerMinX, innerMinY, // Top-left
        LaserDirection::DIAGONAL_DOWN
    );
    
    lasers.emplace_back(
        centerX, centerY,
        innerMaxX, innerMinY, // Top-right
        LaserDirection::DIAGONAL_UP
    );
    
    lasers.emplace_back(
        centerX, centerY,
        innerMaxX, innerMaxY, // Bottom-right
        LaserDirection::DIAGONAL_DOWN
    );
    
    lasers.emplace_back(
        centerX, centerY,
        innerMinX, innerMaxY, // Bottom-left
        LaserDirection::DIAGONAL_UP
    );
    
    return lasers;
}

// Function to create a square that closes in
std::vector<Laser> createClosingSquareAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    int centerX = box.getCenterX();
    int centerY = box.getCenterY();
    
    // Top horizontal laser
    lasers.emplace_back(
        innerMinX, innerMinY,
        innerMaxX, innerMinY,
        LaserDirection::HORIZONTAL
    );
    
    // Right vertical laser
    lasers.emplace_back(
        innerMaxX, innerMinY,
        innerMaxX, innerMaxY,
        LaserDirection::VERTICAL
    );
    
    // Bottom horizontal laser
    lasers.emplace_back(
        innerMaxX, innerMaxY,
        innerMinX, innerMaxY,
        LaserDirection::HORIZONTAL
    );
    
    // Left vertical laser
    lasers.emplace_back(
        innerMinX, innerMaxY,
        innerMinX, innerMinY,
        LaserDirection::VERTICAL
    );
    
    return lasers;
}

// Function to create a cross attack (+ shape)
std::vector<Laser> createCrossAttack(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    int centerX = box.getCenterX();
    int centerY = box.getCenterY();
    
    // Horizontal laser
    lasers.emplace_back(
        innerMinX, centerY,
        innerMaxX, centerY,
        LaserDirection::HORIZONTAL
    );
    
    // Vertical laser
    lasers.emplace_back(
        centerX, innerMinY,
        centerX, innerMaxY,
        LaserDirection::VERTICAL
    );
    
    return lasers;
}

// Function to wait for Enter key
void waitForEnter(const char* message, int x, int y) {
    // Clear the line first
    move(y, x);
    clrtoeol();
    
    // Display message
    mvprintw(y, x, "%s", message);
    refresh();
    
    // Temporarily make getch blocking
    nodelay(stdscr, FALSE);
    
    // Wait for Enter key
    int ch;
    do {
        ch = getch();
    } while (ch != '\n' && ch != KEY_ENTER);
    
    // Reset to non-blocking
    nodelay(stdscr, TRUE);
    
    // Clear the message
    move(y, x);
    clrtoeol();
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);  // Non-blocking input
    
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Enable function keys, arrow keys, etc.
    keypad(stdscr, TRUE);
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);      // Red heart (normal)
        init_pair(2, COLOR_GREEN, COLOR_BLACK);    // Green heart (invincible)
        init_pair(3, COLOR_CYAN, COLOR_BLACK);     // Bright cyan laser (active)
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);   // Yellow laser (warning)
        init_pair(5, COLOR_GREEN, COLOR_BLACK);    // Green snake
        init_pair(6, COLOR_BLUE, COLOR_BLACK);     // Dimmer blue for older trail
        init_pair(7, COLOR_WHITE, COLOR_BLACK);    // White for projectile head
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create battle box and heart
    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    Heart heart(maxX/2, maxY/2);
    
    // Create the snake that follows the heart (initially inactive)
    Snake snake(maxX/2 - 10, maxY/2, 5); // 5 segments length
    
    // Game state variables
    bool running = true;
    bool inAttackCycle = false;
    std::vector<Laser> lasers;
    int messageY = maxY - 6;
    int attackRound = 0; // Track the current round (1-7)
    int phaseTimer = 0;
    int currentPhase = 0;
    int maxPhases = 0;
    bool gameCompleted = false; // Track if all 7 rounds are completed
    
    // Draw the static elements once
    battleBox.draw();
    mvprintw(maxY - 3, 2, "Arrow keys to set direction, Space to stop/start");
    mvprintw(maxY - 2, 2, "Q to quit");
    
    waitForEnter("Welcome to the 7-Round Laser Battle! Press Enter to start the game.", 2, messageY);
    
    // Make sure the heart starts moving
    heart.setDirection(1.0f, 0.0f);
    heart.start();
    
    // Game loop
    while (running) {
        // Check if player died
        if (heart.isDead()) {
            waitForEnter("Game Over! Press Enter to restart.", 2, messageY);
            heart.reset(maxX/2, maxY/2);
            snake.reset(maxX/2 - 10, maxY/2);
            snake.setActive(false); // Deactivate snake on restart
            for (auto& laser : lasers) {
                laser.clear();
            }
            lasers.clear();
            inAttackCycle = false;
            attackRound = 0;
            gameCompleted = false;
            
            // Make sure the heart starts moving
            heart.setDirection(1.0f, 0.0f);
            heart.start();
        }
        
        // Check if all rounds are completed
        if (gameCompleted) {
            waitForEnter("Congratulations! You've completed all 7 rounds! Press Enter to play again.", 2, messageY);
            heart.reset(maxX/2, maxY/2);
            snake.reset(maxX/2 - 10, maxY/2);
            snake.setActive(false); // Deactivate snake on restart
            for (auto& laser : lasers) {
                laser.clear();
            }
            lasers.clear();
            inAttackCycle = false;
            attackRound = 0;
            gameCompleted = false;
            
            // Make sure the heart starts moving
            heart.setDirection(1.0f, 0.0f);
            heart.start();
        }
        
        // Start new attack cycle if not in one and not all rounds completed
        if (!inAttackCycle && !gameCompleted) {
            attackRound++; // Increment round counter
            
            // Check if we've reached the final round
            if (attackRound > 7) {
                gameCompleted = true;
                continue;
            }
            
            // Activate snake after 5th round
            if (attackRound == 6) {
                snake.setActive(true);
                waitForEnter("Warning: A snake has appeared! It will follow you now. Press Enter to continue.", 2, messageY);
            }
            
            // Message based on round number
            char roundMessage[100];
            sprintf(roundMessage, "Round %d of 7: Press Enter to start the attack!", attackRound);
            waitForEnter(roundMessage, 2, messageY);
            
            inAttackCycle = true;
            currentPhase = 0;
            phaseTimer = 0;
            
            // Different attack patterns for each round
            switch (attackRound) {
                case 1:
                    // Round 1: Tic-Tac-Toe pattern
                    lasers = createTicTacToeAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    phaseTimer = 15; // Wait 1/4 second before activating next
                    break;
                    
                case 2:
                    // Round 2: X Cross pattern
                    lasers = createXCrossAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    phaseTimer = 30; // Wait 1/2 second before activating next
                    break;
                    
                case 3:
                    // Round 3: Split Laser pattern
                    lasers = createSplitLaserAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    lasers[1].activate();
                    break;
                    
                case 4:
                    // Round 4: Spiral pattern
                    lasers = createSpiralAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    phaseTimer = 15;
                    break;
                    
                case 5:
                    // Round 5: Closing Square pattern
                    lasers = createClosingSquareAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    phaseTimer = 15;
                    break;
                    
                case 6:
                    // Round 6: Cross pattern + Snake
                    lasers = createCrossAttack(battleBox);
                    maxPhases = 1;
                    lasers[0].activate();
                    lasers[1].activate();
                    break;
                    
                case 7:
                    // Round 7: All attacks combined in phases
                    // Start with Tic-Tac-Toe
                    lasers = createTicTacToeAttack(battleBox);
                    maxPhases = 3; // 3 phases in final round
                    lasers[0].activate();
                    phaseTimer = 15;
                    break;
            }
            
            heart.start(); // Make sure the heart is moving
        }
        
        // Process all available input
        int ch;
        
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            } else if (ch == ' ') {
                // Toggle movement state
                if (heart.isMoving()) {
                    heart.stop(); // Fully stop and don't auto-restart
                } else {
                    heart.start(); // Start moving again
                }
            } else if (ch == KEY_UP) {
                heart.setDirection(0.0f, -1.0f);  // Up
                heart.start(); // Ensure movement is enabled
            } else if (ch == KEY_DOWN) {
                heart.setDirection(0.0f, 1.0f);   // Down
                heart.start(); // Ensure movement is enabled
            } else if (ch == KEY_LEFT) {
                heart.setDirection(-1.0f, 0.0f);  // Left
                heart.start(); // Ensure movement is enabled
            } else if (ch == KEY_RIGHT) {
                heart.setDirection(1.0f, 0.0f);   // Right
                heart.start(); // Ensure movement is enabled
            }
        }
        
        // Only auto-start if heart can be force-started (not explicitly stopped by spacebar)
        if (!heart.isMoving() && heart.canForceStart()) {
            // Get current direction or set a random one if none
            float dirX = heart.getDirectionX();
            float dirY = heart.getDirectionY();
            
            if (dirX == 0.0f && dirY == 0.0f) {
                // Random direction
                heart.setDirection(1.0f, 0.0f);
            } else {
                // Use current direction
                heart.setDirection(dirX, dirY);
            }
            heart.start();
        }
        
        // Update heart position
        heart.update();
        
        // Update snake if it's active
        if (snake.isActive()) {
            snake.clear(); // Clear old snake segments
            snake.update(heart.getIntX(), heart.getIntY()); // Follow the heart
            
            // Check if snake collides with heart
            if (snake.checkCollision(heart)) {
                heart.takeDamage();
            }
        }
        
        // Boundary checking to keep heart inside the battle box
        float heartX = heart.getX();
        float heartY = heart.getY();
        
        // Constrain position
        if (heartX < battleBox.getInnerMinX()) {
            heart.setPosition(static_cast<float>(battleBox.getInnerMinX()), heartY);
        }
        if (heartX > battleBox.getInnerMaxX()) {
            heart.setPosition(static_cast<float>(battleBox.getInnerMaxX()), heartY);
        }
        if (heartY < battleBox.getInnerMinY()) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getInnerMinY()));
        }
        if (heartY > battleBox.getInnerMaxY()) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getInnerMaxY()));
        }
        
        // Update attack phases for each round
        if (inAttackCycle) {
            // Update phase timer and lasers
            phaseTimer++;
            
            // Handle different attack patterns based on round
            switch (attackRound) {
                case 1: // Tic-Tac-Toe attack
                    // Activate lasers sequentially
                    if (phaseTimer == 15) lasers[1].activate();
                    if (phaseTimer == 30) lasers[2].activate();
                    if (phaseTimer == 45) lasers[3].activate();
                    break;
                    
                case 2: // X Cross attack
                    // Activate second laser after delay
                    if (phaseTimer == 30) lasers[1].activate();
                    break;
                    
                case 3: // Split Laser attack
                    // Activate the vertical center laser after delay
                    if (phaseTimer == 60) lasers[2].activate();
                    break;
                    
                case 4: // Spiral attack
                    // Activate lasers in sequence
                    if (phaseTimer == 15) lasers[1].activate();
                    if (phaseTimer == 30) lasers[2].activate();
                    if (phaseTimer == 45) lasers[3].activate();
                    break;
                    
                case 5: // Closing Square attack
                    // Activate lasers in sequence
                    if (phaseTimer == 15) lasers[1].activate();
                    if (phaseTimer == 30) lasers[2].activate();
                    if (phaseTimer == 45) lasers[3].activate();
                    break;
                    
                case 6: // Cross attack + Snake (already activated)
                    // No additional laser activation needed
                    break;
                    
                case 7: // Final round with multiple phases
                    if (currentPhase == 0) { // Tic-Tac-Toe phase
                        // Activate lasers sequentially
                        if (phaseTimer == 15) lasers[1].activate();
                        if (phaseTimer == 30) lasers[2].activate();
                        if (phaseTimer == 45) lasers[3].activate();
                        
                        // Check if phase complete to move to next pattern
                        bool phaseComplete = true;
                        for (auto& laser : lasers) {
                            phaseComplete = phaseComplete && laser.isComplete();
                        }
                        
                        if (phaseComplete && phaseTimer > 150) {
                            currentPhase = 1;
                            phaseTimer = 0;
                            
                            // Clear old lasers
                            for (auto& laser : lasers) {
                                laser.clear();
                            }
                            
                            // Create X Cross attack
                            lasers = createXCrossAttack(battleBox);
                            lasers[0].activate();
                        }
                    }
                    else if (currentPhase == 1) { // X Cross phase
                        // Activate second laser after delay
                        if (phaseTimer == 30) lasers[1].activate();
                        
                        // Check if phase complete
                        bool phaseComplete = true;
                        for (auto& laser : lasers) {
                            phaseComplete = phaseComplete && laser.isComplete();
                        }
                        
                        if (phaseComplete && phaseTimer > 150) {
                            currentPhase = 2;
                            phaseTimer = 0;
                            
                            // Clear old lasers
                            for (auto& laser : lasers) {
                                laser.clear();
                            }
                            
                            // Create Split Laser attack for final phase
                            lasers = createSplitLaserAttack(battleBox);
                            lasers[0].activate();
                            lasers[1].activate();
                        }
                    }
                    else if (currentPhase == 2) { // Split Laser phase
                        // Activate vertical laser after delay
                        if (phaseTimer == 60) lasers[2].activate();
                    }
                    break;
            }
            
            // Check if all lasers are complete for current round
            bool roundComplete = true;
            for (auto& laser : lasers) {
                roundComplete = roundComplete && laser.isComplete();
            }
            
            // End round if all lasers are complete and enough time has passed
            if (roundComplete && phaseTimer > 120) {
                // For round 7 with multiple phases, only end after last phase
                if (attackRound != 7 || currentPhase == maxPhases - 1) {
                    inAttackCycle = false;
                    
                    // Display round completion message
                    char completionMessage[100];
                    sprintf(completionMessage, "Round %d completed! Press Enter to continue.", attackRound);
                    waitForEnter(completionMessage, 2, messageY);
                    
                    // Clear lasers for next round
                    for (auto& laser : lasers) {
                        laser.clear();
                    }
                    lasers.clear();
                }
            }
        }
        
        // Update and check lasers
        for (auto& laser : lasers) {
            laser.update();
            
            // Check if laser collides with heart
            if (laser.checkCollision(heart)) {
                heart.takeDamage();
            }
            
            // Draw the laser
            laser.draw();
        }
        
        // Always redraw the battle box to ensure borders stay visible
        battleBox.draw();
        
        // Draw the snake and heart
        if (snake.isActive()) {
            snake.draw();
        }
        heart.draw();
        
        // Draw health bar
        drawHealthBar(2, maxY - 4, 10, heart.getHP());
        
        // Display current round information
        mvprintw(2, 2, "Round: %d/7", attackRound);

        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60 FPS (1,000,000 microseconds / 60)
    }

    // Clean up
    endwin();
    return 0;
}