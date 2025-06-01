#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>
#include "python.h"

// Forward declarations
class Laser;
class BattleBox;
class Snake;
class Heart;
class Knight;

// Snake class that follows the heart
class Snake {
private:
    std::deque<std::pair<int, int>> segments; // Snake segments positions
    int length;                               // Length of the snake
    int symbol;                               // Symbol to represent the snake
    int colorPair;                            // Color of the snake
    int updateDelay;                          // Movement delay counter
    int updateDelayThreshold;                 // How many frames to wait before updating
    int collisionCooldown;                    // Cooldown after collision
    bool active;                              // Whether the snake is active in the game

public:
    Snake(int startX, int startY, int len) : 
        length(len), symbol(ACS_BLOCK), colorPair(5), updateDelay(0), 
        updateDelayThreshold(6), collisionCooldown(0), active(false) { // Initially inactive, set active after 5th round
        // Initialize all segments at the same starting position
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
    }

    void update(int heartX, int heartY) {
        if (!active) return;  // Skip update if snake is not active
        
        updateDelay++;
        
        // Update cooldown
        if (collisionCooldown > 0) {
            collisionCooldown--;
        }
        
        // Only update every few frames to create a delay effect
        if (updateDelay >= updateDelayThreshold) { // Increased threshold for slower movement
            updateDelay = 0;
            
            // Move head towards heart
            int headX = segments.front().first;
            int headY = segments.front().second;
            
            // Calculate direction
            int dx = 0, dy = 0;
            
            if (heartX > headX) dx = 1;
            else if (heartX < headX) dx = -1;
            
            if (heartY > headY) dy = 1;
            else if (heartY < headY) dy = -1;
            
            // Slow down diagonal movement by only moving in one direction at a time
            // when moving diagonally (alternating between horizontal and vertical)
            if (dx != 0 && dy != 0) {
                // Choose either horizontal or vertical movement based on frame count
                if (rand() % 2 == 0) {
                    dy = 0; // Only move horizontally this frame
                } else {
                    dx = 0; // Only move vertically this frame
                }
            }
            
            // Add new head position
            segments.push_front(std::make_pair(headX + dx, headY + dy));
            
            // Remove tail if snake is too long
            if (segments.size() > length) {
                segments.pop_back();
            }
        }
    }

    void draw() {
        if (!active) return;  // Skip drawing if snake is not active
        
        for (size_t i = 0; i < segments.size(); i++) {
            attron(COLOR_PAIR(colorPair));
            // Use different character for head
            char segmentChar = (i == 0) ? 'O' : symbol;
            mvaddch(segments[i].second, segments[i].first, segmentChar);
            attroff(COLOR_PAIR(colorPair));
        }
    }

    void clear() {
        if (!active) return;  // Skip clearing if snake is not active
        
        for (auto& segment : segments) {
            mvaddch(segment.second, segment.first, ' ');
        }
    }

    bool checkCollision(const Heart& heart) {
        if (!active || collisionCooldown > 0) return false;
        
        int heartX = heart.getIntX();
        int heartY = heart.getIntY();
        
        // Check if heart collides with any snake segment
        for (auto& segment : segments) {
            if (segment.first == heartX && segment.second == heartY) {
                collisionCooldown = 60; // 1 second cooldown
                return true;
            }
        }
        
        return false;
    }

    void reset(int startX, int startY) {
        clear(); // Clear the snake from the screen
        segments.clear();
        
        // Initialize all segments at the same starting position
        for (int i = 0; i < length; i++) {
            segments.push_back(std::make_pair(startX, startY));
        }
        
        updateDelay = 0;
        collisionCooldown = 0;
    }
    
    // Set whether the snake is active
    void setActive(bool isActive) {
        active = isActive;
    }
    
    // Check if snake is active
    bool isActive() const {
        return active;
    }
};

// Laser class for the attack patterns
enum class LaserDirection {
    HORIZONTAL,
    VERTICAL,
    DIAGONAL_DOWN, // Top-left to bottom-right
    DIAGONAL_UP,   // Bottom-left to top-right
    KNIGHT         // Knight move pattern in chess
};

class Laser {
private:
    int startX, startY;       // Starting point of the laser
    int endX, endY;           // End point of the laser
    int warningTimer;         // Timer for warning animation before the laser fires
    int activeTimer;          // Timer for how long the laser is active
    int colorPair;            // Color of the laser
    bool active;              // Whether the laser is currently active
    bool warning;             // Whether the laser is in warning phase
    LaserDirection direction; // Direction of the laser
    char symbol;              // Symbol to draw the laser with
    
    
    // Variables for projectile behavior
    float projectilePos;      // Position of the projectile (0.0 to 1.0)
    float projectileSpeed;    // Speed of the projectile
    std::vector<std::pair<int, int>> trail; // Trail left behind by projectile
    std::vector<int> trailTimers; // Timers for each trail segment
    int maxTrailLength;       // Maximum length of the trail
    
