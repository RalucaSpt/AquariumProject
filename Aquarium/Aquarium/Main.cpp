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
#include "stb_image.h"

#include <codecvt>

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <locale>
#include <map>
#include <math.h> 

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
void DrawColoredBorder(const glm::vec3& fishPosition, Model& fishObjModel, const glm::mat4& fishModel, bool transparent) {
    // Render colored border around fish object
    // Example: Render wireframe or outline mesh around the fish object with a specific color

    // Set the color for the border (e.g., red)
    const glm::vec3 borderColor = glm::vec3(1.0f, 0.0f, 0.0f); // Red color

    // Calculate the model matrix for the border (use the same transformations as the fish object)
    glm::mat4 borderModel = fishModel;

    // Use the border shader
    Shader borderShader("border.vs", "border.fs");
    borderShader.Use();

    // Set the model, view, and projection matrices in the border shader
    borderShader.SetMat4("model", borderModel);
    borderShader.SetMat4("view", pCamera->GetViewMatrix());
    borderShader.SetMat4("projection", pCamera->GetProjectionMatrix());

    // Set the color of the border in the shader
    borderShader.SetVec3("borderColor", borderColor);

    // Set the transparency
    if (transparent) {
        // Set alpha to 0 for full transparency
        borderShader.SetVec4("borderColor", glm::vec4(borderColor, 0.0f));
    }

    // Set the polygon mode to draw only the border lines
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw the border around the fish object
    fishObjModel.DrawBorder(borderShader);

    // Restore the polygon mode to fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Function to switch camera perspective to that of the fish
void SwitchToFishPerspective(Camera* camera, const glm::vec3& fishPosition)
{
    // Set camera position to be behind and above the fish
    glm::vec3 offset = glm::vec3(0.0f, 1.5f, -3.0f);
    camera->SetPosition(fishPosition + offset);

    // Calculate direction vector from camera position to fish position
    glm::vec3 direction = glm::normalize(fishPosition - camera->GetPosition());

    // Calculate yaw and pitch angles from direction vector
    float yaw = glm::degrees(atan2(direction.x, direction.z)) + 90;
    float pitch = glm::degrees(asin(direction.y));

    // Set camera orientation to look at the fish
    camera->SetYaw(yaw);
    camera->SetPitch(pitch);

    // Additional adjustments to camera orientation if needed
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
    unsigned int greyFishTexture = txtr.CreateTexture(currentPath + "\\Models\\GreyFish\\fish.png", 1.0f);
    unsigned int coralTexture = txtr.CreateTexture(strExePath + "\\blue_coral.png", 1.0f);

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

        /*glm::mat4 piratModel = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
        shadowMappingDepthShader.setMat4("model", piratModel);
        piratObjModel.Draw(shadowMappingDepthShader);*/



        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind your texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture); // Assuming floorTexture is the ID of your texture



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


        glm::mat4 greyFish = glm::mat4(1.0f);
        greyFish = glm::translate(greyFish, glm::vec3(-1.0f, 0.0f, -1.0f));
        greyFish = glm::rotate(greyFish, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        greyFish = glm::scale(greyFish, glm::vec3(0.1f, 0.1f, 0.1f));
        shadowMappingShader.SetMat4("model", greyFish);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, greyFishTexture);
        grayFishModel.Draw(shadowMappingShader);


        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, coralBeautyTexture);
        shadowMappingShader.SetInt("coralBeauty", 4);

        // Configurare model matrix pentru Coral Beauty
        glm::mat4 coralBeautyModelMatrix = glm::mat4(1.0f);
        coralBeautyModelMatrix = glm::translate(coralBeautyModelMatrix, glm::vec3(2.0f, 0.0f, -1.0f));
        coralBeautyModelMatrix = glm::scale(coralBeautyModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
        // Desenează modelul Coral Beauty
        shadowMappingShader.SetMat4("model", coralBeautyModelMatrix);
        coralBeautyModel.Draw(shadowMappingShader);

        glActiveTexture(GL_TEXTURE0); // Folosește o altă unitate de textură
        glBindTexture(GL_TEXTURE_2D, fishTexture);
        glm::mat4 fishModel = glm::mat4(1.0f); // Identity matrix
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

        if (isTransparent) {
            shadowMappingShader.SetVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
            fishObjModel.Draw(shadowMappingShader);
            //move the fish along with the camera

        }
        else {
            shadowMappingShader.SetVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            //make fish orientation math auxiliary model

            if (isInFishPerspective)
            {
                float centerX = SCR_WIDTH / 2.0f;
                float centerY = SCR_HEIGHT / 2.0f;

                // Calculate offset from the center of the screen
                float offsetX = mouseX - centerX;
                float offsetY = mouseY - centerY;

                // Sensitivity factor for rotation
                float sensitivity = 0.1f;

                // Calculate rotation angles around x and y axes based on mouse offset
                float rotationAngleX = sensitivity * offsetY;
                float rotationAngleY = sensitivity * offsetX;

                // Optionally, you can calculate rotation angle around the z-axis based on mouse input or other factors
                float rotationAngleZ = sensitivity * (offsetX + offsetY);


                // Clamp rotation angles to avoid excessive rotation
                rotationAngleX = glm::clamp(rotationAngleX, -90.0f, 90.0f);
                rotationAngleY = glm::clamp(rotationAngleY, -90.0f, 90.0f);
                rotationAngleZ = glm::clamp(rotationAngleZ, -90.0f, 90.0f);
                glm::mat4 fishRotationMatrix = glm::mat4(1.0f);
                fishRotationMatrix = glm::rotate(fishRotationMatrix, glm::radians(rotationAngleX), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around x-axis
                fishRotationMatrix = glm::rotate(fishRotationMatrix, glm::radians(rotationAngleY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around y-axis
                fishRotationMatrix = glm::rotate(fishRotationMatrix, glm::radians(rotationAngleZ), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis if needed

                fishModel = fishRotationMatrix * fishModel; // Apply rotation to the fish model
                shadowMappingShader.SetMat4("model", fishModel);
                fishObjModel.Draw(shadowMappingShader);
            }

            else
            {
                fishObjModel.Draw(shadowMappingShader);
            }

            if (IsCameraWithinROI(pCamera, fishPosition, roiRadius)) {
                // Draw colored border around fish object
                DrawColoredBorder(fishPosition, fishObjModel, fishModel, isTransparent);
            }
        }


        transparentShader.Use();
        transparentShader.SetMat4("projection", projection);
        transparentShader.SetMat4("view", view);
        transparentShader.SetVec3("viewPos", pCamera->GetPosition());
        transparentShader.SetVec3("lightPos", lightPos);
        transparentShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        CubeObj cube;

        //make the cube object visible from all angles

        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePaths[0]);

        scene.renderCubes(const_cast<Shader&>(transparentShader), cube, texturePaths[0]);


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