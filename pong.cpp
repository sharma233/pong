#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <cmath>

//could inherit some kind of texture interface
class Score {
public:
	Score();
	~Score();
	void render(int x, int y);
	void setScore(int newScore);
	void incrementScore();
	void free();
private:
	int mScore;
	int mWidth;
	int mHeight;
	SDL_Texture* mTexture;
	void loadTexture();
};

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
Score gPlayerScore;
Score gOppScore;
TTF_Font* gFont = NULL;

//starts up SDL and creates window
bool init();
void close();

bool checkCollision(SDL_Rect a, SDL_Rect b);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

class Paddle {
public:
	static const int PADDLE_WIDTH = 20;
	static const int PADDLE_HEIGHT = 100;

	static constexpr double PADDLE_VEL = 0.1;

	//Paddle();
	Paddle(bool player);

	void handleEvent(SDL_Event& e);
	void move();
	void render();

	SDL_Rect mCollider;

private:
	double mPosX, mPosY;
	//no x vel since we can't move on the x axis
	double mVelY;
};

Paddle::Paddle(bool player) {

	mPosY = SCREEN_HEIGHT/2;
	mVelY = 0;
	mCollider.w = PADDLE_WIDTH;
	mCollider.h = PADDLE_HEIGHT;

	if (player) {
		mPosX = 0;
	}
	else {
		mPosX = SCREEN_WIDTH - PADDLE_WIDTH;
	}
}


void Paddle::handleEvent(SDL_Event& e) {
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP: mVelY -= PADDLE_VEL; break;
		case SDLK_DOWN: mVelY += PADDLE_VEL; break;
		}
	}
	else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP: mVelY += PADDLE_VEL; break;
		case SDLK_DOWN: mVelY -= PADDLE_VEL; break;
		}
	}
}

void Paddle::move() {
	mPosY += mVelY;
	if (mPosY < 0 || mPosY + PADDLE_HEIGHT > SCREEN_HEIGHT) {
		mPosY -= mVelY;
	}
	mCollider.y = mPosY;
	mCollider.x = mPosX;
}

void Paddle::render() {
	SDL_Rect fillRect = { mPosX, mPosY, PADDLE_WIDTH, PADDLE_HEIGHT};
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(gRenderer, &fillRect);
}


class Ball{
public:
	static const int BALL_WIDTH = 10;
	static const int BALL_HEIGHT = 10;

	static constexpr double BALL_VEL = 0.05;

	Ball();

	void move(SDL_Rect& player_paddle_collider, SDL_Rect& opp_paddle_collider);
	void render();

private:
	double mPosX, mPosY;
	double mVelY, mVelX;
	SDL_Rect mCollider;
};

Ball::Ball() {
	mPosY = SCREEN_HEIGHT / 2;
	mPosX = SCREEN_WIDTH / 2;
	mVelY = -BALL_VEL;
	mVelX = -BALL_VEL;
	mCollider.w = BALL_WIDTH;
	mCollider.h = BALL_HEIGHT;
}


void Ball::move(SDL_Rect& player_paddle_collider, SDL_Rect& opp_paddle_collider) {
	//scoring logic, should also get moved out of here
	mPosX += mVelX;
	if (mPosX < 0) {
		gOppScore.incrementScore();
		mPosX = SCREEN_WIDTH / 2;
		mVelX = -mVelX;
	}
	if (mPosX + BALL_WIDTH > SCREEN_WIDTH) {
		gPlayerScore.incrementScore();
		mPosX = SCREEN_WIDTH / 2;
		mVelX = -mVelX;
	}

	mPosY += mVelY;
	if (mPosY < 0 || mPosY + BALL_HEIGHT > SCREEN_HEIGHT) {
		mPosY -= mVelY;
		mVelY = -mVelY;
	}

	//should get moved out of here
	if (checkCollision(mCollider, player_paddle_collider) || checkCollision(mCollider, opp_paddle_collider)) {
		mPosY -= mVelY*2;
		mPosX -= mVelX*2;
		mVelX = -mVelX;
	}
	mCollider.x = mPosX;
	mCollider.y = mPosY;
}

void Ball::render() {
	SDL_Rect fillRect = { mPosX, mPosY, BALL_WIDTH, BALL_HEIGHT };
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(gRenderer, &fillRect);
}



Score::Score() {
	mScore = 0;
	mWidth = 0;
	mHeight = 0;
	mTexture = NULL;
}

Score::~Score() {
	free();
}

void Score::loadTexture() {
	free();
	SDL_Color color = {255, 255, 255};
	SDL_Surface* surface = TTF_RenderText_Solid(gFont, std::to_string(mScore).c_str(), color);
	mTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
	mWidth = surface->w;
	mHeight = surface->h;
	SDL_FreeSurface(surface);
}
void Score::free() {
	if (mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
 }

void Score::render(int x, int y) {
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	SDL_RenderCopyEx(gRenderer, mTexture, NULL, &renderQuad, NULL, NULL, SDL_FLIP_NONE);
}

void Score::setScore(int newScore) {
	mScore = newScore;
	loadTexture();
}

void Score::incrementScore() {
	setScore(mScore + 1);
}

bool init() {
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not be loaded, SDL ERROR: %s\n", SDL_GetError());
		success = false;
	} else {
		gWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		} else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			} else {
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				} else {
					gFont = TTF_OpenFont("lazy.ttf", 28);
					if (gFont == NULL) {
						printf("Font could not be created! SDL Error: %s\n", SDL_GetError());
						success = false;
					}
				}
			}
		}
	}
	return success;
}

void close() {
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b) {
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	if (bottomA <= topB) {
		return false;
	}

	if (topA >= bottomB) {
		return false;
	}

	if (rightA <= leftB) {
		return false;
	}

	if (leftA >= rightB) {
		return false;
	}

	return true;
}

int main(int argc, char* args[]) {
	if (!init()) {
		printf("Failed to initialize!\n");
	}
	else {
		bool quit = false;
		Paddle player_paddle(true);
		Paddle opp_paddle(false);
		Ball ball;

		SDL_Event e;

		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
				player_paddle.handleEvent(e);
			}

			player_paddle.move();
			opp_paddle.move();
			ball.move(player_paddle.mCollider, opp_paddle.mCollider);

			//clear screen
			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(gRenderer);

			//render paddles
			player_paddle.render();
			opp_paddle.render();
			ball.render();
			gPlayerScore.render(SCREEN_WIDTH/4, SCREEN_HEIGHT/4);
			gOppScore.render(SCREEN_WIDTH / 1.5, SCREEN_HEIGHT / 4);

			//update screen
			SDL_RenderPresent(gRenderer);
		}

		close();
		return 0;
	}
}
