#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <time.h>

const int FPS = 30;
const int FRAME_DELAY = 1000/FPS;
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

SDL_Window *window = nullptr;
SDL_Surface *sprite = nullptr;
SDL_Surface *paddleSprite = nullptr;
SDL_Surface *backGroundImage = nullptr;
SDL_Surface *backBuffer = nullptr;

Mix_Chunk *hitSound = nullptr;
Mix_Music *backGroundMusic = nullptr;

TTF_Font *gameFont = nullptr;

float inputDirectionX = 0.0f;
float paddleMovementSpeed = 10.0f;

float ballXDirection = 0.5f;
float ballYDirection = 1.0f;

float ballMovementSpeed = 0.0f;

unsigned int score = 0;
unsigned int highScore = 0;
unsigned int startTime = 0;

bool gameStarted = false;
bool autoPlay = false;
bool hasCollidedTop = false;
bool hasCollidedLeft = false;
bool hasCollidedRight = false;

SDL_Rect ballRect;
SDL_Rect paddleRect;
SDL_Rect bottomWallRect;
SDL_Rect topWallRect;
SDL_Rect leftWallRect;
SDL_Rect rightWallRect;

bool LoadFiles();
void FreeFiles();
bool ProgramIsRunning();
SDL_Surface* LoadImage(const char* fileName);
void DrawImage(SDL_Surface* image, SDL_Surface* destSurface, int x, int y);
void DrawImageFrame(SDL_Surface* image, SDL_Surface* destSurface, int x, int y, int width, int height, int frame);
void DrawText(SDL_Surface* surface, const char* string, int x, int y, TTF_Font* font, Uint8 r, Uint8 g, Uint8 b);
bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2);
void BallCollisionCheck(SDL_Rect rect1);
void ResetRects();