    // Box boundaries to prevent cutting through
    int boxMinX, boxMinY, boxMaxX, boxMaxY;

public:
    Laser(int sX, int sY, int eX, int eY, LaserDirection dir, 
        int minX, int minY, int maxX, int maxY) :
        startX(sX), startY(sY), endX(eX), endY(eY),
        warningTimer(0), activeTimer(60), // Change from 150 to 60 (~1 second active)
        colorPair(3), active(false), warning(false),
        direction(dir), symbol('*'),
        projectilePos(0.0f), projectileSpeed(0.02f), // Move 2% of the path per frame
        maxTrailLength(std::max(abs(eX - sX), abs(eY - sY)) + 1), // ~1 second at 60 FPS
        boxMinX(minX), boxMinY(minY), boxMaxX(maxX), boxMaxY(maxY) {}

    void activate() {
        warning = false; // No warning phase
        active = true;
        activeTimer = 60; // Change from 150 to 60 (~1 second active at 60 FPS)
        trail.clear();
        trailTimers.clear(); // Clear trail timers too
        projectilePos = 0.0f;
    }
    void update() {
        if (active) {
            activeTimer--;
            
            // Update projectile position
            projectilePos += projectileSpeed;
            
            // Add current projectile position to trail
            if (projectilePos <= 1.0f) {
                int x, y;
                getPointOnPath(projectilePos, x, y);
                
                // Check if point is within box boundaries
                if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                    // Only add if it's a new position
                    if (trail.empty() || trail.back().first != x || trail.back().second != y) {
                        trail.push_back(std::make_pair(x, y));
                        trailTimers.push_back(60); // Each trail segment lasts 1 second (60 frames)
                    }
                }
            }
            
            if (activeTimer <= 0 || projectilePos > 1.0f) {
                active = false;
                // Do NOT clear the trail or timers here!
                // Let them fade out naturally below.
            }
        }
        
        // Update timers for each trail segment and remove expired ones
        for (int i = 0; i < trailTimers.size(); i++) {
            trailTimers[i]--;
            
            // Remove expired trail segments
            if (trailTimers[i] <= 0) {
                // Clear this segment from the screen
                mvaddch(trail[i].second, trail[i].first, ' ');
                
                // Remove from vectors
                trail.erase(trail.begin() + i);
                trailTimers.erase(trailTimers.begin() + i);
                i--; // Adjust index since we removed an element
            }
        }
    }

    void clear() {
        // Clear the laser from the screen
        if (!trail.empty()) {
            for (const auto& point : trail) {
                mvaddch(point.second, point.first, ' ');
            }
        }
    }

    void draw() {
        if (active) {
            // Draw the trail
            for (size_t i = 0; i < trail.size(); i++) {
                // Gradient effect - newer trail segments are brighter
                int colorIntensity = 3; // Default to bright cyan
                if (i < trail.size() / 2) {
                    colorIntensity = 6; // Dimmer color for older trail segments
                }
                
                attron(COLOR_PAIR(colorIntensity));
                mvaddch(trail[i].second, trail[i].first, '*');
                attroff(COLOR_PAIR(colorIntensity));
            }
            
            // Draw the projectile head
            if (projectilePos <= 1.0f) {
                int x, y;
                getPointOnPath(projectilePos, x, y);
                
                // Only draw if within box boundaries
                if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                    attron(COLOR_PAIR(7)); // Bright white for projectile head
                    
                    // Use different symbols based on direction
                    if (direction == LaserDirection::HORIZONTAL || direction == LaserDirection::VERTICAL) {
                        mvaddch(y, x, 'R'); // Rook for horizontal/vertical
                    } else if (direction == LaserDirection::DIAGONAL_DOWN || direction == LaserDirection::DIAGONAL_UP) {
                        mvaddch(y, x, 'B'); // Bishop for diagonal
                    } else if (direction == LaserDirection::KNIGHT) {
                        mvaddch(y, x, 'N'); // Knight for knight moves
                    }
                    
                    attroff(COLOR_PAIR(7));
                }
            }
        }
    }

    bool checkCollision(const Heart& heart) {
        // If laser is not active and trail is empty, no collision possible
        if (!active && trail.empty()) return false;
    
        int heartX = heart.getIntX();
        int heartY = heart.getIntY();
    
        // Check collision with trail
        for (const auto& point : trail) {
            if (point.first == heartX && point.second == heartY) {
                return true;  // Heart collided with the trail
            }
        }
    
        // Check collision with the projectile head (only if still on its path)
        if (active && projectilePos <= 1.0f) {
            int x, y;
            getPointOnPath(projectilePos, x, y);
    
            // Ensure the point is within the box boundaries before checking collision
            if (x >= boxMinX && x <= boxMaxX && y >= boxMinY && y <= boxMaxY) {
                if (x == heartX && y == heartY) {
                    return true;  // Heart collided with the projectile head
                }
            }
        }
    
        return false;  // No collision detected
    }

    bool isActive() const { return active; }
    bool isWarning() const { return warning; }
    bool isComplete() const { return !active && trail.empty(); }
    void setActiveTimer(int frames) { activeTimer = frames; }

    // Change end point (for following player)
    void updateEndPoint(int newEndX, int newEndY) {
        endX = newEndX;
        endY = newEndY;
    }

