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

#include "TransparentObjects.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")


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
glm::vec3 bubblePosition = glm::vec3(0.0f, 0.0f, 0.0f);

// Define parameters for spiral motion
float radius = 0.5f;
float verticalSpeed = 0.1f;
float angularSpeed = 0.5f;
float bubbleTime = 0.0f;
float xOffset = 0.0f;
float zOffset = 0.0f;
// Function to update the position of the bubble
void updateBubblePosition() {
    // Compute spiral motion
    float xSpiral = radius * cos(angularSpeed * bubbleTime); // Compute x-coordinate of spiral
    float zSpiral = radius * sin(angularSpeed * bubbleTime); // Compute z-coordinate of spiral
    float y = verticalSpeed * bubbleTime; // Compute y-coordinate for vertical motion

    // Apply random movement
    
    int random = rand() % 100;
    if (random == 3) {
        //xOffset = -0.1f;
        radius += 0.1f;
    }
    else if (random == 7) {
        //xOffset = 0.1f;
        radius -= 0.1f;
    }
    else if (random == 5) {
        //zOffset = -0.1f;
        
    }
    else if (random == 9) {
        //zOffset = 0.1f;
    }

    // Update bubble position with spiral and random movement offsets
    bubblePosition = glm::vec3(xSpiral + xOffset, y, zSpiral + zOffset);

    // Increment time
    bubbleTime += 0.01f;

    // Reset time if it exceeds 2*PI
    /*if (bubbleTime > 2 * glm::pi<float>()) {
        bubbleTime = 0.0f;
    }*/
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
glm::mat4 auxFishModel = glm::mat4(1.0f);
float fishAngleY = 0.0f;
float fishAngleZ = 0.0f;

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
    Shader blendingShader("Blending.vs", "Blending.fs");
    Shader floorShader("Floor.vs", "Floor.fs");

    std::string fishObjFileName = (currentPath + "\\Models\\Fish\\fish.obj");
    Model fishObjModel(fishObjFileName, false);

    std::string fish2ObjFileName = (currentPath + "\\Models\\Fish2\\fish2.obj");
    Model fish2ObjModel(fish2ObjFileName, false);

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
    unsigned int fish2Texture = txtr.CreateTexture(strExePath + "\\Models\\Fish2\\fish2.jpg", 1.0f);
    unsigned int coralBeautyTexture = txtr.CreateTexture(currentPath + "\\Models\\CoralBeauty\\coralBeauty.jpg", 1.0f);
    unsigned int coralTexture = txtr.CreateTexture(strExePath + "\\blue_coral.png", 1.0f);
    unsigned int starfishTexture = txtr.CreateTexture(currentPath + "\\Models\\Starfish\\starFish.jpg", 1.0f);

    // configure depth map FBO
    // -----------------------
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


    // Grass vertices
    float grassVertices[] = {
        -0.25f, -0.25f, 0.0f, 0.0f, 0.0f,
        -0.25f, 0.25f, 0.0f, 0.0f, 1.0f,
        0.25f, 0.25f, 0.0f, 1.0f, 1.0f,


        -0.25f, -0.25f, 0.0f, 0.0f, 0.0f,
        0.25f, 0.25f, 0.0f, 1.0f, 1.0f,
        0.25f, -0.25f, 0.0f, 1.0f, 0.0f
    };

    int gridX = 10; // de exemplu, 10 pătrate pe axa X
    int gridZ = 10; // de exemplu, 10 pătrate pe axa Z

    // Dimensiunea pătratului de iarbă
    float squareSize = 0.5f;

    // Coordonatele de bază ale vertex-urilor pentru un pătrat de iarbă

    // Grass VAO si VBO
    unsigned int grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), &grassVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f); // Set clear color with alpha 0
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable blending
        glEnable(GL_BLEND);
        // Set blend function
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // 1. render depth of scene to texture (from light's perspective)
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;

        if (rotateLight) {
            // Calculăm unghiul pentru rotația luminii
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
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        Scene scene;
        scene.renderScene(shadowMappingDepthShader, texturePaths);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        // 2. render scene as normal using the generated depth/shadow map 
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glDisable(GL_CULL_FACE);
        scene.renderScene(shadowMappingShader, texturePaths);


        glBindTexture(GL_TEXTURE_2D, fishTexture);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fish2Texture);

        glm::mat4 fish2Model = glm::mat4(1.0f);

        fish2Model = glm::translate(fish2Model, glm::vec3(1.0f, 0.0f, -1.0f));
        fish2Model = glm::rotate(fish2Model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        fish2Model = glm::scale(fish2Model, glm::vec3(0.1f, 0.1f, 0.1f));
        shadowMappingShader.SetMat4("model", fish2Model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fish2Texture);
        fish2ObjModel.Draw(shadowMappingShader);


       /* glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, coralBeautyTexture);
        shadowMappingShader.SetInt("coralBeauty", 4);*/

        // Configurare model matrix pentru Coral Beauty
        glm::mat4 coralBeautyModelMatrix = glm::mat4(1.0f);
        coralBeautyModelMatrix = glm::translate(coralBeautyModelMatrix, glm::vec3(2.0f, 0.0f, -1.0f));
        coralBeautyModelMatrix = glm::rotate(coralBeautyModelMatrix, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        coralBeautyModelMatrix = glm::scale(coralBeautyModelMatrix, glm::vec3(1.1f, 1.1f, 1.1f));
        // Desenează modelul Coral Beauty
        shadowMappingShader.SetMat4("model", coralBeautyModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, coralBeautyTexture);
        coralBeautyModel.Draw(shadowMappingShader);
        

        // Setează unitatea de textură pentru starfish
        glActiveTexture(GL_TEXTURE5); // Alege o unitate de textură neutilizată anterior
        glBindTexture(GL_TEXTURE_2D, starfishTexture);
        shadowMappingShader.SetInt("starfish", 5);

        // Configurare model matrix pentru Starfish
        glm::mat4 starfishModelMatrix = glm::mat4(1.0f);
        starfishModelMatrix = glm::translate(starfishModelMatrix, glm::vec3(-1.0f, 0.0f, -1.0f)); // Ajustează poziționarea
        starfishModelMatrix = glm::scale(starfishModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f)); // Ajustează scalarea

        // Desenează modelul Starfish
        shadowMappingShader.SetMat4("model", starfishModelMatrix);
        starfishModel.Draw(shadowMappingShader);


        glActiveTexture(GL_TEXTURE0); // Folosește o altă unitate de textură
        glBindTexture(GL_TEXTURE_2D, fishTexture);
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

        // Calculate new position of the fish along the circular path
        float newX = sin(time * fishSpeed * angularSpeed) * radius; // X-coordinate of the fish's position
        float newZ = cos(time * fishSpeed * angularSpeed) * radius; // Z-coordinate of the fish's position

        // Calculate angle for fish rotation
        float angle = atan2(newZ, newX); // Calculate angle based on fish's position

        // Update fish model matrix with new position
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, fishTexture);
        glm::mat4 greyFish = glm::mat4(1.0f); // Reset fish model matrix
        greyFish = glm::translate(greyFish, glm::vec3(newX, 0.0f, newZ)); // Update position
        greyFish = glm::rotate(greyFish, -angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate fish
        //greyFish = glm::rotate(greyFish, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // Rotate fish
        greyFish = glm::scale(greyFish, glm::vec3(0.1f, 0.1f, 0.1f)); // Scale fish
        shadowMappingShader.SetMat4("model", greyFish); // Pass updated model matrix to shader
        grayFishModel.Draw(shadowMappingShader); // Draw fish object

        glm::mat4 bubbleModelMatrix = glm::mat4(1.0f);

        updateBubblePosition();
        //move bubble to bubblePosition
        bubbleModelMatrix = glm::translate(bubbleModelMatrix, bubblePosition);
        //print coordinates of bubble
        //bubbleModelMatrix = glm::translate(bubbleModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        bubbleModelMatrix = glm::scale(bubbleModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
        shadowMappingShader.SetMat4("model", bubbleModelMatrix);

        bubbleModel.Draw(shadowMappingShader);


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