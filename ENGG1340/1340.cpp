#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <random>
#include <memory>
#include <ctime>
#include <algorithm>

// Game constants
const int INITIAL_PLAYER_HEALTH = 10;
const int LASER_DAMAGE = 1;
const int SPACESHIP_DAMAGE = 1;
const int BOMB_DAMAGE = 1;
const float PROJECTILE_SPEED = 0.5f;
const float SPACESHIP_SPEED = 0.1f;
const float BOMB_SPEED = 0.2f;
const float LASER_SPEED = 0.8f;
const int SCORE_PER_SPACESHIP = 100;
const int SCORE_PER_PROJECTILE = 10;
const int SCORE_PER_BOMB = 50;

class GameObject {
protected:
    float x, y;           // Position with floating-point precision
    int lastDrawnX, lastDrawnY; // Last position where object was drawn
    float directionX, directionY; // Direction vector (normalized)
    float speed;          // Movement speed
    int symbol;           // Symbol to represent the object
    bool active;          // Whether the object is active/alive

public:
    GameObject(float startX, float startY, float dx, float dy, float spd, int sym) : 
        x(startX), y(startY), 
        lastDrawnX(static_cast<int>(round(startX))), 
        lastDrawnY(static_cast<int>(round(startY))),
        directionX(dx), directionY(dy),
        speed(spd), symbol(sym), active(true) {}
    
    virtual ~GameObject() = default;

    virtual void update() {
        // Move in the current direction
        x += directionX * speed;
        y += directionY * speed;
    }

    void clearPrevious() {
        // Clear the previous position
        mvaddch(lastDrawnY, lastDrawnX, ' ');
    }

    virtual void draw() {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position if it's different
            clearPrevious();
            
            // Draw at new position
            mvaddch(currentY, currentX, symbol);
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            // Redraw at same position (in case it was overwritten)
            mvaddch(currentY, currentX, symbol);
        }
    }

    float getX() const { return x; }
    float getY() const { return y; }
    bool isActive() const { return active; }
    void setActive(bool state) { 
        if (active && !state) {
            // If turning from active to inactive, clear from screen
            clearPrevious();
        }
        active = state; 
    }
    void deactivate() {
        clearPrevious();
        refresh();
    }
    
    // Check collision with another game object
    bool collidesWith(const GameObject& other) const {
        int thisX = static_cast<int>(round(x));
        int thisY = static_cast<int>(round(y));
        int otherX = static_cast<int>(round(other.x));
        int otherY = static_cast<int>(round(other.y));
        
        return (thisX == otherX && thisY == otherY);
    }
};

class Heart : public GameObject {
private:
    float aspectRatio;    // Character aspect ratio (width/height)
    bool moving;          // Whether the heart is moving
    int health;           // Player health
    int score;            // Player score
    int invincibilityFrames; // Frames of invincibility after taking damage

public:
    Heart(int startX, int startY) : 
        GameObject(static_cast<float>(startX), static_cast<float>(startY), 0.0f, 0.0f, 0.3f, ACS_DIAMOND),
        aspectRatio(2.0f), moving(false), health(INITIAL_PLAYER_HEALTH), score(0), invincibilityFrames(0) {}

    void update() override {
        if (moving) {
            // Move in the current direction with aspect ratio compensation
            // Horizontal movement is sped up by the aspect ratio
            x += directionX * speed * aspectRatio;
            // Vertical movement stays at base speed
            y += directionY * speed;
        }
        
        // Decrease invincibility frames if player is invincible
        if (invincibilityFrames > 0) {
            invincibilityFrames--;
        }
    }

