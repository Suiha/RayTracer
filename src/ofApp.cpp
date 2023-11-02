#include "ofApp.h"

void ofApp::setup() {
	printf("Setup\n");
	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;

	// camera setup
	mainCam.setDistance(10);
	mainCam.setNearClip(.1);

	sideCam.setPosition(glm::vec3(20, 10, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);

	previewCam.setPosition(renderCam.view.position);
	previewCam.lookAt(renderCam.aim);
	previewCam.setNearClip(.1);


	// plane: origin point, normal vector, color
	Plane* backWall = new Plane(glm::vec3(0, 8, -10), glm::vec3(0, 0, 1), ofColor::gray); // vertical plane, facing forward
	scene.push_back(backWall);
	Plane* leftWall = new Plane(glm::vec3(-5, 8, 0), glm::vec3(1, 0, 0), ofColor::gray); // vertical plane, facing right
	scene.push_back(leftWall);
	Plane* rightWall = new Plane(glm::vec3(5, 8, 0), glm::vec3(-1, 0, 0), ofColor::gray); // vertical plane, facing left
	scene.push_back(rightWall);
	Plane* floor = new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::sandyBrown); // horizontal plane, facing up
	scene.push_back(floor);	

	// create scene
	Sphere* sphere0 = new Sphere(glm::vec3(4, 1, -5), 2.0, ofColor::mediumPurple);
	//scene.push_back(sphere0);
	Sphere* sphere1 = new Sphere(glm::vec3(0, 1, -2), 2.0, ofColor::lightBlue);
	scene.push_back(sphere1);
	Sphere* sphere2 = new Sphere(glm::vec3(-2.5, 0, 0), 1.0, ofColor::pink);
	scene.push_back(sphere2);

	// lights
	addLight(new PointLight(glm::vec3(3, 30, 0), 400));
	addLight(new PointLight(glm::vec3(-3, 20, 0), 200));
	addLight(new PointLight(glm::vec3(4, 25, 0)));
	areaLight = new AreaLight(glm::vec3(0, 30, 0), 10, 5, 5, 10, 10, 1);
	addLight(areaLight);

	// gui
	setupGUI();

	// allocate space for rendered image
	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);

	// load texture maps to objects
	if (!floor->diffuseMap.load("garage-paving/11_garage paving PBR texture_DIFF.jpg")) {
		cout << "Ground Plane: error loading diffuse map" << endl;
	}
	if (!floor->specularMap.load("garage-paving/11_garage paving PBR texture_SPEC.jpg")) {
		cout << "Ground Plane: error loading specular map" << endl;
	}
	floor->numTiles = 1;
	if (!backWall->diffuseMap.load("brick-wall/38_brick wall_DIFF.jpg")) {
		cout << "Back Wall: error loading diffuse map" << endl;
	}
	if (!backWall->specularMap.load("brick-wall/38_brick wall_SPEC.jpg")) {
		cout << "Back Wall:: error loading specular map" << endl;
	}
	backWall->numTiles = 8;
	if (!leftWall->diffuseMap.load("brick-wall/38_brick wall_DIFF.jpg")) {
		cout << "Left Wall: error loading diffuse map" << endl;
	}
	if (!leftWall->specularMap.load("brick-wall/38_brick wall_SPEC.jpg")) {
		cout << "Left Wall:: error loading specular map" << endl;
	}
	leftWall->numTiles = 8;
	if (!rightWall->diffuseMap.load("brick-wall/38_brick wall_DIFF.jpg")) {
		cout << "Right Wall: error loading diffuse map" << endl;
	}
	if (!rightWall->specularMap.load("brick-wall/38_brick wall_SPEC.jpg")) {
		cout << "Right Wall:: error loading specular map" << endl;
	}
	rightWall->numTiles = 8;
	if (!sphere1->diffuseMap.load("cobblestone-pavement/13_cobblestone pavement PBR texture_DIFFUSE.jpg")) {
		cout << "Sphere1: error loading diffuse map" << endl;
	}
	if (!sphere1->specularMap.load("cobblestone-pavement/13_cobblestone pavement PBR texture_SPEC.jpg")) {
		cout << "Sphere1: error loading specular map" << endl;
	}
	sphere1->numTiles = sphere1->radius;
	if (!sphere2->diffuseMap.load("marble-floor/44_marble floor_DIFF.jpg")) {
		cout << "Sphere2: error loading diffuse map" << endl;
	}
	if (!sphere2->specularMap.load("marble-floor/44_marble floor_SPEC.jpg")) {
		cout << "Sphere2: error loading specular map" << endl;
	}
	sphere2->numTiles = sphere2->radius;
}

