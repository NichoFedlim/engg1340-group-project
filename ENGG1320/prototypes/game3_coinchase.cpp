// coin_chase.cpp - Enhanced version with BattleBox-style Arena
// Started by Nicholas F. on Apr. 12 2025, Sat.

#include <ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;

// Game constants
constexpr int BASE_COINS_PER_LEVEL = 5;
constexpr int BASE_TIME_FOR_LEVEL_1 = 60;  // 1 minute for first level
constexpr int TIME_DECREASE_PER_LEVEL = 10; // Decrease by 10 seconds per level
constexpr int MAX_ACTIVE_COINS = 15;
constexpr int SPECIAL_COIN_SPAWN_RATE = 15; // 1 in 15 chance for special coin
constexpr float PLAYER_SPEED = 0.3f;
constexpr int MAX_LEVEL = 5;  // Reduced to 5 rounds
constexpr int COUNTDOWN_DURATION = 5; // 5-second countdown before each level

enum CoinType { REGULAR, SPECIAL, TRAP };

// Forward declare the Arena class
class Arena;

class Coin {
public:
    Coin(float startX = 0.0f, float startY = 0.0f, float spd = 0.1f, int val = 1, CoinType type = REGULAR) :
        x(startX), y(startY), lastDrawnX(static_cast<int>(startX)), lastDrawnY(static_cast<int>(startY)),
        speed(spd), active(false), value(val), type(type), 
        symbol(type == REGULAR ? '*' : (type == SPECIAL ? '$' : 'X')) {}

    void update(const Arena* arena);  // Forward declaration of method
    
    void setValue(int v) { value = v; }

    void draw() const {
        if (!active) return;
        
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            mvaddch(lastDrawnY, lastDrawnX, ' ');
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        }
        
        int colorPair = (type == REGULAR ? 3 : (type == SPECIAL ? 4 : 5));
        attron(COLOR_PAIR(colorPair));
        mvaddch(currentY, currentX, symbol);
        attroff(COLOR_PAIR(colorPair));
    }

    void activate(float newX, float newY, CoinType newType = REGULAR) {
        x = newX;
        y = newY;
        type = newType;
        symbol = (type == REGULAR ? '*' : (type == SPECIAL ? '$' : 'X'));
        active = true;
        lastDrawnX = static_cast<int>(round(x));
        lastDrawnY = static_cast<int>(round(y));
        spawnTime = chrono::steady_clock::now();
        moveCounter = 0;
    }

    void deactivate() {
        if (active) {
            mvaddch(lastDrawnY, lastDrawnX, ' ');
            active = false;
        }
    }

    bool isExpired() const {
        if (!active) return false;
        auto now = chrono::steady_clock::now();
        // Different expiration times based on coin type
        int expirationTime = (type == TRAP) ? 15 : (type == SPECIAL ? 8 : 10);
        return chrono::duration_cast<chrono::seconds>(now - spawnTime).count() >= expirationTime;
    }    
    
    // Getters
    bool isActive() const { return active; }
    int getValue() const { return value; }
    CoinType getType() const { return type; }
    
    // Get position for precise collision detection
    float getX() const { return x; }
    float getY() const { return y; }

    bool checkCollision(float playerX, float playerY) const {
        if (!active) return false;
        float dx = x - playerX;
        float dy = y - playerY;
        // Strict collision detection
        return (dx * dx + dy * dy) < 0.8f;
    }

private:
    float x, y;
    mutable int lastDrawnX, lastDrawnY;
    float speed;
    bool active;
    int value;
    CoinType type;
    char symbol;
    chrono::steady_clock::time_point spawnTime;
    int moveCounter = 0;
};

class Arena {
private:
    int x, y;         // Top-left corner position
    int width, height; // Dimensions including borders
    bool needsRedraw;  // Flag to determine if the box needs redrawing

public:
    Arena(int screenWidth, int screenHeight, int arenaWidth, int arenaHeight) {
        // Calculate the center position
        x = (screenWidth - arenaWidth) / 2;
        y = (screenHeight - arenaHeight) / 2;
        this->width = arenaWidth;
        this->height = arenaHeight;
        needsRedraw = true;
    }
    
