#include "Sprite.h"



Sprite::Sprite(Vector2D setPos, float setAngle, float setRadius) :
	pos(setPos), angle(setAngle), radius(setRadius) {

}



Vector2D Sprite::getPos() {
	return pos;
}



bool Sprite::checkCircleOverlap(Vector2D posCircle, float radiusCircle) {
	return (posCircle - pos).magnitude() <= (radiusCircle + radius);
}


bool Sprite::checkCircleOverlap(Sprite* other) {
	if (other != nullptr)
		return checkCircleOverlap(other->pos, other->radius);

	return false;
}


void Sprite::drawBody(RendererGL* rendererGL, int tileSize, Vector2D posCamera, int offsetPos,
	int offsetInner, SDL_Color color, bool isCircle) {
	if (rendererGL != nullptr) {
		//Draw the shape at the sprite's position, angle, and offset.
		int size = (int)round(tileSize * radius * 2.0f);

		SDL_Rect rect = {
			(int)round(pos.x * tileSize + posCamera.x * tileSize) - size / 2 - offsetPos + offsetInner,
			(int)round(pos.y * tileSize + posCamera.y * tileSize) - size / 2 + offsetPos + offsetInner,
			size - offsetInner * 2,
			size - offsetInner * 2 };
		rendererGL->setDrawColor(color.r, color.g, color.b, color.a);
		if (isCircle)
			rendererGL->fillOval(&rect, angle);
		else
			rendererGL->fillRect(&rect, angle);
	}
}