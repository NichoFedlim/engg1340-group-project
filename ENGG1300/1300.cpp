#include "1300.h"
#include <cmath>
#include <cstring>
#include <unistd.h>

// Paddle movement implementation 
Paddle::Paddle(int startX, int startY, int paddleWidth) : 
    x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
    lastDrawnX(startX), lastDrawnY(startY),
    directionX(0.0f), speed(0.6f), width(paddleWidth), moving(false) {}

void Paddle::update() {
    if (moving) {
        x += directionX * speed;
    }
}

void Paddle::setDirection(float dx) {
    directionX = dx;
    if (dx != 0.0f) {
        moving = true;  // Start moving when a direction is set
    }
}

void Paddle::setSpeed(float newSpeed) {
    speed = newSpeed;
}

void Paddle::stop() {
    moving = false;
}

void Paddle::start() {
    moving = true;
}

bool Paddle::isMoving() const {
    return moving;
}

void Paddle::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

// Clear the previous position
void Paddle::clearPrevious() {
    for (int i = 0; i < width; i++) {
        mvaddch(lastDrawnY, lastDrawnX + i, ' ');
    }
}

void Paddle::draw() {
    int currentX = static_cast<int>(round(x));
    int currentY = static_cast<int>(round(y));
    
    // Only redraw if the position has changed
    if (currentX != lastDrawnX || currentY != lastDrawnY) {
        // Clear previous position if it is different
        clearPrevious();
        
        // Update last drawn position
        lastDrawnX = currentX;
        lastDrawnY = currentY;
    }
    
    // Draw paddle
    attron(COLOR_PAIR(1)); // Paddle color
    for (int i = 0; i < width; i++) {
        mvaddch(currentY, currentX + i, '=');
    }
    attroff(COLOR_PAIR(1));
}

float Paddle::getX() const { return x; }
float Paddle::getY() const { return y; }
int Paddle::getWidth() const { return width; }
float Paddle::getDirectionX() const { return directionX; }
float Paddle::getSpeed() const { return speed; }

Ball::Ball(int startX, int startY) : 
    x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
    lastDrawnX(startX), lastDrawnY(startY),
    directionX(0.7f), directionY(-0.7f), // Initial direction (up and to the right)
    speed(0.35f), active(true) {}  

// Move in the current direction    
void Ball::update() {
    if (active) {
        x += directionX * speed;
        y += directionY * speed;
    }
}

// Set a new direction vector
void Ball::setDirection(float dx, float dy) {
    directionX = dx;
    directionY = dy;
    
    // Normalize the direction vector
    float length = sqrt(dx * dx + dy * dy);
    if (length > 0) { // Avoid division by zero
        directionX /= length;
        directionY /= length;
    }
}

void Ball::reverseX() {
    directionX = -directionX * 0.95f;;
}

void Ball::reverseY() {
    directionY = -directionY * 0.95f;;
}

void Ball::setSpeed(float newSpeed) {
    speed = newSpeed;
}

void Ball::setActive(bool isActive) {
    active = isActive;
}

bool Ball::isActive() const {
    return active;
}

void Ball::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

// Clear the previous position
void Ball::clearPrevious() {
    mvaddch(lastDrawnY, lastDrawnX, ' ');
}

void Ball::draw() {
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

float Ball::getX() const { return x; }
float Ball::getY() const { return y; }
float Ball::getDirectionX() const { return directionX; }
float Ball::getDirectionY() const { return directionY; }
float Ball::getSpeed() const { return speed; }

Block::Block(int startX, int startY, int w, int h, int color) : 
    x(startX), y(startY), width(w), height(h), active(true), colorPair(color) {}

void Block::draw() {
    if (!active) return;
    
    attron(COLOR_PAIR(colorPair));
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            mvaddch(y + row, x + col, ACS_CKBOARD);
        }
    }
    attroff(COLOR_PAIR(colorPair));
}

