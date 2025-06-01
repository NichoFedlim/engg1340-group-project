#include "1330.h"
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>

// Track the player's health between game states
static int lastHeartHP = 10; // Default starting HP
int getLastHP() {
    return lastHeartHP;
}

class Heart {
    private:
        float x, y;
        int lastDrawnX, lastDrawnY;
        float directionX, directionY;
        float baseSpeed;
        float aspectRatio;
        bool moving;
        int symbol;
        int hp;
        bool invincible;
        int invincibleTimer;
        int colorPair;
        bool canBeForceStarted;
        int safeQuadrant;
    
    public:
        Heart(int startX, int startY) : 
            x(static_cast<float>(startX)), y(static_cast<float>(startY)), 
            lastDrawnX(startX), lastDrawnY(startY),
            directionX(0.0f), directionY(0.0f),
            baseSpeed(0.3f), aspectRatio(2.0f),
            moving(false), symbol(ACS_DIAMOND),
            hp(10), invincible(false), invincibleTimer(0), colorPair(1),
            canBeForceStarted(true), safeQuadrant(0), quadrantWarningState(0) {}
    
        void update() {
            clearPrevious();
            if (moving) {
                x += directionX * baseSpeed * aspectRatio;
                y += directionY * baseSpeed;
            }
            
            // Handle invincibility frames after taking damage
            if (invincible) {
                invincibleTimer--;
                if (invincibleTimer <= 0) {
                    invincible = false;
                    colorPair = 1;
                }
            }
        }

        void setHP(int newHP) {
            hp = newHP;
        }
    
