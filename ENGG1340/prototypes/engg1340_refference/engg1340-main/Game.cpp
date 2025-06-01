#include "Game.h"



Game::Game(SDL_Window* window, RendererGL* rendererGL) :
	unitPlayer(Vector2D(tileCountX / 2.0f, tileCountY / 2.0f)) {
	//Run the game.
	if (window != nullptr && rendererGL != nullptr) {
		//Setup the level randomly.
		addRandomUnitEnemies(window);
		addRandomDestructibles(window);


		//Store the current times for the clock.
		auto time1 = std::chrono::system_clock::now();
		auto time2 = std::chrono::system_clock::now();


		//Start the game loop and run until it's time to stop.
		bool running = true;
		while (running) {
			//Determine how much time has elapsed since the last frame.
			time2 = std::chrono::system_clock::now();
			std::chrono::duration<float> timeDelta = time2 - time1;
			float timeDeltaFloat = timeDelta.count();

			//Store the new time for the next frame.
			time1 = time2;

			//The amount of time for each frame (no longer than 20 fps).
			const float dT = std::min(timeDeltaFloat, 1.0f / 20.0f);

			processEvents(running);
			if (unitPlayer.isAlive())
				update(dT, window);
			draw(window, rendererGL);
		}
	}
}


Game::~Game() {
	//Clean up.
}



void Game::processEvents(bool& running) {
	bool mouseDownThisFrame = false;

	//Process events.
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			running = false;
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouseDownThisFrame = (mouseDownStatus == 0);
			if (event.button.button == SDL_BUTTON_LEFT)
				mouseDownStatus = SDL_BUTTON_LEFT;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				mouseDownStatus = SDL_BUTTON_RIGHT;
			break;
		case SDL_MOUSEBUTTONUP:
			mouseDownStatus = 0;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
				//Quit the game.
			case SDL_SCANCODE_ESCAPE:
				running = false;
				break;
			}
		}
	}


	//Process input from the mouse cursor.
	int mouseX = 0, mouseY = 0;
	SDL_GetMouseState(&mouseX, &mouseY);
	//Convert from the window's coordinate system to the game's coordinate system.
	Vector2D posMouseScreen((float)mouseX / tileSize, (float)mouseY / tileSize);
	Vector2D posMouseGlobal(posMouseScreen - posCamera);


	if (unitPlayer.isAlive()) {
		//A more responsive/faster way to check keypresses for the unit movement.
		const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
		//Make the unit move if needed.
		if (keyboardState[SDL_SCANCODE_A])
			unitPlayer.setDirectionX(-1);
		if (keyboardState[SDL_SCANCODE_D])
			unitPlayer.setDirectionX(1);
		if (keyboardState[SDL_SCANCODE_W])
			unitPlayer.setDirectionY(-1);
		if (keyboardState[SDL_SCANCODE_S])
			unitPlayer.setDirectionY(1);

		//Set the angle the player is pointing towards with the position of the mouse.
		unitPlayer.setPosViewTarget(posMouseGlobal);

		//Make unit player shoot if needed.
		if (mouseDownStatus == SDL_BUTTON_LEFT)
			unitPlayer.shootProjectile(*this);
	}
}



void Game::update(float dT, SDL_Window* window) {
	//Update the player.
	unitPlayer.update(dT, *this);

	//Update the camera.
	updateCamera(window);

	//Update the enemy units.
	for (auto it = listUnitEnemies.begin(); it != listUnitEnemies.end();) {
		(*it).update(dT, *this);
		if ((*it).isAlive() == false)
			it = listUnitEnemies.erase(it);
		else
			it++;
	}

	//Update the destructibles.
	for (auto it = listDestructibles.begin(); it != listDestructibles.end();) {
		(*it).update(dT);
		if ((*it).isAlive() == false)
			it = listDestructibles.erase(it);
		else
			it++;
	}

	//Update the projectiles.
	for (auto it = listProjectiles.begin(); it != listProjectiles.end();) {
		(*it).update(dT, *this);
		if ((*it).getCollisionOccurred())
			it = listProjectiles.erase(it);
		else
			it++;
	}
}


void Game::updateCamera(SDL_Window* window) {
	//Update the camera so that unit player is in the center of the screen.
	if (window != nullptr) {
		//Get the dimensions of the window.
		int windowWidth = 0, windowHeight = 0;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);

		posCamera = Vector2D(windowWidth, windowHeight) / 2 / tileSize - unitPlayer.getPos();
	}
}



