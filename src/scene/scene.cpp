#include "scene.h"

#include "rlgl.h"


void Scene::Init()
{
	camera.position = Vector3{ 0, 0, 10 };
	camera.target = { 0.0f, 0.0f, 0.0f };		// Camera looking at point
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;						// Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;		// Camera mode type

	RigidBody2D rb1 = RigidBody2D();
	rb1.position = { -3, 0.3f, 0 };
	rb1.velocity = Vector3{ 1, 0, 0 };
	rb1.inverseMass = 1.0f;
	rb1.rotation = -21;
	rb1.color = BLUE;

	RigidBody2D rb2 = RigidBody2D();
	rb2.position = { 3, -0.3f, 0 };
	rb2.velocity = Vector3{ -1, 0, 0 };
	rb2.inverseMass = 0.5f;
	rb2.color = RED;

	rigidbodies.push_back(rb1);
	rigidbodies.push_back(rb2);

#ifdef TEST_POINT_LINE
	Vector3 closest;
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
		Vector3 acceleration = Vector3{ 0, 0, 0 };
		body.velocity = Vector3Add(body.velocity, Vector3Scale(acceleration, dt));
		body.position = Vector3Add(body.position, Vector3Scale(body.velocity, dt));

		//float torqueY = 100.0f;
		//body.angularVelocity += torqueY * body.inverseMOI * dt;
		body.rotation += body.angularVelocity * dt;
	}

	// Collision detection
	for (int i = 0; i < rigidbodies.size(); i++)
	{
		for (int j = i + 1; j < rigidbodies.size(); j++)
		{
			auto& pi = rigidbodies[i];
			auto& pj = rigidbodies[j];

			Vector3 dir = Vector3Subtract(pj.position, pi.position);
			float dist = Vector3Length(dir);

			// Check 1: Inside bouding sphere
			if (dist < pi.radius + pj.radius)
			{
				// Check 2: Point of contact
				std::vector<CollisionInfo> contacts;
				bool contacted = CollideSquareSquare(pi, pj, contacts);

				if (!contacted) 
					continue;

				int nContacts = contacts.size();

				for (const auto& c : contacts)
				{
					Vector3 piPointAV = Vector3CrossProduct({ 0, 0, pi.angularVelocity }, Vector3Subtract(c.contact, pi.position));
					Vector3 pjPointAV = Vector3CrossProduct({ 0, 0, pj.angularVelocity }, Vector3Subtract(c.contact, pj.position));
					Vector3 piContactV = Vector3Add(pi.velocity, piPointAV);
					Vector3 pjContactV = Vector3Add(pj.velocity, pjPointAV);
					Vector3 contactVelDir = Vector3Subtract(pjContactV, piContactV);

					AddArrow(c.contact, piContactV, BLUE);
					AddArrow(c.contact, pjContactV, RED);

					// Check 3: Heading in same direction
					if (Vector3DotProduct(contactVelDir, dir) < 0)
					{
						nContacts++;
						Vector3 idp = Vector3Subtract(c.contact, pi.position);
						Vector3 jdp = Vector3Subtract(c.contact, pj.position);
						Vector3 irn = Vector3CrossProduct(idp, c.normal);
						Vector3 jrn = Vector3CrossProduct(jdp, c.normal);

						float weightingFactor = 1.0f / (
							pi.inverseMass + pj.inverseMass +
							Vector3LengthSqr(irn) * pi.inverseMOI +
							Vector3LengthSqr(jrn) * pj.inverseMOI
						);
						float impulseSpeed = (1 + cRestitution) *
							Vector3DotProduct(contactVelDir, c.normal)
							* weightingFactor;

						Vector3 impulse = Vector3Scale(c.normal, impulseSpeed);

						pi.velocity = Vector3Add(pi.velocity, Vector3Scale(impulse, pi.inverseMass));
						pj.velocity = Vector3Subtract(pj.velocity, Vector3Scale(impulse, pj.inverseMass));

						pi.angularVelocity += Vector3CrossProduct(idp, impulse).y * pi.inverseMOI;
						pj.angularVelocity -= Vector3CrossProduct(jdp, impulse).y * pj.inverseMOI;
					}
				}
			}
		}
	}
}

