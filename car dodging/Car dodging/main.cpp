#include<SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>

using namespace std;

int score = 0, lvl = 1, prevScore;
int highscore = 0;
bool music = true;

const int SCREEN_WIDTH = 360;
const int SCREEN_HEIGHT = 640;
const int START = 520;
const int line1 = 24, line2 = 114, line3 = 208, line4 = 298;
const int maxLvl = 7;

static const int CAR_WIDTH = 40;
static const int CAR_HEIGHT = 75;
static const int CARS_VEL = 4;
static const int degreeVel = 4;

static const int OBSTACLE_WIDTH = 40;
static const int OBSTACLE_HEIGHT = 40;

static const int OBSTACLE_VEL = 5;
static const int OBJ_START = -100;
static const int OBJ_DISTANCE = 75;
static const int reduceObjDistance = 5;
static const int OBSTACLE_VEL_INC = 1;

class Texture
{
public:
    Texture();

    ~Texture();

    bool loadFromFile(std::string path);

    void free();

    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    void render(int x, int y, int w, int h, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

    int getWidth();
    int getHeight();

    void update_sprite();
    SDL_Texture* mTexture;
    SDL_Rect sprite;
    int x;
    int y;
    int W;
    int H;

    int mWidth;
    int mHeight;
};


class LButton : public Texture
{
public:
    LButton();

    void setPosition(int x, int y, int w, int h);

    bool handleEvent(SDL_Event* e);

private:
    SDL_Point mPosition;
};

class Text : public Texture
{
public:

    void loadFromRenderedText(std::string textureText, SDL_Color textColor);

    void loadText(string text, int size);
};

class Score : public Texture
{
public:

    void loadFromRenderedText(std::string textureText, SDL_Color textColor);

    void loadText(string text, int size);
};

class BlueCar : public Texture
{
public:

    BlueCar();

    void handle_Event(SDL_Event& e);

    void turnL();

    void turnR();

    void move();

    void render();

    int bVel;
    int bdegree;
};

class RedCar : public Texture
{
public:

    RedCar();

    void handle_Event(SDL_Event& e);

    void turnL();

    void turnR();

    void move();

    void render();

    int rVel;
    int rdegree;
};

class Obstacle : public Texture
{
public:
    Obstacle();

    void create(int line);

    void update_Pos();

    void show();

    int obVel, obLine, obType;
};

bool init();

bool loadDot();

bool loadBackground();

bool loadBlueCar();

void close();

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

TTF_Font* gFont = NULL;

Mix_Music* mOpen = NULL;

Mix_Music* mBackgroundMusic = NULL;
Mix_Chunk* mPause = NULL;
Mix_Chunk* mGameOver1 = NULL;
Mix_Chunk* mGameOver2 = NULL;
Mix_Chunk* mGameOver3 = NULL;
Mix_Chunk* mGameOver4 = NULL;
Mix_Chunk* mClick = NULL;
Mix_Chunk* mHighScore = NULL;
Mix_Chunk* mScore = NULL;

Texture gBackground;
LButton gPlay, gDarkBackground, gMusicOn, gMusicOff, gPause, gHighScore, gReplay, gHome;
Texture gBlueCar, gRedCar;
Texture bDot, bSquare, rDot, rSquare;
Text gText;
Score scoreText, highScoreText, gscoreText, ghighScoreText;

Texture::Texture()
{
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

Texture::~Texture()
{
    free();
}

bool Texture::loadFromFile(std::string path)
{
    free();

    SDL_Texture* newTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }
        else
        {
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        SDL_FreeSurface(loadedSurface);
    }

    mTexture = newTexture;
    return mTexture != NULL;
}

void Texture::free()
{
    if (mTexture != NULL)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void Texture::render(int x, int y, int w, int h, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = { x, y, w, h };

    SDL_RenderCopyEx(gRenderer, mTexture, NULL, &renderQuad, angle, center, flip);
}

int Texture::getWidth()
{
    return mWidth;
}

int Texture::getHeight()
{
    return mHeight;
}

void Texture::update_sprite()
{
    sprite.x = x;
    sprite.y = y;
    sprite.h = H;
    sprite.w = W;
}

void Text::loadFromRenderedText(string textureText, SDL_Color textColor)
{
    free();

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface == NULL)
    {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture == NULL)
        {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        SDL_FreeSurface(textSurface);
    }
}

void Text::loadText(string text, int size)
{
    gFont = TTF_OpenFont("ttf/gasalt.thin.ttf", size);
    if (gFont == NULL)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        SDL_Color textColor = { 220, 220, 220 };
        loadFromRenderedText(text, textColor);
    }
}

