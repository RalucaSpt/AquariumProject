#include "Texture.h"
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION

#include <codecvt>

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <locale>
#include <map>
#include <math.h> 

#include "Bubble.h"
#include "TransparentObjects.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

#include <chrono>
#include <thread>
#include <random>

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

bool rotateLight = true;

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;
multimap<float, glm::vec3> sortedMap;
void Cleanup()
{
    delete pCamera;
}

int numFishes = 20;

struct BubbleParams
{
	glm::vec3 position;
	float size;
	float speed;
    int startTime;
};





std::vector<glm::vec3> offsets(numFishes);
std::vector<float> angularSpeeds(numFishes);
void generateBubbleOffsets()
{
	for (int i = 0; i < numFishes; i++)
	{
		offsets[i] = glm::vec3(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f)));
	}
    
	for (int i = 0; i < numFishes; i++)
	{
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distribSpeed(0.1f, 0.5f); // Adjust range as needed
        float randomAngularSpeed = distribSpeed(gen);
		angularSpeeds[i] = randomAngularSpeed;
	}
}

void GenerateNewBubbleOffsets(int index)
{
    offsets[index] = glm::vec3(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f)));

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribSpeed(0.5f, 2.0f); // Adjust range as needed
	float randomAngularSpeed = distribSpeed(gen);
	angularSpeeds[index] = randomAngularSpeed;
}
// Function to draw colored border around fish object
void DrawColoredBorder(Model& fishObjModel, const glm::mat4& fishModel, bool transparent) {
	const glm::vec3 borderColor = glm::vec3(1.0f, 0.0f, 0.0f); // Red color

    glm::mat4 borderModel = fishModel;
    Shader borderShader("border.vs", "border.fs");
    borderShader.Use();

    borderShader.SetMat4("model", borderModel);
    borderShader.SetMat4("view", pCamera->GetViewMatrix());
    borderShader.SetMat4("projection", pCamera->GetProjectionMatrix());
    borderShader.SetVec3("borderColor", borderColor);
    if (transparent) {
        borderShader.SetVec4("borderColor", glm::vec4(borderColor, 0.0f));
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    fishObjModel.DrawBorder(borderShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SwitchToFishPerspective(Camera* camera, const glm::vec3& fishPosition)
{
    glm::vec3 offset = glm::vec3(0.0f, 1.5f, -3.0f);
    camera->SetPosition(fishPosition + offset);
    glm::vec3 direction = glm::normalize(fishPosition - camera->GetPosition());

    float yaw = glm::degrees(atan2(direction.x, direction.z)) + 90;
    float pitch = glm::degrees(asin(direction.y));

    camera->SetYaw(yaw);
    camera->SetPitch(pitch);

}

// Define initial position of the bubble
glm::vec3 bubblePosition;

// Define parameters for spiral motion
float radius = 0.5f;
float verticalSpeed = 0.2f;
float angularSpeed = 0.5f;
float bubbleTime = 0.0f;
//bubble size params
float bubbleSize = 0.1f;


// Function to reset the position and size of the bubble with an offset
void resetBubblePosition(glm::vec3 newPos, glm::vec3 offset) {
    bubblePosition = newPos + offset;
    //generate new random bubble size
    bubbleSize = rand() % 10 / 100.0f;
    bubbleTime= 0.0f;
}

// Function to update the position of the bubble with an offset
void updateBubblePosition(glm::vec3 startPoz, glm::vec3 offset, float randomAngularSpeed) {
    static std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();

    // Check if 2 seconds have passed since the last update
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = currentTime - lastUpdateTime;

    // If 2 seconds have passed, update the bubble position
    lastUpdateTime = currentTime;
    // Compute spiral motion with the randomized angular speed
    float xSpiral = radius * cos(randomAngularSpeed * bubbleTime); // Compute x-coordinate of spiral
    float zSpiral = radius * sin(randomAngularSpeed * bubbleTime); // Compute z-coordinate of spiral
    float y = verticalSpeed * bubbleTime; // Compute y-coordinate for vertical motion

    bubbleSize -= 0.0001f;
    if (bubbleSize <= 0.0f)
    {
        if (elapsedTime.count() < 2.0) {
            resetBubblePosition(startPoz, offset);
            return;
        }
    }

    bubblePosition = startPoz + glm::vec3(xSpiral , y, zSpiral);
    bubbleTime += 0.001f;
}


// Function to generate a random offset for the bubble position
glm::vec3 generateRandomOffset(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribX(minX, maxX);
    std::uniform_real_distribution<float> distribY(minY, maxY);
    std::uniform_real_distribution<float> distribZ(minZ, maxZ);

    float offsetX = distribX(gen);
    float offsetY = distribY(gen);
    float offsetZ = distribZ(gen);

    return glm::vec3(offsetX, offsetY, offsetZ);
}


float roiRadius = 5.0f;
glm::vec3 fishPosition = glm::vec3(0.0f, 0.0f, 0.0f);

// Check if camera is within ROI
bool IsCameraWithinROI(Camera* camera, const glm::vec3& fishPosition, float roiRadius) {
    // Calculate distance between camera position and fish position
    glm::vec3 cameraPosition = camera->GetPosition();
    float distance = glm::distance(cameraPosition, fishPosition);

    // Check if distance is within ROI radius
    return distance < roiRadius;
}

// Define global variables to store mouse coordinates
double mouseX = 0.0;
double mouseY = 0.0;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

int keyPress = 0;
bool isInFishPerspective = false;
bool isTransparent = false;

glm::mat4 fishModel = glm::mat4(1.0f); // Identity matrix


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {

    }
    // Check if F key is pressed to switch camera perspective
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        // Check if camera is within ROI of the fish
        if (IsCameraWithinROI(pCamera, fishPosition, roiRadius)) {
            // Switch camera perspective to that of the fish
            if (keyPress % 2 == 0) {
                SwitchToFishPerspective(pCamera, fishPosition);
                isInFishPerspective = true;
            }
            else {
                // Return to initial camera position
                pCamera->SetPosition(glm::vec3(0.0, 1.0, 3.0));
                isInFishPerspective = false;
            }
            isTransparent = !isTransparent;
            keyPress++;
        }
        else if (keyPress % 2 == 1) {
            // Return to initial camera position
            pCamera->SetPosition(glm::vec3(0.0, 1.0, 3.0));
            keyPress++;
            isInFishPerspective = false;
        }
    }
}


//auxiliar fish model
float fishAngleY = 0.0f;
float fishAngleZ = 0.0f;
Scene scene;

int main(int argc, char** argv)
{
    std::string strFullExeFileName = argv[0];
    std::string strExePath;
    const size_t last_slash_idx = strFullExeFileName.rfind('\\');
    if (std::string::npos != last_slash_idx) {
        strExePath = strFullExeFileName.substr(0, last_slash_idx);
    }

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AQUARIUM PROJECT", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewInit();

    // Create Cam
    pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 1.0, 3.0));

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    wchar_t buffer[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, buffer);

    std::wstring executablePath(buffer);
    std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string currentPath = converter.to_bytes(wscurrentPath);

    // build and compile shaders
    // -------------------------
    Shader shadowMappingShader("ShadowMapping.vs", "ShadowMapping.fs");
    Shader shadowMappingDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");
    Shader transparentShader("TransparentObjShader.vs", "TransparentObjShader.fs");

    std::string fishObjFileName = (currentPath + "\\Models\\Fish\\fish.obj");
    Model fishObjModel(fishObjFileName, false);

    std::string goldFishObjFileName = (currentPath + "\\Models\\Fish2\\fish2.obj");
    Model goldFishObjModel(goldFishObjFileName, false);

    std::string coralBeautyObjFileName = currentPath + "\\Models\\CoralBeauty\\coralBeauty.obj";
    Model coralBeautyModel(coralBeautyObjFileName, false);

    Model grayFishModel(currentPath + "\\Models\\GreyFish\\fish.obj", false);

    std::string starfishObjFileName = currentPath + "\\Models\\Starfish\\starFish.obj";
    Model starfishModel(starfishObjFileName, false);

    Model bubbleModel(currentPath + "\\Models\\Bubble\\bubble.obj", false);

    // load textures
    // -------------
    Texture txtr;
    std::vector<unsigned int> texturePaths;

    unsigned int glassTexture = txtr.CreateTexture(strExePath + "\\Glass.png",
        0.05f);
    unsigned int floorTexture = txtr.CreateTexture(strExePath + "\\sand.jpg", 1.0f);
    texturePaths.push_back(glassTexture);
    texturePaths.push_back(floorTexture);


    unsigned int fishTexture = txtr.CreateTexture(currentPath + "\\Models\\Fish\\fish.jpg", 1.0f);
    
    // configure depth map FBO
    // -----------------------
	unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader configuration
    // --------------------
    shadowMappingShader.Use();
    shadowMappingShader.SetInt("diffuseTexture", 0);
    shadowMappingShader.SetInt("shadowMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

    generateBubbleOffsets();

    while (!glfwWindowShouldClose(window))
    {
        
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.5f, 0.5f, 1.0f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;

        if (rotateLight) {
            float radius = 2.0f;
            float time = glfwGetTime();
            float lightX = cos(time) * radius;
            float lightZ = sin(time) * radius;
            lightPos = glm::vec3(lightX, 4.0f, lightZ);
        }

        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view
        shadowMappingDepthShader.Use();
        shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
       
        scene.renderScene(shadowMappingDepthShader, texturePaths);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadowMappingShader.Use();

        glm::mat4 projection = pCamera->GetProjectionMatrix();
        glm::mat4 view = pCamera->GetViewMatrix();
        shadowMappingShader.SetMat4("projection", projection);
        shadowMappingShader.SetMat4("view", view);
        // set light uniforms
        shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
        shadowMappingShader.SetVec3("lightPos", lightPos);
        shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glDisable(GL_CULL_FACE);
        scene.renderScene(shadowMappingShader, texturePaths);

    	glm::mat4 goldFish = glm::mat4(1.0f);

        goldFish = glm::translate(goldFish, glm::vec3(1.0f, 0.0f, -1.0f));
        goldFish = glm::rotate(goldFish, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        goldFish = glm::scale(goldFish, glm::vec3(0.1f, 0.1f, 0.1f));
        shadowMappingShader.SetMat4("model", goldFish);
    	goldFishObjModel.Draw(shadowMappingShader);


        glm::mat4 coralBeautyModelMatrix = glm::mat4(1.0f);
        coralBeautyModelMatrix = glm::translate(coralBeautyModelMatrix, glm::vec3(2.0f, 0.0f, -1.0f));
        coralBeautyModelMatrix = glm::rotate(coralBeautyModelMatrix, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        coralBeautyModelMatrix = glm::scale(coralBeautyModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        // Desenează modelul Coral Beauty
        shadowMappingShader.SetMat4("model", coralBeautyModelMatrix);
        coralBeautyModel.Draw(shadowMappingShader);

        // Configurare model matrix pentru Starfish
        glm::mat4 starfishModelMatrix = glm::mat4(1.0f);
        starfishModelMatrix = glm::translate(starfishModelMatrix, glm::vec3(-1.0f, 0.0f, -1.0f)); // Ajustează poziționarea
        starfishModelMatrix = glm::scale(starfishModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f)); // Ajustează scalarea

        // Desenează modelul Starfish
        shadowMappingShader.SetMat4("model", starfishModelMatrix);
        starfishModel.Draw(shadowMappingShader);

    	fishModel = glm::mat4(1.0f);
        fishModel = glm::translate(fishModel, fishPosition);
        //rotate around z-axis
        fishModel = glm::rotate(fishModel, glm::radians(
            90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis
        fishModel = glm::rotate(fishModel, glm::radians(
            90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around y-axis
        fishModel = glm::scale(fishModel, glm::vec3(
            0.1f, 0.1f, 0.1f
        )); // Scale uniformly
        shadowMappingShader.SetMat4("model", fishModel);


        // Define animation parameters
        float time = glfwGetTime(); // Get current time
        float fishSpeed = 1.0f; // Speed of fish animation

        // Define circle parameters
        float radius = 8.0f; // Radius of the circular path
        float angularSpeed = 0.1f; // Angular speed of the fish's movement

         // Number of fishes in the school
        float spacing = 0.2f; // Spacing between fishes

        // Loop to create the fish school
        for (int i = 0; i < numFishes; ++i) {
            // Calculate angle for each fish
            float angle = time * fishSpeed * angularSpeed + i * (2.0f * glm::pi<float>()) / numFishes;

            // Calculate position for each fish along the circular path
            float newX = sin(angle) * radius; // X-coordinate of the fish's position
            float newZ = cos(angle) * radius; // Z-coordinate of the fish's position

            // Calculate row and column indices
            int row = i / 5; // Assuming 5 fishes per row
            int col = i % 5;

            // Calculate position for each fish
            float posX = col * spacing;
            float posY = 0.0f;
            float posZ = row * spacing;


            //calculate new position for bubble
            glm::vec3 bubbleInitialPos = glm::vec3(posX + newX, 0.0f, posZ + newZ);

            glm::mat4 greyFish = glm::mat4(1.0f); // Reset fish model matrix
            greyFish = glm::translate(greyFish, glm::vec3(posX + newX, posY, posZ+newZ)); // Update position
            greyFish = glm::rotate(greyFish, -angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate fish
            greyFish = glm::scale(greyFish, glm::vec3(0.1f, 0.1f, 0.1f)); // Scale fish
            shadowMappingShader.SetMat4("model", greyFish); // Pass updated model matrix to shader
            grayFishModel.Draw(shadowMappingShader); // Draw fish object
        	glm::mat4 bubbleModelMatrix = glm::mat4(1.0f);
            shadowMappingShader.SetFloat("alpha", 0.5f);

            //updateBubblePosition(fishPosition, bubbleSize, offsets[i], angularSpeeds[i]);
            updateBubblePosition(bubbleInitialPos, bubblePosition, angularSpeeds[i]);
            
            bubbleModelMatrix = glm::translate(bubbleModelMatrix, bubblePosition);
            bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(bubbleSize, bubbleSize, bubbleSize));
            shadowMappingShader.SetMat4("model", bubbleModelMatrix);
            bubbleModel.Draw(shadowMappingShader);
        }


       // glm::mat4 bubbleModelMatrix = glm::mat4(1.0f);
       //// Bubble bubble;
       // //set alpha for bubble
       // shadowMappingShader.SetFloat("alpha", 0.5f);
       // updateBubblePosition(fishPosition);
       // bubbleModelMatrix = glm::translate(bubbleModelMatrix, bubblePosition);
       // //use bubbleSize to scale the bubble
       // bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(bubbleSize, bubbleSize, bubbleSize));
       // //bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
       // shadowMappingShader.SetMat4("model", bubbleModelMatrix);

       // bubbleModel.Draw(shadowMappingShader);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fishTexture);
        if (isInFishPerspective)
        {
            //Apply fishAngleY to fish model
            fishModel = glm::rotate(fishModel, glm::radians(fishAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
            fishModel = glm::rotate(fishModel, glm::radians(fishAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        shadowMappingShader.SetMat4("model", fishModel);
        fishObjModel.Draw(shadowMappingShader);

        
    	if (IsCameraWithinROI(pCamera, fishPosition, roiRadius) && !isInFishPerspective) 
        {
    		DrawColoredBorder(fishObjModel,  fishModel, isTransparent);
        }

        TransparentObjects transparentObjects;
        transparentObjects.Draw(transparentShader, pCamera, texturePaths[0], lightSpaceMatrix, lightPos, projection, view, scene);

        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    delete pCamera;

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
        if (isInFishPerspective) {
            fishPosition += glm::vec3(0.0f, 0.0f, 0.007f);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (!isInFishPerspective)
        {
            pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        pCamera->ProcessKeyboard(LEFT, (float)deltaTime);

    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        pCamera->ProcessKeyboard(UP, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        rotateLight = true;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        rotateLight = false;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        pCamera->Reset(width, height);

    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
    	if (isInFishPerspective)
    	{
    		fishPosition += glm::vec3(0.0f, 0.0f, 0.007f);
            //modify position with respect fishAngleY and fishAngleZ
            

            pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
		}
	}
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (isInFishPerspective)
        {
            //fishPosition += glm::vec3(0.007f, 0.0f, 0.0f);
            if (fishAngleZ < 90.0f)
            {
                fishAngleZ += 0.1f;
			}
        }
	}

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
    	if (isInFishPerspective)
    	{
            if (fishAngleZ > -90.0f)
            {
            	fishAngleZ -= 0.1f;
			}
		}
    }

    //go up with space
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
    	if (isInFishPerspective)
    	{
    		//fishPosition += glm::vec3(0.0f, 0.007f, 0.0f);
            fishAngleY += 0.1f;
		}
	}

    //go down with left shift
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (isInFishPerspective)
		{
			//fishPosition += glm::vec3(0.0f, -0.007f, 0.0f);
			fishAngleY -= 0.1f;
		}
	}



}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    pCamera->Reshape(width, height);
    //make other adjustments
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    pCamera->MouseControl((float)xpos, (float)ypos);
    mouseX = xpos;
    mouseY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
    pCamera->ProcessMouseScroll((float)yOffset);
}