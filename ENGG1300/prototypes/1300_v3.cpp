#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <chrono>
#include <thread>

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
    Paddle(int startX, int startY, int paddleWidth = 15) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), speed(0.7f), width(paddleWidth), moving(false) {}

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
    // Ball(int startX, int startY) : 
    //     x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
    //     // lastDrawnX(startX), lastDrawnY(startY),

    //     directionX(0.7f), directionY(-0.7f), // Initial direction (up and to the right)
    //     speed(0.5f), active(true) {}

    Ball(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(-1), lastDrawnY(-1), // Initialize to invalid position
        directionX(0.707f), directionY(-0.707f), // 45° normalized vector
        speed(0.55f), active(true) {}

    void update(float deltaTime) {
        if (!active) return;
        
        // Store previous position
        // int prevX = static_cast<int>(round(x));
        // int prevY = static_cast<int>(round(y));
        int prevX = lastDrawnX;
        int prevY = lastDrawnY;
        // Update position
        // x += directionX * speed;
        // y += directionY * speed;
        x += directionX * speed * deltaTime * 60.0f; // 60.0f for consistent speed
        y += directionY * speed * deltaTime * 60.0f;
        
        // Clear previous position if it changed
        // if (prevX != lastDrawnX || prevY != lastDrawnY) {
        //     mvaddch(prevY, prevX, ' ');
        // }
        if (prevX != -1 && prevY != -1) {
            mvaddch(prevY, prevX, ' ');
        }
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
        if (!active) return;
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position if it's different
            clearPrevious();
            attron(COLOR_PAIR(2)); // Ball color
            mvaddch(currentY, currentX, 'O');
            attroff(COLOR_PAIR(2));
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        
    }
    void bounceHorizontal() { 
        directionX = -directionX;
        //changes
        x += (directionX > 0) ? 0.1f : -0.1f;}
    void bounceVertical() { 
        directionY = -directionY; 
        y += (directionY > 0) ? 0.1f : -0.1f;}

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
            mvaddch(y, x + i, '_');              // Top border (space with reverse highlight)
            mvaddch(y + height, x + i, '_');     // Bottom border
        }
    
        // Draw the left and right borders of the battle box
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, '|');              // Left border
            mvaddch(y + i, x + width, '|');      // Right border
            mvaddch(y + i, x-1, '|');            // Left border
            mvaddch(y + i, x+1 + width, '|');    // Right border
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
    
