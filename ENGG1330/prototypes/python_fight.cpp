#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <algorithm> // Add this for std::remove_if
#include "movement_v2.h"

class Heart {
private:
    float x, y;           // Position with floating-point precision for smooth movement
    int lastDrawnX, lastDrawnY; // Last position where the heart was drawn
    float directionX, directionY; // Direction vector (normalized)
    float baseSpeed;      // Base movement speed
    float aspectRatio;    // Character aspect ratio (width/height)
    bool moving;          // Whether the heart is moving
    int symbol;          // Symbol to represent the heart

public:
    Heart(int startX, int startY) : 
        x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
        lastDrawnX(startX), lastDrawnY(startY),
        directionX(0.0f), directionY(0.0f),
        baseSpeed(0.3f), aspectRatio(2.0f), // Assume character cells are about half as wide as they are tall
        moving(false), symbol(ACS_DIAMOND) {}

    void update() {
        if (moving) {
            // Move in the current direction with aspect ratio compensation
            // Horizontal movement is sped up by the aspect ratio
            x += directionX * baseSpeed * aspectRatio;
            // Vertical movement stays at base speed
            y += directionY * baseSpeed;
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
    
    void setSpeed(float speed) {
        baseSpeed = speed;
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
        mvaddch(lastDrawnY, lastDrawnX, ' ');
    }

    void draw() {
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

    float getX() const { return x; }
    float getY() const { return y; }
    float getDirectionX() const { return directionX; }
    float getDirectionY() const { return directionY; }
    float getAspectRatio() const { return aspectRatio; }
    float getSpeed() const { return baseSpeed; }
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

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// New class for player projectiles
class Projectile {
private:
    float x, y;
    int lastDrawnX, lastDrawnY;
    bool active;
    
public:
    Projectile(float startX, float startY) :
        x(startX), y(startY), lastDrawnX(static_cast<int>(startX)), 
        lastDrawnY(static_cast<int>(startY)), active(true) {}
    
    void update() {
        if (active) {
            // Move upward
            y -= 0.5f;
        }
    }
    
    void clearPrevious() {
        if (active) {
            mvaddch(lastDrawnY, lastDrawnX, ' ');
        }
    }
    
    void draw() {
        if (!active) return;
        
        int currentX = static_cast<int>(round(x));
        int currentY = static_cast<int>(round(y));
        
        if (currentX != lastDrawnX || currentY != lastDrawnY) {
            clearPrevious();
            
            // Draw projectile as a vertical bar
            attron(COLOR_PAIR(3)); // Cyan color for projectiles
            mvaddch(currentY, currentX, '|');
            attroff(COLOR_PAIR(3));
            
            lastDrawnX = currentX;
            lastDrawnY = currentY;
        } else {
            attron(COLOR_PAIR(3));
            mvaddch(currentY, currentX, '|');
            attroff(COLOR_PAIR(3));
        }
    }
    
    void deactivate() {
        if (active) {
            active = false;
            clearPrevious();
        }
    }
    
    bool isActive() const { return active; }
    float getX() const { return x; }
    float getY() const { return y; }
};

// New class for snake enemy
class Snake {
private:
    std::vector<std::pair<float, float>> body;
    int lastDrawnHeadX, lastDrawnHeadY;
    float speed;
    int direction; // 1 for right, -1 for left
    int health;
    bool active;
    
public:
    Snake(int boxX, int boxWidth, int boxY) : 
        speed(0.1f), direction(1), health(30), active(true) {
        
        // Start at the left side of the box
        float headX = boxX + 2;
        float headY = boxY + 3; // A bit below the top border
        
        // Create snake body (5 segments)
        for (int i = 0; i < 5; i++) {
            body.push_back(std::make_pair(headX - i, headY));
        }
        
        lastDrawnHeadX = static_cast<int>(headX);
        lastDrawnHeadY = static_cast<int>(headY);
    }
    
    void update(int boxX, int boxWidth) {
        if (!active) return;
        
        // Move the head
        float newHeadX = body[0].first + speed * direction;
        float newHeadY = body[0].second;
        
        // Check if snake hit the box boundaries
        if (newHeadX >= boxX + boxWidth - 1) {
            direction = -1; // Change direction to left
        } else if (newHeadX <= boxX + 1) {
            direction = 1;  // Change direction to right
        }
        
        // Update head position
        newHeadX = body[0].first + speed * direction;
        
        // Move the body (follow the head)
        for (int i = body.size() - 1; i > 0; i--) {
            body[i] = body[i-1];
        }
        
        // Update the head position
        body[0] = std::make_pair(newHeadX, newHeadY);
    }
    
    void clearPrevious() {
        for (const auto& segment : body) {
            mvaddch(static_cast<int>(segment.second), static_cast<int>(segment.first), ' ');
        }
    }
    
    void draw() {
        if (!active) return;
        
        clearPrevious();
        
        // Draw the head with a different symbol
        attron(COLOR_PAIR(2)); // Green color for snake
        
        for (size_t i = 0; i < body.size(); i++) {
            char symbol = (i == 0) ? '@' : '*'; // @ for head, * for body
            mvaddch(static_cast<int>(body[i].second), static_cast<int>(body[i].first), symbol);
        }
        
        attroff(COLOR_PAIR(2));
        
        lastDrawnHeadX = static_cast<int>(body[0].first);
        lastDrawnHeadY = static_cast<int>(body[0].second);
    }
    
    bool checkProjectileCollision(const Projectile& projectile) {
        if (!active || !projectile.isActive()) return false;
        
        // Only check collision with head
        int headX = static_cast<int>(round(body[0].first));
        int headY = static_cast<int>(round(body[0].second));
        int projX = static_cast<int>(round(projectile.getX()));
        int projY = static_cast<int>(round(projectile.getY()));
        
        if (headX == projX && headY == projY) {
            health--;
            if (health <= 0) {
                active = false;
            }
            return true;
        }
        
        return false;
    }
    
    bool checkHeartCollision(const Heart& heart) {
        if (!active) return false;
        
        int heartX = static_cast<int>(round(heart.getX()));
        int heartY = static_cast<int>(round(heart.getY()));
        
        // Check collision with any part of the snake
        for (const auto& segment : body) {
            int segX = static_cast<int>(round(segment.first));
            int segY = static_cast<int>(round(segment.second));
            
            if (heartX == segX && heartY == segY) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isActive() const { return active; }
    int getHealth() const { return health; }
};

void movement() {
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
        init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Green snake
        init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Cyan projectiles
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create battle box and heart
    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    Heart heart(maxX/2, maxY/2);
    
    // Create snake
    Snake snake(battleBox.getX(), battleBox.getWidth(), battleBox.getY());
    
    // Status line buffer to reduce flickering on text updates
    char statusLine[50] = "";  // Added this declaration
    char healthLine[50] = "";  // Added this declaration
    
    // Projectiles vector
    std::vector<Projectile> projectiles;
    
    // Player stats
    int playerHealth = 3;
    int playerScore = 0;
    int damageTimer = 0;
    
    // Draw the static elements once
    battleBox.draw();
    mvprintw(maxY - 4, 2, "Arrow keys to move, F to shoot, Space to stop/start");
    mvprintw(maxY - 3, 2, "Q to quit");
    
    // Game loop
    bool running = true;
    int frameCount = 0;
    
    while (running && playerHealth > 0) {
        frameCount++;
        
        // Process all available input
        int ch;
        bool statusChanged = false;
        
        while ((ch = getch()) != ERR) {
            statusChanged = true;
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
                // Fire a projectile
                projectiles.emplace_back(heart.getX(), heart.getY() - 1);
            }
        }
        
        // Update heart position
        float oldX = heart.getX();
        float oldY = heart.getY();
        heart.update();
        
        // Boundary checking to keep heart inside the battle box
        float heartX = heart.getX();
        float heartY = heart.getY();
        bool positionConstrained = false;
        
        // Constrain position
        if (heartX < battleBox.getX() + 1) {
            heart.setPosition(static_cast<float>(battleBox.getX() + 1), heartY);
            positionConstrained = true;
        }
        if (heartX > battleBox.getX() + battleBox.getWidth() - 1) {
            heart.setPosition(static_cast<float>(battleBox.getX() + battleBox.getWidth() - 1), heartY);
            positionConstrained = true;
        }
        if (heartY < battleBox.getY() + 1) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getY() + 1));
            positionConstrained = true;
        }
        if (heartY > battleBox.getY() + battleBox.getHeight() - 1) {
            heart.setPosition(heartX, static_cast<float>(battleBox.getY() + battleBox.getHeight() - 1));
            positionConstrained = true;
        }
        
        // Update snake
        if (frameCount % 2 == 0) { // Slow down snake movement
            snake.update(battleBox.getX(), battleBox.getWidth());
        }
        
        // Update projectiles
        for (auto& projectile : projectiles) {
            projectile.update();
            
            // Check if projectile is out of bounds
            if (projectile.getY() < battleBox.getY() + 1) {
                projectile.deactivate();
            }
            
            // Check if projectile hit the snake
            if (snake.checkProjectileCollision(projectile)) {
                projectile.deactivate();
                playerScore += 1;
                
                // Respawn snake if it's dead
                if (!snake.isActive() && playerHealth > 0) {
                    snake = Snake(battleBox.getX(), battleBox.getWidth(), battleBox.getY());
                    playerScore += 10; // Bonus for killing the snake
                }
            }
        }
        
        // Check if heart collided with snake
        if (damageTimer <= 0 && snake.checkHeartCollision(heart)) {
            playerHealth--;
            damageTimer = 60; // 1 second invincibility (at 60 FPS)
        }
        
        // Decrease damage timer
        if (damageTimer > 0) {
            damageTimer--;
        }
        
        // Remove inactive projectiles
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(), 
                           [](const Projectile& p) { return !p.isActive(); }),
            projectiles.end());
        
        // Draw everything
        heart.draw();
        snake.draw();
        
        for (auto& projectile : projectiles) {
            projectile.draw();
        }
        
        // Update status
        snprintf(statusLine, sizeof(statusLine), "Health: %d  Score: %d", playerHealth, playerScore);
        mvprintw(maxY - 2, maxX - 25, "%s", statusLine); // Fixed format string warning
        
        if (snake.isActive()) {
            snprintf(healthLine, sizeof(healthLine), "Snake Health: %d", snake.getHealth());
            mvprintw(maxY - 1, maxX - 25, "%s", healthLine); // Fixed format string warning
        } else {
            mvprintw(maxY - 1, maxX - 25, "Snake defeated!");
        }
        
        // Show invincibility status
        if (damageTimer > 0) {
            mvprintw(maxY - 1, 2, "Invincible: %d", damageTimer/10);
        } else {
            mvaddstr(maxY - 1, 2, "                  ");
        }

        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60 FPS (1,000,000 microseconds / 60)
    }
    
    // Game over screen
    if (playerHealth <= 0) {
        clear();
        mvprintw(maxY/2, maxX/2 - 5, "GAME OVER");
        mvprintw(maxY/2 + 1, maxX/2 - 8, "Final Score: %d", playerScore);
        mvprintw(maxY/2 + 3, maxX/2 - 13, "Press any key to exit");
        refresh();
        nodelay(stdscr, FALSE);
        getch();
    }

    // Clean up
    endwin();
}

int main(){
    movement();
    return 0;
}