void ofApp::update() {
	ambientLight.intensity = ambientLightIntensity;

	lights[0]->intensity = light1Intensity;
	lights[0]->position.x = light1X;
	lights[0]->position.y = light1Y;
	lights[0]->position.z = light1Z;

	lights[1]->intensity = light2Intensity;
	lights[1]->position.x = light2X;
	lights[1]->position.y = light2Y;
	lights[1]->position.z = light2Z;

	lights[2]->intensity = light3Intensity;
	lights[2]->position.x = light3X;
	lights[2]->position.y = light3Y;
	lights[2]->position.z = light3Z;

	areaLight->intensity = areaLightIntensity;
	areaLight->position.x = areaLightX;
	areaLight->position.y = areaLightY;
	areaLight->position.z = areaLightZ;
	areaLight->width = areaLightWidth;
	areaLight->height = areaLightHeight;
	areaLight->nDivsWidth = areaLightDivWidth;
	areaLight->nDivsHeight = areaLightDivHeight;
	areaLight->nSamples = areaLightSamples;
}

void ofApp::draw() {
	ofEnableDepthTest();
	theCam->begin();

	// draw scene objects
	ofFill();
	ofPushMatrix();
	for (auto obj : scene) {
		obj->draw();
	}
	for (auto l : lights) {
		l->draw();
	}
	ofPopMatrix();
	ofNoFill();

	// draw render view as white box
	ofPushMatrix();
	ofNoFill();
	ofSetColor(ofColor::white);
	renderCam.view.draw();
	ofPopMatrix();

	ofSetColor(ofColor::lightGray);
	mainCam.draw();
	theCam->end();

	ofDisableDepthTest();

	if (!bHide) gui.draw();

	// rendered image
	if (bRendered) {
		image.draw((ofGetWindowWidth() / 2) - (imageWidth / 2), (ofGetWindowHeight() / 2) - (imageHeight / 2), imageWidth, imageHeight);
	}
}

// listener functions
void ofApp::lambertOnly(bool& val) {
	if (lambertShading) phongShading = false;
}

void ofApp::phongOnly(bool& val) {
	if (phongShading) lambertShading = false;
}

void ofApp::keyPressed(int key) {
	switch (key) {
	case 'h':
		// show/hide gui
		bHide = !bHide;
		break;
	case 'i':
		// show/hide rendered image
		bRendered = !bRendered;
		break;
	case 'r':
		// render image with raytracing
		rayTrace();
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		// look at render cam
		theCam = &previewCam;
		break;
	case OF_KEY_F3:
		theCam = &sideCam;
		break;
	default:
		break;
	}
}

void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
void ofApp::gotMessage(ofMessage msg) {}

