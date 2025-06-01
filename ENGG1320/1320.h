#ifndef COINGAME_H
#define COINGAME_H

#include <vector>
#include <random>
#include <chrono>

// Game constants
constexpr int BASE_COINS_PER_LEVEL = 5;
constexpr int BASE_TIME_FOR_LEVEL_1 = 60;  // 1 minute for first level
constexpr int TIME_DECREASE_PER_LEVEL = 10; // Decrease by 10 seconds per level
constexpr int MAX_ACTIVE_COINS = 15;
constexpr int SPECIAL_COIN_SPAWN_RATE = 15; // 1 in 15 chance for special coin
constexpr float PLAYER_SPEED = 0.3f;
constexpr int MAX_LEVEL = 5;  // 5 rounds
constexpr int COUNTDOWN_DURATION = 5; // 5-second countdown before each level

// Coin types
enum CoinType {
    REGULAR,  // Standard yellow coin
    SPECIAL,  // Blue coin that gives 5 seconds extra time
    TRAP      // Purple coin that costs 5 seconds of time and 3 coins
};

// Forward declarations
class Arena;

// Coin class represents collectible objects
class Coin {
public:
    Coin(float startX = 0.0f, float startY = 0.0f, float spd = 0.0f, int val = 1, 
         CoinType type = REGULAR);
    
    void update(const Arena* arena);
    void draw() const;
    void setValue(int v);
    void activate(float newX, float newY, CoinType newType = REGULAR);
    void deactivate();
    bool isExpired() const;
    bool isActive() const;
    int getValue() const;
    CoinType getType() const;
    float getX() const;
    float getY() const;
    bool checkCollision(float playerX, float playerY) const;
    
private:
    float x, y;           // Position with floating-point precision
    mutable int lastDrawnX, lastDrawnY; // Last drawn position for clearing
    float speed;
    bool active;
    int value;
    CoinType type;
    char symbol;
    int moveCounter;
    std::chrono::steady_clock::time_point spawnTime;
};

// Arena class that contains the game area
class Arena {
public:
    Arena(int screenWidth, int screenHeight, int arenaWidth, int arenaHeight);
    
    void draw();
    void setNeedsRedraw();
    bool contains(float px, float py) const;
    
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    int getInnerX() const;
    int getInnerY() const;
    int getInnerWidth() const;
    int getInnerHeight() const;
    
private:
    int x, y;           // Top-left coordinates
    int width, height;  // Dimensions
    bool needsRedraw;
};

// Player class that the user controls
class Player {
public:
    Player(float startX, float startY, float spd, Arena& arenaRef);
    
    void update();
    void draw() const;
    void setDirection(float newDx, float newDy);
    void stop();
    float getX() const;
    float getY() const;
    void setPosition(float newX, float newY);
    void makeInvincible(int duration);
    bool isInvincible() const;
    
private:
    float x, y;        // Position with floating-point precision
    mutable int lastDrawnX, lastDrawnY; // Last drawn position for clearing
    float dx, dy;      // Direction (normalized)
    float speed;
    float aspectRatio; // For consistent speed horizontally/vertically
    int symbol;        // Player's visual representation
    bool invincible;
    int invincibleTimer;
    Arena* arena;      // Reference to the arena for boundary checking
};

// Game class that manages the game flow
class Game {
public:
    Game(int w, int h);
    ~Game();
    
    void initializeNcurses();
    void cleanupNcurses();
    void initializeGame(int round);
    int runRound(int round);
    void startCountdown();
    void updateCountdown();
    void handleInput();
    void processMovementInput(int ch);
    void updateGameState(int frameCount);
    void updatePlayerPosition();
    void trySpawnCoin();
    int countActiveCoins() const;
    void checkCollisions();
    void checkTimeLimit();
    void renderGame();
    void drawHUD() const;
    void drawCountdown();
    void centerText(int y, const std::string& text) const;
    void resetLevel(int round);
    void showRoundComplete() const;
    void showRoundFailed() const;
    
private:
    int arenaWidth, arenaHeight;
    int coinsCollected, totalCoinsCollected;
    int level;
    int coinsRequired;
    int timeLimit;
    bool gameOver;
    bool victory;
    bool levelComplete;
    bool countdownActive;
    int countdownValue;
    time_t countdownStartTime;
    time_t startTime;
    
    std::mt19937 rng;
    Arena* arena;
    Player* player;
    std::vector<Coin> coins;
};

// Standalone function to run a round (used by the main menu)
int runRound(int round = 1);

#endif // COINGAME_H