    void draw() {
        if (!needsRedraw) return;
        
        // Enable reverse highlighting like in your BattleBox class
        attron(COLOR_PAIR(2) | A_REVERSE);
        
        // Draw the top and bottom borders with the wider style
        for (int i = -1; i <= width+1; i++) {
            mvaddch(y, x + i, ' ');              // Top border
            mvaddch(y + height, x + i, ' ');     // Bottom border
        }
        
        // Draw the left and right borders with the wider style
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');              // Left border
            mvaddch(y + i, x + width, ' ');      // Right border
            mvaddch(y + i, x-1, ' ');            // Left outer border
            mvaddch(y + i, x+1 + width, ' ');    // Right outer border
        }
        
        attroff(COLOR_PAIR(2) | A_REVERSE);
        
        needsRedraw = false;
    }

    void setNeedsRedraw() {
        needsRedraw = true;
    }

    // Boundary checking with floating-point coordinates
    bool contains(float px, float py) const {
        return (px > x + 1 && px < x + width - 1 &&
                py > y + 1 && py < y + height - 1);
    }

    // Getters for position and dimensions:
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    // Methods for inner coordinates
    int getInnerX() const { return x + 1; }
    int getInnerY() const { return y + 1; }
    int getInnerWidth() const { return width - 2; }
    int getInnerHeight() const { return height - 2; }
};

// Now that Arena is defined, we can implement Coin::update
void Coin::update(const Arena* arena) {
    if (!active) return;
    
    // Trap coins move randomly but stay within arena bounds
    if (type == TRAP && moveCounter++ % 10 == 0) {
        float newX = x + (rand() % 3 - 1) * 0.5f;
        float newY = y + (rand() % 3 - 1) * 0.5f;
        
        // Only move if within bounds
        if (arena->contains(newX, newY)) {
            x = newX;
            y = newY;
        }
    }
}

class Player {
public:
    Player(float startX, float startY, float spd, Arena& arenaRef) :
        x(startX), y(startY), lastDrawnX(static_cast<int>(startX)), lastDrawnY(static_cast<int>(startY)),
        dx(0.0f), dy(0.0f), speed(spd), aspectRatio(2.0f), 
        symbol(ACS_DIAMOND), invincible(false), invincibleTimer(0), arena(&arenaRef) {}

    void update() {
        if (invincible && --invincibleTimer <= 0) {
            invincible = false;
        }
        
        // Calculate new position with floating-point precision
        // Use aspect ratio for horizontal movement like in your Heart class
        float newX = x + dx * speed * aspectRatio;
        float newY = y + dy * speed;
        
        // Check boundaries for each axis independently
        if (newX > arena->getX() + 1 && newX < arena->getX() + arena->getWidth() - 1) {
            x = newX;  // Update X position if within bounds
        }
        
        if (newY > arena->getY() + 1 && newY < arena->getY() + arena->getHeight() - 1) {
            y = newY;  // Update Y position if within bounds
        }
    }

    void draw() const {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            mvaddch(lastDrawnY, lastDrawnX, ' ');
        }
        
        if (!invincible || (invincibleTimer / 5) % 2 == 0) {
            attron(COLOR_PAIR(1)); // Red for player
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(1));
        }
        
        lastDrawnX = currentX;
        lastDrawnY = currentY;
    }

    void setDirection(float newDx, float newDy) {
        // Normalize diagonal movement speed
        if (newDx != 0 && newDy != 0) {
            const float factor = 0.7071f; // 1/sqrt(2)
            dx = newDx * factor;
            dy = newDy * factor;
        } else {
            dx = newDx;
            dy = newDy;
        }
    }

    void stop() { dx = 0.0f; dy = 0.0f; }

    float getX() const { return x; }
    float getY() const { return y; }

    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
        lastDrawnX = static_cast<int>(round(x));
        lastDrawnY = static_cast<int>(round(y));
    }

    void makeInvincible(int duration) {
        invincible = true;
        invincibleTimer = duration;
    }

    bool isInvincible() const { return invincible; }

