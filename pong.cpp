#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//starts up SDL and creates window
bool init();

void close();

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

private:
	double mPosX, mPosY;
	//no x vel since we can't move on the x axis
	double mVelY;
};

Paddle::Paddle(bool player) {

	mPosY = SCREEN_HEIGHT/2;
	mVelY = 0;
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

	void move();
	void render();

private:
	double mPosX, mPosY;
	double mVelY, mVelX;
};

Ball::Ball() {
	mPosY = SCREEN_HEIGHT / 2;
	mPosX = SCREEN_WIDTH / 2;
	mVelY = -BALL_VEL;
	mVelX = -BALL_VEL;
}


void Ball::move() {
	mPosX += mVelX;
	if (mPosX < 0 || mPosX + BALL_WIDTH > SCREEN_WIDTH) {
		mPosX -= mVelX;
		mVelX = -mVelX;
	}

	mPosY += mVelY;
	if (mPosY < 0 || mPosY + BALL_HEIGHT > SCREEN_HEIGHT) {
		mPosY -= mVelY;
		mVelY = -mVelY;
	}
}

void Ball::render() {
	SDL_Rect fillRect = { mPosX, mPosY, BALL_WIDTH, BALL_HEIGHT };
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(gRenderer, &fillRect);
}

bool init() {
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not be loaded, SDL ERROR: %s\n", SDL_GetError());
		success = false;
	}
	else {
		gWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
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
			ball.move();

			//clear screen
			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(gRenderer);

			//render paddles
			player_paddle.render();
			opp_paddle.render();
			ball.render();

			//update screen
			SDL_RenderPresent(gRenderer);
		}

		close();
		return 0;
	}
}