void Score::loadFromRenderedText(string textureText, SDL_Color textColor)
{
    free();

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface == NULL)
    {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture == NULL)
        {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        SDL_FreeSurface(textSurface);
    }
}

void Score::loadText(string text, int size)
{
    gFont = TTF_OpenFont("ttf/alienleaguebold.ttf", size);
    if (gFont == NULL)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        SDL_Color textColor = { 220, 220, 220 };
        loadFromRenderedText(text, textColor);
    }
}

LButton::LButton()
{
    mPosition.x = 0;
    mPosition.y = 0;
}

void LButton::setPosition(int x, int y, int w, int h)
{
    mPosition.x = x;
    mPosition.y = y;
    W = w;
    H = h;
}

bool LButton::handleEvent(SDL_Event* e)
{
    if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        bool inside = true;

        if (x < mPosition.x)
        {
            inside = false;
        }
        else if (x > mPosition.x + W)
        {
            inside = false;
        }
        else if (y < mPosition.y)
        {
            inside = false;
        }
        else if (y > mPosition.y + H)
        {
            inside = false;
        }

        if (inside && e->type == SDL_MOUSEBUTTONDOWN)
            return true;
    }
    return false;
}

Obstacle::Obstacle()
{
    mTexture = NULL;
    W = OBSTACLE_WIDTH;
    H = OBSTACLE_HEIGHT;
}

void Obstacle::create(int preLine)
{
    if (y <= OBJ_START || y >= SCREEN_HEIGHT + obVel)
    {
        y = OBJ_START;

        obLine = rand() % 2;
        if (preLine > 180 && preLine < 360) switch (obLine)
        {
        case 0:
            x = line1;
            break;
        case 1:
            x = line2;
            break;
        }

        if (preLine < 180 && preLine > 0) switch (obLine)
        {
        case 0:
            x = line3;
            break;
        case 1:
            x = line4;
            break;
        }
        obType = rand() % 2;
    }
}

void Obstacle::update_Pos()
{
    obVel = OBSTACLE_VEL + lvl / 2 * OBSTACLE_VEL_INC;
    y += obVel;
    if (x == line1 || x == line2 || x == line3 || x == line4)
        update_sprite();
}

void Obstacle::show()
{
    switch (x)
    {
    case line1:
        if (obType == 1) bDot.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        else if (obType == 0) bSquare.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        break;
    case line2:
        if (obType == 1) bDot.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        else if (obType == 0) bSquare.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        break;
    case line3:
        if (obType == 1) rDot.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        else if (obType == 0) rSquare.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        break;
    case line4:
        if (obType == 1) rDot.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        else if (obType == 0) rSquare.render(x, y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
        break;
    }
}

BlueCar::BlueCar()
{
    x = line1;
    y = START;

    bVel = 0;
    bdegree = 0;
    W = CAR_WIDTH;
    H = CAR_HEIGHT;
}

void BlueCar::handle_Event(SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
    {
        if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_a)
        {
            if (x == line1) turnL();
            else if (x == line2) turnR();
        }
    }
}

void BlueCar::turnL()
{
    bVel = -CARS_VEL;
}

void BlueCar::turnR()
{
    bVel = CARS_VEL;
}

