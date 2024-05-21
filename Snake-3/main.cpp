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
bool quit = false; 

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
            case SDLK_p:
            {
                bool loop = true;
                while (loop)
                {
                    while (SDL_PollEvent(&e) != 0) {
                        if (e.type == SDL_QUIT) {
                            quit = true;
                            return true;
                        }
                        else if (e.type == SDL_KEYDOWN) {
                            switch (e.key.keysym.sym) {
                            case SDLK_p: 
                                loop = false;
                                break;
                            }
                        }
                    }
                }
            }
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

    return 0;
}

void renderGame(Snake& snake) {
    SDL_RenderClear(gRenderer);

    // draw Head
    {
        SDL_Rect r = { snake.segments[0].x * GRID_SIZE, snake.segments[0].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        SDL_RenderCopy(gRenderer, snake.headTexture, NULL, &r);
    }

    // draw Body
    for (size_t i = 1; i < snake.segments.size(); ++i) {
        SDL_Rect r = { snake.segments[i].x * GRID_SIZE, snake.segments[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        SDL_RenderCopy(gRenderer, snake.bodyTexture, NULL, &r);
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


Snake snake;
void GamePlay()
{
    bool GameOver = false; // quit means program shutdown. GameOver means it end a game

    renderGame(snake);
    while (true)
    {
        SDL_Delay(timeDelay);
        if (handleInput(snake, quit)) break;
    }

    while (!GameOver && !quit) {
        renderGame(snake);
        SDL_Delay(timeDelay);

        handleInput(snake, quit);
        switch (updateSnake(snake))
        {
        case 1: // crash wall
            Mix_PlayMusic(crashWall, 0);
            for (int i = 0; i < 10; i++)
            {
                SDL_Delay(200);
                if (SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) { 
                    quit = true;
                    GameOver = true;
                    break;
                }
            }
            GameOver = true;
            break;
        case 2: // eat food
            Mix_PlayMusic(bite, 0);
            break;
        case 3: // crash itself
            Mix_PlayMusic(crashSelf, 0);
            for (int i = 0; i < 10; i++)
            {
                SDL_Delay(200);
                if (SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) { 
                    quit = true;
                    GameOver = true;
                    break;
                }
            }
            GameOver = true;
            break;
        default:
            break;
        }
    }
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




void enLarge(SDL_Rect& rect)
{
    rect.x -= rect.w / 4;
    rect.y -= rect.h / 4;
    rect.w *= 1.5;
    rect.h *= 1.5;
}

void deTiny(SDL_Rect& rect)
{
    rect.x += rect.w / 6;
    rect.y += rect.h / 6;
    rect.w /= 1.5;
    rect.h /= 1.5;
}

bool Menu()
{
    /* return 1 if Play, else Quit */
    SDL_SetRenderDrawColor(gRenderer, 100, 200, 255, 255);
    SDL_RenderClear(gRenderer);

    // Draw Logo
    { 
        SDL_Texture* GameLogo = loadTexture("img\\Snake.png", gRenderer);
        SDL_Rect Logo = { SCREEN_WIDTH / 12, SCREEN_HEIGHT / 6, SCREEN_WIDTH / 6 * 5, SCREEN_HEIGHT / 4 };
        SDL_RenderCopy(gRenderer, GameLogo, NULL, &Logo);
    }

    const int maxOption = 2;
    // Draw Play and Exit
    SDL_Texture* Exit = loadTexture("img\\Exit.png", gRenderer);
    SDL_Rect ExitRect;
    {
        ExitRect.h = SCREEN_HEIGHT / 12;
        ExitRect.w = SCREEN_WIDTH / 6;
        ExitRect.x = SCREEN_WIDTH / 2 - ExitRect.w / 2;
        ExitRect.y = SCREEN_HEIGHT / 14 * 11;
        SDL_RenderCopy(gRenderer, Exit, NULL, &ExitRect);
    }
    SDL_Texture* Play = loadTexture("img\\Play.png", gRenderer);
    SDL_Rect PlayRect;
    {
        PlayRect.h = SCREEN_HEIGHT / 9;
        PlayRect.w = SCREEN_WIDTH / 6;
        PlayRect.x = SCREEN_WIDTH / 2 - PlayRect.w / 2;
        PlayRect.y = SCREEN_HEIGHT / 14 * 8;
        enLarge(PlayRect);
        SDL_RenderCopy(gRenderer, Play, NULL, &PlayRect);
    }

    
    int chose = 0;
    // 0 for Play be Larger _ default
    // 1 for Quit be Larger
    bool beLarge[maxOption] = { true, false };
    SDL_Texture* Option[maxOption] = { Play, Exit };
    SDL_Rect* OptionRect[maxOption] = { &PlayRect, &ExitRect };

    
    SDL_RenderPresent(gRenderer);

    while (true)
    {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return false; // QUit game
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym)
                {
                case SDLK_RETURN:
                case SDLK_SPACE:
                    switch (chose)
                    {
                    case 0: return true; // choose to Play
                    case 1: return false; // choose to Quit
                    }
                case SDLK_ESCAPE:
                    return false; // Quit
                case SDLK_UP:
                {
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]); // erase the old thing
                    deTiny(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]); // redraw small thing
                    chose++;
                    if (chose >= maxOption) chose = 0;
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]); // delete the place for new-come big thing
                    enLarge(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]); // draw the new big thing 
                    SDL_RenderPresent(gRenderer);
                }
                    break;

                case SDLK_DOWN:
                {
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]); // same as always
                    deTiny(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]);
                    chose--;
                    if (chose < 0) chose = maxOption - 1;
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]);
                    enLarge(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]);
                    SDL_RenderPresent(gRenderer);
                }
                    break;

                default: 
                    break;
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (y >= PlayRect.y && y <= PlayRect.y + PlayRect.h && chose != 0) // mouse is on area of which was not large
                {
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]);
                    deTiny(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]); 
                    chose = 0;
                    SDL_RenderFillRect(gRenderer, &PlayRect);
                    enLarge(PlayRect);
                    SDL_RenderCopy(gRenderer, Play, NULL, &PlayRect);
                    SDL_RenderPresent(gRenderer);
                }
                else if (y >= ExitRect.y && y <= ExitRect.y + ExitRect.h && chose != 1) // mouse is on area of which was not large
                {
                    SDL_RenderFillRect(gRenderer, OptionRect[chose]);
                    deTiny(*OptionRect[chose]);
                    SDL_RenderCopy(gRenderer, Option[chose], NULL, OptionRect[chose]);
                    chose = 1;
                    SDL_RenderFillRect(gRenderer, &ExitRect);
                    enLarge(ExitRect);
                    SDL_RenderCopy(gRenderer, Exit, NULL, &ExitRect);
                    SDL_RenderPresent(gRenderer);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= OptionRect[chose]->x && x <= OptionRect[chose]->x + OptionRect[chose]->w)
                {
                    switch (chose)
                    {
                    case 0: return true; // choose to Play
                    case 1: return false; // choose to Quit
                    }
                }
            }
        }

        SDL_Delay(100);
    }

    return false;
}

int main(int argc, char* args[]) {
    
    if (!setUpThing()) return 0; 

    while (!quit && Menu())
    {
        initializeGame(snake);
        GamePlay();
    }
    
    DELETE();
    SDL_Quit();

    return 0;
}