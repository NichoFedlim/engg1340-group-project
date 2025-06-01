#ifndef SAFE_TILES_H
#define SAFE_TILES_H

#include <ncursesw/ncurses.h>
#include <vector>
#include <utility>

class Heart {
private:
    float x, y;           // Position with floating-point precision for smooth movement
    int lastDrawnX, lastDrawnY; // Last position where the heart was drawn
    float directionX, directionY; // Direction vector (normalized)
    float baseSpeed;      // Base movement speed
    float aspectRatio;    // Character aspect ratio (width/height)
    bool moving;          // Whether the heart is moving
    int symbol;           // Symbol to represent the heart

public:
    Heart(int startX, int startY);
    void update();
    void setDirection(float dx, float dy);
    void setAspectRatio(float ratio);
    void setSpeed(float speed);
    void stop();
    void start();
    bool isMoving() const;
    void setPosition(float newX, float newY);
    void clearPrevious();
    void draw();

    float getX() const;
    float getY() const;
    float getDirectionX() const;
    float getDirectionY() const;
    float getAspectRatio() const;
    float getSpeed() const;
};

class GameBoard {
private:
    int x, y;             // Top-left corner position
    int width, height;    // Board dimensions in tiles
    int tileWidth, tileHeight; // Dimensions of each tile
    std::vector<std::pair<int, int>> safeTiles; // Coordinates of safe tiles

public:
    GameBoard(int startX, int startY, int w, int h, int tw, int th);
    void draw(bool showSafeTiles = false, bool redBorder = false);
    void generateSafeTiles(int numTiles);
    bool isSafeTile(int tileX, int tileY) const;
    std::pair<float, float> boardToScreen(int tileX, int tileY) const;
    std::pair<int, int> screenToBoard(float screenX, float screenY) const;
    bool isOnBoard(float screenX, float screenY) const;
    bool isOnTile(float screenX, float screenY) const;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    int getTileWidth() const;
    int getTileHeight() const;
};

void countdown(int seconds, GameBoard& board, Heart& heart, int round, int totalRounds, int hp);
void runSafeTilesGame();
void drawHealthBar(WINDOW* win, int x, int y, int maxHP, int currentHP);

#endif // SAFE_TILES_H