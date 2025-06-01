#ifndef GAME_1340_H
#define GAME_1340_H

#include <ncursesw/ncurses.h>
#include <vector>
#include <memory>

// Forward declarations
class GameObject;
class Heart;
class Laser;
class Spaceship;
class Projectile;
class Bomb;
class BattleBox;

class Game1340 {
private:
    // Game objects
    BattleBox* battleBox;
    Heart* heart;
    std::vector<std::unique_ptr<Spaceship>> spaceships;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<Bomb>> bombs;
    std::vector<std::unique_ptr<Laser>> lasers;
    
    // Game state
    int round;
    int frameCount;
    bool gameOver;
    int targetScore;
    int bombSpawnInterval;
    bool spawnBombs;
    
    // Process player input
    bool processInput();
    
    // Update game state
    void update();
    
    // Draw everything
    void draw();
    
    // Spawn enemies
    void spawnEnemies();
    
public:
    Game1340(int maxX, int maxY);
    ~Game1340();
    
    // Initialize the game
    void init();
    
    // Run the game loop for the current round
    int runRound();
    
    // Set up the game for a specific round
    void setupRound(int roundNumber);
    
    // Check if game is over
    bool isGameOver() const;
    
    // Get player score
    int getScore() const;
    
    // Get current round
    int getRound() const;
};

#endif // GAME_1340_H