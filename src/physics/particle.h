#pragma once

#include "raylib-cpp.hpp"

class Particle {

public:
	RVector3 position = RVector3::Zero();
	RVector3 velocity = RVector3::Zero();
	RVector3 force = RVector3::Zero();

	float inverseMass = 1.0f;
	float radius = 1.0f;
	float boundingRadius = 1.8f;

	bool sleeping = false;
	bool doGravity = false;

	Color color = WHITE;

};