private:
    void getPointOnPath(float t, int& x, int& y) {
        // Calculate point at position t (0.0 to 1.0) along the path
        x = static_cast<int>(std::round(startX + t * (endX - startX)));
        y = static_cast<int>(std::round(startY + t * (endY - startY)));
    }
};

// Knight class that spawns randomly and shoots lasers in knight pattern
class Knight {
private:
    int x, y;                // Position
    int spawnTimer;          // Timer for spawning
    bool active;             // Whether the knight is active
    int symbol;              // Symbol to represent the knight
    int colorPair;           // Color of the knight
    std::vector<Laser> lasers; // Lasers shot by the knight
    
    // Box boundaries
    int boxMinX, boxMinY, boxMaxX, boxMaxY;
    
public:
    Knight(int minX, int minY, int maxX, int maxY) : 
        x(0), y(0), spawnTimer(0), active(false), symbol('N'), colorPair(7),
        boxMinX(minX), boxMinY(minY), boxMaxX(maxX), boxMaxY(maxY) {}
    
    void spawn(const BattleBox& box) {
        // Get box bounds
        int minX = box.getInnerMinX();
        int minY = box.getInnerMinY();
        int maxX = box.getInnerMaxX();
        int maxY = box.getInnerMaxY();
        
        // Update boundaries
        boxMinX = minX;
        boxMinY = minY;
        boxMaxX = maxX;
        boxMaxY = maxY;
        
        // Pick a random position inside the box
        x = minX + rand() % (maxX - minX + 1);
        y = minY + rand() % (maxY - minY + 1);
        
        active = true;
        
        // Create knight move lasers
        createKnightLasers();
    }
    
    void update() {
        if (!active) return;
        
        // Update existing lasers
        for (auto& laser : lasers) {
            laser.update();
        }
        
        // Check if all lasers are complete, then deactivate knight
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
        
        // Draw the knight
        attron(COLOR_PAIR(colorPair));
        mvaddch(y, x, symbol);
        attroff(COLOR_PAIR(colorPair));
        
        // Draw all lasers
        for (auto& laser : lasers) {
            laser.draw();
        }
    }
    
    void clear() {
        if (!active) return;
        
        // Clear knight and lasers
        mvaddch(y, x, ' ');
        
        for (auto& laser : lasers) {
            laser.clear();
        }
    }
    
