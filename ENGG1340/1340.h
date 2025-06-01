#ifndef HEART_GAME_H
#define HEART_GAME_H

#include <ncursesw/ncurses.h>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <cmath>

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
    GameObject(float startX, float startY, float dx, float dy, float spd, int sym);
    virtual ~GameObject() = default;

    virtual void update();
    void clearPrevious();
    virtual void draw();

    float getX() const;
    float getY() const;
    bool isActive() const;
    void setActive(bool state);
    void deactivate();
    
    // Check collision with another game object
    bool collidesWith(const GameObject& other) const;
};

class Heart : public GameObject {
private:
    float aspectRatio;    // Character aspect ratio (width/height)
    bool moving;          // Whether the heart is moving
    int health;           // Player health
    int score;            // Player score
    int invincibilityFrames; // Frames of invincibility after taking damage

public:
    Heart(int startX, int startY);

    void update() override;
    void draw() override;
    void setDirection(float dx, float dy);
    void setAspectRatio(float ratio);
    void stop();
    void start();
    bool isMoving() const;
    void setPosition(float newX, float newY);
    void takeDamage(int amount);
    void addScore(int amount);
    
    int getHealth() const;
    int getScore() const;
    float getAspectRatio() const;
    bool isInvincible() const;
    float getDirectionX() const;
    float getDirectionY() const;
};

class Laser : public GameObject {
public:
    Laser(float startX, float startY, float dx, float dy);
    void draw() override;
};

class Spaceship : public GameObject {
private:
    int health;
    int fireCooldown;
    int maxFireCooldown;
    bool reachedLeftEdge;

public:
    Spaceship(float startX, float startY, float dx, float dy);
    
    void update() override;
    bool canFire() const;
    void resetFireCoolDown();
    void draw() override;
    void takeDamage(int amount);
    
    // Check if spaceship has reached left edge
    bool hasReachedLeft(int leftEdge) const;
    void markReachedLeftEdge();
    int getHealth() const;
};

class Projectile : public GameObject {
public:
    Projectile(float startX, float startY, float dx, float dy);
    void draw() override;
};

class Bomb : public GameObject {
private:
    int timer;

public:
    Bomb(float startX, float startY);
    void update() override;
    void draw() override;
    int getTimer() const;
};

class BattleBox {
private:
    int x, y;         // Top-left corner position
    int width, height; // Box dimensions
    bool needsRedraw;  // Flag to determine if the box needs redrawing

public:
    BattleBox(int startX, int startY, int w, int h);

    void draw();
    void setNeedsRedraw();
    
    // Check if a position is inside the battle box (with a small margin)
    bool contains(float checkX, float checkY) const;
    
    // Check if a position is outside the battle box (with a small margin)
    bool isOutside(float checkX, float checkY) const;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
};

// Game function
std::vector<int> runRound(int round, int playerHealth);

#endif // HEART_GAME_H