int main(int argc, char* args[])
{

    srand(time(NULL));

    // place ball
    ballRect.x = SCREEN_WIDTH/2;
    ballRect.y = SCREEN_HEIGHT/2;
    ballRect.w = 20;
    ballRect.h = 20;

    // place paddle
    paddleRect.x = SCREEN_WIDTH / 2;
    paddleRect.y = SCREEN_HEIGHT - 50;
    paddleRect.w = 60;
    paddleRect.h = 3;

    // place bottom wall rect
    bottomWallRect.x = 0;
    bottomWallRect.y = SCREEN_HEIGHT - 1;
    bottomWallRect.w = SCREEN_WIDTH;
    bottomWallRect.h = 20;

    // place top wall rect
    topWallRect.x = 0;
    topWallRect.y = -18;
    topWallRect.w = SCREEN_WIDTH;
    topWallRect.h = 20;

    // place left wall rect
    leftWallRect.x = -18;
    leftWallRect.y = 0;
    leftWallRect.w = 20;
    leftWallRect.h = SCREEN_HEIGHT;

    // place right wall rect
    rightWallRect.x = SCREEN_WIDTH - 2;
    rightWallRect.y = 0;
    rightWallRect.w = 20;
    rightWallRect.h = SCREEN_HEIGHT;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "SDL failed to init!" << std::endl;
        SDL_Quit();
        return 1;
    }

    // load font
    if (TTF_Init() == -1)
        return 2;

    //load sdl mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) 
        return 3;

    // create window
    window = SDL_CreateWindow(
        "Single Player Pong!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0);
    
    backBuffer = SDL_GetWindowSurface(window);
    
    if (LoadFiles()) {

        // play sound
        // Mix_PlayChannel(-1, hitSound, 0);

        // play music
        Mix_PlayMusic(backGroundMusic, -1);

        while(ProgramIsRunning())
        {
            
           // get the time at the start of the frame
            int frameStart = SDL_GetTicks();

            // reset the back buffer with the back ground
            SDL_BlitSurface(backGroundImage, NULL, backBuffer, NULL);

            /*
            // Test walls through coloring
            SDL_FillRect(backBuffer, &topWallRect, 255);
            SDL_FillRect(backBuffer, &leftWallRect, 255);
            SDL_FillRect(backBuffer, &bottomWallRect, 255);
            SDL_FillRect(backBuffer, &rightWallRect, 255);
            */

            // move the paddle
            if(autoPlay == false)
            {
                paddleRect.x = (paddleRect.x + (paddleRect.w/2.0f) < SCREEN_WIDTH) ? (paddleRect.x + (inputDirectionX * paddleMovementSpeed)) : -(paddleRect.w/2.0f) + 1;
                paddleRect.x = (paddleRect.x > -(paddleRect.w/2.0f)) ? paddleRect.x : SCREEN_WIDTH - (paddleRect.w/2.0f) - 1;
            } else
            {
                if((paddleRect.x + (paddleRect.w / 2.0f)) < ballRect.x)
                {
                    paddleRect.x = paddleRect.x + (1 * paddleMovementSpeed);

                } else if((paddleRect.x + (paddleRect.w / 2.0f)) > ballRect.x)
                {
                    paddleRect.x = paddleRect.x + (-1 * paddleMovementSpeed);
                }
                
            }

            // move the ball
            ballRect.x = ballRect.x + (ballXDirection * ballMovementSpeed);
            //ballRect.x = (ballRect.x + (ballRect.w/2.0f) < SCREEN_WIDTH) ? (ballRect.x + (ballXDirection * ballMovementSpeed)) : -(ballRect.w/2.0f) + 1;
            //ballRect.x = (ballRect.x > -(ballRect.w/2.0f)) ? ballRect.x : SCREEN_WIDTH - (ballRect.w/2.0f) - 1;

            ballRect.y = ballRect.y + (ballYDirection * ballMovementSpeed);
            //ballRect.y = (ballRect.y + (ballRect.h/2.0f) < SCREEN_HEIGHT) ? (ballRect.y + (ballYDirection * ballMovementSpeed)) : -(ballRect.h/2.0f) + 1;
            //ballRect.y = (ballRect.y > -(ballRect.h/2.0f)) ? ballRect.y : SCREEN_HEIGHT - (ballRect.h/2.0f) - 1;
            
            //draw the image
            DrawImage(sprite, backBuffer, ballRect.x, ballRect.y);
            DrawImage(paddleSprite, backBuffer, paddleRect.x, paddleRect.y);

             // check for ball collisions
            BallCollisionCheck(ballRect);

            // font
            if(gameStarted == true)
                score = SDL_GetTicks() - startTime;

            DrawText(backBuffer, (std::to_string(score)).c_str(), 100, 100, gameFont, 255u, 255u, 255u);
            DrawText(backBuffer, (std::to_string(highScore)).c_str(), 400, 100, gameFont, 255u, 255u, 255u);

            // end draw frame
            SDL_UpdateWindowSurface(window);

            // find the number of milliseconds 
            int frameTime = SDL_GetTicks() - frameStart;

            // if we are rendering faster than FPS sleep the cpu
            if (frameTime < FRAME_DELAY)
                SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    FreeFiles();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

SDL_Surface* LoadImage(const char* fileName)
{
    SDL_Surface* imageLoaded = NULL;
    SDL_Surface* processedImage = NULL;

    imageLoaded = SDL_LoadBMP(fileName);

    if(imageLoaded != NULL)
    {
        processedImage = SDL_ConvertSurface(imageLoaded, backBuffer->format, 0);
        SDL_FreeSurface(imageLoaded);

        if(processedImage != NULL)
        {
            Uint32 colorKey = SDL_MapRGB(processedImage->format, 0xFF, 0, 0xFF);
            SDL_SetColorKey(processedImage, SDL_TRUE, colorKey);
        }
    }

    return processedImage;
}

bool ProgramIsRunning()
{
    SDL_Event event;
    inputDirectionX = 0.0f;

    // input buffer
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT])
        inputDirectionX = -1.0f;
    
    if (keys[SDL_SCANCODE_RIGHT])
        inputDirectionX = 1.0f;

    if (keys[SDL_SCANCODE_SPACE])
    {
        if(gameStarted == false)
        {
            autoPlay = false;
            gameStarted = true;
            ballMovementSpeed = 5.0f;
            if((rand()%2) == 1)
                ballXDirection *= -1.0f;
            if((rand()%2) == 1)
                ballYDirection *= -1.0f;
            startTime = SDL_GetTicks();
        }
    }

    if (keys[SDL_SCANCODE_A])
    {
        if(gameStarted == false)
        {
            autoPlay = true;
            gameStarted = true;
            ballMovementSpeed = 5.0f;
            if((rand()%2) == 1)
                ballXDirection *= -1.0f;
            if((rand()%2) == 1)
                ballYDirection *= -1.0f;
            startTime = SDL_GetTicks();
        }
    }

    
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return false;
        if (event.type == SDL_KEYDOWN) {
            /*if (event.key.keysym.sym == SDLK_LEFT)
                inputDirectionX = -1.0f;
            if (event.key.keysym.sym == SDLK_RIGHT)
                inputDirectionX = 1.0f;*/
        }
        if (event.type == SDL_MOUSEMOTION) {
            float x = event.motion.x;
            float y = event.motion.y;
        }
        
    }
    
    return true;
}

void DrawImage(SDL_Surface* image, SDL_Surface* destSurface, int x, int y)
{
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;

    SDL_BlitSurface( image, NULL, destSurface, &destRect);
}

void DrawImageFrame(SDL_Surface* image, SDL_Surface* destSurface,
                    int x, int y, int width, int height, int frame)
{
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;

    int collumns = (*image).w/width;

    SDL_Rect sourceRect;
    sourceRect.y = (frame/collumns)*height;
    sourceRect.x = (frame%collumns)*width;
    sourceRect.w = width;
    sourceRect.h = height;

    SDL_BlitSurface(image, &sourceRect, destSurface, &destRect);
}