bool Scene::CollidePointLine(
	Vector3 point, Vector3 lineStart, 
	Vector3 lineEnd, Vector3& outClosestPoint)
{
	Vector3 line = Vector3Subtract(lineEnd, lineStart);
	float t = Vector3DotProduct(Vector3Subtract(point, lineStart), line) 
		/ Vector3LengthSqr(line);

	if (t < 0)
	{
		if (t > -0.001) {
			outClosestPoint = lineStart;
			return true;
		} else return false;
	}
	else if (t > 1)
	{
		if (t < 1.001) {
			outClosestPoint = lineEnd;
			return true;
		} else return false;
	}
	else
	{
		// x1 * (1 - t) + x2 * t => progress along line
		outClosestPoint = Vector3Add(Vector3Scale(lineStart, (1 - t)), Vector3Scale(lineEnd, t));
		return true;
	}
}

bool Scene::CollidePointPolygon(
	Vector3 point, const std::vector<Vector3>& vertices, 
	Vector3 pointShapeCenter, Vector3 verticesCenter, 
	CollisionInfo& outInfo)
{
	bool collided = false;
	float distToPoint = Vector3Length(Vector3Subtract(point, pointShapeCenter));
	float bestDepth = 1e37f;
	
	int numVerts = vertices.size();
	for (int i = 0; i < numVerts; i++)
	{
		Vector3 x1 = vertices[i];
		Vector3 x2 = vertices[(i + 1) % numVerts];
		Vector3 contact;
		bool hitLine = CollidePointLine(point, x1, x2, contact);
		
		// No hit
		if (!hitLine) 
			continue;	

		// If contact is outside of point's shape
		float distToCenter = Vector3Length(Vector3Subtract(contact, pointShapeCenter));
		if (distToCenter > distToPoint)
			continue;

		Vector3 middleOfEdge = Vector3Scale(Vector3Add(x1, x2), 0.5f);
		Vector3 normal = Vector3Normalize(Vector3Subtract(middleOfEdge, verticesCenter));
		float depth = Vector3DotProduct(Vector3Subtract(contact, point), normal);
		float depthFromCenter = Vector3DotProduct(Vector3Subtract(contact, verticesCenter), normal);

		if (depth >= 0 && depth < bestDepth)// && depth < depthFromCenter)
		{
			collided = true;
			bestDepth = depth;

			outInfo.depth = depth;
			outInfo.contact = contact;
			outInfo.normal = normal;
		}
	}

	return collided;
}

bool Scene::CollideSquareSquare(const RigidBody2D& pi, const RigidBody2D& pj, std::vector<CollisionInfo>& outInfos)
{
	// Generate vertices of square, map to world space
	Vector3 square[4] = { Vector3{-1,0,-1}, Vector3{-1,0,1}, Vector3{1,0,1}, Vector3{1,0,-1} };
	std::vector<Vector3> s1, s2;
	for (int i = 0; i < 4; i++)
	{
		s1.push_back(Vector3Add(pi.position, Vector3RotateByAxisAngle(Vector3Scale(square[i], pi.radius), {0,1,0}, pi.rotation)));
		s2.push_back(Vector3Add(pj.position, Vector3RotateByAxisAngle(Vector3Scale(square[i], pj.radius), {0,1,0}, pj.rotation)));
	}

	// Find contacts
	bool collided = false;
	for (int i = 0; i < 4; i++)
	{
		CollisionInfo info;
		bool corner1face2 = CollidePointPolygon(s1[i], s2, pi.position, pj.position, info);
		if (corner1face2)
		{
			collided = true;
			info.normal = Vector3Negate(info.normal);
			outInfos.push_back(info);
			AddMarker(info.contact, GREEN);
		}

		bool corner2face1 = CollidePointPolygon(s2[i], s1, pj.position, pi.position, info);
		if (corner2face1)
		{
			collided = true;
			outInfos.push_back(info);
			AddMarker(info.contact, GREEN);
		}
	}
	return collided;
}

void Scene::AddMarker(Vector3 position, Color color)
{
	markers.push_back(Marker{ position, color });
}

void Scene::AddArrow(Vector3 pos, Vector3 dir, Color color)
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
		rlRotatef(body.rotation, 0, 0, 1);

		//DrawSphereWires(Vector3{}, body.radius, 8, 8, Color{body.color.r, body.color.g, body.color.b, 70});

		Vector3 square[4] = { Vector3{-1,0,-1}, Vector3{-1,0,1}, Vector3{1,0,1}, Vector3{1,0,-1} };
		std::vector<Vector3> s;
		for (int i = 0; i < 4; i++)
			s.push_back(Vector3Add(body.position, Vector3RotateByAxisAngle(Vector3Scale(square[i], body.radius), { 0,0,1 }, body.rotation)));

		DrawLine3D(s[0], s[1], body.color);

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