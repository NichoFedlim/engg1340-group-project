#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <random>

// Forward declarations
class Ball;
class Paddle;
class Block;
class GameManager;

// Paddle class (modified from Heart class)
class Paddle {
private:
    float x, y;           // Position with floating-point precision for smooth movement
    int lastDrawnX, lastDrawnY; // Last position where the paddle was drawn
    float directionX;     // Direction vector (only horizontal movement)
    float speed;          // Movement speed
    int width;            // Paddle width
    bool moving;          // Whether the paddle is moving

public:
    Paddle(int startX, int startY, int paddleWidth = 7) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), speed(0.5f), width(paddleWidth), moving(false) {}

    void update() {
        if (moving) {
            // Move in the current direction (horizontal only)
            x += directionX * speed;
        }
    }

    void setDirection(float dx) {
        // Set a new direction vector (horizontal only)
        directionX = dx;
        if (dx != 0.0f) {
            moving = true;  // Start moving when a direction is set
        }
    }
    
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
    
    void stop() {
        moving = false;
    }
    
    void start() {
        moving = true;
    }
    
    bool isMoving() const {
        return moving;
    }

    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    void clearPrevious() {
        // Clear the previous position
        for (int i = 0; i < width; i++) {
            mvaddch(lastDrawnY, lastDrawnX + i, ' ');
        }
    }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position if it's different
            clearPrevious();
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw paddle (a line of characters)
        attron(COLOR_PAIR(1)); // Paddle color
        for (int i = 0; i < width; i++) {
            mvaddch(currentY, currentX + i, '=');
        }
        attroff(COLOR_PAIR(1));
    }

    float getX() const { return x; }
    float getY() const { return y; }
    int getWidth() const { return width; }
    float getDirectionX() const { return directionX; }
    float getSpeed() const { return speed; }
};

// Ball class
class Ball {
private:
    float x, y;           // Position with floating-point precision
    int lastDrawnX, lastDrawnY; // Last position where the ball was drawn
    float directionX, directionY; // Direction vector
    float speed;          // Movement speed
    bool active;          // Whether the ball is active (moving)

public:
    Ball(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.7f), directionY(-0.7f), // Initial direction (up and to the right)
        speed(0.4f), active(true) {}

    void update() {
        if (active) {
            // Move in the current direction
            x += directionX * speed;
            y += directionY * speed;
        }
    }

    void setDirection(float dx, float dy) {
        // Set a new direction vector
        directionX = dx;
        directionY = dy;
        
        // Normalize the direction vector
        float length = sqrt(dx * dx + dy * dy);
        if (length > 0) { // Avoid division by zero
            directionX /= length;
            directionY /= length;
        }
    }
    
    void reverseX() {
        directionX = -directionX;
    }
    
    void reverseY() {
        directionY = -directionY;
    }
    
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
    
    void setActive(bool isActive) {
        active = isActive;
    }
    
    bool isActive() const {
        return active;
    }

    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    void clearPrevious() {
        // Clear the previous position
        mvaddch(lastDrawnY, lastDrawnX, ' ');
    }

    void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position if it's different
            clearPrevious();
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        // Draw ball
        attron(COLOR_PAIR(2)); // Ball color
        mvaddch(currentY, currentX, 'O');
        attroff(COLOR_PAIR(2));
    }

    float getX() const { return x; }
    float getY() const { return y; }
    float getDirectionX() const { return directionX; }
    float getDirectionY() const { return directionY; }
    float getSpeed() const { return speed; }
};

// Block class
class Block {
private:
    int x, y;             // Position
    int width, height;    // Size
    bool active;          // Whether the block is active (not destroyed)
    int colorPair;        // Color pair to use for the block

public:
    Block(int startX, int startY, int w = 4, int h = 1, int color = 3) : 
        x(startX), y(startY), width(w), height(h), active(true), colorPair(color) {}

