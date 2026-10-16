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

// Sizes and speeds are designed for a 1280x800 screen; scale them so the game
// looks and plays the same on any screen, from a phone to a big monitor
float Scale(){
	return fminf(GetScreenWidth() / 1280.0f, GetScreenHeight() / 800.0f);
}

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
	float radius;

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

		// Touch or mouse: the paddle follows the finger/cursor, a bit faster than with the keys
		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			float target_y = GetMouseY() - height/2;
			float max_step = speed * 2 * FrameScale();
			y += Clamp(target_y - y, -max_step, max_step);
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


// Set sizes, speeds and paddle positions for the current screen size
void ApplyScale()
{
	const float s = Scale();

	ball.radius = 20 * s;
	// Keep the ball's current direction
	ball.speed_x = (ball.speed_x < 0 ? -7 : 7) * s;
	ball.speed_y = (ball.speed_y < 0 ? -7 : 7) * s;

	player.width = 25 * s;
	player.height = 120 * s;
	player.speed = 6 * s;
	player.x = GetScreenWidth() - player.width - 10 * s;

	cpu.width = 25 * s;
	cpu.height = 120 * s;
	cpu.speed = 6 * s;
	cpu.x = 10 * s;
}

void StartGame()
{
	const int screen_height = GetScreenHeight();

	PLAYER_SCORE = 0;
	CPU_SCORE = 0;

	ApplyScale();
	ball.ResetBall();

	player.y = screen_height/2 - player.height/2;
	cpu.y = screen_height/2 - cpu.height/2;

	state = PLAYING;
}

void DrawCourt()
{
	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();

	ClearBackground(Dark_Green);
	DrawCircle(screen_width/2, screen_height/2, 150 * Scale(), Red);
	DrawLine(screen_width/2, 0, screen_width/2, screen_height, WHITE);
}

// Draws text centered on x, with the font size scaled to the screen
void DrawCenteredText(const char *text, int x, int y, int font_size, Color color)
{
	font_size = fmaxf(font_size * Scale(), 10);
	DrawText(text, x - MeasureText(text, font_size)/2, y, font_size, color);
}

void UpdateDrawFrame()
{
	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();
	const float s = Scale();

	if(state == START_SCREEN)
	{
		// A tap on a touch screen counts as a left click
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
		{
			StartGame();
		}

		BeginDrawing();
		DrawCourt();
		DrawRectangle(0, 0, screen_width, screen_height, Fade(BLACK, 0.6f));
		DrawCenteredText("Ping Pong", screen_width/2, screen_height/2 - 100 * s, 100, WHITE);
		// Blink the prompt twice a second
		if((int)(GetTime() * 2) % 2 == 0)
		{
			DrawCenteredText("Click or tap to play", screen_width/2, screen_height/2 + 30 * s, 40, Light_Tone);
		}
		DrawCenteredText("Move with the UP and DOWN arrow keys, or drag on the screen", screen_width/2, screen_height - 60 * s, 24, Light_Tone);
		// Phones in portrait leave the court very narrow
		if(screen_height > screen_width)
		{
			DrawCenteredText("Tip: turn your phone sideways", screen_width/2, screen_height - 120 * s, 24, Yellow);
		}
		EndDrawing();
		return;
	}

	// Rescale everything when the window is resized or a phone is rotated
	if(IsWindowResized())
	{
		ApplyScale();
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
	DrawCenteredText(TextFormat("%i",CPU_SCORE), screen_width/4, 20 * s, 80, WHITE);
	DrawCenteredText(TextFormat("%i",PLAYER_SCORE), 3*screen_width/4, 20 * s, 80, WHITE);
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
