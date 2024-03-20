#pragma once

#include "particle.h"

class RigidBody2D : public Particle {

public:
	RVector3 oldPos = RVector3::Zero();
	float rotation = 0.0f;
	float angularVelocity = 0.0f;
	float inverseMOI = 1.0f;

};