// main ray trace loop, called by 'r' button
void ofApp::rayTrace() {
	printf("rayTrace called\n");

	// go through each pixel in image
	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {

			float u = (i + 0.5) / imageWidth;
			float v = (j + 0.5) / imageHeight;

			Ray ray = renderCam.getRay(u, v);

			// variables to store information from intersection check
			float distance = std::numeric_limits<float>::infinity();
			glm::vec3 closestPoint;
			glm::vec3 normalAtIntersect;
			SceneObject* closestObject = NULL;

			// check all objects in scene for intersection
			for (SceneObject* object : scene) {
				glm::vec3 point;
				glm::vec3 normal;

				// check intersection distance from camera
				if (object->intersect(ray, point, normal)) {
					float intersectDistance = glm::distance(ray.p, point);
					if (intersectDistance < distance) {
						closestObject = object;
						closestPoint = point;
						normalAtIntersect = normal;
						distance = intersectDistance;
					}
				}
			}

			if (closestObject) {
				// default values if object has no texture/shading type not selected
				ofColor color = closestObject->diffuseColor;
				float specular = phongPower;

				// check for textures
				if (closestObject->diffuseMap.isAllocated() && closestObject->specularMap.isAllocated()) {
					
					// check object type (only plane/sphere)
					Plane* plane = dynamic_cast<Plane*>(closestObject);
					Sphere* sphere = dynamic_cast<Sphere*>(closestObject);

					// texture coordinates depend on object type
					float texU, texV;
					if (plane) {
						plane->getTextureCoords(closestPoint, texU, texV);
					}
					else if (sphere) {
						sphere->getTextureCoords(closestPoint, texU, texV);
					}

					// get texture color from diffuse map
					float diffuseX = texU * closestObject->diffuseMap.getWidth();
					float diffuseY = texV * closestObject->diffuseMap.getHeight();
					diffuseX = ofClamp(diffuseX, 0, closestObject->diffuseMap.getWidth() - 1);
					diffuseY = ofClamp(diffuseY, 0, closestObject->diffuseMap.getHeight() - 1);
					color = closestObject->diffuseMap.getColor(diffuseX, diffuseY);

					// get specular coefficient from specular map
					int specX = texU * closestObject->specularMap.getWidth();
					int specY = texV * closestObject->specularMap.getHeight();
					specX = ofClamp(specX, 0, closestObject->specularMap.getWidth() - 1);
					specY = ofClamp(specY, 0, closestObject->specularMap.getHeight() - 1);
					specular = closestObject->specularMap.getColor(specX, specY).getBrightness();
				}

				if (lambertShading) color = lambert(closestPoint, normalAtIntersect, color);
				if (phongShading) color = phong(closestPoint, normalAtIntersect, color, ofColor::lightYellow, specular);
				image.setColor(i, imageHeight - j - 1, color);
			}
			else {
				// default to background color if no object
				image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor());
			}
		}
	}

	// update & save image
	image.update();
	image.save("/renderedImages/render1.png");
	bRendered = true;

	printf("rayTrace done\n");
}

// check if any object in the scene intersects the ray between the light and point
bool ofApp::inShadow(Ray ray) {
	for (auto obj : scene) {
		glm::vec3 intersectPoint;
		glm::vec3 normal;
		// does not account for objects "above" light
		if (obj->intersect(ray, intersectPoint, normal)) {
			return true;
		}
	}
	return false;
}

// lambert shading
ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& norm,
	const ofColor diffuse) {

	ofColor result = ambientLight.intensity * diffuse;
	float totalDiffuse = 0;

	for (auto light : lights) {
		if (light->intensity <= 0) continue; // skip lights with no "light"

		int numRays = light->getRaySamples(p, norm); // get ray(s) from light
		for (int i = 0; i < numRays; i++) {
			if (!inShadow(light->samples[i])) {

				// calculate intensity of light with respect to distance
				float distance = glm::length(light->samplesPos[i] - p);
				float illumination = light->intensity / (distance * distance);

				// lambert formula
				glm::vec3 lightDirection = light->samples[i].d;
				float lambertCalc = glm::max(glm::dot(norm, lightDirection), 0.0f);

				totalDiffuse += lambertCalc * illumination;
			}
		}
		result += diffuse * (totalDiffuse / numRays);
	}

	return result;
}

// phong shading (lambert + specular)
ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm,
	const ofColor diffuse, const ofColor specular, float power) {

	ofColor result = ambientLight.intensity * diffuse;
	float totalDiffuse = 0;
	float totalSpecular = 0;

	for (auto light : lights) {
		if (light->intensity <= 0) continue; // skip lights with no "light"

		int numRays = light->getRaySamples(p, norm); // get ray(s) from light
		for (int i = 0; i < numRays; i++) {

			if (!inShadow(light->samples[i])) {

				// calculate intensity of light with respect to distance
				float distance = glm::length(light->samplesPos[i] - p);
				float illumination = light->intensity / (distance * distance);

				// lambert formula
				glm::vec3 lightDirection = light->samples[i].d;
				float lambertCalc = glm::max(glm::dot(norm, lightDirection), 0.0f);

				// specular formula
				glm::vec3 viewDirection = glm::normalize(renderCam.position - p);
				glm::vec3 h = glm::normalize(viewDirection + lightDirection);
				float specularCalc = glm::pow(glm::max(glm::dot(norm, h), 0.0f), power);

				totalDiffuse += lambertCalc * illumination;
				totalSpecular += specularCalc * illumination;
			}
		}
		result += (diffuse * (totalDiffuse / numRays)) + (specular * (totalSpecular / numRays));
	}

	return result;
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