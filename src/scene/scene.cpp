#include "scene.h"

#include "imgui.h"
#include "rlImGui.h"

void Scene::Init()
{
	const char* fontPath = "resources/Segoe-UI-Variable.ttf";
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;
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

	static const ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_None;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 settingsSize = ImVec2(viewport->Size.x / 5, viewport->Size.y / 2);
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	//ImGui::SetNextWindowSize(settingsSize);
	ImGui::Begin("Settings");

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
	currentScenario = scenario;
	physicsWorld->Init();

	if (currentScenario == 0)
	{
		RigidBody2D rb1 = RigidBody2D();
		rb1.position = { -3, 0.3f, 0 };
		rb1.velocity = Vector3{ 1, 0, 0 };
		rb1.angularVelocity = PI * 0.25f;
		rb1.color = BLUE;

		RigidBody2D rb2 = RigidBody2D();
		rb2.position = { 3, -0.1f, 0 };
		rb2.velocity = Vector3{ -1, 0, 0 };
		rb2.rotation = PI / 4;
		rb2.color = RED;

		physicsWorld->rigidbodies.push_back(rb1);
		physicsWorld->rigidbodies.push_back(rb2);
	}
	if (currentScenario == 1)
	{
		float invMasses[] = { 1, 1.0f / 10, 0 };
		float radii[] = { 1, 10, 100 };
		float xPoses[] = { -11, 0, 120 };
		float xVels[] = { 10, 10, 0 };

		for (int i = 0; i < 3; i++)
		{
			RigidBody2D rb;
			rb.position = RVector3(xPoses[i], 0, -200);
			rb.velocity = RVector3(xVels[i], 0, 0);
			rb.inverseMass = invMasses[i];
			rb.radius = radii[i];
			physicsWorld->rigidbodies.push_back(rb);
		}
	}
	if (currentScenario == 2)
	{
		RigidBody2D r1, r2;
		r1.color = RColor::Blue();
		r1.position = { 0.5f, 0.5f, 3 };
		r1.SetCubeSideLength(0.6f);
		r1.doGravity = true;

		r2.color = RColor::Red();
		r2.position = { 0.2f, -2, 3 };
		r2.inverseMass = 0.0f;
		r2.inverseMOI = 0.0f;

		physicsWorld->rigidbodies.push_back(r1);
		physicsWorld->rigidbodies.push_back(r2);
	}
}
