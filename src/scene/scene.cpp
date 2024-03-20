#include "scene.h"

#include <iostream>
#include "rlgl.h"


void Scene::Init()
{
	camera.position = Vector3{ 0, 0, 10 };
	camera.target = { 0.0f, 0.0f, 0.0f };		// Camera looking at point
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;						// Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;		// Camera mode type

	const int SCENARIO = 0;
	if (SCENARIO == 0)
	{
		RigidBody2D rb1 = RigidBody2D();
		rb1.position = { -2, 0.3f, 0 };
		rb1.velocity = Vector3{ 1, 0, 0 };
		rb1.angularVelocity = 2*PI;
		rb1.color = BLUE;

		RigidBody2D rb2 = RigidBody2D();
		rb2.position = { 2, -0.1f, 0 };
		rb2.velocity = Vector3{ -1, 0, 0 };
		rb2.rotation = PI / 4;
		rb2.color = RED;

		rigidbodies.push_back(rb1);
		rigidbodies.push_back(rb2);
	}
	if (SCENARIO == 1)
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
			rigidbodies.push_back(rb);
		}
	}

#ifdef TEST_POINT_LINE
	RVector3 closest;
	bool b = CollidePointLine({ .6,0,1 }, { 0,0,0 }, { 1,0,0 }, closest);
	printf("%d, closest point: (%f,%f,%f)\n", b, closest.x, closest.y, closest.z);

	exit(0);
#endif
}

void Scene::Update(float dt)
{
	// Fix physics update to 60 fps
	dt = 1.0f / 60.0f;

	// Integration
	for (auto& body : rigidbodies)
	{
		RVector3 acceleration = { 0, 0, 0 };
		body.oldPos = body.position;
		body.velocity += acceleration * dt;
		body.position += body.velocity * dt;

		float torqueY = 0.0f;
		body.angularVelocity += torqueY * body.inverseMOI * dt;
		body.rotation += body.angularVelocity * dt;
	}

	// Collision & resolution
	for (int i = 0; i < rigidbodies.size(); i++)
	{
		for (int j = i + 1; j < rigidbodies.size(); j++)
		{
			RigidBody2D& pi = rigidbodies[i];
			RigidBody2D& pj = rigidbodies[j];

			// Check 1: overlap
			RVector3 dir = pj.position - pi.position;
			float distance = dir.Length();
			
			if (distance >= pi.collisionRadius + pj.collisionRadius)
				continue;

			// Check 2: Find contacts
			CollisionInfo col;
			bool collided = CollideSquareSquare(pi, pj, col);
			if (!collided) continue;

			// Check 3: approaching

			// Velocities @ point of collision (lever arm formula)
			RVector3 iContactVel = pi.velocity + RVector3(0, 0, pi.angularVelocity).CrossProduct(col.contact - pi.position);
			RVector3 jContactVel = pj.velocity + RVector3(0, 0, pj.angularVelocity).CrossProduct(col.contact - pj.position);
			RVector3 contactVel = jContactVel - iContactVel;

			if (contactVel.DotProduct(col.normal) > 0)
				continue;

			// Resolution

			// Contact point displacement from center
			RVector3 dxi = col.contact - pi.position;
			RVector3 dxj = col.contact - pj.position;

			// Relative contact normals
			RVector3 rni = dxi.CrossProduct(col.normal);
			RVector3 rnj = dxj.CrossProduct(col.normal);

			RVector3 impulse = col.normal
				* (1 + cRestitution) * contactVel.DotProduct(col.normal)
				/ (pi.inverseMass + pj.inverseMass +
					rni.DotProduct(rni) * pi.inverseMOI +
					rnj.DotProduct(rnj) * pj.inverseMOI);

			pi.velocity += impulse * pi.inverseMass;
			pj.velocity -= impulse * pj.inverseMass;
			pi.angularVelocity += dxi.CrossProduct(impulse).z * pi.inverseMOI;
			pj.angularVelocity -= dxj.CrossProduct(impulse).z * pj.inverseMOI;

			//AddArrow(col.contact, iContactVel, RColor::Blue());
			//AddArrow(col.contact, jContactVel, RColor::Red());
			//AddArrow(col.contact, contactVel, RColor::Green());
			//AddArrow(col.contact, col.normal, RColor::Pink());
			//AddArrow(pi.position, impulse, RColor::Blue());
			//std::cout << col.normal.ToString() << "\n";
		}
	}

}

/**
*
*/
bool Scene::CollidePointLine(
	RVector3 point, RVector3 lineStart, RVector3 lineEnd,
	RVector3& outClosestPoint)
{
	RVector3 lineDir = lineEnd - lineStart;
	float t = (point - lineStart).DotProduct(lineDir) / (lineDir.Length() * lineDir.Length());

	if (t < 0)	// beyond line start
	{
		if (t > -0.001) // on line start
		{
			outClosestPoint = lineStart;
			return true;
		}
		else return false; // beyond - no collision
	}
	else if (t > 1) // beyond line end
	{
		if (t < 1.001) // on line end
		{
			outClosestPoint = lineEnd;
			return true;
		}
		else return false; // beyond - no collision
	}

	// in between line start & end
	outClosestPoint = lineStart * (1 - t) + lineEnd * t;
	return true;
}

