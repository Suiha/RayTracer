#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <glm/gtx/intersect.hpp>


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

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	// texture objects & functions
	void getTextureCoords(glm::vec3 p, float& u, float& v) {}
	ofImage diffuseMap;
	ofImage specularMap;
	int numTiles;
};


//  General purpose sphere  (assume parametric)
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}

	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	glm::vec3 getNormal(const glm::vec3& p) {
		return glm::normalize(glm::vec3(p - position));
	}

	void draw() {
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
	}

	void getTextureCoords(glm::vec3 p, float& u, float& v);

	float radius = 1.0;
};


//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
class Mesh : public SceneObject {
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	void draw() { }
};


//  General purpose plane 
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::green, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0))
			plane.rotateDeg(-90, 1, 0, 0);
		else if (normal == glm::vec3(0, -1, 0))
			plane.rotateDeg(90, 1, 0, 0);
		else if (normal == glm::vec3(1, 0, 0))
			plane.rotateDeg(90, 0, 1, 0);
		else if (normal == glm::vec3(-1, 0, 0))
			plane.rotateDeg(-90, 0, 1, 0);
	}

	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
		isSelectable = false;
	}

	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	float sdf(const glm::vec3& p);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }

	void draw() {
		ofSetColor(diffuseColor);
		ofFill();

		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
		plane.draw();
	}

	void getTextureCoords(glm::vec3 p, float& u, float& v);

	ofPlanePrimitive plane;
	glm::vec3 normal;
	bool isSelectable;
	float width = 20;
	float height = 20;
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

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}

	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

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
	}

	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum() {}

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};


// general light class for illuminating scene
class Light {
public:
	Light(glm::vec3 p, float i) {
		position = p;
		intensity = i;
	}

	Light() {}

	// virtual functions, must be overloaded
	virtual void draw() = 0;
	virtual int getRaySamples(glm::vec3 p, glm::vec3 norm) = 0;

	glm::vec3 position = glm::vec3(0, 0, 0);
	float intensity;
	vector<Ray> samples;
	vector<glm::vec3> samplesPos;
};


// representation of ambient light
class AmbientLight : public Light {
public:
	AmbientLight(float i) {
		intensity = i;
	}

	AmbientLight() {}

	void draw() {}
	int getRaySamples(glm::vec3 p, glm::vec3 norm) {
		return 0;
	}
};


// point light = a point with one light ray to a given point
class PointLight : public Light {
public:
	PointLight(glm::vec3 p, float i) {
		position = p;
		intensity = i;
	}

	PointLight(glm::vec3 p) {
		position = p;
		intensity = 10.0;
	}

	// proxy sphere to show where the light is
	void draw() {
		ofFill();
		ofSetColor(ofColor::yellow);
		ofDrawSphere(position, 0.2);
	}

	// a point light only ever has one light ray at a time
	int getRaySamples(glm::vec3 p, glm::vec3 norm) {
		samples.clear();

		Ray r = Ray(p + norm * 0.01f, glm::normalize(position - p));
		samples.push_back(r);
		samplesPos.push_back(position);
		return samples.size();
	}
};


// area light = grid with "infinite" light rays
class AreaLight : public Light {
public:
	AreaLight(glm::vec3 p, float i, int w, int h, int nDW, int nDH, int samples) {
		position = p;
		totalIntensity = i;
		intensity = i;
		width = w;
		height = h;
		nDivsWidth = nDW;
		nDivsHeight = nDH;
		nSamples = samples;
	}

