#ifndef GAME_1300_H
#define GAME_1300_H

#include <ncursesw/ncurses.h>
#include <vector>
#include <random>

class Paddle {
private:
    float x, y;           // Position coordinates
    int lastDrawnX, lastDrawnY; // Last position where paddle was drawn
    float directionX;     // Direction vector
    float speed;          // Movement speed
    int width;            // Paddle width
    bool moving;          // Whether the paddle is moving

public:
    Paddle(int startX, int startY, int paddleWidth = 7);
    void update();
    void setDirection(float dx);
    void setSpeed(float newSpeed);
    void stop();
    void start();
    bool isMoving() const;
    void setPosition(float newX, float newY);
    void clearPrevious();
    void draw();
    float getX() const;
    float getY() const;
    int getWidth() const;
    float getDirectionX() const;
    float getSpeed() const;
};

class Ball {
private:
    float x, y;           // Position coordinates
    int lastDrawnX, lastDrawnY; // Last position where the ball was drawn
    float directionX, directionY; // Direction vector
    float speed;          // Movement speed
    bool active;          // Whether the ball is moving

public:
    Ball(int startX, int startY);
    void update();
    void setDirection(float dx, float dy);
    void reverseX();
    void reverseY();
    void setSpeed(float newSpeed);
    void setActive(bool isActive);
    bool isActive() const;
    void setPosition(float newX, float newY);
    void clearPrevious();
    void draw();
    float getX() const;
    float getY() const;
    float getDirectionX() const;
    float getDirectionY() const;
    float getSpeed() const;
};

class Block {
private:
    int x, y;             // Position coordinates
    int width, height;    // Demensions of the block
    bool active;          // Whether the block is still there or destroyed
    int colorPair;        // Color pair to use for the block

public:
    Block(int startX, int startY, int w = 4, int h = 1, int color = 3);
    void draw();
    void clear();
    bool collidesWith(const Ball& ball);
    void setActive(bool isActive);
    bool isActive() const;
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
};

class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;  // Flag to determine if the box needs redrawing
    bool needsRefresh; // Flag to determine if the box needs refreshing

public:
    BattleBox(int startX, int startY, int w, int h);
    BattleBox() : x(0), y(0), width(0), height(0), needsRedraw(false), needsRefresh(false) {}
    void draw();
    void setNeedsRedraw();
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
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
    GameManager(int screenWidth, int screenHeight);
    void initializeBlocks(bool fullDensity = true);
    void setLives(int newLives);
    int getLives() const;
    void decrementLives();
    bool hasLivesRemaining() const;
    bool isBallLost() const;
    void resetBall();
    void reset(bool fullDensity = true, int startLives = 3);
    void update();
    void draw();
    void handleInput(int key);
    bool isGameOver() const;
    bool isGameWon() const;
    bool isPaddleMoving() const;
    void paddleStart();
    void paddleStop();
};

// Function to run a single round of the game
int runRound(int round);

#endif
