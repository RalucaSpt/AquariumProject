#define GLM_FORCE_CTOR_INIT 

#include <GL/glew.h>
#include <glfw3.h>
#include <chrono>
#include <stb_image.h>
#include <cstdlib> // For rand() and srand()
#include <ctime> // For time()

#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Fish.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;
auto t_start = std::chrono::high_resolution_clock::now();

Camera* pCamera;
std::unique_ptr<Mesh> floorObj, cubeObj;
std::unique_ptr<Model> starFishObj, bubbleObj, sandDune, coral, plant, anchor, water, skullObj, treasureChestObj;
std::unique_ptr<Model> clownFishObj, goldFishObj, coralBeautyFishObj, grayFishObj, angelFishObj,
						blueGreenFishObj, rainbowFishObj, blackMoorFishObj, longFinFishObj, doryFishObj, yellowTangFishObj, lineWrasseFishObj, americanFlagFishObj, selectedFishObj;
float timeAcceleration = 0.1f;
glm::vec3 zrotation = glm::vec3(0.0f, 0.0f, 0.0f);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ProcessKeyboard(GLFWwindow* window);

void LoadObjects();
void RenderScene(Shader& shader);

double deltaTime = 0.0f;
double lastFrame = 0.0f;
int m_mapWidth, m_mapHeight, m_mapChannels, m_indicesRez;
std::vector<float> m_vertices;
int numGreyFishes = 40;
int numBubbles = 40;

std::vector<int> fishModels;
int numFishSpecies = 13;
void InitFishModels()
{
	int model = rand() % numFishSpecies;
	for (int i = 0; i < numGreyFishes; i++)
	{
		fishModels.push_back(model);
		model = rand() % numFishSpecies;
	}
}

struct BubbleParams
{
	glm::vec3 position;
	glm::vec3 newPos;
	float size;
	float speed;
	float startTime;
	float radius;
};
float verticalSpeed = 0.2f;
float bubbleTime = 0.0f;
std::vector<BubbleParams> bubbles;

//void DrawColoredBorder(Model& fishObjModel, const glm::mat4& fishModel, bool transparent) {
//	const glm::vec3 borderColor = glm::vec3(1.0f, 0.0f, 0.0f); // Red color
//
//	glm::mat4 borderModel = fishModel;
//	Shader borderShader("../Shaders/Border.vs", "../Shaders/Border.fs");
//	borderShader.Use();
//
//	borderShader.SetMat4("model", borderModel);
//	borderShader.SetMat4("view", pCamera->GetViewMatrix());
//	borderShader.SetMat4("projection", pCamera->GetProjectionMatrix());
//	borderShader.SetVec3("borderColor", borderColor);
//	if (transparent) {
//		borderShader.SetVec4("borderColor", glm::vec4(borderColor, 0.0f));
//	}
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	fishObjModel.RenderModel(borderShader);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//}

//void SwitchToFishPerspective(Camera* camera, const glm::vec3& fishPosition)
//{
//	glm::vec3 offset = glm::vec3(0.0f, 1.5f, -3.0f);
//	camera->SetPosition(fishPosition + offset);
//	glm::vec3 direction = glm::normalize(fishPosition - camera->GetPosition());
//
//	float yaw = glm::degrees(atan2(direction.x, direction.z)) + 90;
//	float pitch = glm::degrees(asin(direction.y));
//
//	camera->SetYaw(yaw);
//	camera->SetPitch(pitch);
//
//}

void generateBubblesParams()
{
	for (int i = 0; i < numBubbles; i++)
	{
		BubbleParams bubble;
		bubble.position = glm::vec3(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f)));
		bubble.size = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (0.5f - 0.1f)));
		bubble.speed = rand() % 10 / 100.0f;
		bubble.startTime = rand() % 10;
		bubble.radius = rand() % 10 / 10.0f;
		bubbles.push_back(bubble);
	}
}

void generateBubbleParams(int index)
{
	bubbles[index].position = glm::vec3(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f)));
	bubbles[index].size = rand() % 10 / 100.0f;
	bubbles[index].speed = rand() % 10 / 100.0f;
	bubbles[index].startTime = 0;
	bubbles[index].radius = rand() % 10 / 10.0f;
}