bool LoadFiles()
{
    // load images
    backGroundImage = LoadImage("assets/graphics/grid_background.bmp");
    sprite = LoadImage("assets/graphics/ball.bmp");
    paddleSprite = LoadImage("assets/graphics/paddle.bmp");

    if(sprite == nullptr)
        return false;

    if(backGroundImage == nullptr)
        return false;
    
    // load font
    gameFont = TTF_OpenFont("assets/fonts/alfphabet.ttf", 30);

    if (gameFont == nullptr)
        return false;

    // load sounds
    hitSound = Mix_LoadWAV("assets\\sounds\\JuhaniJunkala[RetroGameMusicPack]TitleScreen.wav");

    if(hitSound == nullptr)
        return false;

    // load music
    backGroundMusic = Mix_LoadMUS( "assets\\sounds\\JuhaniJunkala[RetroGameMusicPack]TitleScreen.wav" );

    if( backGroundMusic == nullptr )
        return false;

    return true;

}

void FreeFiles()
{
    if(sprite != nullptr)
    {
        SDL_FreeSurface(sprite);
        sprite = nullptr;
    }

    if(paddleSprite != nullptr)
    {
        SDL_FreeSurface(paddleSprite);
        paddleSprite = nullptr;
    }

    if(backGroundImage != nullptr)
    {
        SDL_FreeSurface(backGroundImage);
        backGroundImage = nullptr;
    }

    if (gameFont != nullptr)
    {
        TTF_CloseFont(gameFont);
        gameFont = nullptr;
    }

    if(hitSound != nullptr)
    {
        Mix_FreeChunk(hitSound);
        hitSound = nullptr;
    }

    if(backGroundMusic != nullptr)
    {
        Mix_FreeMusic(backGroundMusic);
        backGroundMusic = nullptr;
    }
}

void DrawText(SDL_Surface* surface, const char* string, int x, int y, TTF_Font* font, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface* renderedText = NULL;

    SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    renderedText = TTF_RenderText_Solid( font, string, color );

    SDL_Rect pos;

    pos.x = x;
    pos.y = y;

    SDL_BlitSurface( renderedText, NULL, surface, &pos );
    SDL_FreeSurface(renderedText);
}

bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2)
{
    if(rect1.x >= rect2.x+rect2.w)
        return false;

    if(rect1.y >= rect2.y+rect2.h)
        return false;

    if(rect2.x >= rect1.x+rect1.w)
        return false;

    if(rect2.y >= rect1.y+rect1.h)
        return false;

    return true;
}

// check for ball collisions
void BallCollisionCheck(SDL_Rect rect1)
{
    
    if(RectsOverlap(rect1, bottomWallRect))
    {
        paddleRect.x = SCREEN_WIDTH / 2;
        paddleRect.y = SCREEN_HEIGHT - 50;

        ballRect.x = SCREEN_WIDTH/2;
        ballRect.y = SCREEN_HEIGHT/2;
        ballXDirection = 0.5f;
        ballYDirection = 1.0f;
        ballMovementSpeed = 0.0f;
        gameStarted = false;
        autoPlay = false;

        if(highScore < score)
            highScore = score;
        score = 0;
        startTime = SDL_GetTicks();
    }

    if(RectsOverlap(rect1, topWallRect))
    {
        if(hasCollidedTop == false)
            ballYDirection = ballYDirection * (-1.0f);
        hasCollidedTop = true;
    } else
    {
        hasCollidedTop = false;
    }

    if(RectsOverlap(rect1, leftWallRect))
    {
        if(hasCollidedLeft == false)
            ballXDirection = ballXDirection * (-1.0f);
        hasCollidedLeft = true;
    } else
    {
        hasCollidedLeft = false;
    }

    if(RectsOverlap(rect1, rightWallRect))
    {
        if(hasCollidedRight == false)
            ballXDirection = ballXDirection * (-1.0f);
        hasCollidedRight = true;
    } else
    {
        hasCollidedRight = false;
    }

    if(RectsOverlap(rect1, paddleRect))
    {
        ballXDirection = ((rect1.x - paddleRect.x) / 30.0f) - 0.8f;
        ballYDirection = -1.0f ;
        if(ballMovementSpeed < 15.0f)
        {
            ballMovementSpeed += 1.0f;
        } else
        {
            ballMovementSpeed += 0.1f;
        }
    }
}
void ResetRects()
{
    ballRect.w = 20;
    ballRect.h = 20;

    paddleRect.w = 60;
    paddleRect.h = 3;

    // place bottom wall rect
    bottomWallRect.x = 0;
    bottomWallRect.y = SCREEN_HEIGHT - 1;
    bottomWallRect.w = SCREEN_WIDTH;
    bottomWallRect.h = 20;

    // place top wall rect
    topWallRect.x = 0;
    topWallRect.y = -18;
    topWallRect.w = SCREEN_WIDTH;
    topWallRect.h = 20;

    // place left wall rect
    leftWallRect.x = -18;
    leftWallRect.y = 0;
    leftWallRect.w = 20;
    leftWallRect.h = SCREEN_HEIGHT;

    // place right wall rect
    rightWallRect.x = SCREEN_WIDTH - 2;
    rightWallRect.y = 0;
    rightWallRect.w = 20;
    rightWallRect.h = SCREEN_HEIGHT;
}