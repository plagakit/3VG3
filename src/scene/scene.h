#pragma once

#include "physics/physics_world.h"
#include "imgui.h"
#include <memory>

class Scene {

public:
	void Init();
	void Update(float dt);
	void Render();

	void SetScenario(int scenario);

private:
	std::unique_ptr<PhysicsWorld> physicsWorld;
	int currentScenario = 0;

	// GUI Stuff
	ImFont* font;
	ImFont* fontMedium;
	ImFont* fontBig;

	void DrawGUI();

};