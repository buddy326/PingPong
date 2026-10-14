#include "raylib.h"
#include "iostream"
#include "resource_dir.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace std;

Color Dark_Green  = Color{18, 55, 42, 255};
Color Green       = Color{46, 204, 113, 255};
Color Red         = Color{231, 76, 60, 255};
Color Light_Tone  = Color{236, 240, 241, 255};
Color Yellow = Color({243, 213, 91, 255});

int PLAYER_SCORE = 0;
int CPU_SCORE = 0;

enum GameState { START_SCREEN, PLAYING };
GameState state = START_SCREEN;

// Speeds are tuned in pixels per frame at 60 FPS; scale by this so the game
// runs at the same pace on any refresh rate (browsers run at the display's rate)
float FrameScale(){
	float dt = GetFrameTime();
	if(dt > 0.05f) dt = 0.05f; // avoid big jumps after the tab was in the background
	return dt * 60.0f;
}

class Ball{
	public:
	float x, y;
	float speed_x, speed_y;
	int radius;

	void Draw(){
		DrawCircle(x, y, radius, Yellow);
	}

	void Update(){
		x += speed_x * FrameScale();
		y += speed_y * FrameScale();

		if(y + radius >= GetScreenHeight() || y - radius <= 0)
		{
			speed_y *= -1;
		}

		if(x + radius >= GetScreenWidth())
		{
			CPU_SCORE++;
			ResetBall();
		}
		if(x - radius <= 0)
		{
			PLAYER_SCORE++;
			ResetBall();
		}
	}

	void ResetBall()
	{
		x = GetScreenWidth()/2;
		y = GetScreenHeight()/2;

		int speed_choices[2] = {-1, 1};
		speed_x *= speed_choices[GetRandomValue(0,1)];
		speed_y *= speed_choices[GetRandomValue(0,1)];
	}

};

class Paddle {

	protected:

	void LimitMovement(){
		if( y <= 0)
		{
			y = 0;
		}
		if(y + height >= GetScreenHeight())
		{
			y = GetScreenHeight() - height;
		}
	}

	public:
	float x, y;
	float width, height;
	float speed;

	void Draw(){
		DrawRectangleRounded(Rectangle{x, y, width, height}, 0.8, 0, Red);
	}

	void Update(){

		if(IsKeyDown(KEY_UP))
		{
			y = y - speed * FrameScale();
		}

		if(IsKeyDown(KEY_DOWN))
		{
			y = y + speed * FrameScale();
		}

		LimitMovement();
	}
};

class CpuPaddle: public Paddle
{
	public:

	void Update(int ball_y){

		if(y + height/2 > ball_y)
		{
			y = y - speed * FrameScale();
		}

		if(y + height/2 <= ball_y)
		{
			y = y + speed * FrameScale();
		}

		LimitMovement();
	}

};

Ball ball;
Paddle player;
CpuPaddle cpu;


void StartGame()
{
	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();

	PLAYER_SCORE = 0;
	CPU_SCORE = 0;

	ball.speed_x = 7;
	ball.speed_y = 7;
	ball.ResetBall();

	player.x = screen_width - player.width - 10;
	player.y = screen_height/2 - player.height/2;

	cpu.x = 10;
	cpu.y = screen_height/2 - cpu.height/2;

	state = PLAYING;
}

void DrawCourt()
{
	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();

	ClearBackground(Dark_Green);
	DrawCircle(screen_width/2, screen_height/2, 150, Red);
	DrawLine(screen_width/2, 0, screen_width/2, screen_height, WHITE);
}

void DrawCenteredText(const char *text, int y, int font_size, Color color)
{
	DrawText(text, GetScreenWidth()/2 - MeasureText(text, font_size)/2, y, font_size, color);
}

void UpdateDrawFrame()
{
	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();

	if(state == START_SCREEN)
	{
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
		{
			StartGame();
		}

		BeginDrawing();
		DrawCourt();
		DrawRectangle(0, 0, screen_width, screen_height, Fade(BLACK, 0.6f));
		DrawCenteredText("Ping Pong", screen_height/2 - 100, 100, WHITE);
		// Blink the prompt twice a second
		if((int)(GetTime() * 2) % 2 == 0)
		{
			DrawCenteredText("Click to play", screen_height/2 + 30, 40, Light_Tone);
		}
		DrawCenteredText("Use the UP and DOWN arrow keys to move", screen_height - 60, 24, Light_Tone);
		EndDrawing();
		return;
	}

	// Keep everything on screen when the window is resized
	if(IsWindowResized())
	{
		player.x = screen_width - player.width - 10;
		ball.ResetBall();
	}

	BeginDrawing();

	//Updating
	ball.Update();
	player.Update();
	cpu.Update(ball.y);

	//Checking for collisions
	// Only bounce when the ball is moving toward the paddle, and push it back out in
	// front of it, so it can't get stuck flipping direction inside the paddle
	if(ball.speed_x > 0 && CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player.x, player.y, player.width, player.height}))
	{
		ball.speed_x *= -1;
		ball.x = player.x - ball.radius;
	}

	if(ball.speed_x < 0 && CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{cpu.x, cpu.y, cpu.width, cpu.height}))
	{
		ball.speed_x *= -1;
		ball.x = cpu.x + cpu.width + ball.radius;
	}

	//Drawing
	DrawCourt();
	ball.Draw();
	cpu.Draw();
	player.Draw();
	DrawText(TextFormat("%i",CPU_SCORE), screen_width/4 -20, 20, 80, WHITE);
	DrawText(TextFormat("%i",PLAYER_SCORE), 3*screen_width/4 -20, 20, 80, WHITE);
	EndDrawing();
}

int main()
{
	cout << "Starting the game" << endl;

	// Resizable lets the game fill the browser window on the web
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	InitWindow(1280, 800, "Ping Pong");
	SetTargetFPS(60);

#if !defined(PLATFORM_WEB)
	// Fill the whole screen on desktop too
	MaximizeWindow();
#endif

	ball.radius = 20;

	player.width = 25;
	player.height = 120;
	player.speed = 6;

	cpu.height = 120;
	cpu.width = 25;
	cpu.speed = 6;

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
	while(WindowShouldClose() == false){
		UpdateDrawFrame();
	}
#endif

	CloseWindow();
	return 0;
}