        void setDirection(float dx, float dy) {
            if (dx != 0.0f || dy != 0.0f) {
                float length = sqrt(dx * dx + dy * dy);
                directionX = dx / length;
                directionY = dy / length;
                moving = true;
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
            canBeForceStarted = false;
        }
        
        void start() {
            moving = true;
            canBeForceStarted = true;
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
    
        // Reset heart to initial state with given position
        void reset(int startX, int startY) {
            clearPrevious();
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
    
        // Clear the heart from previous position
        void clearPrevious() {
            mvaddch(lastDrawnY, lastDrawnX, ' ');
        }
    
        // Draw the heart at its current position
        void draw() {
            int currentX = static_cast<int>(round(x));
            int currentY = static_cast<int>(round(y));
            
            if (currentX != lastDrawnX || currentY != lastDrawnY) {
                clearPrevious();
                
                attron(COLOR_PAIR(colorPair));
                mvaddch(currentY, currentX, symbol);
                attroff(COLOR_PAIR(colorPair));
                
                lastDrawnX = currentX;
                lastDrawnY = currentY;
            } else {
                attron(COLOR_PAIR(colorPair));
                mvaddch(currentY, currentX, symbol);
                attroff(COLOR_PAIR(colorPair));
            }
        }
    
        // Handle damage and trigger invincibility frames
        bool takeDamage() {
            if (!invincible) {
                hp--;
                invincible = true;
                invincibleTimer = 10;
                colorPair = 2;
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
        int quadrantWarningState;
        void setQuadrantWarningState(int state) { quadrantWarningState = state; }
        int getQuadrantWarningState() const { return quadrantWarningState; }
    };


class BattleBox {
private:
    int x, y;
    int width, height;
    bool needsRedraw;

public:
    BattleBox(int startX, int startY, int w, int h) :
        x(startX), y(startY), width(w), height(h), needsRedraw(true) {}

    // Draw the battle box boundaries
    void draw() {
        attron(A_REVERSE);
    
        for (int i = -1; i <= width+1; i++) {
            mvaddch(y, x + i, ' ');
            mvaddch(y + height, x + i, ' ');
        }
    
        for (int i = 0; i <= height; i++) {
            mvaddch(y + i, x, ' ');
            mvaddch(y + i, x + width, ' ');
            mvaddch(y + i, x-1, ' ');
            mvaddch(y + i, x+1 + width, ' ');
        }
    
        attroff(A_REVERSE);
        
        needsRedraw = false;
    }

    void setNeedsRedraw() {
        needsRedraw = true;
    }

    // Accessor methods for box dimensions
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

class Snake {
private:
    std::deque<std::pair<int, int>> segments;
    int length;
    int symbol;
    int colorPair;
    int updateDelay;
    int updateDelayThreshold;
    int collisionCooldown;
    bool active;

public:
    Snake(int startX, int startY, int len) : 
        length(len), symbol(ACS_BLOCK), colorPair(5), updateDelay(0), 
        updateDelayThreshold(6), collisionCooldown(0), active(false) {
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
    }

    // Update snake position to follow the heart
    void update(int heartX, int heartY) {
        if (!active) return;
        
        updateDelay++;
        
        if (collisionCooldown > 0) {
            collisionCooldown--;
        }
        
        if (updateDelay >= updateDelayThreshold) {
            updateDelay = 0;
            
            int headX = segments.front().first;
            int headY = segments.front().second;
            
            int dx = 0, dy = 0;
            
            // Determine direction to move toward heart
            if (heartX > headX) dx = 1;
            else if (heartX < headX) dx = -1;
            
            if (heartY > headY) dy = 1;
            else if (heartY < headY) dy = -1;
            
            // Prevent diagonal movement
            if (dx != 0 && dy != 0) {
                if (rand() % 2 == 0) {
                    dy = 0;
                } else {
                    dx = 0;
                }
            }
            
            segments.push_front(std::make_pair(headX + dx, headY + dy));
            
            if (static_cast<int>(segments.size()) > length) {
                segments.pop_back();
            }
        }
    }

    void draw() {
        if (!active) return;
        
        for (size_t i = 0; i < segments.size(); i++) {
            attron(COLOR_PAIR(colorPair));
            char segmentChar = (i == 0) ? 'O' : symbol;
            mvaddch(segments[i].second, segments[i].first, segmentChar);
            attroff(COLOR_PAIR(colorPair));
        }
    }

    void clear() {
        if (!active) return;
        
        for (auto& segment : segments) {
            mvaddch(segment.second, segment.first, ' ');
        }
    }

    // Check if snake collides with heart
    bool checkCollision(const Heart& heart) {
        if (!active || collisionCooldown > 0) return false;
        
        int heartX = heart.getIntX();
        int heartY = heart.getIntY();
        
        for (auto& segment : segments) {
            if (segment.first == heartX && segment.second == heartY) {
                collisionCooldown = 60;
                return true;
            }
        }
        
        return false;
    }

    void reset(int startX, int startY) {
        clear();
        segments.clear();
        
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
        
        updateDelay = 0;
        collisionCooldown = 0;
    }
    
    void setActive(bool isActive) {
        active = isActive;
    }
    
    bool isActive() const {
        return active;
    }
};


class Laser {
private:
    int startX, startY;
    int endX, endY;
    int warningTimer;
    int activeTimer;
    int colorPair;
    bool active;
    bool warning;
    LaserDirection direction;
    char symbol;
    
    float projectilePos;
    float projectileSpeed;
    std::vector<std::pair<int, int>> trail;
    std::vector<int> trailTimers;
    int maxTrailLength;
    
    int boxMinX, boxMinY, boxMaxX, boxMaxY;

public:
    Laser(int sX, int sY, int eX, int eY, LaserDirection dir, 
        int minX, int minY, int maxX, int maxY) :
        startX(sX), startY(sY), endX(eX), endY(eY),
        warningTimer(0), activeTimer(60),
        colorPair(3), active(false), warning(false),
        direction(dir), symbol('*'),
        projectilePos(0.0f), projectileSpeed(0.02f),
        maxTrailLength(std::max(abs(eX - sX), abs(eY - sY)) + 1),
        boxMinX(minX), boxMinY(minY), boxMaxX(maxX), boxMaxY(maxY) {}

    // Activate the laser and prepare for firing
    void activate() {
        warning = false;
        active = true;
        activeTimer = 60;
        trail.clear();
        trailTimers.clear();
        projectilePos = 0.0f;
    }

    // Update laser position and trail
    void update() {
        if (active) {
            activeTimer--;
            
            projectilePos += projectileSpeed;
            
            if (projectilePos <= 1.0f) {
                int x, y;
                getPointOnPath(projectilePos, x, y);
                
                if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                    if (trail.empty() || trail.back().first != x || trail.back().second != y) {
                        trail.push_back(std::make_pair(x, y));
                        trailTimers.push_back(60);
                    }
                }
            }
            
            if (activeTimer <= 0 || projectilePos > 1.0f) {
                active = false;
            }
        }
        
        // Update trail timers and remove old trail segments
        for (size_t i = 0; i < trailTimers.size(); i++) {
            trailTimers[i]--;
            
            if (trailTimers[i] <= 0) {
                mvaddch(trail[i].second, trail[i].first, ' ');
                
                trail.erase(trail.begin() + i);
                trailTimers.erase(trailTimers.begin() + i);
                i--;
            }
        }
    }

    void clear() {
        if (!trail.empty()) {
            for (const auto& point : trail) {
                mvaddch(point.second, point.first, ' ');
            }
        }
    }

    // Draw laser and its trail
    void draw() {
        if (active) {
            for (size_t i = 0; i < trail.size(); i++) {
                int colorIntensity = 3;
                if (i < trail.size() / 2) {
                    colorIntensity = 6;
                }
                
                attron(COLOR_PAIR(colorIntensity));
                mvaddch(trail[i].second, trail[i].first, '*');
                attroff(COLOR_PAIR(colorIntensity));
            }
            
            if (projectilePos <= 1.0f) {
                int x, y;
                getPointOnPath(projectilePos, x, y);
                
                if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                    attron(COLOR_PAIR(7));
                    
                    // Draw different symbols based on laser direction
                    if (direction == LaserDirection::HORIZONTAL || direction == LaserDirection::VERTICAL) {
                        mvaddch(y, x, 'R');
                    } else if (direction == LaserDirection::DIAGONAL_DOWN || direction == LaserDirection::DIAGONAL_UP) {
                        mvaddch(y, x, 'B');
                    } else if (direction == LaserDirection::KNIGHT) {
                        mvaddch(y, x, 'N');
                    }
                    
                    attroff(COLOR_PAIR(7));
                }
            }
        }
    }

    // Check collision between laser and heart
    bool checkCollision(const Heart& heart) {
        if (!active && trail.empty()) return false;
    
        int heartX = heart.getIntX();
        int heartY = heart.getIntY();
    
        for (const auto& point : trail) {
            if (point.first == heartX && point.second == heartY) {
                return true;
            }
        }
    
        if (active && projectilePos <= 1.0f) {
            int x, y;
            getPointOnPath(projectilePos, x, y);
    
            if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                if (x == heartX && y == heartY) {
                    return true;
                }
            }
        }
    
        return false;
    }

    bool isActive() const { return active; }
    bool isWarning() const { return warning; }
    bool isComplete() const { return !active && trail.empty(); }
    void setActiveTimer(int frames) { activeTimer = frames; }

    void updateEndPoint(int newEndX, int newEndY) {
        endX = newEndX;
        endY = newEndY;
    }

private:
    // Calculate point along the laser path based on interpolation parameter t
    void getPointOnPath(float t, int& x, int& y) {
        x = static_cast<int>(std::round(startX + t * (endX - startX)));
        y = static_cast<int>(std::round(startY + t * (endY - startY)));
    }
};

class Knight {
private:
    int x, y;
    int spawnTimer;
    bool active;
    int symbol;
    int colorPair;
    std::vector<Laser> lasers;
    
    int boxMinX, boxMinY, boxMaxX, boxMaxY;
    
public:
    Knight(int minX, int minY, int maxX, int maxY) : 
        x(0), y(0), spawnTimer(0), active(false), symbol('N'), colorPair(7),
        boxMinX(minX), boxMinY(minY), boxMaxX(maxX), boxMaxY(maxY) {}
    
    // Spawn knight at random position in battle box
    void spawn(const BattleBox& box) {
        int minX = box.getInnerMinX();
        int minY = box.getInnerMinY();
        int maxX = box.getInnerMaxX();
        int maxY = box.getInnerMaxY();
        
        boxMinX = minX;
        boxMinY = minY;
        boxMaxX = maxX;
        boxMaxY = maxY;
        
        x = minX + rand() % (maxX - minX + 1);
        y = minY + rand() % (maxY - minY + 1);
        
        active = true;
        
        createKnightLasers();
    }
    
    void update() {
        if (!active) return;
        
        for (auto& laser : lasers) {
            laser.update();
        }
        
        // Deactivate knight when all lasers are complete
        bool allComplete = true;
        for (const auto& laser : lasers) {
            if (!laser.isComplete()) {
                allComplete = false;
                break;
            }
        }
        
        if (allComplete && !lasers.empty()) {
            active = false;
            lasers.clear();
        }
    }
    
    void draw() {
        if (!active) return;
        
        attron(COLOR_PAIR(colorPair));
        mvaddch(y, x, symbol);
        attroff(COLOR_PAIR(colorPair));
        
        for (auto& laser : lasers) {
            laser.draw();
        }
    }
    
    void clear() {
        if (!active) return;
        
        mvaddch(y, x, ' ');
        
        for (auto& laser : lasers) {
            laser.clear();
        }
    }
    
    bool checkCollision(const Heart& heart) {
        if (!active) return false;
        
        for (auto& laser : lasers) {
            if (laser.checkCollision(heart)) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isActive() const {
        return active;
    }
    
private:
    // Create L-shaped laser patterns in 8 possible knight-move directions
    void createKnightLasers() {
        lasers.clear();
        
        const int knightMoves[8][2] = {
            {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, 
            {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
        };
        
        for (int i = 0; i < 8; i++) {
            int targetX = x + knightMoves[i][0];
            int targetY = y + knightMoves[i][1];
            
            if (targetX >= boxMinX && targetX <= boxMaxX && 
                targetY >= boxMinY && targetY <= boxMaxY) {
                Laser laser(x, y, targetX, targetY, LaserDirection::KNIGHT,
                           boxMinX, boxMinY, boxMaxX, boxMaxY);
                laser.activate();
                lasers.push_back(laser);
            }
        }
    }
};

// Draw health bar UI at specified position
void drawHealthBar(int x, int y, int maxHP, int currentHP) {
    mvprintw(y, x, "HP: %d/%d [", currentHP, maxHP);
    
    int barWidth = 20;
    int filledWidth = static_cast<int>((static_cast<float>(currentHP) / maxHP) * barWidth);
    
    for (int i = 0; i < barWidth; i++) {
        if (i < filledWidth) {
            attron(COLOR_PAIR(1));
            addch('=');
            attroff(COLOR_PAIR(1));
        } else {
            addch('-');
        }
    }
    addch(']');
}

// Create horizontal laser patterns with specified count
std::vector<Laser> createHorizontalLines(const BattleBox& box, int count, int playerY) {
    std::vector<Laser> lasers;

    int innerMinX = box.getInnerMinX()-1;
    int innerMaxX = box.getInnerMaxX();
    int innerMinY = box.getInnerMinY();
    int innerMaxY = box.getInnerMaxY();

    int middleLineY = playerY;
    if (middleLineY < innerMinY) middleLineY = innerMinY;
    if (middleLineY > innerMaxY) middleLineY = innerMaxY;

    int lineSpacing = 2;

    lasers.emplace_back(
        innerMinX, middleLineY,
        innerMaxX, middleLineY,
        LaserDirection::HORIZONTAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );

    int numSideLines = count / 2;
    for (int i = 1; i <= numSideLines; i++) {
        int topLineY = middleLineY - i * lineSpacing;
        if (topLineY >= innerMinY) {
            lasers.emplace_back(
                innerMinX, topLineY,
                innerMaxX, topLineY,
                LaserDirection::HORIZONTAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }

        int bottomLineY = middleLineY + i * lineSpacing;
        if (bottomLineY <= innerMaxY) {
            lasers.emplace_back(
                innerMinX, bottomLineY,
                innerMaxX, bottomLineY,
                LaserDirection::HORIZONTAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }
    }

    return lasers;
}

// Create vertical laser patterns with specified count
std::vector<Laser> createVerticalLines(const BattleBox& box, int count, int playerX) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMaxX = box.getInnerMaxX();
    int innerMinY = box.getInnerMinY();
    int innerMaxY = box.getInnerMaxY();
    
    int middleLineX = playerX;
    
    if (middleLineX < innerMinX) middleLineX = innerMinX;
    if (middleLineX > innerMaxX) middleLineX = innerMaxX;
    
    int lineSpacing = 2;
    
    lasers.emplace_back(
        middleLineX, innerMinY,
        middleLineX, innerMaxY,
        LaserDirection::VERTICAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    int numSideLines = count / 2;
    for (int i = 1; i <= numSideLines; i++) {
        int leftLineX = middleLineX - i * lineSpacing;
        if (leftLineX >= innerMinX) {
            lasers.emplace_back(
                leftLineX, innerMinY,
                leftLineX, innerMaxY,
                LaserDirection::VERTICAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }
        
        int rightLineX = middleLineX + i * lineSpacing;
        if (rightLineX <= innerMaxX) {
            lasers.emplace_back(
                rightLineX, innerMinY,
                rightLineX, innerMaxY,
                LaserDirection::VERTICAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }
    }
    
    return lasers;
}

// Create grid pattern of lasers (tic-tac-toe style)
std::vector<Laser> createTicTacToePattern(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int boxWidth = box.getWidth();
    int boxHeight = box.getHeight();
    int innerMinX = box.getInnerMinX()-1;
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    int thirdWidth = boxWidth / 3;
    int thirdHeight = boxHeight / 3;
    
    int line1Y = innerMinY + thirdHeight;
    int line2Y = innerMinY + 2 * thirdHeight;
    
    lasers.emplace_back(
        innerMinX, line1Y,
        innerMaxX, line1Y,
        LaserDirection::HORIZONTAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    lasers.emplace_back(
        innerMinX, line2Y,
        innerMaxX, line2Y,
        LaserDirection::HORIZONTAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    int line1X = innerMinX + thirdWidth;
    int line2X = innerMinX + 2 * thirdWidth;
    
    lasers.emplace_back(
        line1X, innerMinY,
        line1X, innerMaxY,
        LaserDirection::VERTICAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    lasers.emplace_back(
        line2X, innerMinY,
        line2X, innerMaxY,
        LaserDirection::VERTICAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    return lasers;
}

// Create X-shaped crossing diagonal lasers
std::vector<Laser> createXCrossPattern(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    lasers.emplace_back(
        innerMinX, innerMinY,
        innerMaxX, innerMaxY,
        LaserDirection::DIAGONAL_DOWN,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    lasers.emplace_back(
        innerMaxX, innerMinY,
        innerMinX, innerMaxY,
        LaserDirection::DIAGONAL_UP,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    return lasers;
}

// Create attack that targets three quadrants, leaving one safe
std::vector<Laser> createQuadrantAttack(const BattleBox& box, int safeQuadrant) {
    std::vector<Laser> lasers;

    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    int midX = (innerMinX + innerMaxX + 1) / 2;
    int midY = (innerMinY + innerMaxY + 1) / 2;

    int laserDuration = 60;

    bool attackTopLeft = (safeQuadrant != 0);
    bool attackTopRight = (safeQuadrant != 1);
    bool attackBottomLeft = (safeQuadrant != 2);
    bool attackBottomRight = (safeQuadrant != 3);

    // Create lasers for each quadrant that should be attacked
    if (attackTopLeft) {
        for (int y = innerMinY; y < midY; ++y) {
            lasers.emplace_back(innerMinX, y, midX-1, y, LaserDirection::HORIZONTAL, 
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
        
        for (int x = innerMinX; x < midX; ++x) {
            lasers.emplace_back(x, innerMinY, x, midY-1, LaserDirection::VERTICAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
    }
    
    if (attackTopRight) {
        for (int y = innerMinY; y < midY; ++y) {
            lasers.emplace_back(midX, y, innerMaxX, y, LaserDirection::HORIZONTAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
        
        for (int x = midX; x <= innerMaxX; ++x) {
            lasers.emplace_back(x, innerMinY, x, midY-1, LaserDirection::VERTICAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
    }
    
    if (attackBottomLeft) {
        for (int y = midY; y <= innerMaxY; ++y) {
            lasers.emplace_back(innerMinX, y, midX-1, y, LaserDirection::HORIZONTAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
        
        for (int x = innerMinX; x < midX; ++x) {
            lasers.emplace_back(x, midY, x, innerMaxY, LaserDirection::VERTICAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
    }
    
    if (attackBottomRight) {
        for (int y = midY; y <= innerMaxY; ++y) {
            lasers.emplace_back(midX, y, innerMaxX, y, LaserDirection::HORIZONTAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
        
        for (int x = midX; x <= innerMaxX; ++x) {
            lasers.emplace_back(x, midY, x, innerMaxY, LaserDirection::VERTICAL,
                              innerMinX, innerMinY, innerMaxX, innerMaxY);
            lasers.back().activate();
            lasers.back().setActiveTimer(laserDuration);
        }
    }

    return lasers;
}

// Display visual warnings in quadrants that will be attacked
void drawDangerQuadrantWarnings(const BattleBox& box, int safeQuadrant) {
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();

    int midX = (innerMinX + innerMaxX + 1) / 2;
    int midY = (innerMinY + innerMaxY + 1) / 2;

    for (int q = 0; q < 4; ++q) {
        if (q == safeQuadrant) continue;

        int quadMinX, quadMaxX, quadMinY, quadMaxY;
        switch (q) {
            case 0:
                quadMinX = innerMinX;
                quadMaxX = midX - 1;
                quadMinY = innerMinY;
                quadMaxY = midY - 1;
                break;
            case 1:
                quadMinX = midX;
                quadMaxX = innerMaxX;
                quadMinY = innerMinY;
                quadMaxY = midY - 1;
                break;
            case 2:
                quadMinX = innerMinX;
                quadMaxX = midX - 1;
                quadMinY = midY;
                quadMaxY = innerMaxY;
                break;
            case 3:
                quadMinX = midX;
                quadMaxX = innerMaxX;
                quadMinY = midY;
                quadMaxY = innerMaxY;
                break;
        }
        if (quadMinX > quadMaxX || quadMinY > quadMaxY)
            continue;

        int centerX = (quadMinX + quadMaxX) / 2;
        int centerY = (quadMinY + quadMaxY) / 2;

        attron(COLOR_PAIR(4));
        mvprintw(centerY, centerX-1, "!!!");
        attroff(COLOR_PAIR(4));
    }
}

// Clean up all game elements
void clearAllElements(std::vector<Laser>& lasers, Snake& snake, Knight& knight) {
    for (auto& laser : lasers) {
        laser.clear();
    }
    lasers.clear();

    snake.clear();
    knight.clear();
    refresh();
}

// Display message and wait for Enter key press
void waitForEnter(const char* message, int x, int y) {
    move(y, x);
    clrtoeol();
    
    mvprintw(y, x, "%s", message);
    refresh();
    
    nodelay(stdscr, FALSE);
    
    int ch;
    do {
        ch = getch();
    } while (ch != '\n' && ch != KEY_ENTER && ch != 10 && ch != 13);
    
    nodelay(stdscr, TRUE);
    
    move(y, x);
    clrtoeol();
}

// Main game loop for a single round
bool runRound(int round, int initialHP) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    Heart heart(maxX/2, maxY/2);
    
    // Set the heart's HP to the passed value
    heart.setHP(initialHP);

    int boxMinX = battleBox.getInnerMinX();
    int boxMinY = battleBox.getInnerMinY();
    int boxMaxX = battleBox.getInnerMaxX();
    int boxMaxY = battleBox.getInnerMaxY();

    Snake snake(maxX/2 - 10, maxY/2, 5);
    Knight knight(boxMinX, boxMinY, boxMaxX, boxMaxY);
    int knightSpawnTimer = 0;

    bool running = true;
    bool inAttackCycle = false;
    std::vector<Laser> lasers;
    int messageY = maxY/2;
    int waveTimer = 0;
    int currentWave = 0;
    int maxWaves = 0;
    int randomAttackType = 0;
    bool roundCompleted = false;
    int initialDelay = 180;
    bool gameStarted = false;
    int newSafeQuadrant = 0;

    battleBox.draw();
    mvprintw(maxY - 3, 2, "Arrow keys to set direction, Space to stop/start");

    heart.setDirection(1.0f, 0.0f);
    heart.start();


    while (running) {
        if (!gameStarted) {
            mvprintw(messageY, maxX/2 - 17, "Get ready! Starting in %d seconds...", (initialDelay + 59) / 60);
            refresh();

            initialDelay--;
            if (initialDelay <= 0) {
                gameStarted = true;
                mvprintw(messageY, maxX/2 - 17, "                                  ");
            } else {
                usleep(16667);
                continue;
            }
        }

        if (heart.isDead()) {
            clearAllElements(lasers, snake, knight);
            return false;
        }

        if (roundCompleted) {
            clearAllElements(lasers, snake, knight);
            return true;
        }

        if (!inAttackCycle && !roundCompleted) {
            inAttackCycle = true;
            currentWave = 0;
            waveTimer = 0;

            // Initialize round-specific attack patterns
            switch (round) {
                case 1:
                    maxWaves = 3;
                    lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                    for (auto& laser : lasers) laser.activate();
                    break;
                case 2:
                    maxWaves = 3;
                    lasers = createTicTacToePattern(battleBox);
                    for (auto& laser : lasers) laser.activate();
                    break;
                case 3:
                    maxWaves = 5;
                    if (currentWave == 0 && waveTimer == 0) {
                        randomAttackType = rand() % 4;
                        if (randomAttackType == 0) {
                            lasers = createTicTacToePattern(battleBox);
                        } else if (randomAttackType == 1) {
                            lasers = createXCrossPattern(battleBox);
                        } else if (randomAttackType == 2) {
                            lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                        } else {
                            lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                        }
                        for (auto& laser : lasers) laser.activate();
                    }
                    break;
                case 4:
                    maxWaves = 4;
                    newSafeQuadrant = rand() % 4;
                    heart.setSafeQuadrant(newSafeQuadrant);
                    heart.setQuadrantWarningState(0);
                    break;
                case 5:
                    maxWaves = 4;
                    snake.setActive(true);
                    randomAttackType = rand() % 4;
                    if (randomAttackType == 0) {
                        std::vector<Laser> tt1 = createTicTacToePattern(battleBox);
                        std::vector<Laser> tt2 = createTicTacToePattern(battleBox);
                        lasers = tt1;
                        lasers.insert(lasers.end(), tt2.begin(), tt2.end());
                    } else if (randomAttackType == 1) {
                        std::vector<Laser> d1 = createXCrossPattern(battleBox);
                        std::vector<Laser> d2 = createXCrossPattern(battleBox);
                        lasers = d1;
                        lasers.insert(lasers.end(), d2.begin(), d2.end());
                    } else if (randomAttackType == 2) {
                        lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                    } else {
                        lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                    }
                    for (auto& laser : lasers) laser.activate();
                    break;
                case 6:
                    maxWaves = 6;
                    randomAttackType = rand() % 4;
                    if (randomAttackType == 0) {
                        std::vector<Laser> tt1 = createTicTacToePattern(battleBox);
                        std::vector<Laser> tt2 = createTicTacToePattern(battleBox);
                        lasers = tt1;
                        lasers.insert(lasers.end(), tt2.begin(), tt2.end());
                    } else if (randomAttackType == 1) {
                        std::vector<Laser> d1 = createXCrossPattern(battleBox);
                        std::vector<Laser> d2 = createXCrossPattern(battleBox);
                        lasers = d1;
                        lasers.insert(lasers.end(), d2.begin(), d2.end());
                    } else if (randomAttackType == 2) {
                        lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                    } else {
                        lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                    }
                    for (auto& laser : lasers) laser.activate();
                    knightSpawnTimer = 0;
                    break;
                case 7:
                    maxWaves = 8;  // Increase from 6 to 8 for more waves
                    lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                    for (auto& laser : lasers) laser.activate();
                    snake.setActive(true);
                    knight.spawn(battleBox);  // Spawn a knight from the start
                    break;
            }
            heart.start();
        }

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                return false;
            } else if (ch == ' ') {
                if (heart.isMoving()) heart.stop();
                else heart.start();
            } else if (ch == KEY_UP) {
                heart.setDirection(0.0f, -1.0f); heart.start();
            } else if (ch == KEY_DOWN) {
                heart.setDirection(0.0f, 1.0f); heart.start();
            } else if (ch == KEY_LEFT) {
                heart.setDirection(-1.0f, 0.0f); heart.start();
            } else if (ch == KEY_RIGHT) {
                heart.setDirection(1.0f, 0.0f); heart.start();
            }
        }

        // Auto-start with current direction if heart can be force started
        if (!heart.isMoving() && heart.canForceStart()) {
            float dirX = heart.getDirectionX();
            float dirY = heart.getDirectionY();
            if (dirX == 0.0f && dirY == 0.0f) {
                heart.setDirection(1.0f, 0.0f);
            }
            heart.start();
        }

        heart.update();

        // Special handling for knight in round 6
        if (round == 6 && inAttackCycle) {
            knightSpawnTimer++;
            if (knightSpawnTimer >= 60) {
                knightSpawnTimer = 0;
                if (knight.isActive()) knight.clear();
                knight.spawn(battleBox);
            }
            knight.update();
            if (knight.checkCollision(heart)) heart.takeDamage();
        }

        // Handle snake movement and collision
        if (snake.isActive()) {
            snake.clear();
            snake.update(heart.getIntX(), heart.getIntY());
            if (snake.checkCollision(heart)) heart.takeDamage();
        }

        // Keep heart within battle box boundaries
        float heartX = heart.getX();
        float heartY = heart.getY();
        if (heartX < battleBox.getInnerMinX())
            heart.setPosition(static_cast<float>(battleBox.getInnerMinX()), heartY);
        if (heartX > battleBox.getInnerMaxX())
            heart.setPosition(static_cast<float>(battleBox.getInnerMaxX()), heartY);
        if (heartY < battleBox.getInnerMinY())
            heart.setPosition(heartX, static_cast<float>(battleBox.getInnerMinY()));
        if (heartY > battleBox.getInnerMaxY())
            heart.setPosition(heartX, static_cast<float>(battleBox.getInnerMaxY()));

        if (inAttackCycle) {
            waveTimer++;
            switch (round) {
                case 1:
                    if (currentWave == 0 && waveTimer >= 60) {
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave = 1;
                        waveTimer = 0;
                        lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                        for (auto& laser : lasers) laser.activate();
                    } else if (currentWave == 1 && waveTimer >= 150) {
                        for (auto& laser : lasers) laser.clear();
                        currentWave = 2;
                        waveTimer = 0;
                        lasers = createVerticalLines(battleBox, 5, heart.getIntX());
                        for (auto& laser : lasers) laser.activate();
                    }
                    break;
                case 2:
                    if (currentWave == 0 && waveTimer >= 60) {
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave = 1;
                        waveTimer = 0;
                        lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                        for (auto& laser : lasers) laser.activate();
                    } else if (currentWave == 1 && waveTimer >= 150) {
                        for (auto& laser : lasers) laser.clear();
                        currentWave = 2;
                        waveTimer = 0;
                        lasers = createTicTacToePattern(battleBox);
                        for (auto& laser : lasers) laser.activate();
                    } else if (currentWave == 2 && waveTimer == 60) {
                        std::vector<Laser> diagonalLasers = createXCrossPattern(battleBox);
                        for (auto& laser : diagonalLasers) {
                            laser.activate();
                            lasers.push_back(laser);
                        }
                    }
                    break;
                case 3:
                    if (waveTimer >= 120 && currentWave < maxWaves - 1) {
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        randomAttackType = rand() % 4;
                        if (randomAttackType == 0) {
                            lasers = createTicTacToePattern(battleBox);
                        } else if (randomAttackType == 1) {
                            lasers = createXCrossPattern(battleBox);
                        } else if (randomAttackType == 2) {
                            lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                        } else {
                            lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                        }
                        for (auto& laser : lasers) laser.activate();
                    }
                    break;
                case 4:
                    if (heart.getQuadrantWarningState() == 0) {
                        drawDangerQuadrantWarnings(battleBox, heart.getSafeQuadrant());
                
                        // Progressively faster warning times based on wave number
                        int warningTime = 120 - (currentWave * 25);  // 120, 100, 80, 60
                
                        if (waveTimer >= warningTime) {
                            int innerMinX = battleBox.getInnerMinX();
                            int innerMinY = battleBox.getInnerMinY();
                            int innerMaxX = battleBox.getInnerMaxX();
                            int innerMaxY = battleBox.getInnerMaxY();
                            int midX = (innerMinX + innerMaxX + 1) / 2;
                            int midY = (innerMinY + innerMaxY + 1) / 2;
                
                            for (int q = 0; q < 4; ++q) {
                                if (q == heart.getSafeQuadrant()) continue;
                
                                int centerX, centerY;
                                switch (q) {
                                    case 0: centerX = (innerMinX + midX - 1) / 2; centerY = (innerMinY + midY - 1) / 2; break;
                                    case 1: centerX = (midX + innerMaxX) / 2; centerY = (innerMinY + midY - 1) / 2; break;
                                    case 2: centerX = (innerMinX + midX - 1) / 2; centerY = (midY + innerMaxY) / 2; break;
                                    case 3: centerX = (midX + innerMaxX) / 2; centerY = (midY + innerMaxY) / 2; break;
                                    default: centerX = -1; centerY = -1;
                                }
                                if (centerX != -1) {
                                    mvaddstr(centerY, centerX - 1, "   ");
                                }
                            }
                
                            lasers = createQuadrantAttack(battleBox, heart.getSafeQuadrant());
                
                            heart.setQuadrantWarningState(1);
                            waveTimer = 0;
                        }
                    }
                    else if (heart.getQuadrantWarningState() == 1) {
                        bool allLasersComplete = true;
                        for (auto& laser : lasers) {
                            if (!laser.isComplete()) {
                                allLasersComplete = false;
                                break;
                            }
                        }
                        
                        // Progressively faster laser durations based on wave number
                        int laserDuration = 180 - (currentWave * 35);  // 180, 160, 140, 120
                    
                        if (allLasersComplete || waveTimer >= laserDuration) {
                            for (auto& laser : lasers) laser.clear();
                            lasers.clear();
                    
                            if (currentWave >= maxWaves - 1) {
                                inAttackCycle = false;
                                roundCompleted = true;
                            } else {
                                heart.setQuadrantWarningState(2);
                                waveTimer = 0;
                            }
                        }
                    }
                    else if (heart.getQuadrantWarningState() == 2) {
                        // Progressively faster recovery times based on wave number
                        int recoveryTime = 60 - (currentWave * 12);  // 60, 50, 40, 30
                        
                        if (waveTimer >= recoveryTime) {
                            currentWave++;
                
                            int previousSafeQuadrant = heart.getSafeQuadrant();
                            do {
                                newSafeQuadrant = rand() % 4;
                            } while (newSafeQuadrant == previousSafeQuadrant);
                            heart.setSafeQuadrant(newSafeQuadrant);
                
                            heart.setQuadrantWarningState(0);
                            waveTimer = 0;
                        }
                    }
                    break;
                case 5:
                    if (waveTimer >= 120 && currentWave < maxWaves - 1) {
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        randomAttackType = rand() % 4;
                        if (randomAttackType == 0) {
                            std::vector<Laser> tt1 = createTicTacToePattern(battleBox);
                            std::vector<Laser> tt2 = createTicTacToePattern(battleBox);
                            lasers = tt1;
                            lasers.insert(lasers.end(), tt2.begin(), tt2.end());
                        } else if (randomAttackType == 1) {
                            std::vector<Laser> d1 = createXCrossPattern(battleBox);
                            std::vector<Laser> d2 = createXCrossPattern(battleBox);
                            lasers = d1;
                            lasers.insert(lasers.end(), d2.begin(), d2.end());
                        } else if (randomAttackType == 2) {
                            lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                        } else {
                            lasers = createVerticalLines(battleBox, 3, heart.getIntX());
                        }
                        for (auto& laser : lasers) laser.activate();
                    }
                    break;
                case 6:
                    if (waveTimer >= 60 && currentWave < maxWaves - 1) {
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        
                        if (currentWave == 1) {
                            lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        } else if (currentWave == 2) {
                            lasers = createXCrossPattern(battleBox);
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        } else if (currentWave == 3) {
                            std::vector<Laser> horizontal = createHorizontalLines(battleBox, 2, heart.getIntY());
                            std::vector<Laser> vertical = createVerticalLines(battleBox, 2, heart.getIntX());
                            lasers = horizontal;
                            lasers.insert(lasers.end(), vertical.begin(), vertical.end());
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        }
                    }
                    break; // Added break to fix fall-through
                case 7:
                    if (currentWave < maxWaves - 1 && waveTimer >= 60) {
                        for (auto& laser : lasers) laser.clear();
                        currentWave++;
                        waveTimer = 0;
                        switch (currentWave) {
                            case 1: lasers = createVerticalLines(battleBox, 3, heart.getIntX()); break;
                            case 2: 
                                {
                                    // Horizontal and vertical lines together
                                    std::vector<Laser> horizontal = createHorizontalLines(battleBox, 3, heart.getIntY());
                                    std::vector<Laser> vertical = createVerticalLines(battleBox, 3, heart.getIntX());
                                    lasers = horizontal;
                                    lasers.insert(lasers.end(), vertical.begin(), vertical.end());
                                    knight.spawn(battleBox); // Spawn a new knight
                                }
                                break;
                            case 3: lasers = createTicTacToePattern(battleBox); break;
                            case 4: 
                                {
                                    lasers = createXCrossPattern(battleBox); 
                                    knight.spawn(battleBox); // Spawn a new knight
                                }
                                break;
                            case 5: lasers = createHorizontalLines(battleBox, 5, heart.getIntY()); break;
                            case 6: 
                                {
                                    std::vector<Laser> horizontal = createHorizontalLines(battleBox, 3, heart.getIntY() - 2);
                                    std::vector<Laser> vertical = createVerticalLines(battleBox, 3, heart.getIntX() - 2);
                                    lasers = horizontal;
                                    lasers.insert(lasers.end(), vertical.begin(), vertical.end());
                                    knight.spawn(battleBox); // Spawn a new knight
                                }
                                break;
                            case 7: 
                                {
                                    // Final wave with more complex pattern
                                    std::vector<Laser> diagonals = createXCrossPattern(battleBox);
                                    std::vector<Laser> grid = createTicTacToePattern(battleBox);
                                    lasers = diagonals;
                                    lasers.insert(lasers.end(), grid.begin(), grid.end());
                                    knight.spawn(battleBox); // Spawn a new knight
                                }
                                break;
                        }
                        for (auto& laser : lasers) laser.activate();
                    }
                    
                    // Always update and check knight collision regardless of current wave
                    if (knight.isActive()) {
                        knight.update();
                        if (knight.checkCollision(heart)) heart.takeDamage();
                    }
                    break;
            }

            // Update lasers and check for collisions
            for (auto& laser : lasers) {
                laser.update();
                if (laser.checkCollision(heart)) {
                    heart.takeDamage();
                }
            }

            // Check if round is complete (except for quadrant-based round 4)
            if (round != 4) {
                bool allLasersComplete = true;
                for (auto& laser : lasers) {
                    if (!laser.isComplete()) {
                        allLasersComplete = false;
                        break;
                    }
                }
                if (allLasersComplete && waveTimer > 60 && currentWave == maxWaves - 1) {
                    if (knight.isActive()) knight.clear();
                    inAttackCycle = false;
                    roundCompleted = true;
                    for (auto& laser : lasers) laser.clear();
                    lasers.clear();
                }
            }
        }

        battleBox.draw();

        if ((round == 6 || round == 7) && inAttackCycle && knight.isActive()) {
            knight.draw();
        }

        for (auto& laser : lasers) {
            laser.draw();
        }

        if (snake.isActive()) {
            snake.draw();
        }
        heart.draw();

        drawHealthBar(2, maxY - 4, 10, heart.getHP());

        mvprintw(2, 2, "Round: %d/7", round);

        refresh();
        usleep(16667);  // ~60 fps timing
        lastHeartHP = heart.getHP();
    }

    lastHeartHP = heart.getHP();
    return false;
}