glm::vec3 bubblePosition;

void resetBubblePosition(int index) {
	bubbles[index].position = bubbles[index].newPos;
	bubbles[index].size = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (0.5f - 0.1f)));
	bubbles[index].startTime = 0.0f;
}

void updateBubblePosition(int index) {
	static std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedTime = currentTime - lastUpdateTime;

	lastUpdateTime = currentTime;
	float xSpiral = 0.5f * cos(bubbles[index].speed * bubbles[index].startTime); // Compute x-coordinate of spiral
	float zSpiral = 0.5f * sin(bubbles[index].speed * bubbles[index].startTime); // Compute z-coordinate of spiral
	float y = verticalSpeed * bubbles[index].startTime; // Compute y-coordinate for vertical motion
	bubbles[index].size -= 0.0001f;
	bubbles[index].startTime += 0.01f;

	if (bubbles[index].size <= 0.1f)
	{
		if (elapsedTime.count() < 2.0)
		{
			resetBubblePosition(index);
			return;
		}
	}
	bubbles[index].position = bubbles[index].newPos + glm::vec3(xSpiral, y, zSpiral);
}

void bubbleFlurry(int numBubbles, glm::vec3 position) {
	for (int i = 0; i < numBubbles; ++i) {
		BubbleParams bubble;

		// Generate a random offset from the given position
		float offsetX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0f) - 1.0f;
		float offsetY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0f) - 1.0f;
		float offsetZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0f) - 1.0f;

		bubble.position = position + glm::vec3(offsetX, offsetY, offsetZ);

		// Randomize other bubble parameters
		bubble.size = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.4f);
		bubble.speed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.1f);
		bubble.startTime = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 10.0f);
		bubble.radius = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 1.0f);

		bubbles.push_back(bubble);
	}
}

std::vector<Fish> fishes;

glm::vec3 GeneratePosition()
{
	float halfSideLength = 12.5f;

	float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (2 * halfSideLength)) - halfSideLength;
	float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (2 * halfSideLength)) - halfSideLength;
	float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (2 * halfSideLength)) - halfSideLength;

	return glm::vec3(x, y, z);
}

void InitFishParams()
{
	for (int i = 0; i < numGreyFishes; i++)
	{
		Fish fish;

		fish.SetPos(GeneratePosition());
		fish.SetFishSize(0.1f);

		float randomYaw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 360.0f); // Yaw between 0 and 360 degrees
		float randomPitch = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 90.0f) - 45.0f; // Pitch between -45 and 45 degrees
		float randomRoll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 360.0f); // Roll between 0 and 360 degrees

		fish.SetYaw(randomYaw);
		fish.SetPitch(randomPitch);
		fish.SetRoll(randomRoll);

		fish.InitialFishVectors();

		float timer = 0.0f;
		fish.SetFishMovementTimer(timer);
		fishes.push_back(fish);
	}
}


void resetFishTimer(int index)
{
	fishes[index].SetFishMovementTimer(1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5.0f - 2.0f))));
}

void UpdateFishPosition(int index, EFishMovementType direction)
{
	fishes[index].Move(direction); 
	fishes[index].MoveFish(deltaTime);
	float currFishTimer = fishes[index].GetFishMovementTimer();

	while (currFishTimer <= 0)
	{
		resetFishTimer(index);
		currFishTimer = fishes[index].GetFishMovementTimer();
		//return;
	}
	fishes[index].SetFishMovementTimer(currFishTimer - 0.01f);
}


float roiRadius = 5.0f;
glm::vec3 fishPosition = glm::vec3(0.0f, 3.0f, 0.0f);
Fish movingFish(fishPosition);
bool IsCameraWithinROI(Camera* camera, const glm::vec3& fishPosition, float roiRadius) {
	glm::vec3 cameraPosition = camera->GetPosition();
	float distance = glm::distance(cameraPosition, fishPosition);
	return distance < roiRadius;
}

