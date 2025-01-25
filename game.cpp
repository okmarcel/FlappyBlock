#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const float GRAVITY = 0.6;
const int FALL_SPEED_CAP = 22;
const int BIRD_X = 200;
const int BIRD_SPEED = 4;
const int BIRD_JUMP_V = -12;
const int BIRD_SIZE = 40;
const int PIPE_WIDTH = 110;
const int PIPE_GAP_HEIGHT = 220;

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Color skyColor = {123, 218, 224};
SDL_Color birdColor = {255, 235, 84};
SDL_Color pipeColor = {46, 158, 28};

typedef struct bird {
    int y; // position
    float vy; // fall speed vector
    bool jumped; // jump flag
} bird_t;

typedef struct pipe_s {
    int x; // position
    int h; // hole position
} pipe_t;

static bird_t bird;
static pipe_s pipes[3];
static int score = 0;
static int highscore = 0;

void SetUpRand() {
    time_t t;
    srand((unsigned int)time(&t));
}

void SetUpSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); 

    window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

void placePipe(pipe_s* pipe) {
    pipe->h = rand()%(SCREEN_HEIGHT-PIPE_GAP_HEIGHT-40)+20;
    pipe->x = SCREEN_WIDTH+(PIPE_WIDTH+90)/3;
}

void startGame() {
    score = 0;
    bird.y = (SCREEN_HEIGHT-BIRD_SIZE)/2;
    bird.vy = BIRD_JUMP_V;
    bird.jumped = false;

    // place pipes
    for (int i = 0; i < 3; i++) {
        placePipe(&pipes[i]);
        pipes[i].x += i*(SCREEN_WIDTH+PIPE_WIDTH+90)/3;
    }
}

void birdJump() {
    if (!bird.jumped) bird.vy = BIRD_JUMP_V;
    bird.jumped = true;
}

void moveBird() {
    if (bird.vy > 10) {
        bird.vy = 10;
    }
    bird.y += bird.vy;
    bird.vy += GRAVITY;
}

void movePipes() {
    for (int i = 0; i < 3; i++) {
        pipes[i].x -= BIRD_SPEED;
        if (pipes[i].x+PIPE_WIDTH < 0) {
            placePipe(&pipes[i]);
        }
    }
}

pipe_t* findNextPipe() {
    pipe_t* nextPipe = &pipes[0];
    for (int i = 0; i < 3; i++) {
        if (pipes[i].x < nextPipe->x && pipes[i].x+PIPE_WIDTH > BIRD_X) {
            nextPipe = &pipes[i];
        }
    }
    return nextPipe;
}

bool checkCollisions() {
    // ground collision
    if (bird.y+BIRD_SIZE > SCREEN_HEIGHT) return true;

    // pipe collision
    pipe_t* nextPipe = findNextPipe();
    if (nextPipe->x < BIRD_X+BIRD_SIZE && nextPipe->x+PIPE_WIDTH > BIRD_X) {
        if (bird.y <= nextPipe->h || bird.y+BIRD_SIZE >= nextPipe->h+PIPE_GAP_HEIGHT) {
            return true;
        }
    }

    return false;
}

void updateScore()
{
    pipe_t* nextPipe = findNextPipe();
    if (BIRD_X == nextPipe->x) {
        std::cout << "Score: " << ++score << '\n';
    }
}

void drawBackground() {
    SDL_SetRenderDrawColor(renderer, skyColor.r, skyColor.g, skyColor.b, 255);
    SDL_RenderClear(renderer);
}

void drawBird() {
    SDL_Rect rect;
    SDL_SetRenderDrawColor(renderer, birdColor.r, birdColor.g, birdColor.b, 255);


    rect.h = BIRD_SIZE;
    rect.w = BIRD_SIZE;
    rect.x = BIRD_X;
    rect.y = bird.y;

    SDL_RenderFillRect(renderer, &rect);
}

void drawPipes() {
    SDL_Rect upper, lower;
    SDL_SetRenderDrawColor(renderer, pipeColor.r, pipeColor.g, pipeColor.b, 255);

    for (int i = 0; i < 3; i++) {
        upper.h = pipes[i].h;
        upper.w = PIPE_WIDTH;
        upper.x = pipes[i].x;
        upper.y = 0;

        lower.h = SCREEN_HEIGHT-pipes[i].h-PIPE_WIDTH;
        lower.w = PIPE_WIDTH;
        lower.x = pipes[i].x;
        lower.y = pipes[i].h+PIPE_GAP_HEIGHT;

        SDL_RenderFillRect(renderer, &upper);
        SDL_RenderFillRect(renderer, &lower);
    }
}

int main() {
    SetUpRand();
    SetUpSDL();
    startGame();

    bool isRunning = true;
    bool playing = true;
    Uint32 frameDelay = 16;
    SDL_Event event;

    while (isRunning) {
        Uint32 frameStart = SDL_GetTicks();
        SDL_PumpEvents();
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);


        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }
        if (keystate[SDL_SCANCODE_ESCAPE]) {
            isRunning = false;
        }


        if (playing) {
            if (keystate[SDL_SCANCODE_SPACE]) {
                if (!bird.jumped) {
                    birdJump();
                }
            } else bird.jumped = false;

            moveBird();
            movePipes();

            updateScore();

            if (checkCollisions()) {
                std::cout << "Game over\n";
                std::cout << "Your Score: " << score << '\n';
                
                //      Need a "play again" implementation before highscore implementation
                /*
                if (score > highscore) {
                    std::cout << "New High Score set!\n";
                    highscore = score;
                }
                */
                playing = false;
            }

        } else {
            //      TODO: SDL_ttf - Display "Game Over" text with option to play again
        }

        drawBackground();
        drawPipes();
        drawBird();
        // drawScore();         -- NOT IMPLEMENTED

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}