private:
    Arena* arena;
    float x, y;
    mutable int lastDrawnX, lastDrawnY;
    float dx, dy;
    float speed;
    float aspectRatio;
    int symbol;
    bool invincible;
    int invincibleTimer;
};

class Game {
public:
    Game(int w, int h)
        : arenaWidth(w), arenaHeight(h),
            coinsCollected(0), totalCoinsCollected(0), level(1),
            gameOver(false), victory(false), levelComplete(false),
            countdownActive(false), countdownValue(COUNTDOWN_DURATION),
            rng(chrono::system_clock::now().time_since_epoch().count()),
            arena(nullptr), player(nullptr) {

        initializeNcurses();   // Must be first to get COLS/LINES

        // Ensure terminal is large enough before calculating positions
        if (COLS < w + 10 || LINES < h + 10) {
            endwin();
            printf("Terminal too small. Need at least %dx%d, got %dx%d\n", 
                w + 10, h + 10, COLS, LINES);
            exit(1);
        }
        
        initializeGame();
    }

    ~Game() {
        delete player;
        delete arena;
    }

    void run() {
        mainGameLoop();
        cleanupNcurses();
    }

private:
    int arenaWidth, arenaHeight;
    Arena* arena;
    Player* player;
    vector<Coin> coins;
    int coinsCollected, totalCoinsCollected, level;
    bool gameOver, victory, levelComplete;
    bool countdownActive;
    int countdownValue;
    time_t countdownStartTime;
    int timeLimit;
    int coinsRequired;
    time_t startTime;
    default_random_engine rng;

    void initializeGame() {
        coins.resize(MAX_ACTIVE_COINS);

        // Create arena centered on terminal
        arena = new Arena(COLS, LINES, arenaWidth, arenaHeight);

        // Create player centered in arena
        float playerX = arena->getX() + arena->getWidth() / 2.0f;
        float playerY = arena->getY() + arena->getHeight() / 2.0f;
        player = new Player(playerX, playerY, PLAYER_SPEED, *arena);

        resetLevel();
    }

    void initializeNcurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        nodelay(stdscr, TRUE);

        if (has_colors()) {
            start_color();
            use_default_colors(); 
            init_pair(1, COLOR_RED, COLOR_BLACK);     // Player
            init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Arena
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Regular coins
            init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Special coins
            init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Trap coins
            init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Messages
        } else {
            endwin();
            cout << "Your terminal does not support color" << endl;
            exit(1);
        }

