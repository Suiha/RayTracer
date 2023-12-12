#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/intersect.hpp"
#include <glm/gtc/matrix_transform.hpp>


//  General Purpose Ray class 
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d; // direction (d) must be normalized
};


//  Base class for any renderable object in the scene
class SceneObject {
public:
	// pure virtual funcs - must be overloaded
	virtual void draw() = 0;
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual glm::vec3 getNormal(const glm::vec3& p) { return glm::vec3(0, 0, 0); }
	virtual void setupGUI() = 0;
	virtual void updateGUI() = 0;
	
	// any data common to all scene objects goes here
	string name;
	glm::vec3 position = glm::vec3(0, 0, 0);
	bool isSelectable = false;
	bool bSelected = false;

	// gui elements & functions
	ofxPanel gui;
	ofParameter<glm::vec3> objPos;
	ofxLabel texture;
	ofParameter<int> nTiles;

	// material properties
	ofColor diffuseColor = ofColor::lightGray;   
	ofColor specularColor = ofColor::white;

	// texture objects & functions
	void getTextureCoords(glm::vec3 p, float& u, float& v) {}
	string textureName = "None";
	ofImage diffuseMap;
	ofImage specularMap;
	int numTiles = 1;
};


// general light class for illuminating scene
class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) {
		position = p;
		intensity = i;
	}
	Light() { intensity = 0; }

	void setupGUI() {}
	void updateGUI() {}

	void draw() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }

	// virtual functions - must be overloaded
	virtual int getRaySamples(glm::vec3 p, glm::vec3 norm) = 0;

	float intensity;
	vector<Ray> samples;
	vector<glm::vec3> samplesPos;

	ofParameter<float> lightIntensity;
};


// representation of ambient light
class AmbientLight : public Light {
public:
	AmbientLight(float i) {
		intensity = i;
	}
	AmbientLight() {}

	void setupGUI() {}
	void updateGUI() {}
	
	void draw() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	int getRaySamples(glm::vec3 p, glm::vec3 norm) {
		return 0;
	}
};


// point light = a point with one light ray to a given point
class PointLight : public Light {
public:
	PointLight(glm::vec3 p, float i) {
		name = string("Point Light ") + to_string(PointLight::ext++);
		position = p;
		intensity = i;
		isSelectable = true;
		setupGUI();
	}

	PointLight(glm::vec3 p) {
		name = string("Point Light ") + to_string(PointLight::ext++);
		position = p;
		intensity = 10.0;
		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(lightIntensity.set("Intensity", intensity, 0, 500));
		gui.add(objPos.set("Position", position, glm::vec3(-50, 0, -50),
			glm::vec3(50, 50, 50)));
	}

	void updateGUI() {
		intensity = lightIntensity;
		position = objPos;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, 0.2, point, normal));
	}
	int getRaySamples(glm::vec3 p, glm::vec3 norm);

	static int PointLight::ext;
};


// area light = grid with "infinite" light rays
class AreaLight : public Light {
public:
	AreaLight(glm::vec3 p, float i, int w, int h, int nDW, int nDH, int samples) {
		name = string("Area Light ") + to_string(AreaLight::ext++);
		position = p;
		intensity = i;
		width = w;
		height = h;
		nDivsWidth = nDW;
		nDivsHeight = nDH;
		nSamples = samples;

		isSelectable = true;
		setupGUI();
	}

	AreaLight(glm::vec3 p) {
		name = string("Area Light ") + to_string(AreaLight::ext++);
		position = p;
		intensity = 500;
		width = 10;
		height = 10;
		nDivsWidth = 5;
		nDivsHeight = 5;
		nSamples = 1;

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(lightIntensity.set("Intensity", intensity, 0, 1000));
		gui.add(objPos.set("Position", position, glm::vec3(-50, 0, -50),
			glm::vec3(50, 50, 50)));
		gui.add(alWidth.set("Area Light Width", width, 0, 10));
		gui.add(alHeight.set("Area Light Height", height, 0, 10));
		gui.add(divsWidth.set("# Subdivisions (Width)", nDivsWidth, 0, 20));
		gui.add(divsHeight.set("# Subdivisions (Height)", nDivsHeight, 0, 20));
		gui.add(numSamples.set("# Light Samples / Cell", nSamples, 1, 5));
	}

