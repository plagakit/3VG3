#include "raylib-cpp.hpp"
#include "imgui.h"
#include "rlImGui.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include "scene/scene.h"

int screenWidth = 1280;
int screenHeight = 720;
Scene scene;

void UpdateDrawFrame(void);

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Engine");
	SetTargetFPS(60);
	rlImGuiSetup(true);

	scene.Init();

#ifdef PLATFORM_WEB
	// Web main loop
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

	// Desktop main loop
	while (!WindowShouldClose())
		UpdateDrawFrame();
#endif

	rlImGuiShutdown();
	CloseWindow();
	return 0;
}


void UpdateDrawFrame(void)
{ 
	scene.Update(GetFrameTime());

	BeginDrawing();

		ClearBackground(BLACK);
		scene.Render();
	EndDrawing();
}