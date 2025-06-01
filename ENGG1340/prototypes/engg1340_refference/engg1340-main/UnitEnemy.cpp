#include "UnitEnemy.h"
#include "Game.h"



UnitEnemy::UnitEnemy(Vector2D setPos) :
	Unit(setPos, 0.375f, 1) {

}



void UnitEnemy::update(float dT, Game& game) {
	Unit::update(dT);

	//If the player is within weapons range then rotate towards it and shoot it.
	if (Vector2D(game.getUnitPlayer().getPos() - pos).magnitude() <= weaponRange) {
		if (updateAngle(dT, game))
			Unit::shootProjectile(game, false);
	}
}


bool UnitEnemy::updateAngle(float dT, Game& game) {
	//Rotate this towards the target point (which is set below).
	bool reachedAngleTarget = false;

	//Determine the direction normal to the unit player.
	Vector2D directionNormalTarget = (game.getUnitPlayer().getPos() - pos).normalize();

	//Determine the angle to the target.
	float angleToTarget = Vector2D(angle).angleBetween(directionNormalTarget);

	//Determine the angle to move this frame.
	float angleMove = copysign(speedAngular * dT, angleToTarget);

	//Update the angle as required.
	if (abs(angleMove) > abs(angleToTarget)) {
		//It will point directly at it's target this frame.
		angle = directionNormalTarget.angle();
		reachedAngleTarget = true;
	}
	else {
		//It won't reach it's target this frame.
		angle += angleMove;
	}

	return reachedAngleTarget;
}





void UnitEnemy::draw(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
	drawTurret(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 137, 137, 137, 255 });
	drawTurret(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 103, 103, 103, 255 });
	drawBody(rendererGL, tileSize, posCamera, 0, 0, SDL_Color{ 255, 132, 132, 255 });
	drawBody(rendererGL, tileSize, posCamera, 0, 4, SDL_Color{ 255, 6, 6, 255 });
}


void UnitEnemy::drawShadow(RendererGL* rendererGL, int tileSize, Vector2D posCamera) {
	drawTurret(rendererGL, tileSize, posCamera, 8, 0, SDL_Color{ 109, 25, 72, 255 });
	drawBody(rendererGL, tileSize, posCamera, 8, 0, SDL_Color{ 109, 25, 72, 255 });
}



int UnitEnemy::getExperienceGiven() {
	return experienceGiven;
}