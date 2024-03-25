#include "scene.h"

#include "imgui.h"
#include "rlImGui.h"
#include <iostream>

void Scene::Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;

	const char* fontPath = "resources/Segoe-UI-Variable.ttf";
	font = io.Fonts->AddFontFromFileTTF(fontPath, 22.0f);
	fontMedium = io.Fonts->AddFontFromFileTTF(fontPath, 28.0f);
	fontBig = io.Fonts->AddFontFromFileTTF(fontPath, 32.0f);

	rlImGuiSetup(true);

	physicsWorld = std::make_unique<PhysicsWorld>();
	SetScenario(0);
}

void Scene::Update(float dt)
{
	if (physicsWorld != nullptr)
		physicsWorld->Update(dt);
}

void Scene::Render()
{
	rlImGuiBegin();
	ImGui::PushFont(font);

	if (physicsWorld == nullptr)
	{
		ImGui::Button("Physics world doesn't exist!", ImVec2(100, 100));
		rlImGuiEnd();
		return;
	}

	physicsWorld->Render();
	DrawGUI();

	ImGui::PopFont();
	rlImGuiEnd();

}

void Scene::DrawGUI()
{
	//ImGui::ShowDemoWindow();

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	static const ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_None
		| ImGuiWindowFlags_NoResize;

	DrawFPS(viewport->Size.x - 90, 10);

	ImVec2 settingsSize = ImVec2(viewport->Size.x / 5, viewport->Size.y / 2);
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(0, 0)); // size 0 will make the window autofit conte
	ImGui::Begin("Settings", NULL, WINDOW_FLAGS);

	// Scenario
	ImGui::Text("Scenario: %d", currentScenario);
	ImGui::SameLine();
	if (ImGui::Button("+"))
		SetScenario(currentScenario + 1);

	bool canDecrementScenario = currentScenario <= 0;
	if (canDecrementScenario) ImGui::BeginDisabled();
		ImGui::SameLine();
		if (ImGui::Button("-"))
			SetScenario(currentScenario - 1);
	if (canDecrementScenario) ImGui::EndDisabled();

	// Camera
	ImGui::Checkbox("Turn on orthographic camera", &isCameraOrthographic);
	physicsWorld->camera.projection = isCameraOrthographic ? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;
	ImGui::PushItemWidth(150);
	if (isCameraOrthographic)
	{
		physicsWorld->camera.fovy = orthographicNearWidth;
		ImGui::DragFloat("Camera Width", &orthographicNearWidth, 1.0f, 0, 100, "%.1f");
	} 
	else 
	{
		physicsWorld->camera.fovy = perspectiveFOV;
		ImGui::DragFloat("Camera FOV", &perspectiveFOV, 1.0f, 5.0f, 90.0f, "%.0f");
	}
	ImGui::DragFloat3("Camera Position", cameraPos, 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
	RVector3 cameraVecPos = RVector3(cameraPos[0], cameraPos[1], cameraPos[2]);
	physicsWorld->camera.SetPosition(cameraVecPos);
	physicsWorld->camera.SetTarget(cameraVecPos + cameraLookAtOffset);

	// Other settings
	ImGui::Checkbox("Draw bounding spheres", &physicsWorld->drawBoundingSpheres);

	ImGui::PushItemWidth(70);
	ImGui::InputFloat("Coefficient of restitution", &physicsWorld->cRestitution);

	// Rigidbodies
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		for (int rbIdx = 0; rbIdx < physicsWorld->rigidbodies.size(); rbIdx++)
		{
			RigidBody2D& rb = physicsWorld->rigidbodies[rbIdx];
			ImGui::PushID(rbIdx);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			bool node_open = ImGui::TreeNode("RigidBody", "Body %u", rbIdx);
			ImGui::TableSetColumnIndex(1);

			if (node_open)
			{

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();



	ImGui::End();
}


void Scene::SetScenario(int scenario)
{
	// Reset & update physics world settings
	physicsWorld->Init();
	physicsWorld->camera.projection = isCameraOrthographic ? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;

	// Add current scenario
	currentScenario = scenario;
	if (currentScenario == 0)
	{
		RigidBody2D rb1 = RigidBody2D();
		rb1.position = RVector3(- 3, 0.3f, 0);
		rb1.velocity = RVector3(1, 0, 0);
		rb1.angularVelocity = PI * 0.25f;
		rb1.color = BLUE;

		RigidBody2D rb2 = RigidBody2D();
		rb2.position = RVector3(3, -0.1f, 0);
		rb2.velocity = RVector3(-1, 0, 0 );
		rb2.rotation = PI / 4;
		rb2.color = RED;

		physicsWorld->rigidbodies.push_back(rb1);
		physicsWorld->rigidbodies.push_back(rb2);
	}
	if (currentScenario == 1)
	{
		float invMasses[] = { 1, 1.0f / 10, 0 };
		float radii[] = { 1, 10, 100 };
		float xPoses[] = { -20, 0, 120 };
		float xVels[] = { 10, 10, 0 };

		for (int i = 0; i < 3; i++)
		{
			RigidBody2D rb;
			rb.position = RVector3(xPoses[i], 0, -200);
			rb.velocity = RVector3(xVels[i], 0, 0);
			rb.inverseMass = invMasses[i];
			rb.radius = radii[i];
			rb.boundingRadius = rb.radius * 1.8f;
			physicsWorld->rigidbodies.push_back(rb);
		}
		physicsWorld->rigidbodies[1].rotation = PI / 4;
	}
	if (currentScenario == 2)
	{
		RigidBody2D r1, r2;
		r1.color = RColor::Blue();
		r1.position = RVector3(0.5f, 0.5f, 3);
		r1.SetCubeSideLength(0.6f);
		r1.doGravity = true;

		r2.color = RColor::Red();
		r2.position = RVector3(0.2f, -2, 3);
		r2.inverseMass = 0.0f;
		r2.inverseMOI = 0.0f;

		physicsWorld->rigidbodies.push_back(r1);
		physicsWorld->rigidbodies.push_back(r2);
	}

	// Reset scene settings that are dependant on scenario
	cameraPos[0] = physicsWorld->camera.position.x;
	cameraPos[1] = physicsWorld->camera.position.y;
	cameraPos[2] = physicsWorld->camera.position.z;
	cameraLookAtOffset =  RVector3(physicsWorld->camera.target) - RVector3(physicsWorld->camera.position);
}
