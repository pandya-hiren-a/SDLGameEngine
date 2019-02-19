#include "Game.h"
#include "Input.h"
#include "Time.h"
#include "RenderModule.h"

SDL_Window* Game::gWindow = NULL;
SDL_Renderer* Game::gRenderer = NULL;
int Game::screenWidth = 800;
int Game::screenHeight = 600;
bool Game::quit = false;

Game::Game()
{

}


Game::~Game()
{
}

void Game::Awake()
{
	
}

bool Game::Init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Camera::width, Camera::height, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}

	InitializeModules();

	return success;
}

void Game::InitializeModules()
{
	Input * input = new Input();
	input->Start();
	modules.push_back(input);

	Time * time = new Time();
	time->Start();
	modules.push_back(time);

	RenderModule * renderModule = new RenderModule();
	renderModule->Start();
	modules.push_back(renderModule);
}



void Game::Setup()
{

}

void Game::SetScene(Scene* _scene)
{
	if (currentScene != NULL)
	{
		currentScene->Cleanup();
		delete currentScene;
	}
	currentScene = _scene;
	currentScene->game = this;
	currentScene->Setup();
}


void Game::Update()
{
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(gRenderer);	

	for (Module* m : modules)
	{
		m->Update();
	}


	for (GameObject* g : gameObjects)
	{
		if (g->isActive())
		{

			g->Update();
			
		}
	}

	for (GameObject* g : objectsToDestroy)
	{
		gameObjects.remove(g);
		if (currentScene)
		{
			currentScene->ReleaseFromScene(g);
		}
		delete g;
		g = NULL;
	}

	objectsToDestroy.clear();
	//Update screen			
	SDL_RenderPresent(gRenderer);

	
}

void Game::End()
{

	for (GameObject* g : gameObjects)
	{
		g->Cleanup();
	}

	for (Module* m : modules)
	{
		m->CleanUp();
	}

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

GameObject* Game::CreateGameObject(std::string name, float x, float y, float angle)
{
	GameObject* go = new GameObject(this, name, x, y, angle);
	gameObjects.push_back(go);
	return go;
}

GameObject* Game::Instantiate(std::string name, const float& x, const float& y, const float& angle)
{
	GameObject* go = new GameObject(this, name, x, y, angle);
	gameObjects.push_back(go);
	go->Awake();
	return go;
}

GameObject* Game::Instantiate(GameObject* go, const float& x, const float& y, const float& angle)
{
	go->transform->SetAbsolutePosition(Vector2(x, y));
	go->transform->SetAbsoluteAngle(angle);
	gameObjects.push_back(go);
	go->Awake();
	return go;
}

GameObject* Game::Instantiate(std::string name, const Vector2& position, const float& angle)
{
	return Instantiate(name, position.x, position.y, angle);
}

GameObject* Game::Instantiate(GameObject* go, const Vector2& position, const float& angle)
{
	return Instantiate(go, position.x, position.y, angle);
}

void Game::Destroy(GameObject* go)
{
	objectsToDestroy.push_back(go);
}

void Game::AddPrefab(std::string name, std::function<GameObject*()> prefabRecipe)
{
	prefabs.insert(std::pair<std::string, std::function<GameObject*()>>(name, prefabRecipe));
}

GameObject* Game::Prefab(std::string name)
{
	return prefabs.find(name)->second();
}