void Game::draw(SDL_Window* window, RendererGL* rendererGL) {
	if (window != nullptr && rendererGL != nullptr) {
		//Draw.
		//Set the background color.
		glClearColor(0.429f, 0.099f, 0.284f, 1.0f);
		//Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT);


		//Get the dimensions of the window.
		int windowWidth = 0, windowHeight = 0;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);


		//**********Background**********
		//Draw a rectangle that represents the area of the world that exists.

		//The positions of the sides of the camera in pixels (as the camera moves around).
		const int cameraLeft = (int)round(-posCamera.x * tileSize);
		const int cameraTop = (int)round(-posCamera.y * tileSize);
		const int cameraRight = cameraLeft + windowWidth;
		const int cameraBottom = cameraTop + windowHeight;

		//The positions of the world in pixels (it's not moving around).
		const int worldLeft = 0;
		const int worldTop = 0;
		const int worldRight = tileCountX * tileSize;
		const int worldBottom = tileCountY * tileSize;

		//The offset from the sides of the camera to the sides of the world in pixels.
		//Eg. if the camera's left side, is left of the left edge of the world, leftOffset is
		//the amount between the two.  Otherwise it's 0.
		const int leftOffset = std::max(0, worldLeft - cameraLeft);
		const int topOffset = std::max(0, worldTop - cameraTop);
		const int rightOffset = std::max(0, cameraRight - worldRight);
		const int bottomOffset = std::max(0, cameraBottom - worldBottom);

		//Draw a rectangle at the correct position and offset.
		SDL_Rect rectBackground = {
			leftOffset,
			topOffset,
			windowWidth - rightOffset - leftOffset,
			windowHeight - bottomOffset - topOffset
		};

		if (rectBackground.w > 0 && rectBackground.h > 0) {
			rendererGL->setDrawColor(218, 50, 143, 255);
			rendererGL->fillRect(&rectBackground, 0.0f);
		}


		//**********Shadows**********
		for (auto& selected : listDestructibles)
			selected.drawShadow(rendererGL, tileSize, posCamera);

		for (auto& selected : listUnitEnemies)
			selected.drawShadow(rendererGL, tileSize, posCamera);

		unitPlayer.drawShadow(rendererGL, tileSize, posCamera);

		for (auto& selected : listProjectiles)
			selected.drawShadow(rendererGL, tileSize, posCamera);


		//**********Shapes**********
		for (auto& selected : listDestructibles)
			selected.draw(rendererGL, tileSize, posCamera);

		for (auto& selected : listUnitEnemies)
			selected.draw(rendererGL, tileSize, posCamera);

		unitPlayer.draw(rendererGL, tileSize, posCamera);

		for (auto& selected : listProjectiles)
			selected.draw(rendererGL, tileSize, posCamera);


		//If the player is no longer alive then draw a transparent red overlay over the entire window.
		if (unitPlayer.isAlive() == false) {
			SDL_Rect rect = { 0, 0, windowWidth, windowHeight };
			rendererGL->setDrawColor(255, 0, 0, 255 / 2);
			rendererGL->fillRect(&rect, 0.0f);
		}

		//Send the image to the window.
		rendererGL->swapWindow();
	}
}



bool Game::checkIfPositionOkForSpawn(Vector2D posCheck, bool avoidPlayer) {
	//Check if the input position is far enough away from other things in the game.
	const float radiusBtw = 0.75;
	const float radiusBtwPlayer = (avoidPlayer ? 10.0f : radiusBtw);

	//Check overlap with the player.
	if (unitPlayer.checkCircleOverlap(posCheck, radiusBtwPlayer))
		return false;

	//Check overlap with the enemies.
	for (auto& selected : listUnitEnemies)
		if (selected.checkCircleOverlap(posCheck, radiusBtw))
			return false;

	//Check overlap with the destructibles.
	for (auto& selected : listDestructibles)
		if (selected.checkCircleOverlap(posCheck, radiusBtw))
			return false;

	return true;
}


bool Game::computeRandomPositionForSpawn(Vector2D& posOutput, bool avoidPlayer) {
	//Try to find a position at random that's far enough away from other things in the game.
	//Keep trying random positions until one is found, or a maximum number of tries are attempted.
	for (int count = 0; count < 10; count++) {
		Vector2D posPossible(MathAddon::randFloat() * (tileCountX - 2) + 1.0f,
			MathAddon::randFloat() * (tileCountY - 2) + 1.0f);
		if (checkIfPositionOkForSpawn(posPossible, avoidPlayer)) {
			posOutput = posPossible;
			return true;
		}
	}

	return false;
}


void Game::addRandomUnitEnemies(SDL_Window* window) {
	Vector2D pos;

	for (int count = 0; count < 12; count++) {
		if (computeRandomPositionForSpawn(pos, true))
			listUnitEnemies.push_back(UnitEnemy(pos));
	}
}


void Game::addRandomDestructibles(SDL_Window* window) {
	Vector2D pos;

	for (int count = 0; count < 60; count++) {
		if (computeRandomPositionForSpawn(pos, false))
			listDestructibles.push_back(Destructible(pos));
	}
}