void BlueCar::move()
{
    if ((x >= line1) && (x <= line2))
    {
        x -= bVel;
    }

    if ((x > line1) && (x < line2))
        if (x > 68) bdegree -= degreeVel;
        else if (x <= 68) bdegree += degreeVel;

    if (x > line2) x = line2;
    else if (x < line1) x = line1;
    update_sprite();
}

void BlueCar::render()
{
    gBlueCar.render(x, y, CAR_WIDTH, CAR_HEIGHT, bdegree);

}

RedCar::RedCar()
{
    x = line4;
    y = START;

    rVel = 0;
    rdegree = 0;
    W = CAR_WIDTH;
    H = CAR_HEIGHT;
}

void RedCar::handle_Event(SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
    {
        if (e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_d)
        {
            if (x == line3) turnR();
            else if (x == line4) turnL();
        }
    }
}

void RedCar::turnL()
{
    rVel = CARS_VEL;
}

void RedCar::turnR()
{
    rVel = -CARS_VEL;
}

void RedCar::move()
{
    if ((x >= line3) && (x <= line4))
    {
        x -= rVel;
    }

    if ((x > line3) && (x < line4))
        if (x > 252) rdegree -= degreeVel;
        else if (x <= 252) rdegree += degreeVel;

    if (x > line4) x = line4;
    else if (x < line3) x = line3;
    update_sprite();
}

void RedCar::render()
{
    gRedCar.render(x, y, CAR_WIDTH, CAR_HEIGHT, rdegree);
}

