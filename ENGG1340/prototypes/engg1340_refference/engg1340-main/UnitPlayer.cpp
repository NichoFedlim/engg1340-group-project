#include "UnitPlayer.h"
#include "Game.h"



UnitPlayer::UnitPlayer(Vector2D setPos) :
	Unit(setPos, 0.375f, 20) {
	//Ensure the unit's stats are the correct amounts (rather than the defaults).
	setStatsWithLevel();
}



void UnitPlayer::update(float dT, Game& game) {
	Unit::update(dT);

	if (directionX != 0 || directionY != 0) {
		//Create a vector called add for the direction this unit wants to move.
		Vector2D add((float)directionX, (float)directionY);

		//If moving in both directions then the magnitude would be greater than 1.
		//Therefore normalize it to ensure that the magnitude is 1;
		add.normalize();

		//Because add is currently just the direction of movement, multiply it by the 
		//distance to move in this frame, so that it's the total amount to move in this frame.
		add *= speed * dT;

		//Add the amount of distance to move the unit's position.
		pos += add;

		//Ensure that the unit is in bounds of the level.
		const float offsetFromEdge = 0.75f;
		pos.x = std::min(std::max(offsetFromEdge, pos.x), game.getTileCountX() - offsetFromEdge);
		pos.y = std::min(std::max(offsetFromEdge, pos.y), game.getTileCountY() - offsetFromEdge);
	}


	//Set the angle with posViewTarget.
	angle = (posViewTarget - pos).angle();

	//Reset the variables for movement.
	directionX = 0;
	directionY = 0;
}



void UnitPlayer::draw(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
	drawTurret(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 137, 137, 137, 255 });
	drawTurret(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 103, 103, 103, 255 });
	drawBody(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 0, 126, 255, 255 });
	drawBody(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 0, 67, 216, 255 });
}


void UnitPlayer::drawShadow(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
	drawTurret(rendererGL, tileSize, posCamera, 8, 0, SDL_Color{ 109, 25, 72, 255 });
	drawBody(rendererGL, tileSize, posCamera, 8, 0, SDL_Color{ 109, 25, 72, 255 });
}



void UnitPlayer::addExperience(int amount) {
	experienceSum += amount;
	if (experienceSum >= experienceRequiredNextLevel && level < 4) {
		experienceSum -= experienceRequiredNextLevel;
		level++;
		setStatsWithLevel();
	}
}


void UnitPlayer::setStatsWithLevel() {
	if (level > 0) {
		experienceRequiredNextLevel = level * 5;
		timerWeaponCooldown = Timer(1.0f / (level));
		radius = 0.3125f + level * 0.0625f;
	}
}



void UnitPlayer::setDirectionX(int setDirectionX) {
	directionX = setDirectionX;
}


void UnitPlayer::setDirectionY(int setDirectionY) {
	directionY = setDirectionY;
}


void UnitPlayer::setPosViewTarget(Vector2D setPosViewTarget) {
	posViewTarget = setPosViewTarget;
}



void UnitPlayer::shootProjectile(Game& game) {
	Unit::shootProjectile(game, true);
}