    void draw() override {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            // Clear previous position if it's different
            clearPrevious();
            
            // Draw at new position with appropriate color
            if (invincibilityFrames > 0 && invincibilityFrames % 2 == 0) {
                // Flashing effect during invincibility
                attron(COLOR_PAIR(2)); // Yellow for invincibility
            } else {
                attron(COLOR_PAIR(1)); // Red heart color
            }
            
            mvaddch(currentY, currentX, symbol);
            
            if (invincibilityFrames > 0 && invincibilityFrames % 2 == 0) {
                attroff(COLOR_PAIR(2));
            } else {
                attroff(COLOR_PAIR(1));
            }
            
            // Update last drawn position
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            // Redraw at same position (in case it was overwritten)
            if (invincibilityFrames > 0 && invincibilityFrames % 2 == 0) {
                attron(COLOR_PAIR(2));
            } else {
                attron(COLOR_PAIR(1));
            }
            
            mvaddch(currentY, currentX, symbol);
            
            if (invincibilityFrames > 0 && invincibilityFrames % 2 == 0) {
                attroff(COLOR_PAIR(2));
            } else {
                attroff(COLOR_PAIR(1));
            }
        }
    }

    void setDirection(float dx, float dy) {
        // Set a new direction vector
        if (dx != 0.0f || dy != 0.0f) {
            // Normalize the direction vector
            float length = sqrt(dx * dx + dy * dy);
            directionX = dx / length;
            directionY = dy / length;
            moving = true;  // Start moving when a direction is set
        }
    }
    
    void setAspectRatio(float ratio) {
        aspectRatio = ratio;
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

    void takeDamage(int amount) {
        if (invincibilityFrames <= 0) {
            health -= amount;
            if (health < 0) health = 0;
            invincibilityFrames = 30; // About half a second at 60FPS
        }
    }
    
    void addScore(int amount) {
        score += amount;
    }
    
    int getHealth() const { return health; }
    int getScore() const { return score; }
    float getAspectRatio() const { return aspectRatio; }
    bool isInvincible() const { return invincibilityFrames > 0; }
    float getDirectionX() const { return directionX; }
    float getDirectionY() const { return directionY; }
};

class Laser : public GameObject {
public:
    Laser(float startX, float startY, float dx, float dy) :
        GameObject(startX, startY, dx, dy, LASER_SPEED, '-') {}
    
    void draw() override {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        // Only redraw if position has changed
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            
            // Draw with cyan color
            attron(COLOR_PAIR(3)); // Cyan for laser
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(3));
            
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(3));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(3));
        }
    }
};

class Spaceship : public GameObject {
private:
    int health;
    int fireCooldown;
    int maxFireCooldown;
    bool reachedLeftEdge;

public:
    Spaceship(float startX, float startY, float dx, float dy) :
        GameObject(startX, startY, dx, dy, SPACESHIP_SPEED, 'C'),
        health(1),
        fireCooldown(0),
        maxFireCooldown(120),
        reachedLeftEdge(false) {}
    
    void update() override {
        GameObject::update();
        //Decrease cooldown if active
        if (fireCooldown > 0){
            fireCooldown--;
        }
    }

    bool canFire() const {
        return fireCooldown <= 0;
    }

    void resetFireCoolDown() {
        fireCooldown = maxFireCooldown;
    }

    void draw() override {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            
            // Draw with blue color
            attron(COLOR_PAIR(4)); // Blue for spaceships
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(4));
            
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(4));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(4));
        }
    }
    
    void takeDamage(int amount) {
        health -= amount;
        if (health <= 0) {
            active = false;
        }
    }
    
    // Check if spaceship has reached left edge
    bool hasReachedLeft(int leftEdge) const {
        return static_cast<int>(round(x)) <= leftEdge && !reachedLeftEdge;
    }
    
    void markReachedLeftEdge() {
        reachedLeftEdge = true;
    }
    
    int getHealth() const { return health; }
};

class Projectile : public GameObject {
public:
    Projectile(float startX, float startY, float dx, float dy) :
        GameObject(startX, startY, dx, dy, PROJECTILE_SPEED, '+') {}
        
    void draw() override {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            
            // Draw with green color
            attron(COLOR_PAIR(5)); // Green for projectiles
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(5));
            
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(5));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(5));
        }
    }
};

