#include <iostream>
#include <memory>
#include "SDL2/SDL.h"
#include "Game.h"



int main(int argc, char* args[]) {
	//Seed the random number generator with the current time so that it will generate different 
	//numbers every time the game is run.
	srand((unsigned)time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Error: Couldn't initialize SDL Video = " << SDL_GetError() << std::endl;
		return 1;
	}
	else {
		//OpenGL version 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		//Ensure core profile is used
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create the window.
		SDL_Window* window = SDL_CreateWindow("OpenGL RPG Game",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 512,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

		if (window == nullptr) {
			std::cout << "Error: Couldn't create window = " << SDL_GetError() << std::endl;
			return 1;
		}
		else {
			//Create context
			SDL_GLContext glContext = SDL_GL_CreateContext(window);
			if (glContext == nullptr)
			{
				std::cout << "Error: Couldn't create OpenGL context = " << SDL_GetError() << std::endl;
				return 1;
			}
			else
			{
				//Initialize GLEW
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewInit() != GLEW_OK)
				{
					std::cout << "Error initializing GLEW = " << glewGetErrorString(glewError) << std::endl;
					return 1;
				}
				else {
					//Ensure transparent graphics are drawn correctly.
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					//Create an instance of the rendererGL class to use OpenGL for drawing.
					std::unique_ptr<RendererGL> rendererGL = std::make_unique<RendererGL>(window);
					if (rendererGL != nullptr) {
						//Enable vsync
						SDL_GL_SetSwapInterval(1);

						//Start the game.
						Game game(window, rendererGL.get());
					}
				}

				//Clean up.
				SDL_GL_DeleteContext(glContext);
			}

			//Clean up.
			SDL_DestroyWindow(window);
		}

		//Clean up.
		SDL_Quit();
	}
	return 0;
}