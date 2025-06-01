#pragma once
#include <chrono>
#include <vector>
#include "SDL2/SDL.h"
#include "RendererGL.h"
#include "UnitPlayer.h"
#include "UnitEnemy.h"
#include "Projectile.h"
#include "Destructible.h"



class Game
{
public:
	Game(SDL_Window* window, RendererGL* rendererGL);
	~Game();
	UnitPlayer& getUnitPlayer() { return unitPlayer; }
	std::vector<UnitEnemy>& getListUnitEnemies() { return listUnitEnemies; }
	std::vector<Projectile>& getListProjectiles() { return listProjectiles; }
	std::vector<Destructible>& getListDestructibles() { return listDestructibles; }
	int getTileCountX() { return tileCountX; }
	int getTileCountY() { return tileCountY; }


private:
	void processEvents(bool& running);
	void update(float dT, SDL_Window* window);
	void updateCamera(SDL_Window* window);
	void draw(SDL_Window* window, RendererGL* rendererGL);
	bool checkIfPositionOkForSpawn(Vector2D posCheck, bool avoidPlayer);
	bool computeRandomPositionForSpawn(Vector2D& posOutput, bool avoidPlayer);
	void addRandomUnitEnemies(SDL_Window* window);
	void addRandomDestructibles(SDL_Window* window);


	int mouseDownStatus = 0;

	const int tileSize = 64;
	const int tileCountX = 45, tileCountY = 24;

	UnitPlayer unitPlayer;
	std::vector<UnitEnemy> listUnitEnemies;
	std::vector<Projectile> listProjectiles;
	std::vector<Destructible> listDestructibles;

	Vector2D posCamera;
};