// Clear the block from the screen
void Block::clear() {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            mvaddch(y + row, x + col, ' ');
        }
    }
}

bool Block::collidesWith(const Ball& ball) {
    if (!active) return false;
    
    float ballX = ball.getX();
    float ballY = ball.getY();
    
    return (ballX >= x && ballX < x + width &&
            ballY >= y && ballY < y + height);
}

void Block::setActive(bool isActive) {
    if (active && !isActive) {
        // If being deactivated, clear from screen
        clear();
    }
    active = isActive;
}

bool Block::isActive() const {
    return active;
}

int Block::getX() const { return x; }
int Block::getY() const { return y; }
int Block::getWidth() const { return width; }
int Block::getHeight() const { return height; }

BattleBox::BattleBox(int startX, int startY, int w, int h) :
    x(startX), y(startY), width(w), height(h), needsRedraw(true), needsRefresh(false) {}

void BattleBox::draw() {
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

void BattleBox::setNeedsRedraw() {
    needsRedraw = true;
}

int BattleBox::getX() const { return x; }
int BattleBox::getY() const { return y; }
int BattleBox::getWidth() const { return width; }
int BattleBox::getHeight() const { return height; }

GameManager::GameManager(int screenWidth, int screenHeight) : 
    battleBox(screenWidth/2 - 20,screenHeight/2 - 15, 40, 30),
    paddle(screenWidth/2 - 3,screenHeight/2 + 14),
    ball(screenWidth/2,screenHeight/2 + 13),
    blockCount(0),
    gameOver(false),
    gameWon(false),
    lives(3),
    ballLost(false) {
    
    // Initialize blocks
    initializeBlocks(true);
}

void GameManager::initializeBlocks(bool fullDensity) {
    // Clear existing blocks
    blocks.clear();
    blockCount = 0;
    
    // Calculate the number of blocks that fit in the battle box
    int blockWidth = 4;
    int blockHeight = 1;
    int padding = 1; // Space between blocks
    
    int boxWidth = battleBox.getWidth();
    int boxX = battleBox.getX() + 3; 
    int boxY = battleBox.getY() + 2; 
    
    int blocksPerRow = (boxWidth / (blockWidth + padding))-1;
    int maxRows = 5; 
    
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

void GameManager::setLives(int newLives) {
    lives = newLives;
}

int GameManager::getLives() const {
    return lives;
}

void GameManager::decrementLives() {
    if (lives > 0) {
        lives--;
    }
    
    if (lives <= 0) {
        gameOver = true;
    } else {
        ballLost = true;
    }
}

bool GameManager::hasLivesRemaining() const {
    return lives > 0;
}

bool GameManager::isBallLost() const {
    return ballLost;
}

void GameManager::reset(bool fullDensity, int startLives) {
    // Reset game 
    gameOver = false;
    gameWon = false;
    ballLost = false;
    lives = startLives;
    
    // Reset both paddle and ball positions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    paddle.setPosition(maxX/2 - 3, maxY/2 + 14);
    ball.setPosition(maxX/2, maxY/2 + 13);
    
    // Set ball direction
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> distX(-0.7f, 0.7f);
    float dirX = distX(gen);
    
    std::uniform_real_distribution<float> distY(-1.0f, -0.7f);
    float dirY = distY(gen);
    
    ball.setDirection(dirX, dirY);
    
    // Reinitialize blocks with specified density
    initializeBlocks(fullDensity);
    
    // Force battleBox redraw when game is reset
    battleBox.setNeedsRedraw();
}

void GameManager::resetBall() {
    ballLost = false;
    
    // Reset both paddle and ball positions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    paddle.setPosition(maxX/2 - 3, maxY/2 + 14);
    ball.setPosition(maxX/2, maxY/2 + 13);
    
    // Set ball direction (mostly upward)
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> distX(-0.7f, 0.7f);
    float dirX = distX(gen);
    
    std::uniform_real_distribution<float> distY(-1.0f, -0.7f);
    float dirY = distY(gen);
    
    ball.setDirection(dirX, dirY);
    
    // Force battleBox redraw when ball is reset
    battleBox.setNeedsRedraw();
}

void GameManager::update() {
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
            
            // reverse direction based on ball's movement direction
            if (abs(ballDirX) > abs(ballDirY)) {
                ball.reverseX(); 
            } else {
                ball.reverseY(); 
            }
            
            // Check if all blocks are destroyed (win condition)
            if (blockCount <= 0) {
                gameWon = true;
                return;  // exit immediately when game is won
            }
            
            // Only handle one collision per update
            break;
        }
    }
    
    // Double-checking win condition (safety check for the reported bug)
    if (blockCount <= 0) {
        gameWon = true;
        return;
    }
}