bool init()
{
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

        gWindow = SDL_CreateWindow("2Cars", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }

                if (TTF_Init() == -1)
                {
                    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                    success = false;
                }

                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
                {
                    printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadAudio()
{
    bool success = true;
    mScore = Mix_LoadWAV("sound/Score.wav");
    if (mScore == NULL)
    {
        printf("Failed to load score sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mHighScore = Mix_LoadWAV("sound/HighScore.mp3");
    if (mHighScore == NULL)
    {
        printf("Failed to load high score sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mGameOver1 = Mix_LoadWAV("sound/GameOver_Punch.mp3");
    if (mGameOver1 == NULL)
    {
        printf("Failed to load game over 1 sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mGameOver2 = Mix_LoadWAV("sound/GameOver_Nope.mp3");
    if (mGameOver2 == NULL)
    {
        printf("Failed to load GameOver2  sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mGameOver3 = Mix_LoadWAV("sound/GameOver_Wasted.mp3");
    if (mGameOver3 == NULL)
    {
        printf("Failed to load game over 3 sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mGameOver4 = Mix_LoadWAV("sound/GameOver_OOF.mp3");
    if (mGameOver4 == NULL)
    {
        printf("Failed to load game over 4 sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mClick = Mix_LoadWAV("sound/Click.wav");
    if (mClick == NULL)
    {
        printf("Failed to load click sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }
    mPause = Mix_LoadWAV("sound/Pause.mp3");

    mBackgroundMusic = Mix_LoadMUS("sound/backgroundMusic.mp3");
    if (mBackgroundMusic == NULL)
    {
        printf("Failed to load background music! SDL_mixer Error: %s\n", Mix_GetError());
        success = false;
    }

    return success;
}

bool loadMedia()
{
    bool success = true;

    if (!gPlay.loadFromFile("graphic/play.png")
        || (!gHome.loadFromFile("graphic/home.png"))
        || !gPause.loadFromFile("graphic/Pause_button.png")
        || !gMusicOn.loadFromFile("graphic/music.png")
        || !gMusicOff.loadFromFile("graphic/music_off.png")
        || !gDarkBackground.loadFromFile("graphic/black_background.png")
        || !gReplay.loadFromFile("graphic/replaybutton.png")
        || !gHighScore.loadFromFile("graphic/trophy.png"))
    {
        printf("Failed to load Media!\n");
        success = false;
    }

    return success;
}

bool loadBackground()
{
    bool success = true;

    if (!gBackground.loadFromFile("graphic/map.png"))
    {
        printf("Failed to load background texture!\n");
        success = false;
    }

    return success;
}

bool loadBlueCar()
{
    bool success = true;

    if (!gBlueCar.loadFromFile("graphic/Car_blue.png"))
    {
        printf("Failed to load car texture!\n");
        success = false;
    }

    return success;
}

bool loadRedCar()
{
    bool success = true;

    if (!gRedCar.loadFromFile("graphic/Car_red.png"))
    {
        printf("Failed to load car texture!\n");
        success = false;
    }

    return success;
}

bool loadDot()
{
    bool success = true;

    if (!bDot.loadFromFile("graphic/dot_blue.png")
        || !rDot.loadFromFile("graphic/dot_red.png")
        || !bSquare.loadFromFile("graphic/square_blue.png")
        || !rSquare.loadFromFile("graphic/square_red.png"))
    {
        printf("Failed to load car texture!\n");
        success = false;
    }

    return success;
}

bool hitASquare(BlueCar& blue, RedCar& red, Obstacle& obstacle)
{
    if (SDL_HasIntersection(&blue.sprite, &obstacle.sprite)
        || SDL_HasIntersection(&red.sprite, &obstacle.sprite))
        if (obstacle.obType == 1)
        {
            score++;
            if (music == true) Mix_PlayChannel(-1, mScore, 0);
            obstacle.obType = 2;
            return false;
        }
        else if (obstacle.obType == 0)
        {
            return true;
        }
    return false;
}

bool missAPoint(Obstacle& obstacle)
{
    if ((obstacle.y + OBSTACLE_HEIGHT >= SCREEN_HEIGHT)
        && (obstacle.obType == 1)
        && (obstacle.y <= SCREEN_HEIGHT + obstacle.obVel))
    {
        return true;
    }
    else if ((obstacle.y >= SCREEN_HEIGHT)
        && (obstacle.obType == 0 || obstacle.obType == 2)
        && (obstacle.y < 1004))
    {
        return false;
    }
    return false;
}

void close()
{
    rDot.free();
    bDot.free();
    rSquare.free();
    bSquare.free();
    gBackground.free();
    gBlueCar.free();
    gDarkBackground.free();
    gPlay.free();
    gReplay.free();
    gHighScore.free();
    gMusicOff.free();
    gMusicOn.free();
    gHome.free();
    gPause.free();

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* args[])
{
    srand(time(NULL));
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        if (!loadAudio()
            || !loadBackground()
            || (!loadDot())
            || (!loadBlueCar())
            || (!loadRedCar())
            || !loadMedia())
        {
            printf("Failed to load media!\n");
        }
        else
        {
            bool play = false;
            bool pause = false;
            bool quit = false;
            bool home = true;
            bool replay = false;

            SDL_Event e;

            Obstacle O1, O2, O3, O4, O5, O6;

            BlueCar blueCar;

            RedCar redCar;

            Text message, textScore, textHighScore;

            while (!quit)
            {
                if (home)
                {
                    if (music == true)
                    {
                        Mix_PlayMusic(mBackgroundMusic, -1);
                    }
                    gText.loadText("2Cars", 150);
                    Mix_PlayMusic(mOpen, -1);
                }
                while (home)
                {
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (gPlay.handleEvent(&e) || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_SPACE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            home = false;
                            play = true;
                            Mix_HaltMusic();
                        }
                        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            quit = true;
                            home = false;
                        }
                        if (gMusicOn.handleEvent(&e))
                        {
                            if (music == true) {
                                Mix_PlayChannel(-1, mClick, 0);
                                music = false;
                                Mix_HaltMusic();
                            }
                            else if (music == false)
                            {
                                Mix_PlayChannel(-1, mClick, 0);
                                music = true;
                                Mix_PlayMusic(mBackgroundMusic, -1);
                            }
                        }
                        if (gHighScore.handleEvent(&e))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            if (music == true) Mix_PlayChannel(-1, mHighScore, 0);
                            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                            SDL_RenderClear(gRenderer);

                            gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                            gDarkBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

                            ghighScoreText.loadText("Best  ", 100);
                            highScoreText.loadText(to_string(highscore), 150);
                            ghighScoreText.render(100, 100, ghighScoreText.mWidth, ghighScoreText.mHeight - 10);
                            if (highscore >= 10)
                                highScoreText.render(133, 260, highScoreText.mWidth, highScoreText.mHeight - 30);
                            else if (highscore < 10)
                                highScoreText.render(150, 260, highScoreText.mWidth, highScoreText.mHeight - 30);
                            SDL_RenderPresent(gRenderer);
                            SDL_Delay(2000);
                        }
                        if (gPlay.handleEvent(&e) || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_SPACE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            home = false;
                            play = true;

                            O1.y = -1004;
                            if (rand() % 2 == 1) O1.x = line4;
                            else O1.x = line1;
                            O2.y = -1004;
                            O3.y = -1004;
                            O4.y = -1004;
                            O5.y = -1004;
                            O6.y = 1004;
                            blueCar.x = line1;
                            redCar.x = line4;
                            blueCar.bdegree = 0;
                            redCar.rdegree = 0;
                            score = 0;
                            blueCar.bVel = 0;
                            redCar.rVel = 0;
                            lvl = 1;
                        }
                    }
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    gPlay.setPosition(82, 220, 210, 210);
                    gHighScore.setPosition(80, 460, 80, 80);
                    gMusicOn.setPosition(200, 460, 80, 80);

                    gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    gDarkBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    gPlay.render(82, 220, 210, 210);
                    gHighScore.render(80, 460, 80, 80);
                    if (music == true) gMusicOn.render(200, 460, 80, 80);
                    else if (music == false) gMusicOff.render(200, 460, 80, 80);
                    gText.render(25, 60, gText.mWidth, gText.mHeight);


                    SDL_RenderPresent(gRenderer);
                }

                while (pause)
                {
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE))
                        {
                            quit = true;
                            pause = false;
                        }
                        if (gPlay.handleEvent(&e) || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_SPACE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            pause = false;
                            play = true;
                        }
                        if (gHome.handleEvent(&e))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            pause = false;
                            home = true;
                        }
                        if (gMusicOn.handleEvent(&e))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            if (music == true) music = false;
                            else if (music == false) music = true;
                        }
                    }
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    blueCar.render();
                    redCar.render();
                    O1.show();
                    O2.show();
                    O3.show();
                    O4.show();
                    O5.show();
                    O6.show();
                    gPause.render(10, 5, 40, 53);

                    gText.loadText("Continue?", 90);
                    gPlay.setPosition(98, 270, 180, 180);
                    gHome.setPosition(300, 80, 50, 50);
                    gMusicOn.setPosition(300, 15, 50, 50);

                    gDarkBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    gPlay.render(98, 270, 180, 180);
                    gHome.render(300, 80, 50, 50);
                    if (music == true) gMusicOn.render(300, 15, 50, 50);
                    else if (music == false) gMusicOff.render(300, 15, 50, 50);
                    gText.render(50, 180, gText.mWidth, gText.mHeight);

                    SDL_RenderPresent(gRenderer);

                    if (play == true)
                    {
                        if (music == true) Mix_PlayChannel(-1, mPause, 0);
                        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderClear(gRenderer);
                        gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                        blueCar.render();
                        redCar.render();
                        O1.show();
                        O2.show();
                        O3.show();
                        O4.show();
                        O5.show();
                        O6.show();
                        scoreText.loadText("3", 150);
                        scoreText.render((SCREEN_WIDTH - scoreText.mWidth) / 2, (SCREEN_HEIGHT - scoreText.mHeight) / 2 - 100, scoreText.mWidth, scoreText.mHeight - 30);
                        SDL_RenderPresent(gRenderer);

                        SDL_Delay(1000);

                        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderClear(gRenderer);
                        gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                        blueCar.render();
                        redCar.render();
                        O1.show();
                        O2.show();
                        O3.show();
                        O4.show();
                        O5.show();
                        O6.show();
                        scoreText.loadText("2", 150);
                        scoreText.render((SCREEN_WIDTH - scoreText.mWidth) / 2, (SCREEN_HEIGHT - scoreText.mHeight) / 2 - 100, scoreText.mWidth, scoreText.mHeight - 30);
                        SDL_RenderPresent(gRenderer);

                        SDL_Delay(1000);

                        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderClear(gRenderer);
                        gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                        blueCar.render();
                        redCar.render();
                        O1.show();
                        O2.show();
                        O3.show();
                        O4.show();
                        O5.show();
                        O6.show();
                        scoreText.loadText("1", 150);
                        scoreText.render((SCREEN_WIDTH - scoreText.mWidth) / 2, (SCREEN_HEIGHT - scoreText.mHeight) / 2 - 100, scoreText.mWidth, scoreText.mHeight - 30);
                        SDL_RenderPresent(gRenderer);

                        SDL_Delay(1000);

                        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderClear(gRenderer);
                        gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                        blueCar.render();
                        redCar.render();
                        O1.show();
                        O2.show();
                        O3.show();
                        O4.show();
                        O5.show();
                        O6.show();
                        scoreText.loadText("GO", 150);
                        scoreText.render((SCREEN_WIDTH - scoreText.mWidth) / 2, (SCREEN_HEIGHT - scoreText.mHeight) / 2 - 100, scoreText.mWidth, scoreText.mHeight - 30);
                        SDL_RenderPresent(gRenderer);
                        SDL_Delay(1000);
                    }
                }

                while (play)
                {
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE))
                        {
                            quit = true;
                            play = false;
                        }
                        blueCar.handle_Event(e);
                        redCar.handle_Event(e);
                        if (gPause.handleEvent(&e) || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_SPACE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            play = false;
                            pause = true;
                        }
                    }

                    if ((O6.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O6.y < OBJ_DISTANCE + O6.obVel - reduceObjDistance * lvl) || O6.y == 1004) O1.create(O6.x);
                    if (O1.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O1.y < OBJ_DISTANCE + O1.obVel - reduceObjDistance * lvl) O2.create(O1.x);
                    if (O2.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O2.y < OBJ_DISTANCE + O2.obVel - reduceObjDistance * lvl) O3.create(O2.x);
                    if (O3.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O3.y < OBJ_DISTANCE + O3.obVel - reduceObjDistance * lvl) O4.create(O3.x);
                    if (O4.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O4.y < OBJ_DISTANCE + O4.obVel - reduceObjDistance * lvl) O5.create(O4.x);
                    if (O5.y >= OBJ_DISTANCE - reduceObjDistance * lvl && O5.y < OBJ_DISTANCE + O5.obVel - reduceObjDistance * lvl) O6.create(O5.x);

                    blueCar.move();
                    redCar.move();
                    O1.update_Pos();
                    O2.update_Pos();
                    O3.update_Pos();
                    O4.update_Pos();
                    O5.update_Pos();
                    O6.update_Pos();

                    if (hitASquare(blueCar, redCar, O1)
                        || hitASquare(blueCar, redCar, O2)
                        || hitASquare(blueCar, redCar, O3)
                        || hitASquare(blueCar, redCar, O4)
                        || hitASquare(blueCar, redCar, O5)
                        || hitASquare(blueCar, redCar, O6))
                    {
                        play = false;
                        replay = true;
                    }

                    if (missAPoint(O1)
                        || missAPoint(O2)
                        || missAPoint(O3)
                        || missAPoint(O4)
                        || missAPoint(O5)
                        || missAPoint(O6))
                    {
                        play = false;
                        replay = true;
                    }

                    gPause.setPosition(10, 5, 40, 53);
                    scoreText.loadText(to_string(score), 60);


                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    blueCar.render();
                    redCar.render();
                    O1.show();
                    O2.show();
                    O3.show();
                    O4.show();
                    O5.show();
                    O6.show();
                    gPause.render(10, 5, 40, 53);
                    if (score >= 10) scoreText.render(305, 8, scoreText.mWidth, scoreText.mHeight - 10);
                    else if (score < 10) scoreText.render(320, 8, scoreText.mWidth, scoreText.mHeight - 10);

                    SDL_RenderPresent(gRenderer);

                    if ((score % 5 == 0) && (score > prevScore) && (lvl <= maxLvl)) lvl++;
                    prevScore = score;

                    if (replay == true && play == false)
                    {
                        switch (rand() % 4)
                        {
                        case 0:
                            if (music == true) Mix_PlayChannel(-1, mGameOver1, 0);
                            SDL_Delay(500);
                            break;
                        case 1:
                            if (music == true) Mix_PlayChannel(-1, mGameOver2, 0);
                            SDL_Delay(150);
                            break;
                        case 2:
                            if (music == true) Mix_PlayChannel(-1, mGameOver3, 0);
                            SDL_Delay(2400);
                            break;
                        case 3:
                            if (music == true) Mix_PlayChannel(-1, mGameOver4, 0);
                            SDL_Delay(360);
                            break;
                        }

                        if (score >= highscore) highscore = score;

                    }
                }

                if (replay == true)
                {
                    gText.loadText("GAME OVER", 80);
                    ghighScoreText.loadText("Best  ", 50);
                    gscoreText.loadText("Score ", 50);
                    highScoreText.loadText(to_string(highscore), 50);
                    scoreText.loadText(to_string(score), 50);
                }
                while (replay)
                {
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            quit = true;
                            replay = false;
                        }
                        if (gReplay.handleEvent(&e) || (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_SPACE))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            replay = false;
                            play = true;

                            O1.y = -1004;
                            if (rand() % 2 == 1) O1.x = line4;
                            else O1.x = line1;
                            O2.y = -1004;
                            O3.y = -1004;
                            O4.y = -1004;
                            O5.y = -1004;
                            O6.y = 1004;
                            blueCar.x = line1;
                            redCar.x = line4;
                            blueCar.bdegree = 0;
                            redCar.rdegree = 0;
                            blueCar.bVel = 0;
                            redCar.rVel = 0;
                            score = 0;
                            lvl = 1;
                        }
                        if (gHome.handleEvent(&e))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            replay = false;
                            home = true;
                        }
                        if (gMusicOn.handleEvent(&e))
                        {
                            if (music == true) Mix_PlayChannel(-1, mClick, 0);
                            if (music == true) music = false;
                            else if (music == false) music = true;
                        }
                    }

                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderClear(gRenderer);

                    gBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    blueCar.render();
                    redCar.render();
                    O1.show();
                    O2.show();
                    O3.show();
                    O4.show();
                    O5.show();
                    O6.show();
                    gPause.render(10, 5, 40, 53);

                    gReplay.setPosition(105, 310, 150, 150);
                    gHome.setPosition(90, 480, 70, 70);
                    gMusicOn.setPosition(205, 480, 70, 70);

                    gDarkBackground.render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                    gReplay.render(105, 310, 150, 150);
                    gHome.render(90, 480, 70, 70);
                    if (music == true) gMusicOn.render(205, 480, 70, 70);
                    else gMusicOff.render(205, 480, 70, 70);
                    gText.render(15, 80, gText.mWidth, gText.mHeight);
                    ghighScoreText.render(80, 225, ghighScoreText.mWidth, ghighScoreText.mHeight - 10);
                    gscoreText.render(80, 180, gscoreText.mWidth, gscoreText.mHeight - 10);
                    highScoreText.render(240, 225, highScoreText.mWidth, highScoreText.mHeight - 10);
                    scoreText.render(240, 180, scoreText.mWidth, scoreText.mHeight - 10);

                    SDL_RenderPresent(gRenderer);
                }
            }
        }
    }

    close();
    return 0;
}