class Bomb : public GameObject {
private:
    int timer;

public:
    Bomb(float startX, float startY) :
        GameObject(startX, startY, 0.0f, 1.0f, BOMB_SPEED, 'O'),
        timer(120) {} // About 1 second at 60 FPS
        
    void update() override {
        GameObject::update();
        timer--;
        if (timer <= 0) {
            active = false; // Bomb "explodes" and disappears
        }
    }
    
    void draw() override {
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            
            // Draw with magenta color
            attron(COLOR_PAIR(6)); // Magenta for bombs
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(6));
            
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(6));
            mvaddch(currentY, currentX, symbol);
            attroff(COLOR_PAIR(6));
        }
    }
    
    int getTimer() const { return timer; }
};

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

    // Check if a position is inside the battle box (with a small margin)
    bool contains(float checkX, float checkY) const {
        int ix = static_cast<int>(round(checkX));
        int iy = static_cast<int>(round(checkY));
        return (ix > x && ix < x + width && iy > y && iy < y + height);
    }
    
    // Check if a position is outside the battle box (with a small margin)
    bool isOutside(float checkX, float checkY) const {
        return !contains(checkX, checkY);
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

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

// Runs a round of the game with behaviors specific to each round
std::vector<int> runRound(int round, int playerHealth) {
    
    // Define different parameters for each round
    int spaceship_spawn_interval; 
    int bomb_spawn_interval;
    int max_enemies;
    
    // Set difficulty based on round number
    switch(round) {
        case 1:
            spaceship_spawn_interval = 60;
            bomb_spawn_interval = -1; // No bombs in round 1
            max_enemies = 8;
            break;
        case 2:
            spaceship_spawn_interval = 50; // Faster spawning
            bomb_spawn_interval = 120;
            max_enemies = 10;
            break;
        case 3:
            spaceship_spawn_interval = 40; // Even faster spawning
            bomb_spawn_interval = 60;      // More bombs
            max_enemies = 12;
            break;
        default:
            spaceship_spawn_interval = 60;
            bomb_spawn_interval = -1;
            max_enemies = 8;
    }
    
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);  // Non-blocking input
    
    // Enable function keys, arrow keys, etc.
    keypad(stdscr, TRUE);
    
    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);     // Red heart
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Yellow for invincibility
        init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Cyan for laser
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Blue for spaceships
        init_pair(5, COLOR_GREEN, COLOR_BLACK);   // Green for projectiles
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Magenta for bombs
        init_pair(7, COLOR_WHITE, COLOR_BLACK);   // White for text
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create battle box and heart
    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    Heart heart(maxX/2, maxY/2);

    // Initialize heart with passed health and score values
    // Reset health to the passed value (carefully handling invincibility frames)
    heart.takeDamage(INITIAL_PLAYER_HEALTH - playerHealth);
    
    // Game objects
    std::vector<std::unique_ptr<Spaceship>> spaceships;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<Bomb>> bombs;
    std::vector<std::unique_ptr<Laser>> lasers;
    
    // Game state
    int frameCount = 0;
    bool gameOver = false;
    int resultCode = 0;
    
    // Draw the static elements once
    battleBox.draw();
    
    // Game loop
    bool running = true;
    while (running && !gameOver) {
        // Process all available input
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            } else if (ch == ' ') {
                // Space toggles movement
                if (heart.isMoving()) {
                    heart.stop();
                } else {
                    heart.start();
                }
            } else if (ch == KEY_UP) {
                heart.setDirection(0.0f, -1.0f);  // Up
            } else if (ch == KEY_DOWN) {
                heart.setDirection(0.0f, 1.0f);   // Down
            } else if (ch == KEY_LEFT) {
                heart.setDirection(-1.0f, 0.0f);  // Left
            } else if (ch == KEY_RIGHT) {
                heart.setDirection(1.0f, 0.0f);   // Right
            } else if (ch == 'f' || ch == 'F') {
                float dx = 1.0f;
                float dy = 0.0f;
                // Create and add a new laser (replaced make_unique with direct new)
                lasers.push_back(std::unique_ptr<Laser>(new Laser(
                    heart.getX(), heart.getY(), dx, dy
                )));
            }
        }
        
        // Update heart position
        heart.update();
        
        // Boundary checking to keep heart inside the battle box
        float heartX = heart.getX();
        float heartY = heart.getY();
        
        // Constrain position
        if (heartX < battleBox.getX() + 1) {
            heart.setPosition(static_cast<float>(battleBox.getX() + 1), heartY);
        }
        if (heartX > battleBox.getX() + battleBox.getWidth() - 1) {
            heart.setPosition(static_cast<float>(battleBox.getX() + battleBox.getWidth() - 1), heartY);
        }
        if (heartY < battleBox.getY() + 1) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getY() + 1));
        }
        if (heartY > battleBox.getY() + battleBox.getHeight() - 1) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getY() + battleBox.getHeight() - 1));
        }
        
        // Spawn new enemies at regular intervals
        frameCount++;
        // Spawn new enemies at regular intervals
        if (frameCount % spaceship_spawn_interval == 0 && 
            static_cast<int>(spaceships.size()) < max_enemies) {
            float startX, startY;
            float dirX, dirY;
            
            // Start from right edge
            startX = static_cast<float>(battleBox.getX() + battleBox.getWidth() - 1);
            // Random Y position
            startY = static_cast<float>(battleBox.getY() + 1 + std::rand() % (battleBox.getHeight() - 2));
            
            // Always move left (negative X direction)
            dirX = -1.0f;
            
            // Calculate maximum allowed vertical movement to ensure reaching left edge
            // Distance to left edge
            float distanceToLeftEdge = startX - (battleBox.getX() + 1);
            // Distance to top edge (how far we can go up)
            float distanceToTopEdge = startY - (battleBox.getY() + 1);
            // Distance to bottom edge (how far we can go down)
            float distanceToBottomEdge = (battleBox.getY() + battleBox.getHeight() - 1) - startY;
            
            // Maximum allowed vertical movement per horizontal unit to reach left edge
            float maxUpSlope = distanceToTopEdge / distanceToLeftEdge;
            float maxDownSlope = distanceToBottomEdge / distanceToLeftEdge;
            
            // Random vertical direction within bounds that ensure reaching left edge
            float verticalRange = std::min(maxUpSlope, 1.0f) + std::min(maxDownSlope, 1.0f);
            float normalizedPosition = static_cast<float>(std::rand()) / RAND_MAX; // 0.0 to 1.0
            
            // Convert to range from -maxUpSlope to +maxDownSlope
            dirY = -std::min(maxUpSlope, 1.0f) + normalizedPosition * verticalRange;
            
            // Normalize the direction vector
            float length = std::sqrt(dirX * dirX + dirY * dirY);
            dirX /= length;
            dirY /= length;
            
            // Create the spaceship (replaced make_unique with direct new)
            spaceships.push_back(std::unique_ptr<Spaceship>(new Spaceship(startX, startY, dirX, dirY)));
        }
        // Separately spawn bombs randomly from the top, not in round 1
        if (bomb_spawn_interval > 0 && frameCount % bomb_spawn_interval == 0 && 
            static_cast<int>(bombs.size()) < max_enemies / 2) {
            // Random x position along the top of the battle box
            float bombX = static_cast<float>(
                battleBox.getX() + 1 + std::rand() % (battleBox.getWidth() - 2)
            );
            float bombY = static_cast<float>(battleBox.getY() + 1); // Start at the top
            
            bombs.push_back(std::unique_ptr<Bomb>(new Bomb(bombX, bombY)));
        }
        
        // Randomly fire projectiles from spaceships
        for (auto& ship : spaceships) {
            if (ship->isActive() && ship->canFire() && ::rand() % 50 == 0) {
                // Direction towards player
                float dx = heart.getX() - ship->getX();
                float dy = heart.getY() - ship->getY();
                float length = std::sqrt(dx * dx + dy * dy);
                
                // Add some randomness to aim
                dx = dx / length + (std::rand() % 100 - 50) / 500.0f;
                dy = dy / length + (std::rand() % 100 - 50) / 500.0f;
                
                // Normalize again
                length = std::sqrt(dx * dx + dy * dy);
                dx /= length;
                dy /= length;
                
                projectiles.push_back(std::unique_ptr<Projectile>(new Projectile(
                    ship->getX(), ship->getY(), dx, dy
                )));

                //Reset cooldown
                ship->resetFireCoolDown();
            }
        }
        
        // Update all game objects
        
        // Update lasers
        for (auto& laser : lasers) {
            if (laser->isActive()) {
                laser->update();
                
                // Check if laser has left the battle box
                if (battleBox.isOutside(laser->getX(), laser->getY())) {
                    laser->setActive(false);
                    laser->deactivate();
                    continue;
                }

                bool collisionDetected = false;
                
                // Check for collisions with enemies
                for (auto& ship : spaceships) {
                    if (ship->isActive() && laser->collidesWith(*ship)) {
                        ship->takeDamage(LASER_DAMAGE);
                        laser->setActive(false);
                        collisionDetected = true;
                        if (!ship->isActive()) {
                            ship->deactivate();
                            heart.addScore(SCORE_PER_SPACESHIP);
                        }
                        break;
                    }
                }

                // Skip further checks if laser is no longer active
                if (!laser->isActive()) continue;
                // Skip further collision checks if a collision was already detected
                if (collisionDetected) continue;
                
                // Check for collisions with projectiles
                for (auto& proj : projectiles) {
                    if (proj->isActive() && laser->collidesWith(*proj)) {
                        proj->setActive(false);
                        laser->setActive(false);
                        heart.addScore(SCORE_PER_PROJECTILE); // Small score for destroying projectile
                        break;
                    }
                }
                
                // Skip further checks if laser is no longer active
                if (!laser->isActive()) continue;
                // Skip further collision checks if a collision was already detected
                if (collisionDetected) continue;
                
                // Check for collisions with bombs
                for (auto& bomb : bombs) {
                    if (bomb->isActive() && laser->collidesWith(*bomb)) {
                        bomb->setActive(false);
                        laser->setActive(false);
                        heart.addScore(SCORE_PER_BOMB); // Medium score for destroying bomb
                        break;
                    }
                }
            }
        }
        
        // Update spaceships
        for (auto& ship : spaceships) {
            if (ship->isActive()) {
                ship->update();
                
                // Check if ship has left the battle box
                if (battleBox.isOutside(ship->getX(), ship->getY())) {
                    ship->setActive(false);
                    continue;
                }
                
                // Check if ship has reached left edge of battlebox
                if (ship->hasReachedLeft(battleBox.getX() + 1)) {
                    heart.takeDamage(SPACESHIP_DAMAGE);
                    ship->markReachedLeftEdge();
                    ship->setActive(false);
                    continue;
                }
                
                // Check for collision with player
                if (ship->collidesWith(heart) && !heart.isInvincible()) {
                    heart.takeDamage(SPACESHIP_DAMAGE);
                    ship->setActive(false);
                }
            }
        }
        
        // Update projectiles
        for (auto& proj : projectiles) {
            if (proj->isActive()) {
                proj->update();
                
                // Check if projectile has left the battle box
                if (battleBox.isOutside(proj->getX(), proj->getY())) {
                    proj->setActive(false);
                    continue;
                }
                
                // Check for collision with player
                if (proj->collidesWith(heart) && !heart.isInvincible()) {
                    heart.takeDamage(1);
                    proj->setActive(false);
                }
            }
        }
        
        // Update bombs
        for (auto& bomb : bombs) {
            if (bomb->isActive()) {
                bomb->update();
                
                // Check if bomb has left the battle box
                if (battleBox.isOutside(bomb->getX(), bomb->getY())) {
                    bomb->setActive(false);
                    continue;
                }
                
                // Check for collision with player
                if (bomb->collidesWith(heart) && !heart.isInvincible()) {
                    heart.takeDamage(BOMB_DAMAGE);
                    bomb->setActive(false);
                }
            }
        }
        
        // Clean up inactive objects - replace auto with explicit types in lambdas
        lasers.erase(
            std::remove_if(
                lasers.begin(), lasers.end(),
                [](const std::unique_ptr<Laser>& laser) { return !laser->isActive(); }
            ),
            lasers.end()
        );
        
        spaceships.erase(
            std::remove_if(
                spaceships.begin(), spaceships.end(),
                [](const std::unique_ptr<Spaceship>& ship) { return !ship->isActive(); }
            ),
            spaceships.end()
        );
        
        projectiles.erase(
            std::remove_if(
                projectiles.begin(), projectiles.end(),
                [](const std::unique_ptr<Projectile>& proj) { return !proj->isActive(); }
            ),
            projectiles.end()
        );
        
        bombs.erase(
            std::remove_if(
                bombs.begin(), bombs.end(),
                [](const std::unique_ptr<Bomb>& bomb) { return !bomb->isActive(); }
            ),
            bombs.end()
        );
        
        // Check if game over conditions met
        if (heart.getHealth() <= 0) {
            gameOver = true;
            resultCode = -1; // Player died
        } else if (heart.getScore() >= 2000) {
            gameOver = true;
            resultCode = 1;  // Player won (reached score)
        }
        
        // Draw all game objects
        heart.draw();
        
        for (auto& laser : lasers) {
            if (laser->isActive()) {
                laser->draw();
            }
        }
        
        for (auto& ship : spaceships) {
            if (ship->isActive()) {
                ship->draw();
            }
        }
        
        for (auto& proj : projectiles) {
            if (proj->isActive()) {
                proj->draw();
            }
        }
        
        for (auto& bomb : bombs) {
            if (bomb->isActive()) {
                bomb->draw();
            }
        }
        
        // Display game stats
        attron(COLOR_PAIR(7)); // White text
        mvprintw(battleBox.getY() - 2, battleBox.getX(), "Round: %d  Score: %d", 
                round, heart.getScore());
                
        // Draw health bar on the same line after the score
        drawHealthBar(battleBox.getX(), battleBox.getY() - 1, INITIAL_PLAYER_HEALTH, heart.getHealth());

        attroff(COLOR_PAIR(7));
        
        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60 FPS (1,000,000 microseconds / 60)
    }
    
    // Game over screen
    if (gameOver) {
        clear();
        mvprintw(maxY / 2 - 2, maxX / 2 - 5, "ROUND %d %s", 
                 round, resultCode > 0 ? "COMPLETE" : "FAILED");
        mvprintw(maxY / 2, maxX / 2 - 10, "Score: %d", heart.getScore());
        mvprintw(maxY / 2 + 1, maxX / 2 - 10, "Health: %d", heart.getHealth());
        mvprintw(maxY / 2 + 3, maxX / 2 - 15, "Press ENTER to continue...");
        
        // Clear the input buffer by reading all pending input
        nodelay(stdscr, TRUE);  // Non-blocking mode
        while (getch() != ERR) {
            // Consume all pending input
        }
        // Now wait for the ENTER key specifically
        nodelay(stdscr, FALSE);  // Blocking mode
        int ch;
        do {
            ch = getch();
        } while (ch != '\n' && ch != KEY_ENTER && ch != '\r');
    }

    // Clean up
    endwin();
    
    // Return the result vector: [resultCode, health, score]
    std::vector<int> result = {resultCode, heart.getHealth(), heart.getScore()};
    return result;
}