	void updateGUI() {
		intensity = lightIntensity;
		position = objPos;
		width = alWidth;
		height = alHeight;
		nDivsWidth = divsWidth;
		nDivsHeight = divsHeight;
		nSamples = numSamples;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	int getRaySamples(glm::vec3 p, glm::vec3 norm);

	static int AreaLight::ext;

	float width, height;			// overall width and height of the grid    (default (5 x 5);
	glm::vec3 position;				// x, y, z position of the grid's origin in space
	int nDivsWidth, nDivsHeight;	// number of subdivisions of grid (default 10x10): width = vertical, height = horizontal
	int nSamples;					// number of samples per grid cell (default = 1)

	ofParameter<float> alWidth, alHeight;
	ofParameter<int> divsWidth, divsHeight, numSamples;
};


//  General purpose sphere  (assume parametric)
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::white) {
		name = string("Sphere ") + to_string(Sphere::ext++);
		position = p;
		radius = r;
		diffuseColor = diffuse;

		isSelectable = true;
		setupGUI();
	}

	Sphere() {
		name = string("Sphere ") + to_string(Sphere::ext++);
		isSelectable = true; 
		setupGUI(); 
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(sphereRadius.set("Radius", radius, 1, 10));
		gui.add(sphereColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));

		gui.add(texture.setup("Texture: " + string(textureName)));
		gui.add(nTiles.set("Texture Tiles", numTiles, 1, 10));
	}

	void updateGUI() {
		position = objPos;
		radius = sphereRadius;
		diffuseColor = sphereColor;
		
		texture = "Texture: " + string(textureName);
		numTiles = nTiles;
	}
	
	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	glm::vec3 getNormal(const glm::vec3& p) {
		return glm::normalize(glm::vec3(p - position));
	}
	void getTextureCoords(glm::vec3 p, float& u, float& v);

	static int Sphere::ext; // keep track of # of spheres created
	float radius = 1.0;

	ofParameter<float> sphereRadius;
	ofParameter<ofColor> sphereColor;
};


//  General purpose plane 
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::white, float w = 20, float h = 20) {
		name = string("Plane ") + to_string(Plane::ext++);
		position = p; 
		normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		
		// plane by default faces (0, 0, 1), rotate according to normal
		if (normal == glm::vec3(0, 1, 0))
			plane.rotateDeg(-90, 1, 0, 0);
		else if (normal == glm::vec3(0, -1, 0))
			plane.rotateDeg(90, 1, 0, 0);
		else if (normal == glm::vec3(1, 0, 0))
			plane.rotateDeg(90, 0, 1, 0);
		else if (normal == glm::vec3(-1, 0, 0))
			plane.rotateDeg(-90, 0, 1, 0);
		else if (normal == glm::vec3(0, 0, -1))
			plane.rotateDeg(180, 1, 0, 0);

		isSelectable = true;
		setupGUI();
	}

	Plane() {
		name = string("Plane ") + to_string(Plane::ext++);
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(planeWidth.set("Width", width, 0.1, 50));
		gui.add(planeHeight.set("Height", height, 0.1, 50));

		faceUp.addListener(this, &Plane::upNormal);
		faceDown.addListener(this, &Plane::downNormal);
		faceLeft.addListener(this, &Plane::leftNormal);
		faceRight.addListener(this, &Plane::rightNormal);
		faceForward.addListener(this, &Plane::forwardNormal);
		faceBackward.addListener(this, &Plane::backwardNormal);

		normalOptions.setName("Plane Normal Options");
		normalOptions.add(faceUp.set("Normal: (0, 1, 0)", (normal == glm::vec3(0, 1, 0)) ? true : false));
		normalOptions.add(faceDown.set("Normal: (0, -1, 0)", (normal == glm::vec3(0, -1, 0)) ? true : false));
		normalOptions.add(faceLeft.set("Normal: (-1, 0, 0)", (normal == glm::vec3(-1, 0, 0)) ? true : false));
		normalOptions.add(faceRight.set("Normal: (1, 0, 0)", (normal == glm::vec3(1, 0, 0)) ? true : false));
		normalOptions.add(faceForward.set("Normal: (0, 0, 1)", (normal == glm::vec3(0, 0, 1)) ? true : false));
		normalOptions.add(faceBackward.set("Normal: (0, 0, -1)", (normal == glm::vec3(0, 0, -1)) ? true : false));
		normalOptions.add(planeColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));
		gui.add(normalOptions);

		gui.add(texture.setup("Texture: " + string(textureName)));
		gui.add(nTiles.set("Texture Tiles", numTiles, 1, 10));
	}

	void updateGUI() {
		position = objPos;
		width = planeWidth;
		height = planeHeight;
		diffuseColor = planeColor;

		texture = "Texture: " + string(textureName);
		numTiles = nTiles;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }
	void getTextureCoords(glm::vec3 p, float& u, float& v);

	// listener functions for changing normal
	void upNormal(bool& val);
	void downNormal(bool& val);
	void leftNormal(bool& val);
	void rightNormal(bool& val);
	void forwardNormal(bool& val);
	void backwardNormal(bool& val);

	ofPlanePrimitive plane;
	glm::vec3 normal;

	static int Plane::ext;
	float width = 20;
	float height = 20;

	ofParameter<float> planeWidth, planeHeight;
	ofParameter<ofColor> planeColor;
	ofParameterGroup normalOptions;
	ofParameter<bool> faceUp, faceDown, faceLeft, faceRight, faceForward, faceBackward;
};


// view plane for render camera
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void setupGUI() {}
	void updateGUI() {}

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}

	float width() { return (max.x - min.x); }
	float height() { return (max.y - min.y); }

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
		isSelectable = false;
	}

	Ray getRay(float u, float v);
	void setupGUI() {}
	void updateGUI() {}
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum() {}

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};