/**
	Tests a point against a shape's list of vertices, and returns true if the point
	is on the line when projected onto it. The collision info
	returned is the best collision, that is the contact furthest away from the vertices'
	shape's center.

	\param point The point we are testing against.
	\param vertices The vertices of the shape we are testing against.
	\param pointCenter The original center of the shape that the tested point belongs to.
	\param verticesCenter The center of the shape we are testing against.
*/
bool Scene::CollidePointPolygon(
	RVector3 point, const std::vector<RVector3>& vertices, RVector3 pointCenter, Vector3 verticesCenter,
	CollisionInfo& outInfo)
{
	int nVerts = vertices.size();

	bool contactExists = false;
	float bestDepth = FLT_MAX;
	RVector3 bestContact;
	RVector3 bestNormal;

	float distToPoint = (point - pointCenter).Length();
	for (int i = 0; i < nVerts; i++)
	{
		RVector3 lineStart = vertices[i];
		RVector3 lineEnd = vertices[(i + 1) % nVerts];

		RVector3 contact;
		bool collided = CollidePointLine(point, lineStart, lineEnd, contact);

		if (!collided)
			continue;	// no contact
		if ((contact - pointCenter).Length() > distToPoint)
			continue;	// contact is outside point's shape

		RVector3 lineMiddle = (lineStart + lineEnd) * 0.5f;
		RVector3 normal = (lineMiddle - verticesCenter).Normalize();
		float depth = (contact - point).DotProduct(normal);
		float depthFromVertCen = (contact - verticesCenter).DotProduct(normal);

		// If depth is ok and smallest so far
		if (depth >= 0 && depth < depthFromVertCen && depth < bestDepth)
		{
			contactExists = true;
			bestDepth = depth;
			bestContact = contact;
			bestNormal = normal;
		}
	}

	outInfo = { bestDepth, bestContact, bestNormal };
	return contactExists;
}

/**
*
*/
bool Scene::CollideSquareSquare(
	const RigidBody2D& rb1, const RigidBody2D& rb2,
	CollisionInfo& outInfo)
{
	// Transform shape vertices to world space
	std::vector<RVector3> s1, s2;
	RVector3 square[] = { {-1,-1,0}, {-1,1,0}, {1,1,0}, {1,-1,0} };
	for (int i = 0; i < 4; i++)
	{
		s1.push_back(rb1.position + (square[i] * rb1.radius).RotateByQuaternion(RQuaternion::FromAxisAngle({ 0,0,1 }, rb1.rotation)));
		s2.push_back(rb2.position + (square[i] * rb2.radius).RotateByQuaternion(RQuaternion::FromAxisAngle({ 0,0,1 }, rb2.rotation)));
	}

	// Find contacts
	std::vector<CollisionInfo> contacts;
	for (int i = 0; i < 4; i++)
	{
		CollisionInfo info;
		bool collided;

		// Body 1 corner hits body 2 edge
		collided = CollidePointPolygon(s1[i], s2, rb1.position, rb2.position, info);
		if (collided)
		{
			// normal faces from 2->1, make it 1->2
			info.normal = -info.normal;
			contacts.push_back(info);
		}

		// Body 2 corner hits body 2 edge
		collided = CollidePointPolygon(s2[i], s1, rb2.position, rb1.position, info);
		if (collided) contacts.push_back(info);
	}

	if (contacts.size() > 0)
	{
		outInfo = contacts[0];
		return true;
	}

	return false;
}


void Scene::AddMarker(RVector3 position, Color color)
{
	markers.push_back(Marker{ position, color });
}

void Scene::AddArrow(RVector3 pos, RVector3 dir, Color color)
{
	arrows.push_back(Arrow{ pos, dir, color });
}

void Scene::Render()
{
	BeginMode3D(camera);

	for (auto& body : rigidbodies)
	{
		rlPushMatrix();
		rlTranslatef(body.position.x, body.position.y, body.position.z);
		rlRotatef(body.rotation * RAD2DEG, 0, 0, 1); // rlRotatef ASSUMES ITS IN DEGREES UGHHHHHHHHHHHHHH

		Color c = body.color; c.a = 70;
		DrawSphereWires(Vector3{}, body.collisionRadius, 8, 8, c);
		DrawCubeWires(Vector3{}, body.collisionRadius, body.collisionRadius, body.collisionRadius, body.color);

		rlPopMatrix();
	}

	for (auto& marker : markers)
		DrawSphere(marker.position, 0.05f, marker.color);

	for (auto& arrow : arrows)
	{
		Vector3 end = Vector3Add(arrow.position, Vector3Scale(arrow.direction, 1.0f));
		DrawLine3D(arrow.position, end, arrow.color);
		DrawSphere(end, 0.05f, arrow.color);
	}
		

	EndMode3D();
	DrawFPS(10, 10);
}

