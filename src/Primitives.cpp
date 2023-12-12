#include "Primitives.h"


int PointLight::ext = 0;

void PointLight::draw() {
	// draw axis if selected
	if (bSelected) { 
		ofPushMatrix();
		ofTranslate(position);
		ofSetLineWidth(2.0);

		// X Axis
		ofSetColor(ofColor(255, 0, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0.3, 0, 0));

		// Y Axis
		ofSetColor(ofColor(0, 255, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0.3, 0));

		// Z Axis
		ofSetColor(ofColor(0, 0, 255));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 0.3));

		ofPopMatrix();
	}

	// proxy sphere to show where the light is
	ofFill();
	ofSetColor(ofColor::yellow);
	ofDrawSphere(position, 0.2);
}

int PointLight::getRaySamples(glm::vec3 p, glm::vec3 norm) {
	// a point light only ever has one light ray at a time
	samples.clear();

	Ray r = Ray(p + norm * 0.01f, glm::normalize(position - p));
	samples.push_back(r);
	samplesPos.push_back(position);
	return samples.size();
}

int AreaLight::ext = 0;

void AreaLight::draw() {
	// draw axis if selected
	if (bSelected) { 
		ofPushMatrix();
		ofTranslate(position);
		ofSetLineWidth(2.0);

		// X Axis
		ofSetColor(ofColor(255, 0, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));

		// Y Axis
		ofSetColor(ofColor(0, 255, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

		// Z Axis
		ofSetColor(ofColor(0, 0, 255));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

		ofPopMatrix();
	}

	// grid dimensions relative to origin point (center)
	float leftX = -width / 2;
	float rightX = width / 2;
	float topZ = -height / 2;
	float botZ = height / 2;

	// size & width of each cell
	float cellWidth = width / nDivsWidth;
	float cellHeight = height / nDivsHeight;

	// draw grid as lines
	if (bSelected) ofSetColor(ofColor::white);
	else ofSetColor(ofColor::yellow);

	// vertical lines
	for (int i = 0; i <= nDivsWidth; i++) {
		// offset is how far line is from left (# cells from left)
		float offset = i * cellWidth;

		glm::vec3 startLine = glm::vec3(leftX + offset, 0, topZ) + position;
		glm::vec3 endLine = glm::vec3(leftX + offset, 0, botZ) + position;
		ofDrawLine(startLine, endLine);
	}
	// horizontal lines
	for (int j = 0; j <= nDivsHeight; j++)
	{
		// offset is how far the line is from top (# cells from top)
		float offset = j * cellHeight;

		glm::vec3 startLine = glm::vec3(leftX, 0, topZ + offset) + position;
		glm::vec3 endLine = glm::vec3(rightX, 0, topZ + offset) + position;
		ofDrawLine(startLine, endLine);
	}
}

// an area light functionaly is like a plane
bool AreaLight::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, glm::vec3(0, 1, 0),
		dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = glm::vec3(0, 1, 0);

		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 yrange = glm::vec2(position.y - width / 2, position.y + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);

		if (point.x < xrange[1] && point.x > xrange[0] && point.z <
			zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

int AreaLight::getRaySamples(glm::vec3 p, glm::vec3 norm) {
	samples.clear();
	samplesPos.clear();

	// grid dimensions relative to origin point (center)
	float leftX = -width / 2;
	float rightX = width / 2;
	float topZ = -height / 2;
	float botZ = height / 2;

	// size & width of each cell
	float cellWidth = width / nDivsWidth;
	float cellHeight = height / nDivsHeight;

	// for each cell in the grid, get nSamples random rays
	for (int i = 0; i < nDivsWidth; i++) {
		for (int j = 0; j < nDivsHeight; j++) {

			// get dimensions of cell
			float cellLeftX = leftX + (i * cellWidth);
			float cellRightX = leftX + (i * cellWidth) + cellWidth;
			float cellTopZ = topZ + (j * cellHeight);
			float cellBotZ = topZ + (j * cellHeight) + cellHeight;

			// get randomized point in cell as ray
			for (int s = 0; s < nSamples; s++) {
				glm::vec3 samplePos = glm::vec3(ofRandom(cellLeftX, cellRightX), 0, ofRandom(cellTopZ, cellBotZ)) + position;
				Ray r = Ray(p + norm * 0.01f, glm::normalize(samplePos - p));
				samples.push_back(r);
				samplesPos.push_back(samplePos);
			}
		}
	}

	return samples.size();
}

int Sphere::ext = 0;

void Sphere::draw() {
	// draw axis if selected
	if (bSelected) { 
		ofPushMatrix();
		ofTranslate(position);
		ofSetLineWidth(2.0);

		// X Axis
		ofSetColor(ofColor(255, 0, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(radius * 1.5, 0, 0));

		// Y Axis
		ofSetColor(ofColor(0, 255, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, radius * 1.5, 0));

		// Z Axis
		ofSetColor(ofColor(0, 0, 255));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, radius * 1.5));

		ofPopMatrix();
	}

	ofFill();
	ofSetColor(diffuseColor);
	ofDrawSphere(position, radius);
}

// get texture coordinates from point on sphere
void Sphere::getTextureCoords(glm::vec3 p, float& u, float& v) {

	// project current point onto the sphere
	glm::vec3 point = p - position;
	float theta = asin(point.y / sqrt(point.x * point.x + point.y * point.y + point.z * point.z));
	float phi = atan2(point.z, point.x);
	u = ofMap(phi, 0, 2 * PI, 0, radius * 4);
	v = ofMap(theta, -PI, PI, 0, radius * 4);

	// calculate coordinates using fmod w/ frequency of tile repetition
	// more numTiles = less repetition
	u = fmod(u / numTiles, 1.0f);
	v = fmod(v / numTiles, 1.0f);
	if (u < 0) u += 1.0f;
	if (v < 0) v += 1.0f;
}


int Plane::ext = 0;

void Plane::draw() {
	// draw axis if selected
	if (bSelected) {
		ofPushMatrix();
		ofTranslate(position);
		ofSetLineWidth(2.0);

		// X Axis
		ofSetColor(ofColor(255, 0, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));

		// Y Axis
		ofSetColor(ofColor(0, 255, 0));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

		// Z Axis
		ofSetColor(ofColor(0, 0, 255));
		ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

		ofPopMatrix();
	}

	ofFill();
	ofSetColor(diffuseColor);

	plane.setPosition(position);
	plane.setWidth(width);
	plane.setHeight(height);
	plane.setResolution(4, 4);
	plane.drawWireframe();
	plane.draw();
}

// Intersect Ray with Plane  (wrapper on glm::intersect*)
bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal,
		dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 yrange = glm::vec2(position.y - width / 2, position.y + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);
		// horizontal
		if (normal == glm::vec3(0, 1, 0) || normal == glm::vec3(0, -1, 0)) {
			if (point.x < xrange[1] && point.x > xrange[0] && point.z <
				zrange[1] && point.z > zrange[0]) {
				insidePlane = true;
			}
		}
		// front or back
		else if (normal == glm::vec3(0, 0, 1) || normal == glm::vec3(0, 0, -1))
		{
			if (point.x < xrange[1] && point.x > xrange[0] && point.y <
				yrange[1] && point.y > yrange[0]) {
				insidePlane = true;
			}
		}
		// left or right
		else if (normal == glm::vec3(1, 0, 0) || normal == glm::vec3(-1, 0, 0))
		{
			if (point.y < yrange[1] && point.y > yrange[0] && point.z <
				zrange[1] && point.z > zrange[0]) {
				insidePlane = true;
			}
		}
	}
	return insidePlane;
}

