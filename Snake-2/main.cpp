#include <iostream>
#include <vector>
#include <SDL.h>
#include<SDL_image.h>
#include <SDL_mixer.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int GRID_SIZE = 40;
const int INITIAL_SNAKE_LENGTH = 3;
const int timeDelay = 130;

SDL_Texture* snakeHeadTexture = NULL;
SDL_Texture* snakeBodyTexture = NULL;
SDL_Texture* foodTexture = NULL;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Event e;

Mix_Music* bite;
Mix_Music* crashWall;
Mix_Music* crashSelf;

struct Point {
    int x, y;
};

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct Snake {
    std::vector<Point> segments;
    Direction direction;
    SDL_Texture* headTexture;
    SDL_Texture* bodyTexture;
};

Point food;

SDL_Texture* loadTexture(const char* filename, SDL_Renderer *renderer)
{
    /*SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO,
        "Loading %s", filename);

    SDL_Texture* texture = IMG_LoadTexture(renderer, filename);
    if (texture == NULL) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
            "Load texture %s", IMG_GetError());
    }*/

    SDL_Texture* texture = NULL;
    SDL_Surface* load_surface = IMG_Load(filename);
    if (load_surface != NULL)
    {
        texture = SDL_CreateTextureFromSurface(gRenderer, load_surface);
        if (texture == NULL) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                "Load texture %s", IMG_GetError());
        }
    }

    return texture;
}

void placeFood(Snake& snake) {
    bool onSnake = true;

    while (onSnake) {
        food.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
        food.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);

        onSnake = false;
        for (const auto& segment : snake.segments) {
            if (food.x == segment.x && food.y == segment.y) {
                onSnake = true;
                break;
            }
        }
    }
}

void initializeGame(Snake& snake) {
    snake.segments.clear();
    snake.headTexture = snakeHeadTexture;
    snake.bodyTexture = snakeBodyTexture;

    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        snake.segments.push_back({ SCREEN_WIDTH / 2 / GRID_SIZE, SCREEN_HEIGHT / 2 / GRID_SIZE + i });
    }

    snake.direction = Direction::UP;
    placeFood(snake);
}

bool handleInput(Snake& snake, bool& quit) {

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
            return true;
        }
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_UP:
                if (snake.direction != Direction::DOWN) {
                    snake.direction = Direction::UP;
                }
                return true;
                break;
            case SDLK_DOWN:
                if (snake.direction != Direction::UP) {
                    snake.direction = Direction::DOWN;
                }
                return true;
                break;
            case SDLK_LEFT:
                if (snake.direction != Direction::RIGHT) {
                    snake.direction = Direction::LEFT;
                }
                return true;
                break;
            case SDLK_RIGHT:
                if (snake.direction != Direction::LEFT) {
                    snake.direction = Direction::RIGHT;
                }
                return true;
                break;
            default:
                break;
            }
        }
    }
    return false;
}

int updateSnake(Snake& snake) {
    Point newHead = snake.segments[0];

    switch (snake.direction) {
    case Direction::UP:
        newHead.y -= 1;
        break;
    case Direction::DOWN:
        newHead.y += 1;
        break;
    case Direction::LEFT:
        newHead.x -= 1;
        break;
    case Direction::RIGHT:
        newHead.x += 1;
        break;
    }

    // Crashing the wall
    if (newHead.x < 0 || newHead.x >= SCREEN_WIDTH / GRID_SIZE || newHead.y < 0 || newHead.y >= SCREEN_HEIGHT / GRID_SIZE) {
        initializeGame(snake);
        return 1;
    }

    // Eating food or keep moving
    if (newHead.x == food.x && newHead.y == food.y) { // eating
        snake.segments.insert(snake.segments.begin(), newHead);
        placeFood(snake);
        return 2;
    }
    else {
        snake.segments.pop_back(); // or not
        snake.segments.insert(snake.segments.begin(), newHead);
    }

    // If snake crashing on it self
    for (size_t i = 1; i < snake.segments.size(); i++) {
        if (newHead.x == snake.segments[i].x && newHead.y == snake.segments[i].y) {
            initializeGame(snake);
            return 3;
        }
    }

    return 0; // nothing to be concern
}

void renderGame(Snake& snake) {
    SDL_RenderClear(gRenderer);

    for (size_t i = 0; i < snake.segments.size(); ++i) {
        SDL_Rect r = { snake.segments[i].x * GRID_SIZE, snake.segments[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        if (i == 0) {
            SDL_RenderCopy(gRenderer, snake.headTexture, NULL, &r);
        }
        else {
            SDL_RenderCopy(gRenderer, snake.bodyTexture, NULL, &r);
        }
    }

    SDL_Rect rFood = { food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
    SDL_RenderCopy(gRenderer, foodTexture, NULL, &rFood);

    SDL_RenderPresent(gRenderer);
}

bool setUpThing()
{
    // CHECK INIT
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }
    // CREATE WINDOW AND RENDERER
    gWindow = SDL_CreateWindow("SNAKE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }

    // CREATE TEXTURE
    snakeHeadTexture = loadTexture("img\\snake_head.png", gRenderer);
    snakeBodyTexture = loadTexture("img\\snake_body.png", gRenderer);
    foodTexture = loadTexture("img\\apple.png", gRenderer);
    if (snakeHeadTexture == NULL || snakeBodyTexture == NULL || foodTexture == NULL) {
        std::cout << "Failed to load textures." << std::endl;
        return 1;
    }

    // LOADING SOUND EFFECTS
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    bite = Mix_LoadMUS("sfx\\bite.mp3");
    crashWall = Mix_LoadMUS("sfx\\crashWall.mp3");
    crashSelf = Mix_LoadMUS("sfx\\uwu.mp3");


    return 1;
}

void DELETE()
{
    Mix_FreeMusic(bite);
    Mix_FreeMusic(crashWall);
    Mix_FreeMusic(crashSelf);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;
}

int main(int argc, char* args[]) {
    
    if (!setUpThing()) return 0; 

    Snake snake;
    bool quit = false;
    bool newgame = true;

    initializeGame(snake);
    SDL_SetRenderDrawColor(gRenderer, 100, 200, 255, 255);
    while (!quit) {
        renderGame(snake);
        SDL_Delay(timeDelay);
        while (newgame)
        {
            SDL_Delay(timeDelay);
            if (handleInput(snake, quit)) newgame = false;
        }

        handleInput(snake, quit);
        //int n = ;
        switch (updateSnake(snake))
        {
        case 1: // crash wall
            Mix_PlayMusic(crashWall, 0);
            newgame = true;
            for (int i = 0; i < 10; i++)
            {
                SDL_Delay(200);
                if (SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
                    quit = true;
                    newgame = false;
                    break;
                }
            }
            break;
        case 2: // eat food
            Mix_PlayMusic(bite, 0);
            break;
        case 3: // crash itself
            Mix_PlayMusic(crashSelf, 0);
            newgame = true;
            for (int i = 0; i < 10; i++)
            {
                SDL_Delay(200);
                if (SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
                    quit = true;
                    newgame = false;
                    break;
                }
            }
            break;
        default:
            break;
        }
    }

    
    DELETE();
    SDL_Quit();

    return 0;
}