int keyPress = 0;
bool isInFishPerspective = false;
bool isTransparent = false;

glm::mat4 fishModel = glm::mat4(1.0f);


float fishAngleY = 0.0f;
float fishAngleZ = 0.0f;

Fish greyFish(glm::vec3(0.0f, 3.0f, 0.0f));

int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "blip blop Simulator", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetKeyCallback(window, KeyCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 20, 0));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);

	Shader shadowMappingShader("../Shaders/ShadowMapping.vs", "../Shaders/ShadowMapping.fs");
	Shader shadowMappingDepthShader("../Shaders/ShadowMappingDepth.vs", "../Shaders/ShadowMappingDepth.fs");
	Shader borderShader("../Shaders/Border.vs", "../Shaders/Border.fs");
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	shadowMappingShader.Use();
	shadowMappingShader.SetInt("diffuseTexture", 0);
	shadowMappingShader.SetInt("shadowMap", 1);

	LoadObjects();

	glm::vec3 lightPos(-2.0f, 25.0f, -1.0f);
	float hue = 1.0;
	float floorHue = 0.9;
	bool rotateLight = true;
	generateBubblesParams();
	InitFishParams();
	InitFishModels();
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		ProcessKeyboard(window);
		movingFish.MoveMovingFish(deltaTime);
		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// 1. render depth of scene to texture (from light's perspective)
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 5.0f, far_plane = 500.f;

		if (rotateLight) {
			float radius = 2.0f;
			float time = glfwGetTime();
			float lightX = cos(time) * radius;
			float lightZ = sin(time) * radius;
			lightPos = glm::vec3(lightX, 14.0f, lightZ);
		}

		lightProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// render scene from light's point of view
		shadowMappingDepthShader.Use();
		shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(shadowMappingDepthShader);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. render scene as normal using the generated depth/shadow map 
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMappingShader.Use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix(movingFish);
		shadowMappingShader.SetMat4("projection", projection);
		shadowMappingShader.SetMat4("view", view);
		shadowMappingShader.SetFloat("hue", floorHue);


		// set light uniforms
		shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
		shadowMappingShader.SetVec3("lightPos", lightPos);
		shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDisable(GL_CULL_FACE);
		RenderScene(shadowMappingShader);

		hue = 1;
		floorHue = 1;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void ProcessKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (pCamera->GetCameraMode() == ThirdPerson)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::FORWARD);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::BACKWARD);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::LEFT);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::RIGHT);
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::UP);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			movingFish.Move(EFishMovementType::DOWN);
	}
	else if (pCamera->GetCameraMode() == FreeLook)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			pCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			pCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			pCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			pCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		pCamera->SetCameraMode(FreeLook);
		isInFishPerspective = false;
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		if (IsCameraWithinROI(pCamera, movingFish.GetPos(), roiRadius))
		{
			pCamera->SetCameraMode(ThirdPerson);
			isInFishPerspective = true;
		}
	}


	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}
}


