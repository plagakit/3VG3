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

	// Settings
	bool isCameraOrthographic = false;
	float perspectiveFOV = 45.0f;
	float orthographicNearWidth = 10.0f;
	float cameraPos[3];
	RVector3 cameraLookAtOffset = RVector3::Zero();

	// GUI Stuff
	ImFont* font;
	ImFont* fontMedium;
	ImFont* fontBig;

	void DrawGUI();

};