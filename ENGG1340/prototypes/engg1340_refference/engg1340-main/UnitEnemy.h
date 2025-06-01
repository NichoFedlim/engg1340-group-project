#pragma once
#include "Unit.h"
#include "MathAddon.h"



class UnitEnemy : public Unit
{
public:
	UnitEnemy(Vector2D setPos);
	void update(float dT, Game& game);
	void draw(RendererGL* rendererGL, int tileSize, Vector2D posCamera);
	void drawShadow(RendererGL* rendererGL, int tileSize, Vector2D posCamera);

	int getExperienceGiven();


private:
	bool updateAngle(float dT, Game& game);


	float speedAngular = MathAddon::angleDegToRad(180.0f);

	int experienceGiven = 2;
};