// get texture coordinates from point on plane
void Plane::getTextureCoords(glm::vec3 p, float& u, float& v) {

	// project current point onto the plane
	glm::vec3 point = p - position;
	u = glm::dot(point, glm::normalize(glm::cross(normal, plane.getUpDir())));
	v = glm::dot(point, glm::normalize(plane.getUpDir()));

	// calculate coordinates using fmod w/ frequency of tile repetition
	// more numTiles = less repetition
	u = fmod(u / numTiles, 1.0f);
	v = fmod(v / numTiles, 1.0f);
	if (u < 0) u += 1.0f;
	if (v < 0) v += 1.0f;
}

// listener functions for plane
void Plane::upNormal(bool& val) {
	if (faceUp) {
		// reset plane and rotate to normal
		plane = ofPlanePrimitive();
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(-90, 1, 0, 0);

		faceDown = false;
		faceLeft = false;
		faceRight = false;
		faceForward = false;
		faceBackward = false;
	}
}
void Plane::downNormal(bool& val) {
	if (faceDown) {
		// reset plane and rotate to normal
		plane = ofPlanePrimitive();
		normal = glm::vec3(0, -1, 0);
		plane.rotateDeg(90, 1, 0, 0);

		faceUp = false;
		faceLeft = false;
		faceRight = false;
		faceForward = false;
		faceBackward = false;
	}
}
void Plane::leftNormal(bool& val) {
	if (faceLeft) {
		// reset plane and rotate to normal
		plane = ofPlanePrimitive();
		normal = glm::vec3(-1, 0, 0);
		plane.rotateDeg(-90, 0, 1, 0);

		faceUp = false;
		faceDown = false;
		faceRight = false;
		faceForward = false;
		faceBackward = false;
	}
}
void Plane::rightNormal(bool& val) {
	if (faceRight) {
		// reset plane and rotate to normal
		plane = ofPlanePrimitive();
		normal = glm::vec3(1, 0, 0);
		plane.rotateDeg(90, 0, 1, 0);

		faceUp = false;
		faceDown = false;
		faceLeft = false;
		faceForward = false;
		faceBackward = false;
	}
}
void Plane::forwardNormal(bool& val) {
	if (faceForward) {
		// by default plane faces (0, 0, 1)
		plane = ofPlanePrimitive();
		normal = glm::vec3(0, 0, 1);

		faceUp = false;
		faceDown = false;
		faceLeft = false;
		faceRight = false;
		faceBackward = false;
	}
}
void Plane::backwardNormal(bool& val) {
	if (faceBackward) {
		// reset plane and rotate to normal
		plane = ofPlanePrimitive();
		normal = glm::vec3(0, 0, -1);
		plane.rotateDeg(180, 1, 0, 0);

		faceUp = false;
		faceDown = false;
		faceLeft = false;
		faceRight = false;
		faceForward = false;
	}
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on the ViewPlane
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}