void GameManager::draw() {
    battleBox.draw();
    
    // Draw blocks
    for (auto& block : blocks) {
        block.draw();
    }
    
    // Draw the paddle and ball
    paddle.draw();
    ball.draw();
    
    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Display lives count 
    mvprintw(maxY - 40, maxX / 2 -17, "Blocks remaining: %02d     Lives: %02d", blockCount, lives); 

    // Calculate position for status text 
    int statusY = battleBox.getY() - 2; 
    
    // Clear the status area first to prevent text overlap
    for (int i = 0; i < 1; i++) {
        mvprintw(statusY + i, maxX / 2 - 20, "                                                  ");
    }
    
    // Display lives count and blocks remaining with proper spacing
    mvprintw(statusY, maxX / 2 - 17, "Blocks remaining: %d     Lives: %d", blockCount, lives);
    
    if (gameOver) {
        attron(COLOR_PAIR(1)); // Red for game over
        mvprintw(maxY / 2, maxX / 2 - 4.5, "GAME OVER");
        mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to restart");
        attroff(COLOR_PAIR(1));
    } else if (gameWon) {
        attron(COLOR_PAIR(3)); // Green for win
        mvprintw(maxY / 2, maxX / 2 - 12, "YOU WIN! ALL BLOCKS CLEARED");
        mvprintw(maxY / 2 + 1, maxX / 2 - 11, "Press ENTER to continue");
        attroff(COLOR_PAIR(3));
    } else if (ballLost) {
        attron(COLOR_PAIR(1)); // Red for ball lost
        mvprintw(maxY / 2, maxX / 2 - 11, "BALL LOST! LIVES LEFT: %02d", lives);
        mvprintw(maxY / 2 + 1, maxX / 2 - 16, "Press ENTER to launch another ball");
        attroff(COLOR_PAIR(1));
    }
}

void GameManager::handleInput(int key) {
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

bool GameManager::isGameOver() const {
    return gameOver;
}

bool GameManager::isGameWon() const {
    return gameWon;
}

bool GameManager::isPaddleMoving() const {
    return paddle.isMoving();
}

void GameManager::paddleStart() {
    paddle.start();
}

void GameManager::paddleStop() {
    paddle.stop();
}

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
            game.reset(false, 5);  // false = half density, 3 lives
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 1: Half blocks, 5 tries");
            break;
        case 2:
            // Round 2: Normal blocks, 3 tries
            game.reset(true, 5);   // true = full density, 3 lives
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 2: Full blocks, 5 tries");
            break;
        case 3:
            // Round 3: Normal blocks, 1 try
            game.reset(true, 3);   // true = full density, 1 life
            game.draw();
            mvprintw(maxY / 2, maxX / 2 - 14, "Round 3: Full blocks, 3 try");
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
    // Clear center area of the screen 
    for (int i = 0; i < 2; i++) {
        mvprintw(maxY / 2 + i, maxX / 2 - 19, "                                       ");
    }
    // Game loop for this round
    bool running = true;
    
    while (running) {
        // Processing all available input
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
        
        // Update game
        game.update();
        
        // Draw game
        game.draw();

        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60  FPS
    }

    return -1;  // Default to failure if we exit the loop unexpectedly
}