void LoadObjects()
{
	// Texture loading
	Texture floorTexture("../Models/Floor/marble.jpg");
	Texture cubeTexture("../Models/Cube/glass.png");
	const float floorSize = 5000.0f;
	std::vector<Vertex> floorVertices({
		// positions              // normals          // texcoords
		{ floorSize, -13.0f,  floorSize,  0.0f, 1.0f, 0.0f,    1.0f, 0.0f},
		{-floorSize, -13.0f,  floorSize,  0.0f, 1.0f, 0.0f,    0.0f, 0.0f},
		{-floorSize, -13.0f, -floorSize,  0.0f, 1.0f, 0.0f,    0.0f, 1.0f},

		{ floorSize, -13.0f,  floorSize,  0.0f, 1.0f, 0.0f,    1.0f, 0.0f},
		{-floorSize, -13.0f, -floorSize,  0.0f, 1.0f, 0.0f,    0.0f, 1.0f},
		{ floorSize, -13.0f, -floorSize,  0.0f, 1.0f, 0.0f,    1.0f, 1.0f}
		});

	std::vector<Vertex> cubeVertices({
		// Back face
		Vertex(-12.5f, -12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(12.5f, -12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex(12.5f, 12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex(12.5f, 12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex(-12.5f, 12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f),
		Vertex(-12.5f, -12.5f, -12.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),

		// Front face
		Vertex(-12.5f, -12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
		Vertex(12.5f, -12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f),
		Vertex(12.5f, 12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(12.5f, 12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-12.5f, 12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-12.5f, -12.5f, 12.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),

		// Left face
		Vertex(-12.5f, 12.5f, 12.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(-12.5f, 12.5f, -12.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-12.5f, -12.5f, -12.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-12.5f, -12.5f, -12.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-12.5f, -12.5f, 12.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-12.5f, 12.5f, 12.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),

		// Right face
		Vertex(12.5f, 12.5f, 12.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(12.5f, -12.5f, 12.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		Vertex(12.5f, -12.5f, -12.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(12.5f, -12.5f, -12.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(12.5f, 12.5f, -12.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(12.5f, 12.5f, 12.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),

		// Bottom face
		Vertex(-12.5f, -12.5f, -12.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(12.5f, -12.5f, -12.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f),
		Vertex(12.5f, -12.5f, 12.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(12.5f, -12.5f, 12.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-12.5f, -12.5f, 12.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-12.5f, -12.5f, -12.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f)
		});

	stbi_set_flip_vertically_on_load(false);

	// Objects loading
	angelFishObj = std::make_unique<Model>("../Models/Fishes/AngelFish/fish.obj", false);
	blackMoorFishObj = std::make_unique<Model>("../Models/Fishes/BlackMoorFish/fish.obj", false);
	blueGreenFishObj = std::make_unique<Model>("../Models/Fishes/BlueGreenFish/fish.obj", false);
	clownFishObj = std::make_unique<Model>("../Models/Fishes/ClownFish/fish.obj", false);
	doryFishObj = std::make_unique<Model>("../Models/Fishes/DoryFish/fish.obj", false);
	goldFishObj = std::make_unique<Model>("../Models/Fishes/GoldFish/fish.obj", false);
	grayFishObj = std::make_unique<Model>("../Models/Fishes/GrayFish/fish.obj", false);
	lineWrasseFishObj = std::make_unique<Model>("../Models/Fishes/LineWrasseFish/fish.obj", false);
	longFinFishObj = std::make_unique<Model>("../Models/Fishes/LongFinFish/fish.obj", false);
	rainbowFishObj = std::make_unique<Model>("../Models/Fishes/RainbowFish/fish.obj", false);
	yellowTangFishObj = std::make_unique<Model>("../Models/Fishes/YellowTang/fish.obj", false);
	americanFlagFishObj = std::make_unique<Model>("../Models/Fishes/AmericanFlagFish/fish.obj", false);

	selectedFishObj = std::make_unique<Model>("../Models/Fishes/ClownFish/selectedFish.obj", false);
	cubeObj = std::make_unique<Mesh>(cubeVertices, std::vector<unsigned int>(), std::vector<Texture>{cubeTexture});
	starFishObj = std::make_unique<Model>("../Models/StarFish/starFish.obj", false);

	bubbleObj = std::make_unique<Model>("../Models/Bubble/bubble.obj", false);
	sandDune = std::make_unique<Model>("../Models/BigFauna/fauna.obj", false);
	water  = std::make_unique<Model>("../Models/Water/water.obj", false);
	skullObj = std::make_unique<Model>("../Models/Skull/skull.obj", false);
	treasureChestObj = std::make_unique<Model>("../Models/TreasureChest/treasureChest.obj", false);
	floorObj = std::make_unique<Mesh>(floorVertices, std::vector<unsigned int>(), std::vector<Texture>{floorTexture});
}

EFishMovementType direction = EFishMovementType::FORWARD;

int movementIndex = 0;

void RenderScene(Shader& shader)
{

	//Starfish
	glm::mat4 starfishModelMatrix = glm::mat4(1.0f);
	starfishModelMatrix = glm::translate(starfishModelMatrix, glm::vec3(-1.0f, 0.0f, -1.0f)); // Ajustează poziționarea
	starfishModelMatrix = glm::scale(starfishModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f)); // Ajustează scalarea
	starFishObj->RenderModel(shader, starfishModelMatrix);

	//static fish
	fishModel = glm::mat4(1.0f);
	fishModel = glm::translate(fishModel, movingFish.GetPos());
	fishModel = glm::rotate(fishModel, glm::radians(movingFish.GetYaw()), glm::vec3(0, 1, 0));
	fishModel = glm::rotate(fishModel, -glm::radians(movingFish.GetPitch()), glm::vec3(1, 0, 0));
	fishModel = glm::scale(fishModel, glm::vec3(
		0.1f, 0.1f, 0.1f
	)); // Scale uniformly

	float tankHalfSideLength = 12.5f * 5;
	float tankHeight = 10.0f; // Set this to the actual height of your tank

	for (int i = 0; i < numGreyFishes; ++i) {
		fishes[i].CheckWalls(tankHalfSideLength, tankHeight);
		direction = EFishMovementType::LEFT;
		glm::mat4 fish = glm::mat4(1.0f);
		float currFishTimer = fishes[i].GetFishMovementTimer();
		if (currFishTimer <= 0) {
			resetFishTimer(i);
			direction = fishes[i].GetMove(movementIndex);
			movementIndex++;
			currFishTimer = fishes[i].GetFishMovementTimer();

			fishes[i].StartNewMovement(currFishTimer, direction);
		}

		float deltaTime = 0.1f;
		fishes[i].MoveFish(deltaTime);
		fishes[i].SetFishMovementTimer(currFishTimer - deltaTime);

		auto pos = fishes[i].GetPos();

		fish = glm::translate(fish, pos);
		fish = glm::rotate(fish, glm::radians(90.0f), glm::vec3(0, 1, 0));
		fish = glm::rotate(fish, -glm::radians(fishes[i].GetYaw()), glm::vec3(0, 1, 0));
		fish = glm::rotate(fish, -glm::radians(fishes[i].GetPitch()), glm::vec3(1, 0, 0));
		fish = glm::scale(fish, glm::vec3(0.5f, 0.5f, 0.5f));

		glm::vec3 bubbleInitialPos = pos;

		int randomFishObject = fishModels[i];
		switch (randomFishObject)
		{
		case 0:
			grayFishObj->RenderModel(shader, fish);
			break;
		case 1:
			goldFishObj->RenderModel(shader, fish);
			break;
		case 2:
			clownFishObj->RenderModel(shader, fish);
			break;
		case 3:
			angelFishObj->RenderModel(shader, fish);
			break;
		case 4:
			blueGreenFishObj->RenderModel(shader, fish);
			break;
		case 5:
			rainbowFishObj->RenderModel(shader, fish);
			break;
		case 6:
			blackMoorFishObj->RenderModel(shader, fish);
			break;
		case 7:
			longFinFishObj->RenderModel(shader, fish);
			break;
		case 8:
			doryFishObj->RenderModel(shader, fish);
			break;
		case 9:
			yellowTangFishObj->RenderModel(shader, fish);
			break;
		case 10:
			lineWrasseFishObj->RenderModel(shader, fish);
			break;
		case 11:
			americanFlagFishObj->RenderModel(shader, fish);
			break;
		}
		bubbles[i].newPos = bubbleInitialPos;
	}

	

	glm::mat4 sandDuneModelMatrix = glm::mat4(1.0f);
	sandDuneModelMatrix = glm::translate(sandDuneModelMatrix, glm::vec3(0.0f, -12.f, 0.0f)); 
	sandDuneModelMatrix = glm::scale(sandDuneModelMatrix, glm::vec3(12.f, 9.f, 12.f));
	sandDune->RenderModel(shader, sandDuneModelMatrix);

	glm::mat4 anchorModelMatrix = glm::mat4(1.0f);
	anchorModelMatrix = glm::translate(anchorModelMatrix, glm::vec3(20.0f, -14.f, -15.0f));
	anchorModelMatrix = glm::scale(anchorModelMatrix, glm::vec3(0.13f, 0.13f, 0.13f));
	//anchor->RenderModel(shader, anchorModelMatrix);

	glm::mat4 coralModelMatrix = glm::mat4(1.0f);
	coralModelMatrix = glm::translate(coralModelMatrix, glm::vec3(0.0f, 0.f, 0.0f));
	coralModelMatrix = glm::scale(coralModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	//coral->RenderModel(shader, coralModelMatrix);

	glm::mat4 plantModelMatrix = glm::mat4(1.0f);
	plantModelMatrix = glm::translate(plantModelMatrix, glm::vec3(5.0f, -1.f, 8.0f));
	plantModelMatrix = glm::scale(plantModelMatrix, glm::vec3(10.f, 10.f, 10.f));
	plantModelMatrix = glm::rotate(plantModelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//plant->RenderModel(shader, plantModelMatrix);

	glm::mat4 skullMatrix = glm::mat4(1.0f);
	skullMatrix = glm::translate(skullMatrix, glm::vec3(25.0f, -8.0f, -5.0f));
	skullMatrix = glm::scale(skullMatrix, glm::vec3(1.f, 1.f, 1.f));
	skullMatrix = glm::rotate(skullMatrix, glm::radians(290.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
	skullObj->RenderModel(shader, skullMatrix);
	
	glm::mat4 treasureChestMatrix = glm::mat4(1.0f);
	treasureChestMatrix = glm::translate(treasureChestMatrix, glm::vec3(35.0f, -8.0f, 15.0f));
	treasureChestMatrix = glm::scale(treasureChestMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	treasureChestMatrix = glm::rotate(treasureChestMatrix, glm::radians(250.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
	treasureChestObj->RenderModel(shader, treasureChestMatrix);

	if (isInFishPerspective)
	{
		//Apply fishAngleY to fish model
		fishModel = glm::rotate(fishModel, glm::radians(fishAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
		fishModel = glm::rotate(fishModel, glm::radians(fishAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glEnable(GL_CULL_FACE);

	for (int i = 0; i < numBubbles; i++)
	{

		//shadowMappingShader.SetFloat("alpha", 0.5f);
		glm::mat4 bubbleModelMatrix = glm::mat4(1.0f);
		updateBubblePosition(i);
		bubbleModelMatrix = glm::translate(bubbleModelMatrix, bubbles[i].position);
		bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(bubbles[i].size, bubbles[i].size, bubbles[i].size));
		bubbleObj->RenderModel(shader, bubbleModelMatrix);
	}
	//call bubble flurry method for 20 bubbles in the corner of the cube
	bubbleFlurry(20, glm::vec3(0.0f, 0.0f, 0.0f));

	for (int i = 0; i < 20; i++)
	{
		glm::mat4 bubbleModelMatrix = glm::mat4(1.0f);
		bubbleModelMatrix = glm::translate(bubbleModelMatrix, bubbles[i].newPos);
		bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(bubbles[i].size, bubbles[i].size, bubbles[i].size));
		bubbleObj->RenderModel(shader, bubbleModelMatrix);
	}

	glDisable(GL_CULL_FACE);
	if (IsCameraWithinROI(pCamera, movingFish.GetPos(), roiRadius) && !isInFishPerspective)
	{
		selectedFishObj->RenderModel(shader, fishModel);
	}
	else clownFishObj->RenderModel(shader, fishModel);

	floorObj->RenderMesh(shader);

	glm::mat4 waterModelMatrix = glm::mat4(1.0f);
	waterModelMatrix = glm::translate(waterModelMatrix, glm::vec3(0.0f, -12.f, 0.0f));
	waterModelMatrix = glm::scale(waterModelMatrix, glm::vec3(12.f, 8.f, 12.f));
	water->RenderModel(shader, waterModelMatrix);

	glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
	cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(5.f, 4.0f, 5.f));
	cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 9.5f, 0.0f));
	cubeObj->RenderMesh(shader, cubeModelMatrix);
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		timeAcceleration += 0.05f;
	}
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	pCamera->Reshape(width, height);
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