	void draw() {
		// grid dimensions relative to origin point (center)
		float leftX = -width / 2;
		float rightX = width / 2;
		float topZ = -height / 2;
		float botZ = height / 2;

		// size & width of each cell
		float cellWidth = width / nDivsWidth;
		float cellHeight = height / nDivsHeight;

		// draw grid as lines
		ofSetColor(ofColor::white);
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

	int getRaySamples(glm::vec3 p, glm::vec3 norm) {
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

	float totalIntensity;			// original intensity of light; "intensity" will be scaled down based on # of samples
	float width, height;			// overall width and height of the grid    (default (5 x 5);
	glm::vec3 position;				// x, y, z position of the grid's origin in space
	int nDivsWidth, nDivsHeight;	// number of subdivisions of grid (default 10x10): width = vertical, height = horizontal
	int nSamples;					// number of samples per grid cell (default = 1)
};



class ofApp : public ofBaseApp {

public:
	void setup();

	void setupGUI() {
		gui.setup("RayTrace Settings");
		gui.add(ambientLightIntensity.set("Ambient Light Intesnity", 0.1, 0.05, 0.5));

		light1Settings.setName("Light 1");
		light1Settings.add(light1Intensity.set("Intensity", 100, 0, 500));
		light1Settings.add(light1X.set("X", 3, -50, 50));
		light1Settings.add(light1Y.set("Y", 30, 0, 50));
		light1Settings.add(light1Z.set("Z", 0, -50, 50));

		light2Settings.setName("Light 2");
		light2Settings.add(light2Intensity.set("Intensity", 200, 0, 500));
		light2Settings.add(light2X.set("X", -3, -50, 50));
		light2Settings.add(light2Y.set("Y", 20, 0, 50));
		light2Settings.add(light2Z.set("Z", 0, -50, 50));

		light3Settings.setName("Light 3");
		light3Settings.add(light3Intensity.set("Intensity", 50, 0, 500));
		light3Settings.add(light3X.set("X", 4, -50, 50));
		light3Settings.add(light3Y.set("Y", 25, 0, 50));
		light3Settings.add(light3Z.set("Z", 0, -50, 50));

		areaLightSettings.setName("Area Light");
		areaLightSettings.add(areaLightIntensity.set("Intensity", 500, 0, 1000));
		areaLightSettings.add(areaLightX.set("X", 0, -50, 50));
		areaLightSettings.add(areaLightY.set("Y", 30, 0, 50));
		areaLightSettings.add(areaLightZ.set("Z", 0, -50, 50));
		areaLightSettings.add(areaLightWidth.set("Width", 5, 0, 10));
		areaLightSettings.add(areaLightHeight.set("Height", 5, 0, 10));
		areaLightSettings.add(areaLightDivWidth.set("# Width Divisions", 10, 0, 20));
		areaLightSettings.add(areaLightDivHeight.set("# Height Divisions", 10, 0, 20));
		areaLightSettings.add(areaLightSamples.set("Samples per Cell", 1, 1, 5));

		lambertShading.addListener(this, &ofApp::lambertOnly);
		phongShading.addListener(this, &ofApp::phongOnly);

		shading.setName("Shading Type");
		shading.add(lambertShading.set("Lambert Shading", false));
		shading.add(phongShading.set("Phong Shading", false));

		phongSettings.setName("Phong Settings");
		phongSettings.add(phongPower.set("Phong p value", 10, 0, 50));

		gui.add(light1Settings);
		gui.add(light2Settings);
		gui.add(light3Settings);
		gui.add(areaLightSettings);
		gui.add(shading);
		gui.add(phongSettings);
	}

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	// clarity functions that keep only one type of shading selected at one time
	void lambertOnly(bool& val);
	void phongOnly(bool& val);

	void rayTrace();
	bool inShadow(Ray ray);
	ofColor lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse);
	ofColor phong(const glm::vec3& p, const glm::vec3& norm,
		const ofColor diffuse, const ofColor specular, float power);

	void drawGrid() {}
	void drawAxis(glm::vec3 position) {}

	void addLight(Light* l) {
		lights.push_back(l);
	}

	// camera objects
	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera* theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image through
	RenderCam renderCam;
	ofImage image;

	// scene objects
	vector<SceneObject*> scene;

	// light objects
	vector<Light*> lights;
	AmbientLight ambientLight;
	AreaLight* areaLight;

	int imageWidth = 1200;
	int imageHeight = 800;
	bool bRendered = false;

	// gui
	ofxPanel gui;
	bool bHide = false;
	ofParameter<float> ambientLightIntensity;

	ofParameterGroup light1Settings;
	ofParameter<float> light1Intensity;
	ofParameter<float> light1X;
	ofParameter<float> light1Y;
	ofParameter<float> light1Z;

	ofParameterGroup light2Settings;
	ofParameter<float> light2Intensity;
	ofParameter<float> light2X;
	ofParameter<float> light2Y;
	ofParameter<float> light2Z;

	ofParameterGroup light3Settings;
	ofParameter<float> light3Intensity;
	ofParameter<float> light3X;
	ofParameter<float> light3Y;
	ofParameter<float> light3Z;

	ofParameterGroup areaLightSettings;
	ofParameter<float> areaLightIntensity;
	ofParameter<float> areaLightX;
	ofParameter<float> areaLightY;
	ofParameter<float> areaLightZ;
	ofParameter<int> areaLightWidth;
	ofParameter<int> areaLightHeight;
	ofParameter<int> areaLightDivWidth;
	ofParameter<int> areaLightDivHeight;
	ofParameter<int> areaLightSamples;

	ofParameterGroup shading;
	ofParameter<bool> lambertShading;
	ofParameter<bool> phongShading;

	ofParameterGroup phongSettings;
	ofParameter<float> phongPower;
};