public:
    GameManager(int screenWidth, int screenHeight) : 
        battleBox(screenWidth/2 - 20, screenHeight/2 - 15, 40, 30),
        paddle(screenWidth/2 - 3, screenHeight/2 + 10),
        ball(screenWidth/2, screenHeight/2 + 9),
        blockCount(0),
        gameOver(false),
        gameWon(false) {
        
        // Initialize blocks
        initializeBlocks();
    }
    
    void initializeBlocks() {
        // Clear existing blocks
        blocks.clear();
        blockCount = 0;
        
        // Calculate the number of blocks that fit in the battle box
        int blockWidth = 4;
        int blockHeight = 1;
        int padding = 1; // Space between blocks
        
        int boxWidth = battleBox.getWidth() - 2; // Accounting for border
        int boxX = battleBox.getX() + 1; // Starting position (inside border)
        int boxY = battleBox.getY() + 2; // Starting a bit down from the top
        
        int blocksPerRow = (boxWidth + padding) / (blockWidth + padding);
        int maxRows = 5; // Number of rows of blocks
        
        // Create the blocks
        for (int row = 0; row < maxRows; row++) {
            for (int col = 0; col < blocksPerRow; col++) {
                int blockX = boxX + col * (blockWidth + padding);
                int blockY = boxY + row * (blockHeight + padding);
                
                // Use different colors for different rows
                int blockColor = 3 + (row % 5);
                
                blocks.push_back(Block(blockX, blockY, blockWidth, blockHeight, blockColor));
                blockCount++;
            }
        }
    }
    
    void update(float deltaTime) {
        if (gameOver || gameWon) return;
        
        // Update paddle position
        paddle.update();
        
        // Constrain paddle position to stay within battle box
        float paddleX = paddle.getX();
        float paddleY = paddle.getY();
        
        if (paddleX < battleBox.getX() + 1) {
            paddle.setPosition(static_cast<float>(battleBox.getX()), paddleY);
        } else if (paddleX + paddle.getWidth() > battleBox.getX() + battleBox.getWidth()) {
            paddle.setPosition(static_cast<float>(battleBox.getX() + battleBox.getWidth() - paddle.getWidth()), paddleY);
        }
        
        // // Update ball position
        // ball.update();
        ball.update(deltaTime);
        
        // Ball collision with walls
        float ballX = ball.getX();
        float ballY = ball.getY();
        
        // Left and right walls
        // if (ballX <= battleBox.getX() + 1 || ballX >= battleBox.getX() + battleBox.getWidth() - 1) {
        //     ball.reverseX();
        // }
        
        if (ballX <= battleBox.getX() + 1) {
            ball.bounceHorizontal();
            ball.setPosition(battleBox.getX() + 0.1f, ball.getY()); // Prevent sticking
        }
        else if (ballX >= battleBox.getX() + battleBox.getWidth() - 1) {
            ball.bounceHorizontal();
            ball.setPosition(battleBox.getX() + battleBox.getWidth() - 0.1f, ball.getY());
        }
        // Top wall
        // if (ballY <= battleBox.getY() + 1) {
        //     ball.reverseY();
        // }
        if (ballY <= battleBox.getY() + 1) {
            ball.bounceVertical();
            ball.setPosition(ball.getX(), battleBox.getY() + 0.1f);
        }
        
        // Bottom edge - game over
        if (ballY >= battleBox.getY() + battleBox.getHeight() - 1) {
            gameOver = true;
            return;
        }
        
        // Ball collision with paddle
        // if (ballY >= paddleY - 1 && ballY <= paddleY &&
        //     ballX >= paddleX && ballX < paddleX + paddle.getWidth()) {
        //     if (ballY + 1 >= paddleY && ballY <= paddleY + 1 &&
        //         ballX >= paddleX && ballX <= paddleX + paddle.getWidth()) {
            
        //     // Ball hit paddle - bounce upward
        //     ball.reverseY();
            
        //     // Change ball's horizontal direction based on where it hit the paddle
        //     // This gives more control to the player
        //     float hitPosition = (ballX - paddleX) / paddle.getWidth(); // 0.0 to 1.0
        //     float newDirX = 2.0f * (hitPosition - 0.5f); // -1.0 to 1.0
            
        //     // Set new direction, keeping the y-direction the same but reversing it
        //     // float dirY = -abs(ball.getDirectionY()); // Ensure ball goes upward
        //     ball.setDirection(newDirX, -0.5f);
        //     // bates perubahan
        // }
        if (ballY + 0.5f >= paddle.getY() && 
        ballY - 0.5f <= paddle.getY() + 1 &&
        ballX >= paddle.getX() && 
        ballX <= paddle.getX() + paddle.getWidth()) {
        
        // Calculate bounce angle based on hit position
        float hitPos = (ballX - paddle.getX()) / paddle.getWidth();
        float angle = (hitPos * 120.0f) - 60.0f; // -60° to +60° range
        angle *= 3.14159f / 180.0f; // Convert to radians
        
        ball.setDirection(cos(angle), -sin(angle));
    }
        
        // Ball collision with blocks
        for (auto& block : blocks) {
            if (block.isActive() && block.collidesWith(ball)) {
                // Block hit - deactivate it
                block.setActive(false);
                blockCount--;
                
                // Bounce the ball
                // Determine if the ball hit the side or top/bottom of the block
                // float ballDirX = ball.getDirectionX();
                // float ballDirY = ball.getDirectionY();
                
                // // Simple approach: reverse direction based on ball's movement direction
                // if (abs(ballDirX) > abs(ballDirY)) {
                //     ball.reverseX(); // Likely hit the side
                // } else {
                //     ball.reverseY(); // Likely hit the top/bottom
                // }
                
            float ballCenterX = ball.getX() + 0.5f;
            float ballCenterY = ball.getY() + 0.5f;
            float blockCenterX = block.getX() + block.getWidth() / 2.0f;
            float blockCenterY = block.getY() + block.getHeight() / 2.0f;

            float dx = ballCenterX - blockCenterX;
            float dy = ballCenterY - blockCenterY;

            if (abs(dx) > abs(dy)) {
                ball.reverseX();
            } else {
                ball.reverseY();
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
        
        mvprintw(maxY - 3, 2, "Left/Right arrows to move paddle    Blocks remaining: %d", blockCount);
        mvprintw(maxY - 2, 2, "Space to stop/restart    Q to quit");
        
        if (gameOver) {
            attron(COLOR_PAIR(1)); // Red for game over
            mvprintw(maxY / 2, maxX / 2 - 5, "GAME OVER");
            mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press SPACE to restart");
            attroff(COLOR_PAIR(1));
        } else if (gameWon) {
            attron(COLOR_PAIR(3)); // Green for win
            mvprintw(maxY / 2, maxX / 2 - 9, "YOU WIN! ALL BLOCKS CLEARED");
            mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press SPACE to restart");
            attroff(COLOR_PAIR(3));
        }
    }
    
    void handleInput(int key) {
        if (gameOver || gameWon) {
            // Allow restart with space when game is over
            if (key == ' ') {
                reset();
            }
            return;
        }
        
        if (key == KEY_LEFT) {
            paddle.setDirection(-1.0f);
            paddle.start();
        } else if (key == KEY_RIGHT) {
            paddle.setDirection(1.0f);
            paddle.start();
        } else if (key == ' ') {
            paddle.stop();
        }
    }
    
    void reset() {
        // Reset game state
        gameOver = false;
        gameWon = false;
        
        // Reset paddle and ball positions
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        
        paddle.setPosition(maxX/2 - 3, maxY/2 + 10);
        ball.setPosition(maxX/2, maxY/2 + 9);
        
        // Reset ball direction
        ball.setDirection(0.7f, -0.7f);
        
        // Reinitialize blocks
        initializeBlocks();
    }
    
    bool isGameOver() const {
        return gameOver;
    }
    
    bool isGameWon() const {
        return gameWon;
    }
};

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

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create game manager
    GameManager game(maxX, maxY);
    
    // Game loop
    bool running = true;

    //changes
    auto lastTime = std::chrono::high_resolution_clock::now();
    while (running) {
        // Calculate deltaTime
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        // Process all available input
        int ch;
        
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            } else {
                game.handleInput(ch);
            }
        }
        
        // Update game state
        // game.update();
        
        // // Draw the game
        // // game.draw();
        // clear();
        // game.draw();

        // // Refresh screen and control frame rate
        // refresh();
        // usleep(16667);  // ~60 FPS (1,000,000 microseconds / 60)

        // Clear entire screen
        erase();
        
        // Update and draw
        game.update(deltaTime);
        game.draw();
        
        // Refresh at consistent rate
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Clean up
    endwin();
    return 0;
}