    void draw() {
        if (!active) return;
        
        attron(COLOR_PAIR(colorPair));
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                mvaddch(y + row, x + col, '#');
            }
        }
        attroff(COLOR_PAIR(colorPair));
    }

    void clear() {
        // Clear the block from the screen
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                mvaddch(y + row, x + col, ' ');
            }
        }
    }

    // Check collision with the ball
    bool collidesWith(const Ball& ball) {
        if (!active) return false;
        
        float ballX = ball.getX();
        float ballY = ball.getY();
        
        // Simple bounding box collision
        return (ballX >= x && ballX < x + width &&
                ballY >= y && ballY < y + height);
    }
    
    void setActive(bool isActive) {
        if (active && !isActive) {
            // If being deactivated, clear from screen
            clear();
        }
        active = isActive;
    }
    
    bool isActive() const {
        return active;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Battle box (play area) - kept mostly the same
class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;  // Flag to determine if the box needs redrawing

public:
    BattleBox(int startX, int startY, int w, int h) :
        x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    void draw() {
        if (!needsRedraw) return;
        
        // Enable reverse highlighting
        attron(A_REVERSE);
    
        // Draw the top and bottom borders of the battle box
        for (int i = -1; i <= width+1; i++) {
            mvaddch(y, x + i, ' ');              // Top border (space with reverse highlight)
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
    
        // Draw the left and right borders of the battle box
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
            mvaddch(y + i, x-1, ' ');            // Left border
            mvaddch(y + i, x+1 + width, ' ');    // Right border
        }
    
        // Disable reverse highlighting
        attroff(A_REVERSE);
        
        needsRedraw = false;
    }

    void setNeedsRedraw() {
        needsRedraw = true;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Game Manager class to handle game state
class GameManager {
    private:
        BattleBox battleBox;
        Paddle paddle;
        Ball ball;
        std::vector<Block> blocks;
        int blockCount;
        bool gameOver;
        bool gameWon;
        int lives;  // Add lives counter
        bool ballLost; // Flag for when a ball is lost but game isn't over
        
    public:
        GameManager(int screenWidth, int screenHeight) : 
            battleBox(screenWidth/2 - 20, screenHeight/2 - 15, 40, 30),
            paddle(screenWidth/2 - 3, screenHeight/2 + 14),
            ball(screenWidth/2, screenHeight/2 + 13),
            blockCount(0),
            gameOver(false),
            gameWon(false),
            lives(3),
            ballLost(false) {
            
            // Initialize blocks (full density by default)
            initializeBlocks(true);
        }
        
        // Modified to accept a density parameter (full or half blocks)
        void initializeBlocks(bool fullDensity = true) {
            // Clear existing blocks
            blocks.clear();
            blockCount = 0;
            
            // Calculate the number of blocks that fit in the battle box
            int blockWidth = 4;
            int blockHeight = 1;
            int padding = 1; // Space between blocks
            
            int boxWidth = battleBox.getWidth() - 2; // Accounting for border
            int boxX = battleBox.getX() + 2; // Starting position (inside border)
            int boxY = battleBox.getY() + 2; // Starting a bit down from the top
            
            int blocksPerRow = (boxWidth + padding) / (blockWidth + padding);
            int maxRows = 5; // Number of rows of blocks
            
            // Create the blocks
            for (int row = 0; row < maxRows; row++) {
                for (int col = 0; col < blocksPerRow; col++) {
                    // If not fullDensity, skip every other block
                    if (!fullDensity && (row * blocksPerRow + col) % 2 == 1) {
                        continue;
                    }
                    
                    int blockX = boxX + col * (blockWidth + padding);
                    int blockY = boxY + row * (blockHeight + padding);
                    
                    // Use different colors for different rows
                    int blockColor = 3 + (row % 5);
                    
                    blocks.push_back(Block(blockX, blockY, blockWidth, blockHeight, blockColor));
                    blockCount++;
                }
            }
        }
        
        // Lives management methods
        void setLives(int newLives) {
            lives = newLives;
        }
        
        int getLives() const {
            return lives;
        }
        
        void decrementLives() {
            if (lives > 0) {
                lives--;
            }
            
            if (lives <= 0) {
                gameOver = true;
            } else {
                ballLost = true;
            }
        }
        
        bool hasLivesRemaining() const {
            return lives > 0;
        }
        
        bool isBallLost() const {
            return ballLost;
        }
        
        // Method to reset ball position after losing a life
        void resetBall() {
            ballLost = false;
            
            // Reset paddle and ball positions
            int maxY, maxX;
            getmaxyx(stdscr, maxY, maxX);
            
            paddle.setPosition(maxX/2 - 3, maxY/2 + 14);
            ball.setPosition(maxX/2, maxY/2 + 13);
            
            // Set random ball direction (mostly upward)
            std::random_device rd;
            std::mt19937 gen(rd());
            
            std::uniform_real_distribution<float> distX(-0.7f, 0.7f);
            float dirX = distX(gen);
            
            std::uniform_real_distribution<float> distY(-1.0f, -0.7f);
            float dirY = distY(gen);
            
            ball.setDirection(dirX, dirY);
        }
        
        // Full reset for new round
        void reset(bool fullDensity = true, int startLives = 3) {
            // Reset game state
            gameOver = false;
            gameWon = false;
            ballLost = false;
            lives = startLives;
            
            // Reset paddle and ball positions
            int maxY, maxX;
            getmaxyx(stdscr, maxY, maxX);
            
            paddle.setPosition(maxX/2 - 3, maxY/2 + 14);
            ball.setPosition(maxX/2, maxY/2 + 13);
            
            // Set random ball direction (mostly upward)
            std::random_device rd;
            std::mt19937 gen(rd());
            
            std::uniform_real_distribution<float> distX(-0.7f, 0.7f);
            float dirX = distX(gen);
            
            std::uniform_real_distribution<float> distY(-1.0f, -0.7f);
            float dirY = distY(gen);
            
            ball.setDirection(dirX, dirY);
            
            // Reinitialize blocks with specified density
            initializeBlocks(fullDensity);
        }
        
        void update() {
            if (gameOver || gameWon || ballLost) return;
            
            // Update paddle position
            paddle.update();
            
            // Constrain paddle position to stay within battle box
            float paddleX = paddle.getX();
            float paddleY = paddle.getY();
            
            if (paddleX < battleBox.getX() + 1) {
                paddle.setPosition(static_cast<float>(battleBox.getX() + 1), paddleY);
            } else if (paddleX + paddle.getWidth() > battleBox.getX() + battleBox.getWidth()) {
                paddle.setPosition(static_cast<float>(battleBox.getX() + battleBox.getWidth() - paddle.getWidth()), paddleY);
            }
            
            // Update ball position
            ball.update();
            
            // Ball collision with walls
            float ballX = ball.getX();
            float ballY = ball.getY();
            
            // Left and right walls
            if (ballX <= battleBox.getX() + 1 || ballX >= battleBox.getX() + battleBox.getWidth() - 1) {
                ball.reverseX();
                
                // Fix the position to prevent the ball from getting stuck in the wall
                if (ballX <= battleBox.getX() + 1) {
                    ball.setPosition(battleBox.getX() + 1.1f, ball.getY());
                } else if (ballX >= battleBox.getX() + battleBox.getWidth() - 1) {
                    ball.setPosition(battleBox.getX() + battleBox.getWidth() - 1.1f, ball.getY());
                }
                
                // Force battle box redraw to fix the dent
                battleBox.setNeedsRedraw();
            }
    
            // Top wall
            if (ballY <= battleBox.getY() + 1) {
                ball.reverseY();
                
                // Fix the position to prevent the ball from getting stuck in the wall
                ball.setPosition(ball.getX(), battleBox.getY() + 1.1f);
                
                // Force battle box redraw to fix the dent
                battleBox.setNeedsRedraw();
            }
            
            // Bottom edge - lose a life
            if (ballY >= battleBox.getY() + battleBox.getHeight() - 1) {
                decrementLives();
                return;
            }
            
            // Ball collision with paddle
            if (ballY >= paddleY - 1 && ballY <= paddleY &&
                ballX >= paddleX && ballX < paddleX + paddle.getWidth()) {
                
                // Ball hit paddle - bounce upward
                ball.reverseY();
                
                // Change ball's horizontal direction based on where it hit the paddle
                float hitPosition = (ballX - paddleX) / paddle.getWidth(); // 0.0 to 1.0
                float newDirX = 2.0f * (hitPosition - 0.5f); // -1.0 to 1.0
                newDirX = std::max(-0.8f, std::min(0.8f, newDirX));
                
                // Add a small influence from the paddle's movement for more realistic physics
                if (paddle.isMoving()) {
                    newDirX += 0.2f * paddle.getDirectionX();
                    // Keep within bounds
                    newDirX = std::max(-0.8f, std::min(0.8f, newDirX));
                }
                
                // Set new direction, keeping the y-direction properly negative (upward)
                float newDirY = -0.7f; // Consistent upward direction
                
                ball.setDirection(newDirX, newDirY);
            }
            
            // Ball collision with blocks
            for (auto& block : blocks) {
                if (block.isActive() && block.collidesWith(ball)) {
                    // Block hit - deactivate it
                    block.setActive(false);
                    blockCount--;
                    
                    // Bounce the ball
                    float ballDirX = ball.getDirectionX();
                    float ballDirY = ball.getDirectionY();
                    
                    // Simple approach: reverse direction based on ball's movement direction
                    if (abs(ballDirX) > abs(ballDirY)) {
                        ball.reverseX(); // Likely hit the side
                    } else {
                        ball.reverseY(); // Likely hit the top/bottom
                    }
                    
                    // Check if all blocks are destroyed (win condition)
                    if (blockCount <= 0) {
                        gameWon = true;
                    }
                    
                    // Only handle one collision per update
                    break;
                }
            }
        }
        
        void draw() {
            battleBox.draw();
            
            // Draw blocks
            for (auto& block : blocks) {
                block.draw();
            }
            
            // Draw paddle and ball
            paddle.draw();
            ball.draw();
            
            // Draw game state messages
            int maxY, maxX;
            getmaxyx(stdscr, maxY, maxX);
            
            // Display lives count as well
            mvprintw(maxY - 35, maxX / 2 -17, "Blocks remaining: %d     Lives: %d", blockCount, lives);
            
            if (gameOver) {
                attron(COLOR_PAIR(1)); // Red for game over
                mvprintw(maxY / 2, maxX / 2 - 5, "GAME OVER");
                mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to restart");
                attroff(COLOR_PAIR(1));
            } else if (gameWon) {
                attron(COLOR_PAIR(3)); // Green for win
                mvprintw(maxY / 2, maxX / 2 - 13, "YOU WIN! ALL BLOCKS CLEARED");
                mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to continue");
                attroff(COLOR_PAIR(3));
            } else if (ballLost) {
                attron(COLOR_PAIR(1)); // Red for ball lost
                mvprintw(maxY / 2, maxX / 2 - 10, "BALL LOST! LIVES LEFT: %d", lives);
                mvprintw(maxY / 2 + 1, maxX / 2 - 16, "Press ENTER to launch another ball");
                attroff(COLOR_PAIR(1));
            }
        }
        
        void handleInput(int key) {
            if (gameOver || gameWon) {
                // Only handle ENTER in these states
                return;
            } else if (ballLost) {
                if (key == '\n') {
                    // Clear the ball lost message before resetting the ball
                    int maxY, maxX;
                    getmaxyx(stdscr, maxY, maxX);
                    
                    // Clear the center area of the screen 
                    for (int i = 0; i < 2; i++) {
                        mvprintw(maxY / 2 + i, maxX / 2 - 19, "                                       ");
                    }
                    resetBall(); // Launch a new ball
                }
                return;
            }
            
            if (key == KEY_LEFT) {
                paddle.setDirection(-1.0f);
                paddle.start();
            } else if (key == KEY_RIGHT) {
                paddle.setDirection(1.0f);
                paddle.start();
            } else if (key == '\n') {
                paddle.stop();
            }
        }
        
        bool isGameOver() const {
            return gameOver;
        }
        
        bool isGameWon() const {
            return gameWon;
        }
    
        bool isPaddleMoving() const {
            return paddle.isMoving();
        }
    
        void paddleStart() {
            paddle.start();
        }
    
        void paddleStop() {
            paddle.stop();
        }
    };
    

// Function to run a single round of the game
int runRound(int round) {
    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create game manager for this round
    GameManager game(maxX, maxY);
    
    // Configure game based on round
    switch (round) {
        case 1:
            // Round 1: Half blocks, 3 tries
            game.reset(false, 3);  // false = half density, 3 lives
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 1: Half blocks, 3 tries");
            break;
        case 2:
            // Round 2: Normal blocks, 3 tries
            game.reset(true, 3);   // true = full density, 3 lives
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 2: Full blocks, 3 tries");
            break;
        case 3:
            // Round 3: Normal blocks, 1 try
            game.reset(true, 1);   // true = full density, 1 life
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 3: Full blocks, 1 try");
            break;
        default:
            // Invalid round
            return -1;
    }
    
    // Add instructions for the user
    mvprintw((maxY / 2) + 1, maxX / 2 - 17, "Press any key to start the round...");
    refresh();
    
    // Temporarily enable blocking input to wait for a key press
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);  // Return to non-blocking input for the game
    // Clear the center area of the screen 
    for (int i = 0; i < 2; i++) {
        mvprintw(maxY / 2 + i, maxX / 2 - 19, "                                       ");
    }
    // Game loop for this round
    bool running = true;
    
    while (running) {
        // Process all available input
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                // Quit the round and return failure
                return -1;
            } else if (ch == ' ') {
                if (game.isPaddleMoving()){
                    game.paddleStop();
                } else {
                    game.paddleStart();
                }
            } else if (ch == '\n' && (game.isGameOver() || game.isGameWon())) {
                // Handle ENTER key for game over or win
                if (game.isGameWon()) {
                    return 1;  // Success - proceed to next round
                } else {
                    return -1; // Failure - retry this round
                }
            } else {
                game.handleInput(ch);
            }
        }
        
        // Update game state
        game.update();
        
        // Draw the game
        game.draw();

        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60 FPS
    }

    return -1;  // Default to failure if we exit the loop unexpectedly
}

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

    // Run all three rounds in sequence
    int result1 = runRound(1);
    if (result1 > 0) {
        clear();
        int result2 = runRound(2);
        if (result2 > 0) {
            clear();
            int result3 = runRound(3);
            if (result3 > 0) {
                // Player completed all rounds
                clear();
                mvprintw(LINES/2, COLS/2 - 11, "Congratulations! All rounds complete!");
                mvprintw(LINES/2 + 1, COLS/2 - 10, "Press any key to exit...");
                refresh();
                nodelay(stdscr, FALSE);
                getch();
            }
        }
    }

    // Clean up
    endwin();
    return 0;
}