    bool checkCollision(const Heart& heart) {
        if (!active) return false;
        
        // Check laser collisions
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
    void createKnightLasers() {
        lasers.clear();
        
        // All 8 possible knight moves (4 units away, 2 units next to it)
        const int knightMoves[8][2] = {
            {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, 
            {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
        };
        
        // Create lasers for all knight moves
        for (int i = 0; i < 8; i++) {
            int targetX = x + knightMoves[i][0];
            int targetY = y + knightMoves[i][1];
            
            // Add laser if target is within box boundaries
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

// Function to draw health bar
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

// Functions to create attack patterns
std::vector<Laser> createHorizontalLines(const BattleBox& box, int count, int playerY) {
    std::vector<Laser> lasers;

    int innerMinX = box.getInnerMinX()-1;
    int innerMaxX = box.getInnerMaxX();
    int innerMinY = box.getInnerMinY();
    int innerMaxY = box.getInnerMaxY();

    // Ensure playerY is within box boundaries
    int middleLineY = playerY;
    if (middleLineY < innerMinY) middleLineY = innerMinY;
    if (middleLineY > innerMaxY) middleLineY = innerMaxY;

    // Line spacing (distance between adjacent lines)
    int lineSpacing = 2;

    // Create the middle horizontal laser
    lasers.emplace_back(
        innerMinX, middleLineY,  // Start at the leftmost boundary
        innerMaxX, middleLineY,  // End at the rightmost boundary
        LaserDirection::HORIZONTAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );

    // Create lines above and below the middle line
    int numSideLines = count / 2;
    for (int i = 1; i <= numSideLines; i++) {
        // Line above
        int topLineY = middleLineY - i * lineSpacing;
        if (topLineY >= innerMinY) {
            lasers.emplace_back(
                innerMinX, topLineY,
                innerMaxX, topLineY,
                LaserDirection::HORIZONTAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }

        // Line below
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

std::vector<Laser> createVerticalLines(const BattleBox& box, int count, int playerX) {
    std::vector<Laser> lasers;
    
    int boxHeight = box.getHeight();
    int innerMinX = box.getInnerMinX();
    int innerMaxX = box.getInnerMaxX();
    int innerMinY = box.getInnerMinY();
    int innerMaxY = box.getInnerMaxY();
    
    // Make middle line follow player position
    int middleLineX = playerX;
    
    // Ensure we're within box boundaries
    if (middleLineX < innerMinX) middleLineX = innerMinX;
    if (middleLineX > innerMaxX) middleLineX = innerMaxX;
    
    // Calculate offset for other lines (2 tiles away)
    int lineSpacing = 2;
    
    // Create the middle vertical laser
    lasers.emplace_back(
        middleLineX, innerMinY,
        middleLineX, innerMaxY,
        LaserDirection::VERTICAL,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    // Create lines to the left and right of the middle one
    int numSideLines = count / 2;
    for (int i = 1; i <= numSideLines; i++) {
        // Line to the left
        int leftLineX = middleLineX - i * lineSpacing;
        if (leftLineX >= innerMinX) {
            lasers.emplace_back(
                leftLineX, innerMinY,
                leftLineX, innerMaxY,
                LaserDirection::VERTICAL,
                innerMinX, innerMinY, innerMaxX, innerMaxY
            );
        }
        
        // Line to the right
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

std::vector<Laser> createTicTacToePattern(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int boxWidth = box.getWidth();
    int boxHeight = box.getHeight();
    int innerMinX = box.getInnerMinX()-1;
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    // Calculate positions for the grid lines
    int thirdWidth = boxWidth / 3;
    int thirdHeight = boxHeight / 3;
    
    // Horizontal lines
    int line1Y = innerMinY + thirdHeight;
    int line2Y = innerMinY + 2 * thirdHeight;
    
    // Create horizontal lasers (----)
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
    
    // Create vertical lasers (|)
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

std::vector<Laser> createXCrossPattern(const BattleBox& box) {
    std::vector<Laser> lasers;
    
    int innerMinX = box.getInnerMinX()-1;
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    // Create diagonal lasers (\)
    lasers.emplace_back(
        innerMinX, innerMinY,
        innerMaxX, innerMaxY,
        LaserDirection::DIAGONAL_DOWN,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    // Create diagonal lasers (/)
    lasers.emplace_back(
        innerMaxX, innerMinY,
        innerMinX, innerMaxY,
        LaserDirection::DIAGONAL_UP,
        innerMinX, innerMinY, innerMaxX, innerMaxY
    );
    
    return lasers;
}

std::vector<Laser> createQuadrantAttack(const BattleBox& box, int safeQuadrant) {
    std::vector<Laser> lasers;

    int innerMinX = box.getInnerMinX()-1;
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();
    
    // Calculate midpoints consistently with warning function
    int midX = (innerMinX + innerMaxX) / 2;
    int midY = (innerMinY + innerMaxY) / 2;

    // Calculate half widths/heights
    int halfWidth = (innerMaxX - innerMinX + 1) / 2;
    int halfHeight = (innerMaxY - innerMinY + 1) / 2;

    // Lasers last for 1 second (60 frames)
    int laserDuration = 60;

    // Horizontal laser 1 (top half, from minY to midY-1)
    std::vector<Laser> h1;
    for (int y = innerMinY; y < midY; ++y) {
        h1.emplace_back(innerMinX, y, innerMaxX, y, LaserDirection::HORIZONTAL, innerMinX, innerMinY, innerMaxX, innerMaxY);
        h1.back().activate();
        h1.back().setActiveTimer(laserDuration);
    }

    // Horizontal laser 2 (bottom half, from midY to maxY)
    std::vector<Laser> h2;
    for (int y = midY; y <= innerMaxY; ++y) {
        h2.emplace_back(innerMinX, y, innerMaxX, y, LaserDirection::HORIZONTAL, innerMinX, innerMinY, innerMaxX, innerMaxY);
        h2.back().activate();
        h2.back().setActiveTimer(laserDuration);
    }

    // Vertical laser 3 (left half, from minX to midX-1)
    std::vector<Laser> v3;
    for (int x = innerMinX; x < midX; ++x) {
        v3.emplace_back(x, innerMinY, x, innerMaxY, LaserDirection::VERTICAL, innerMinX, innerMinY, innerMaxX, innerMaxY);
        v3.back().activate();
        v3.back().setActiveTimer(laserDuration);
    }

    // Vertical laser 4 (right half, from midX to maxX)
    std::vector<Laser> v4;
    for (int x = midX; x <= innerMaxX; ++x) {
        v4.emplace_back(x, innerMinY, x, innerMaxY, LaserDirection::VERTICAL, innerMinX, innerMinY, innerMaxX, innerMaxY);
        v4.back().activate();
        v4.back().setActiveTimer(laserDuration);
    }

    // Activate the right pairs based on safe quadrant
    // 0: Top-left safe → fire h2, v4
    // 1: Top-right safe → fire h2, v3
    // 2: Bottom-left safe → fire h1, v4
    // 3: Bottom-right safe → fire h1, v3

    switch (safeQuadrant) {
        case 0: // Top-left safe
            lasers.insert(lasers.end(), h2.begin(), h2.end());
            lasers.insert(lasers.end(), v4.begin(), v4.end());
            break;
        case 1: // Top-right safe
            lasers.insert(lasers.end(), h2.begin(), h2.end());
            lasers.insert(lasers.end(), v3.begin(), v3.end());
            break;
        case 2: // Bottom-left safe
            lasers.insert(lasers.end(), h1.begin(), h1.end());
            lasers.insert(lasers.end(), v4.begin(), v4.end());
            break;
        case 3: // Bottom-right safe
            lasers.insert(lasers.end(), h1.begin(), h1.end());
            lasers.insert(lasers.end(), v3.begin(), v3.end());
            break;
    }
    return lasers;
}

// Function to draw a warning in the safe quadrant
// Modified function to draw warnings for dangerous quadrants
void drawDangerQuadrantWarnings(const BattleBox& box, int safeQuadrant) {
    int innerMinX = box.getInnerMinX();
    int innerMinY = box.getInnerMinY();
    int innerMaxX = box.getInnerMaxX();
    int innerMaxY = box.getInnerMaxY();

    // Calculate midpoints
    int midX = (innerMinX + innerMaxX) / 2;
    int midY = (innerMinY + innerMaxY) / 2;

    // Quadrant boundaries:
    // 0: Top-left     (innerMinX ... midX, innerMinY ... midY)
    // 1: Top-right    (midX+1 ... innerMaxX, innerMinY ... midY)
    // 2: Bottom-left  (innerMinX ... midX, midY+1 ... innerMaxY)
    // 3: Bottom-right (midX+1 ... innerMaxX, midY+1 ... innerMaxY)

    // Calculate the exact center for each quadrant
    int qx[4], qy[4];
    // Top-left
    qx[0] = (innerMinX + midX) / 2;
    qy[0] = (innerMinY + midY) / 2;
    // Top-right
    qx[1] = (midX + 1 + innerMaxX) / 2;
    qy[1] = (innerMinY + midY) / 2;
    // Bottom-left
    qx[2] = (innerMinX + midX) / 2;
    qy[2] = (midY + 1 + innerMaxY) / 2;
    // Bottom-right
    qx[3] = (midX + 1 + innerMaxX) / 2;
    qy[3] = (midY + 1 + innerMaxY) / 2;

    for (int q = 0; q < 4; ++q) {
        if (q == safeQuadrant) continue;
        attron(COLOR_PAIR(4)); // Warning color (yellow)
        mvprintw(qy[q], qx[q] - 1, "!!!");
        attroff(COLOR_PAIR(4));
    }
}

void clearAllElements(std::vector<Laser>& lasers, Snake& snake, Knight& knight) {
    for (auto& laser : lasers) {
        laser.clear();
    }
    lasers.clear();

    snake.clear();
    knight.clear();
    refresh();
}
// Function to wait for Enter key
void waitForEnter(const char* message, int x, int y) {
    // Clear the line first
    move(y, x);
    clrtoeol();
    
    // Display message
    mvprintw(y, x, "%s", message);
    refresh();
    
    // Temporarily make getch blocking
    nodelay(stdscr, FALSE);
    
    // Wait for Enter key
    int ch;
    do {
        ch = getch();
    } while (ch != '\n' && ch != KEY_ENTER);
    
    // Reset to non-blocking
    nodelay(stdscr, TRUE);
    
    // Clear the message
    move(y, x);
    clrtoeol();
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    nodelay(stdscr, TRUE);  // Non-blocking input

    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));

    // Set up colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);      // Red heart (normal)
        init_pair(2, COLOR_GREEN, COLOR_BLACK);    // Green heart (invincible)
        init_pair(3, COLOR_CYAN, COLOR_BLACK);     // Bright cyan laser (active)
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);   // Yellow laser (warning)
        init_pair(5, COLOR_GREEN, COLOR_BLACK);    // Green snake
        init_pair(6, COLOR_BLUE, COLOR_BLACK);     // Dimmer blue for older trail
        init_pair(7, COLOR_WHITE, COLOR_BLACK);    // White for projectile head
    }

    // Get terminal dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create battle box and heart
    BattleBox battleBox(maxX/2 - 20, maxY/2 - 8, 40, 16);
    Heart heart(maxX/2, maxY/2);

    // Get box boundaries for containment checks
    int boxMinX = battleBox.getInnerMinX();
    int boxMinY = battleBox.getInnerMinY();
    int boxMaxX = battleBox.getInnerMaxX();
    int boxMaxY = battleBox.getInnerMaxY();

    // Create the snake that follows the heart (initially inactive)
    Snake snake(maxX/2 - 10, maxY/2, 5); // 5 segments length

    // Create knight that spawns randomly and shoots lasers
    Knight knight(boxMinX, boxMinY, boxMaxX, boxMaxY);
    int knightSpawnTimer = 0;

    // Game state variables
    bool running = true;
    bool inAttackCycle = false;
    std::vector<Laser> lasers;
    int messageY = maxY - 6;
    int attackRound = 0; // Track the current round (1-7)
    int waveTimer = 0;
    int currentWave = 0;
    int maxWaves = 0;
    int randomAttackType = 0; // For random attack selection
    bool gameCompleted = false; // Track if all 7 rounds are completed
    int initialDelay = 180; // 3 second delay (60 frames per second)
    bool gameStarted = false;

    // Draw the static elements once
    battleBox.draw();
    mvprintw(maxY - 3, 2, "Arrow keys to set direction, Space to stop/start");
    mvprintw(maxY - 2, 2, "Q to quit");

    waitForEnter("Welcome to the 7-Round Laser Battle! Press Enter to start the game.", 2, messageY);

    // Make sure the heart starts moving
    heart.setDirection(1.0f, 0.0f);
    heart.start();

    std::vector<int> round4SafeQuadrants;
    int round4State = 0; // 0 = warning, 1 = attack, 2 = post-attack delay
    int round4SafeQuadrant = -1;
    // Game loop
    while (running) {
        // Add initial 3-second delay
        if (!gameStarted) {
            mvprintw(messageY, 2, "Get ready! Starting in %d seconds...", (initialDelay + 59) / 60);
            refresh();

            initialDelay--;
            if (initialDelay <= 0) {
                gameStarted = true;
                move(messageY, 2);
                clrtoeol();
            } else {
                usleep(16667);  // ~60 FPS
                continue;  // Skip the rest of the loop during countdown
            }
        }

        // Check if player died
        if (heart.isDead()) {
            // Clear everything first
            clearAllElements(lasers, snake, knight);

            waitForEnter("Game Over! Press Enter to restart.", 2, messageY);
            heart.reset(maxX/2, maxY/2);
            snake.reset(maxX/2 - 10, maxY/2);
            snake.setActive(false); // Deactivate snake on restart
            knight.clear();
            lasers.clear();
            inAttackCycle = false;
            attackRound = 0;
            gameCompleted = false;
            knightSpawnTimer = 0;

            // Make sure the heart starts moving
            heart.setDirection(1.0f, 0.0f);
            heart.start();
            continue; // Restart the loop after resetting
        }

        // Check if all rounds are completed
        if (gameCompleted) {
            clearAllElements(lasers, snake, knight);
            waitForEnter("Congratulations! You've completed all 7 rounds! Press Enter to play again.", 2, messageY);
            heart.reset(maxX/2, maxY/2);
            snake.reset(maxX/2 - 10, maxY/2);
            snake.setActive(false);
            knight.clear();
            lasers.clear();
            inAttackCycle = false;
            attackRound = 0;
            gameCompleted = false;
            knightSpawnTimer = 0;

            // Make sure the heart starts moving
            heart.setDirection(1.0f, 0.0f);
            heart.start();
            continue; // Restart loop after resetting
        }

        // Start new attack cycle if not in one and not all rounds completed
        if (!inAttackCycle && !gameCompleted) {
            attackRound++; // Increment round counter

            // Check if we've reached the final round
            if (attackRound > 7) {
                gameCompleted = true;
                continue;
            }

            // Activate snake after 5th round
            if (attackRound == 5) {
                snake.setActive(true);
                waitForEnter("Warning: A snake has appeared! It will follow you now. Press Enter to continue.", 2, messageY);
            }

            // Message based on round number
            char roundMessage[100];
            sprintf(roundMessage, "Round %d of 7: Press Enter to start the attack!", attackRound);
            waitForEnter(roundMessage, 2, messageY);

            inAttackCycle = true;
            currentWave = 0;
            waveTimer = 0;

            // Different attack patterns for each round
            switch (attackRound) {
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
                    maxWaves = 3;
                    // For the first wave initialization
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
                // In the switch statement for round 4 (case 4:), modify the code to:
                if (attackRound == 4 && inAttackCycle) {
                    waveTimer++;
                
                    // State machine for warning/attack/delay
                    switch (round4State) {
                        case 0: // Warning phase
                            // Pick new safe quadrant at the start of each warning
                            if (waveTimer == 1) {
                                int prevQuadrant = round4SafeQuadrant;
                                do {
                                    round4SafeQuadrant = rand() % 4;
                                } while (round4SafeQuadrant == prevQuadrant);
                            }
                            drawDangerQuadrantWarnings(battleBox, round4SafeQuadrant);
                            if (waveTimer >= 120) { // 2 seconds warning
                                // Begin attack: clear any old lasers, fire new ones
                                for (auto& laser : lasers) laser.clear();
                                lasers = createQuadrantAttack(battleBox, round4SafeQuadrant);
                                for (auto& laser : lasers) laser.activate();
                                round4State = 1;
                                waveTimer = 0;
                            }
                            break;
                        case 1: // Attack phase
                            // Update lasers and check for collision
                            for (auto& laser : lasers) {
                                laser.update();
                                if (laser.checkCollision(heart)) heart.takeDamage();
                            }
                            // Wait for all lasers to finish
                            {
                                bool allLasersDone = true;
                                for (auto& laser : lasers) {
                                    if (!laser.isComplete()) {
                                        allLasersDone = false;
                                        break;
                                    }
                                }
                                if (allLasersDone && waveTimer > 60) { // At least 1 second after attack
                                    round4State = 2;
                                    waveTimer = 0;
                                    for (auto& laser : lasers) laser.clear();
                                    lasers.clear();
                                }
                            }
                            break;
                        case 2: // Inter-wave delay
                            if (waveTimer >= 60) { // 1 second between waves
                                currentWave++;
                                waveTimer = 0;
                                if (currentWave >= maxWaves) {
                                    // End of round 4
                                    inAttackCycle = false;
                                    char completionMessage[100];
                                    sprintf(completionMessage, "Round %d completed! Press Enter to continue.", attackRound);
                                    waitForEnter(completionMessage, 2, messageY);
                                    round4SafeQuadrant = -1;
                                    round4State = 0;
                                    currentWave = 0;
                                } else {
                                    round4State = 0;
                                }
                            }
                            break;
                    }
                    // Skip the rest of the attackCycle logic for round 4
                    goto skip_normal_attack_update;
                }
                skip_normal_attack_update:;
                case 5:
                    maxWaves = 4;
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
                    maxWaves = 4;
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
                upd:
                    maxWaves = 6;
                    lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                    for (auto& laser : lasers) laser.activate();
                    snake.setActive(true);
                    break;
            }
            heart.start(); // Make sure the heart is moving
        }

        // Process all available input
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
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

        // Only auto-start if heart can be force-started (not explicitly stopped by spacebar)
        if (!heart.isMoving() && heart.canForceStart()) {
            float dirX = heart.getDirectionX();
            float dirY = heart.getDirectionY();
            if (dirX == 0.0f && dirY == 0.0f) {
                heart.setDirection(1.0f, 0.0f);
            }
            heart.start();
        }

        heart.update();

        // Update knight in round 6
        if (attackRound == 6 && inAttackCycle) {
            knightSpawnTimer++;
            if (knightSpawnTimer >= 60) {
                knightSpawnTimer = 0;
                if (knight.isActive()) knight.clear();
                knight.spawn(battleBox);
            }
            knight.update();
            if (knight.checkCollision(heart)) heart.takeDamage();
        }

        // Update knight in round 7 (random knight spawns!)
        if (attackRound == 7 && inAttackCycle) {
            knightSpawnTimer++;
            if (knightSpawnTimer >= 60) { // Every 1 second
                knightSpawnTimer = 0;
                if (knight.isActive()) knight.clear();
                knight.spawn(battleBox);
            }
            knight.update();
            if (knight.checkCollision(heart)) heart.takeDamage();
        }

        // Update snake if it's active
        if (snake.isActive()) {
            snake.clear();
            snake.update(heart.getIntX(), heart.getIntY());
            if (snake.checkCollision(heart)) heart.takeDamage();
        }

        // Boundary checking to keep heart inside the battle box
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

        // Update attack waves for each round
        if (inAttackCycle) {
            waveTimer++;
            switch (attackRound) {
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
                        // Clear the current lasers
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        // Choose a new random attack for the next wave
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
                    if (waveTimer >= 120 && currentWave < maxWaves - 1) {
                        // Clear the current lasers
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        
                        // Pick a new safe quadrant (different from previous)
                        int prevQuadrant = heart.getSafeQuadrant();
                        int newQuadrant;
                        do {
                            newQuadrant = rand() % 4;
                        } while (newQuadrant == prevQuadrant);
                        
                        heart.setSafeQuadrant(newQuadrant);
                        
                        lasers = createQuadrantAttack(battleBox, newQuadrant);
                        for (auto& laser : lasers) laser.activate();
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
                    if (waveTimer >= 120 && currentWave < maxWaves - 1) {
                        // Clear the current lasers
                        for (auto& laser : lasers) laser.clear();
                        lasers.clear();
                        currentWave++;
                        waveTimer = 0;
                        
                        // Different attack pattern for each wave
                        if (currentWave == 1) {
                            // Wave 2: Horizontal lines with knight
                            lasers = createHorizontalLines(battleBox, 3, heart.getIntY());
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        } else if (currentWave == 2) {
                            // Wave 3: X-cross pattern with knight
                            lasers = createXCrossPattern(battleBox);
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        } else if (currentWave == 3) {
                            // Wave 4: Both horizontal and vertical lines with knight
                            std::vector<Laser> horizontal = createHorizontalLines(battleBox, 2, heart.getIntY());
                            std::vector<Laser> vertical = createVerticalLines(battleBox, 2, heart.getIntX());
                            lasers = horizontal;
                            lasers.insert(lasers.end(), vertical.begin(), vertical.end());
                            for (auto& laser : lasers) laser.activate();
                            knight.spawn(battleBox);
                        }
                    }
                case 7:
                    if (currentWave < maxWaves - 1 && waveTimer >= 60) {
                        for (auto& laser : lasers) laser.clear();
                        currentWave++;
                        waveTimer = 0;
                        switch (currentWave) {
                            case 1: lasers = createVerticalLines(battleBox, 3, heart.getIntX()); break;
                            case 2: lasers = createTicTacToePattern(battleBox); break;
                            case 3: lasers = createXCrossPattern(battleBox); break;
                            case 4: lasers = createHorizontalLines(battleBox, 5, heart.getIntY()); break;
                            case 5: lasers.clear(); knight.spawn(battleBox); break;
                        }
                        for (auto& laser : lasers) laser.activate();
                    }
                    break;
            }

            // Update all lasers
            for (auto& laser : lasers) {
                laser.update();
                if (laser.checkCollision(heart)) {
                    heart.takeDamage();
                }
            }

            // Check if all lasers are complete for current wave
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
                char completionMessage[100];
                sprintf(completionMessage, "Round %d completed! Press Enter to continue.", attackRound);
                waitForEnter(completionMessage, 2, messageY);
                for (auto& laser : lasers) laser.clear();
                lasers.clear();
            }
        }

        // Always redraw the battle box to ensure borders stay visible
        battleBox.draw();

        // Draw knight (Round 6 & 7)
        if ((attackRound == 6 || attackRound == 7) && inAttackCycle && knight.isActive()) {
            knight.draw();
        }

        // Draw the lasers
        for (auto& laser : lasers) {
            laser.draw();
        }

        // Draw the snake and heart
        if (snake.isActive()) {
            snake.draw();
        }
        heart.draw();

        // Draw health bar
        drawHealthBar(2, maxY - 4, 10, heart.getHP());

        // Display current round information
        mvprintw(2, 2, "Round: %d/7", attackRound);

        // Refresh screen and control frame rate
        refresh();
        usleep(16667);  // ~60 FPS
    }

    // Clean up
    endwin();
    return 0;
}