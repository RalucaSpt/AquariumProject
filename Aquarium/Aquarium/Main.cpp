#include "Texture.h"

#include <stdlib.h>
#include <GL/glew.h>
#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>a
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glfw3.h>
#include "Camera.h"
#include <iostream>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION

#include <codecvt>

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include <Windows.h>
#include <locale>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool rotateLight = true;
bool isTransparent = false;


GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;

void Cleanup()
{
    delete pCamera;
}


// Define ROI parameters
glm::vec3 fishPosition;
float roiRadius = 5.0f; // Adjust as needed

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
void SwitchToFishPerspective(Camera* camera, const glm::vec3& fishPosition) {
    // Set camera position and orientation to match that of the fish
    camera->SetPosition(fishPosition);
    // Additional adjustments to camera orientation if needed
}

// Check if camera is within ROI
bool IsCameraWithinROI(Camera* camera, const glm::vec3& fishPosition, float roiRadius) {
    // Calculate distance between camera position and fish position
    glm::vec3 cameraPosition = camera->GetPosition();
    float distance = glm::distance(cameraPosition, fishPosition);

    // Check if distance is within ROI radius
    return distance < roiRadius;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

int keyPress = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {

    }
    // Check if F key is pressed to switch camera perspective
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        // Check if camera is within ROI of the fish
        if (IsCameraWithinROI(pCamera, fishPosition, roiRadius)) {
            // Switch camera perspective to that of the fish
            if (keyPress %2 == 0) {
                SwitchToFishPerspective(pCamera, fishPosition);
            }
			else {
				// Return to initial camera position
				pCamera->SetPosition(glm::vec3(0.0, 1.0, 3.0));
			}
            isTransparent = !isTransparent;
            keyPress++;
        }
    }
}

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab8 - Maparea umbrelor", NULL, NULL);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    std::string objFileName = (currentPath + "\\Models\\CylinderProject.obj");
    Model objModel(objFileName, false);

    std::string piratObjFileName = (currentPath + "\\Models\\Pirat\\Pirat.obj");
    Model piratObjModel(piratObjFileName, false);

    std::string fishObjFileName = (currentPath + "\\Models\\Fish\\fish.obj");
    Model fishObjModel(fishObjFileName, false);

    // load textures
    // -------------
    Texture txtr;

    unsigned int floorTexture = txtr.CreateTexture(strExePath + "\\Glass.png",
        		0.05f);
    Texture txtr2(strExePath + "\\Fish.jpg", 1.0f);
    //unsigned int fishTexture = txtr.CreateTexture(currentPath + "\\Models\\Fish\\fish.jpg", 1.0f);
    unsigned int fishTexture = txtr2.GetTextureID();

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

    glEnable(GL_CULL_FACE);


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
        scene.renderScene(shadowMappingDepthShader,fishObjModel);
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
        scene.renderScene(shadowMappingShader, fishObjModel);

        


        glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
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

        //if the isTransparent variable is true, the fish will be transparent
        if (isTransparent) {
			shadowMappingShader.SetVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
            //fishObjModel.Draw(shadowMappingShader);

		}
		else {
			shadowMappingShader.SetVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            fishObjModel.Draw(shadowMappingShader);
            if (IsCameraWithinROI(pCamera, fishPosition, roiRadius)) {
                // Draw colored border around fish object
                DrawColoredBorder(fishPosition, fishObjModel, fishModel, isTransparent);
            }
		}



        
        // Check if camera is within ROI of the fish
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
        pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
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
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
    pCamera->ProcessMouseScroll((float)yOffset);
}

