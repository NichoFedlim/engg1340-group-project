#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>

class Heart {
    private:
        float x, y;           // Position with floating-point precision for smooth movement
        int lastDrawnX, lastDrawnY; // Last position where the heart was drawn
        float directionX, directionY; // Direction vector (normalized)
        float baseSpeed;      // Base movement speed
        float aspectRatio;    // Character aspect ratio (width/height)
        bool moving;          // Whether the heart is moving
        int symbol;           // Symbol to represent the heart
        int hp;               // Health points
        bool invincible;      // Invincibility state after being hit
        int invincibleTimer;  // Timer for invincibility frames
        int colorPair;        // Current color of the heart
        bool canBeForceStarted; // Whether the heart can be force-started by the game
        int safeQuadrant; // For tracking the safe quadrant in round 4
    
    public:
        Heart(int startX, int startY) : 
            x(static_cast<float>(startX)), quadrantWarningState(0), y(static_cast<float>(startY)), 
            lastDrawnX(startX), lastDrawnY(startY),
            directionX(0.0f), directionY(0.0f),
            baseSpeed(0.3f), aspectRatio(2.0f), // Assume character cells are about half as wide as they are tall
            moving(false), symbol(ACS_DIAMOND),
            hp(10), invincible(false), invincibleTimer(0), colorPair(1),
            canBeForceStarted(true) {}
    
        void update() {
            if (moving) {
                // Move in the current direction with aspect ratio compensation
                // Horizontal movement is sped up by the aspect ratio
                x += directionX * baseSpeed * aspectRatio;
                // Vertical movement stays at base speed
                y += directionY * baseSpeed;
            }
            
            // Update invincibility state
            if (invincible) {
                invincibleTimer--;
                if (invincibleTimer <= 0) {
                    invincible = false;
                    colorPair = 1; // Reset to red heart
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
        
        void setSpeed(float speed) {
            baseSpeed = speed;
        }
        
        void stop() {
            moving = false;
            canBeForceStarted = false; // Don't force start when explicitly stopped
        }
        
        void start() {
            moving = true;
            canBeForceStarted = true; // Allow force starting again
        }
        
        bool isMoving() const {
            return moving;
        }
        
        bool canForceStart() const {
            return canBeForceStarted;
        }
    
        void setPosition(float newX, float newY) {
            x = newX;
            y = newY;
        }
    
        void reset(int startX, int startY) {
            clearPrevious(); // Clear the heart from its current position
            x = static_cast<float>(startX);
            y = static_cast<float>(startY);
            lastDrawnX = startX;
            lastDrawnY = startY;
            directionX = 0.0f;
            directionY = 0.0f;
            moving = false;
            hp = 10;
            invincible = false;
            invincibleTimer = 0;
            colorPair = 1;
            canBeForceStarted = true;
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
                attron(COLOR_PAIR(colorPair)); // Current heart color
                mvaddch(currentY, currentX, symbol);
                attroff(COLOR_PAIR(colorPair));
                
                // Update last drawn position
                lastDrawnX = currentX;
                lastDrawnY = currentY;
            } else {
                // Redraw at same position (in case it was overwritten)
                attron(COLOR_PAIR(colorPair)); // Current heart color
                mvaddch(currentY, currentX, symbol);
                attroff(COLOR_PAIR(colorPair));
            }
        }
    
        bool takeDamage() {
            if (!invincible) {
                hp--;
                invincible = true;
                invincibleTimer = 10; // ~0.5 seconds at 60 FPS
                colorPair = 2; // Change to green while invincible
                return true;
            }
            return false;
        }
    
        int getHP() const { return hp; }
        bool isInvincible() const { return invincible; }
        float getX() const { return x; }
        float getY() const { return y; }
        int getIntX() const { return static_cast<int>(round(x)); }
        int getIntY() const { return static_cast<int>(round(y)); }
        float getDirectionX() const { return directionX; }
        float getDirectionY() const { return directionY; }
        float getAspectRatio() const { return aspectRatio; }
        float getSpeed() const { return baseSpeed; }
        bool isDead() const { return hp <= 0; }
        void setSafeQuadrant(int quadrant) { safeQuadrant = quadrant; }
        int getSafeQuadrant() const { return safeQuadrant; }
        int quadrantWarningState; // State for round 4 quadrant warnings
        void setQuadrantWarningState(int state) { quadrantWarningState = state; }
        int getQuadrantWarningState() const { return quadrantWarningState; }
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
    int getInnerMinX() const { return x + 1; }
    int getInnerMinY() const { return y + 1; }
    int getInnerMaxX() const { return x + width - 1; }
    int getInnerMaxY() const { return y + height - 1; }
    int getCenterX() const { return x + width / 2; }
    int getCenterY() const { return y + height / 2; }
};