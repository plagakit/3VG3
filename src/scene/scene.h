#pragma once

#include "raylib-cpp.hpp"
#include "rigidbody.h"
#include <vector>

struct CollisionInfo {
	float depth;
	Vector3 contact;
	Vector3 normal;
};

struct Marker {
	Vector3 position;
	Color color;
};

struct Arrow {
	Vector3 position;
	Vector3 direction;
	Color color;
};

class Scene {

public:
	void Init();
	void Update(float dt);
	void Render();

private:
	const float cRestitution = 1.0f;

	Camera3D camera;
	std::vector<RigidBody2D> rigidbodies;

	bool CollidePointLine(Vector3 point, Vector3 lineStart, Vector3 lineEnd, 
						  Vector3& outClosestPoint);	

	bool CollidePointPolygon(Vector3 point, const std::vector<Vector3>& vertices, Vector3 pointShapeCenter, Vector3 verticesCenter,
							 CollisionInfo& outInfo);

	bool CollideSquareSquare(const RigidBody2D& rb1, const RigidBody2D& rb2,
							 std::vector<CollisionInfo>& outInfos);

	// Debug drawing
	std::vector<Marker> markers;
	std::vector<Arrow> arrows;

	void AddMarker(Vector3 position, Color color);
	void AddArrow(Vector3 pos, Vector3 dir, Color color);

};