#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Primitives.h"
#include <glm/gtx/intersect.hpp>


class ofApp : public ofBaseApp {
public:
	void setup();

	void setupGUI() {
		gui.setup("RayTrace Settings");

		updateRender.addListener(this, &ofApp::updateRenderCam);
		createPlane.addListener(this, &ofApp::addPlane);
		createSphere.addListener(this, &ofApp::addSphere);
		createPointLight.addListener(this, &ofApp::addPointLight);
		createAreaLight.addListener(this, &ofApp::addAreaLight);
		delObject.addListener(this, &ofApp::deleteSelected);

		gui.add(updateRender.setup("Update RenderCam (TAB)"));
		gui.add(createPlane.setup("Create New Plane"));
		gui.add(createSphere.setup("Create New Sphere"));
		gui.add(createPointLight.setup("Create New PointLight"));
		gui.add(createAreaLight.setup("Create New AreaLight"));
		gui.add(delObject.setup("Delete Selected (DEL)"));

		res1200x800.addListener(this, &ofApp::res12X8);
		res600x400.addListener(this, &ofApp::res6X4);

		imageSettings.setName("Render Image Resolution");
		imageSettings.add(res1200x800.set("1200 x 800", true));
		imageSettings.add(res600x400.set("600 x 400", false));

		gui.add(imageSettings);

		lambertShading.addListener(this, &ofApp::lambertOnly);
		phongShading.addListener(this, &ofApp::phongOnly);

		shading.setName("Shading Settings");
		shading.add(ambientLightIntensity.set("Ambient Light Intensity", 0.1, 0.05, 0.5));
		shading.add(lambertShading.set("Lambert Shading", false));
		shading.add(phongShading.set("Phong Shading", false));
		shading.add(phongPower.set("Phong p value", 10, 0, 50));

		gui.add(shading);

		renderScene.addListener(this, &ofApp::rayTrace);
		gui.add(renderScene.setup("Render (R)"));
		gui.add(bRendered.set("Show Image (I)", false));

		noTexture.addListener(this, &ofApp::applyNoTexture);
		brickWall.addListener(this, &ofApp::applyBrickWall);
		cobblestonePavement.addListener(this, &ofApp::applyCobblestone);
		garagePaving.addListener(this, &ofApp::applyGaragePaving);
		marbleFloor.addListener(this, &ofApp::applyMarbleFloor);

		textures.setName("Texture Options");
		textures.add(noTexture.set("No Texture", false));
		textures.add(brickWall.set("Brick Wall", false));
		textures.add(cobblestonePavement.set("Cobblestone Pavement", false));
		textures.add(garagePaving.set("Garage Paving", false));
		textures.add(marbleFloor.set("Marble Floor", false));

		gui.add(textures);
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
	bool mouseToDragPlane(int x, int y, glm::vec3& point);
	bool objSelected() { return (selected.size() ? true : false); };

	// listener functions for gui
	void ofApp::updateRenderCam() {
		// update render cam to match current cam
		renderCam.setPosition(theCam->getPosition());
		renderCam.lookAt(theCam->getLookAtDir());
		renderCam.setOrientation(theCam->getGlobalOrientation());
	}
	void deleteSelected() {
		if (objSelected()) {
			Light* light = dynamic_cast<Light*>(selected[0]);
			if (light) removeLight(light);
			else removeSceneObject(selected[0]);
		}
	}
	void res6X4(bool& val) { 
		if (val) {
			imageWidth = 600;
			imageHeight = 400;
			image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
			res1200x800 = false;
		}
	}
	void res12X8(bool& val) {
		if (val) {
			imageWidth = 1200;
			imageHeight = 800;
			image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
			res600x400 = false;

		}
	}
	void lambertOnly(bool& val) { if (lambertShading) phongShading = false; }
	void phongOnly(bool& val) { if (phongShading) lambertShading = false; }
	void applyNoTexture(bool& val);
	void applyBrickWall(bool& val);
	void applyCobblestone(bool& val);
	void applyGaragePaving(bool& val);
	void applyMarbleFloor(bool& val);

	void rayTrace();
	bool inShadow(Ray ray);
	ofColor lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse);
	ofColor phong(const glm::vec3& p, const glm::vec3& norm,
		const ofColor diffuse, const ofColor specular, float power);
	
	void drawGrid() {}

	// functions for adding/removing objects
	void removeSceneObject(SceneObject* obj);
	void addPlane();
	void addSphere();
	void addLight(Light* l) { lights.push_back(l); } // will probably delete this function
	void removeLight(Light* l);
	void addPointLight();
	void addAreaLight();

	// camera objects
	ofEasyCam mainCam;
	ofEasyCam sideCam;
	ofEasyCam renderCam;
	ofEasyCam* theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image through
	//RenderCam renderCam;
	ofImage image;

	// scene objects
	vector<SceneObject*> scene, selected;

	// light objects
	vector<Light*> lights;
	ofLight keyLight, fillLight, rimLight;
	ofLight sceneLight; // pre-render light
	AmbientLight ambientLight;
	AreaLight* areaLight;

	// render image
	static int ofApp::ext;
	int imageWidth = 1200;
	int imageHeight = 800;

	// texture maps
	ofImage garageDiffuse, garageSpecular;
	ofImage brickDiffuse, brickSpecular;
	ofImage cobbleDiffuse, cobbleSpecular;
	ofImage marbleDiffuse, marbleSpecular;
	
	// state
	bool bDrag;
	glm::vec3 lastPoint;

	// gui
	ofxPanel gui;
	bool bHide = false;
	ofxButton updateRender;
	ofxButton createPlane, createSphere, createPointLight, createAreaLight, delObject;

	// image settings
	ofParameterGroup imageSettings;
	ofParameter<bool> res600x400, res1200x800;
	ofxButton renderScene;
	ofParameter<bool> bRendered;

	// shading options
	ofParameterGroup shading;
	ofParameter<float> ambientLightIntensity;
	ofParameter<bool> lambertShading, phongShading;
	ofParameter<float> phongPower;

	// texture application
	ofParameterGroup textures;
	ofParameter<bool> noTexture;
	ofParameter<bool> garagePaving;
	ofParameter<bool> brickWall;
	ofParameter<bool> cobblestonePavement;
	ofParameter<bool> marbleFloor;

};


