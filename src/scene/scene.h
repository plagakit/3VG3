#pragma once

#include "raylib-cpp.hpp"
#include "rigidbody.h"
#include <vector>

struct CollisionInfo {
	float depth;
	RVector3 contact;
	RVector3 normal;
};

struct Marker {
	RVector3 position;
	Color color;
};

struct Arrow {
	RVector3 position;
	RVector3 direction;
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

	bool CollidePointLine(RVector3 point, RVector3 lineStart, RVector3 lineEnd,
		RVector3& outClosestPoint);

	bool CollidePointPolygon(RVector3 point, const std::vector<RVector3>& vertices, RVector3 pointShapeCenter, Vector3 verticesCenter,
							 CollisionInfo& outInfo);

	bool CollideSquareSquare(const RigidBody2D& rb1, const RigidBody2D& rb2,
							 CollisionInfo& outInfos);

	// Debug drawing
	std::vector<Marker> markers;
	std::vector<Arrow> arrows;

	void AddMarker(RVector3 position, Color color);
	void AddArrow(RVector3 pos, RVector3 dir, Color color);

};