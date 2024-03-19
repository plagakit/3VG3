#pragma once

#include "raylib-cpp.hpp"

class Particle {

public:
	Vector3 position;
	Vector3 velocity;

	float inverseMass = 1.0f;
	float radius = 1.0f;
	float collisionRadius = 1.8f;

	bool sleeping = false;
	bool doGravity = false;

	Color color = WHITE;

};