        // Check terminal size
        if (COLS < 60 || LINES < 24) {
            endwin();
            printf("Please resize your terminal to at least 60x24 characters\n");
            exit(1); 
        }
    }

    void cleanupNcurses() {
        endwin();
    }

    void mainGameLoop() {
        int frameCount = 0;
        startCountdown(); // Start countdown before level begins
        
        startTime = time(nullptr);
        
        while (!gameOver && !victory) {
            handleInput();
         
            if (countdownActive) {
                updateCountdown();
            }
            else if (!levelComplete && !gameOver) {
                updateGameState(frameCount);
            }
            
            renderGame();
            usleep(16667); // ~60 FPS
            frameCount++;
        }
        
        if (victory) {
            showVictoryScreen();
        } else {
            showGameOverScreen();
        }
    }

    void startCountdown() {
        countdownActive = true;
        countdownValue = COUNTDOWN_DURATION;
        countdownStartTime = time(nullptr);
        
        // Clear any existing coins
        for (auto& coin : coins) {
            coin.deactivate();
        }
        
        // Center player
        player->setPosition(
            arena->getX() + arena->getWidth() / 2,
            arena->getY() + arena->getHeight() / 2
        );
        player->stop();
        player->makeInvincible(30);
    }

    void updateCountdown() {
        time_t currentTime = time(nullptr);
        int elapsed = currentTime - countdownStartTime;
        countdownValue = max(0, COUNTDOWN_DURATION - elapsed);
        
        if (countdownValue <= 0) {
            countdownActive = false;
            startTime = time(nullptr); // Start level timer

            // Clear the countdown area
            int centerY = LINES / 2;
            move(centerY, 0);
            clrtoeol();
            refresh();
            
            // Spawn initial coins
            for (int i = 0; i < min(5, coinsRequired); i++) {
                trySpawnCoin();
            }
            arena->setNeedsRedraw(); // Force arena redraw
        }
    }

    void handleInput() {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            gameOver = true;
        } else if (levelComplete && (ch == '\n' || ch == ' ')) {
            advanceLevel();
        } else if (!levelComplete) {
            processMovementInput(ch);
        }
    }

    void processMovementInput(int ch) {
        switch (ch) {
            case KEY_UP: player->setDirection(0.0f, -1.0f); break;
            case KEY_DOWN: player->setDirection(0.0f, 1.0f); break;
            case KEY_LEFT: player->setDirection(-1.0f, 0.0f); break;
            case KEY_RIGHT: player->setDirection(1.0f, 0.0f); break;
            case ' ': player->stop(); break;
        }
    }

    void updateGameState(int frameCount) {
        updatePlayerPosition();
        
        // Spawn coins periodically with chance for special coins
        int spawnRate = 25 - min(level, 20);
        if (frameCount % spawnRate == 0 && countActiveCoins() < min(coinsRequired + 2, MAX_ACTIVE_COINS)) {
            trySpawnCoin();
        }
        
        // Update all game elements
        for (auto& coin : coins) {
            coin.update(arena); // Pass arena for boundary checking
            if (coin.isExpired()) {
                coin.deactivate();
            }
        }
        
        checkCollisions();
        if (!levelComplete) {
            checkTimeLimit();
        }
    }

    void updatePlayerPosition() {
        player->update();
    }

    void trySpawnCoin() {
        // Create coin positions within arena bounds with some margin
        int marginX = 3;  // Stay away from borders
        int marginY = 3;
        
        uniform_int_distribution<int> xDist(arena->getX() + marginX, arena->getX() + arena->getWidth() - marginX);
        uniform_int_distribution<int> yDist(arena->getY() + marginY, arena->getY() + arena->getHeight() - marginY);
    
        int roll = rng() % 100;
        CoinType type;
        
        // Determine coin type based on level and random chance
        if (roll < 5 + level * 3) {  // Trap chance increases with level
            type = TRAP;
        } else if (roll < 5 + level * 3 + 100/SPECIAL_COIN_SPAWN_RATE) {
            type = SPECIAL;
        } else {
            type = REGULAR;
        }
    
        for (auto& coin : coins) {
            if (!coin.isActive()) {
                int xPos, yPos;
                bool validPosition;
                int attempts = 0;
                
                do {
                    xPos = xDist(rng);
                    yPos = yDist(rng);
                    
                    float playerX = player->getX();
                    float playerY = player->getY();
                    float dx = xPos - playerX;
                    float dy = yPos - playerY;
                    
                    validPosition = (dx*dx + dy*dy > 25);  // Min distance of 5 units from player
                    
                    // Check it's not too close to other coins
                    for (const auto& otherCoin : coins) {
                        if (otherCoin.isActive()) {
                            float coinDx = xPos - otherCoin.getX();
                            float coinDy = yPos - otherCoin.getY();
                            if (coinDx*coinDx + coinDy*coinDy < 4) {  // Min distance of 2 units between coins
                                validPosition = false;
                                break;
                            }
                        }
                    }
                    
                    attempts++;
                } while (!validPosition && attempts < 10);
                
                int coinValue = 1;  // All coins are worth 1
                
                coin.activate(static_cast<float>(xPos), static_cast<float>(yPos), type);
                coin.setValue(coinValue);
                break;
            }
        }
    }

    int countActiveCoins() const {
        return count_if(coins.begin(), coins.end(), 
                      [](const Coin& c) { return c.isActive(); });
    }

    void checkCollisions() {
        for (auto& coin : coins) {
            if (coin.isActive() && coin.checkCollision(player->getX(), player->getY())) {
                CoinType coinType = coin.getType();
                
                if (coinType == TRAP) {
                    if (!player->isInvincible()) {
                        timeLimit -= 5;  // Lose time for trap
                        
                        // Reduce collected coins by 3 (but not below 0)
                        int coinsLost = min(3, coinsCollected);
                        coinsCollected = max(0, coinsCollected - coinsLost);
                        totalCoinsCollected = max(0, totalCoinsCollected - coinsLost);  // Also reduce total
                        
                        attron(COLOR_PAIR(5) | A_BOLD);
                        mvprintw(arena->getY() - 1, arena->getX() + arena->getWidth() / 2 - 16, 
                                "-5 seconds! -%d coins!", coinsLost);
                        attroff(COLOR_PAIR(5) | A_BOLD);
                        refresh();
                        
                        player->makeInvincible(30);
                    }
                } else {
                    coinsCollected++;
                    totalCoinsCollected++;  // Track total coins across all levels
                    
                    if (coinType == SPECIAL) {
                        timeLimit += 5;
                        attron(COLOR_PAIR(4) | A_BOLD);
                        mvprintw(arena->getY() - 1, arena->getX() + arena->getWidth() / 2 - 5, "+5 seconds!");
                        attroff(COLOR_PAIR(4) | A_BOLD);
                        refresh();
                    }
                }
                
                coin.deactivate();
                
                if (coinsCollected >= coinsRequired) {
                    levelComplete = true;
                }
            }
        }
    }

    void checkTimeLimit() {
        time_t currentTime = time(nullptr);
        if (currentTime - startTime > timeLimit && !levelComplete) {
            gameOver = true;
        }
    }

    void renderGame() {
        arena->draw();
        
        for (const auto& coin : coins) {
            coin.draw();
        }
        
        player->draw();
        drawHUD();
        
        if (countdownActive) {
            drawCountdown();
        }

        refresh();
    }

    void drawCountdown() {
        if (!countdownActive) return;  // Only draw if countdown is active

        int centerX = COLS / 2;
        int centerY = LINES / 2;
        
        attron(A_BOLD | COLOR_PAIR(4));
        
        string countdownText;
        if (countdownValue > 0) {
            countdownText = "Get Ready: " + to_string(countdownValue);
        } else {
            countdownText = "GO!";
        }

        // Save what's behind the countdown text
        string background(centerX * 2, ' ');
        for (int i = 0; i < countdownText.length(); i++) {
            background[centerX - countdownText.length()/2 + i] = mvinch(centerY, centerX - countdownText.length()/2 + i) & A_CHARTEXT;
        }
        
        mvprintw(centerY, centerX - countdownText.length() / 2, countdownText.c_str());
        attroff(A_BOLD | COLOR_PAIR(4));

        // Clear the countdown text when it's time to disappear
        if (countdownValue == 0) {
            usleep(500000); // Show "GO!" for 0.5 seconds
            // Restore what was behind the text
            for (int i = 0; i < countdownText.length(); i++) {
                mvaddch(centerY, centerX - countdownText.length()/2 + i, background[centerX - countdownText.length()/2 + i]);
            }
            refresh();
        }
    }

    void centerText(int y, const string& text) const {
        mvaddstr(y, COLS / 2 - text.length() / 2, text.c_str());
    }    

    void drawHUD() const {
        if (countdownActive) return;

        int hudY = arena->getY() - 5;
        
        if (hudY < 1) hudY = 1;
        
        for (int i = hudY; i < arena->getY(); i++) {
            move(i, 0);
            clrtoeol();
        }
        
        attron(A_BOLD);
        centerText(hudY, "COIN CHASE - Level: " + to_string(level) + "/" + to_string(MAX_LEVEL));
        attroff(A_BOLD);
        
        string coinStatus = "Coins: " + to_string(coinsCollected) + "/" + to_string(coinsRequired);
        centerText(hudY + 2, coinStatus);
    
        time_t currentTime = time(nullptr);
        int timeLeft = std::max(0, static_cast<int>(timeLimit - (currentTime - startTime)));
        
        if (timeLeft <= 5) {
            attron(COLOR_PAIR(1) | A_BOLD); // Red for low time
        } else if (timeLeft <= 10) {
            attron(COLOR_PAIR(3)); // Yellow for medium time
        }
        
        centerText(hudY + 3, "Time: " + to_string(timeLeft) + " sec");
        
        if (timeLeft <= 10) {
            attroff(COLOR_PAIR(timeLeft <= 5 ? 1 : 3) | (timeLeft <= 5 ? A_BOLD : 0));
        }
    
        if (levelComplete) {
            attron(COLOR_PAIR(4) | A_BOLD);
            string completeText = "LEVEL COMPLETE! Press SPACE to continue";
            centerText(LINES / 2, completeText);
            attroff(COLOR_PAIR(4) | A_BOLD);
        } else if (hudY + 4 < arena->getY()) {
            attron(COLOR_PAIR(6));
            string hintText = "$ = +Time +Coin | X = Danger! (-3 coins)";
            centerText(hudY + 4, hintText);
            attroff(COLOR_PAIR(6));
        }
    }

    void advanceLevel() {
        level++;
    
        if (level > MAX_LEVEL) {
            victory = true;
            return;
        }
        resetLevel();
    }
    
    void resetLevel() {
        coinsCollected = 0;
        coinsRequired = BASE_COINS_PER_LEVEL + level * 3;
        
        // Calculate time for this level - decreasing by 10 seconds per level
        timeLimit = BASE_TIME_FOR_LEVEL_1 - (level - 1) * TIME_DECREASE_PER_LEVEL;
        
        levelComplete = false;
        startCountdown(); // Start countdown instead of immediately starting level
        startTime = time(nullptr);
        
        for (auto& coin : coins) {
            coin.deactivate();
        }
        
        player->setPosition(
            arena->getX() + arena->getWidth() / 2,
            arena->getY() + arena->getHeight() / 2
        );
        player->stop();
        player->makeInvincible(30);
        
        clear();
        arena->setNeedsRedraw();
        
        for (int i = 0; i < min(5, coinsRequired); i++) {
            trySpawnCoin();
        }
    }

    void showVictoryScreen() const {
        clear();
    
        int centerY = LINES / 2;
    
        attron(A_BOLD | COLOR_PAIR(4));
        centerText(centerY - 3, "CONGRATULATIONS!");
        attroff(A_BOLD | COLOR_PAIR(4));
    
        attron(COLOR_PAIR(3));
        centerText(centerY - 1, "You completed all " + to_string(MAX_LEVEL) + " levels!");
        centerText(centerY, "Total Coins Collected: " + to_string(totalCoinsCollected));
        attroff(COLOR_PAIR(3));
        
        centerText(centerY + 4, "Press any key to exit");
    
        refresh();
        nodelay(stdscr, FALSE);
        getch();
    }
    
    void showGameOverScreen() const {
        clear();
        
        int centerY = LINES / 2;
    
        attron(A_BOLD | COLOR_PAIR(1));
        centerText(centerY - 3, "GAME OVER!");
        attroff(A_BOLD | COLOR_PAIR(1));
    
        centerText(centerY - 1, "Level Reached: " + to_string(level) + "/" + to_string(MAX_LEVEL));
        
        attron(COLOR_PAIR(3));
        centerText(centerY + 2, "You collected " + to_string(coinsCollected) + " coins this level.");
        centerText(centerY + 3, "Total Coins: " + to_string(totalCoinsCollected));
        attroff(COLOR_PAIR(3));
        
        centerText(centerY + 5, "Press any key to exit");
    
        refresh();
        nodelay(stdscr, FALSE);
        getch();
    }
};

int main() {
    try {
        Game game(60, 20); // Main Initialization
        game.run(); 
    }
    catch (const exception& e) {
        endwin();
        cerr << "Error: " << e.what() << endl; // Error Handling
        return 1;
    }
    return 0;
}