#pragma once

#include "particle.h"

class RigidBody2D : public Particle {

public:
	float rotation;
	float angularVelocity;
	float inverseMOI = 1.0f;

};