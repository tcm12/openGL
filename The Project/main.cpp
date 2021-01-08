#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Windows.h> 
#pragma comment(lib, "Winmm.lib")

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat4 lightRotation;
glm::mat3 normalMatrix;
glm::mat3 lightDirMatrix;
typedef glm::mat4(*modelMatrix)();

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec4 pointLightSource;

int pointLightSourceLoc;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 10.0f, 3.0f),
    glm::vec3(0.0f, 10.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
    );

GLfloat cameraSpeed;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D stoneFloor;
gps::Model3D cat;
gps::Model3D moon;
gps::Model3D moonBuilding1;
gps::Model3D moonBuilding2;
gps::Model3D moonBuilding3;
gps::Model3D moonBuilding4;
gps::Model3D moonTower;
gps::Model3D saturn;
gps::Model3D rocket;
gps::Model3D wall;
gps::Model3D cube;
gps::Model3D fence;

GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
gps::Shader depthMapShader;
gps::Shader lightShader;

//camera movement
double yaw = -90.0f;
double pitch = 0.0f;
float fieldOfView = 45.0f;

//mouse state
bool firstMouse = true;
bool pause = false;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float mouseSensitivity = 0.1f;

//skybox stuff
std::vector<const GLchar*> faces;

//framebuffer
GLuint shadowMapFBO;
GLuint depthMapTexture;

//depthmap related
const GLfloat near_plane = 0.1f, far_plane = 100.0f;

//floor placement
float floorOffset = 20.0f;
float fLastX = 1.5f;
float fLastY = 0.5f;
float fLastZ = 0.5f;

float wallOffset = 4.0f;
float wLastX = 51.5f;
float wLastY = 2.5f;
float wLastZ = 0.0f;

float moonOrbit = 0.0f;

float catX = -4.5f;
float catY = 0.5f;
float catZ = -2.5f;
float catOffset = 0.2f;
float catAngle = 0.0f;

float rocketX = 2.0f;
float rocketY = 15.3f;
float rocketZ = 35.0f;
float rocketOffset = 0.01f;
bool launch = false;
bool playedSound = false;

float lightAngle = 0.0f;

float fenceOffset = 13.5f;
float feLastX = -46.0f;
float feLastY = 0.5f;
float feLastZ = 15.0f;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

    projection = glm::perspective(glm::radians(fieldOfView), (float)width / (float)height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void updatePerspective() {
    projection = glm::perspective(glm::radians(fieldOfView), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (pause) {
        return;
    }

    if (firstMouse) { //set xoffset and yoffset to 0 for the same effect
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (pause) {
        return;
    }

    if (fieldOfView >= 1.0f && fieldOfView <= 45.0f)
        fieldOfView -= yoffset;
    if (fieldOfView <= 1.0f)
        fieldOfView = 1.0f;
    if (fieldOfView >= 45.0f)
        fieldOfView = 45.0f;

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    updatePerspective();
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    lightDirMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirMatrix");

    pointLightSourceLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightSource");

    // create projection matrix
    projection = glm::perspective(glm::radians(fieldOfView),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //skybox
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(fieldOfView), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(50.0f, 4.0f, 50.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_T]) {
        myCamera.reset();
        firstMouse = true;
        yaw = -90.0f;
        pitch = 0.0f;
        lastX = 800.0f / 2.0;
        lastY = 600.0f / 2.0;
    }

    //U J K H are used to move the cat
    if (pressedKeys[GLFW_KEY_U]) {
        catAngle = 0.0f;
        if (catZ + catOffset < 14.0f) {
            catZ += catOffset;
        }
    }

    if (pressedKeys[GLFW_KEY_J]) {
        catAngle = 180.0f;
        if (catZ - catOffset > -19.0f) {
            catZ -= catOffset;
        }
    }

    if (pressedKeys[GLFW_KEY_H]) {
        catAngle = 90.0f;
        if (catX + catOffset < 50.5f) {
            catX += catOffset;
        }
    }

    if (pressedKeys[GLFW_KEY_K]) {
        catAngle = -90.0f;
        if (catX - catOffset > -47.5f) {
            catX -= catOffset;
        }
    }

    if (pressedKeys[GLFW_KEY_L]) {//launch rocket
        launch = true;
    }

    if (pressedKeys[GLFW_KEY_SPACE]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT])
        cameraSpeed = 50.0f * deltaTime;
    else
        cameraSpeed = 10.0f * deltaTime;

    if (pressedKeys[GLFW_KEY_Q]) {

        lightAngle += 1.0f;
        if (lightAngle > 360.0f)
            lightAngle -= 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        lightAngle -= 1.0f;
        if (lightAngle < 0.0f)
            lightAngle += 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
}

void processPause() {
    if (pressedKeys[GLFW_KEY_P] && pause == false) //Pause
    {
        pause = true;
    }

    if (pressedKeys[GLFW_KEY_O] && pause == true) //Unpause
    {
        pause = false;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scrollCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initializeSkyBoxFaces() {
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    skyboxShader.useShaderProgram();
}

void initModels() {
    moon.LoadModel("models/moon/10467_Cratered_Moon_v2_Iterations-2.obj");
    moonBuilding1.LoadModel("models/moon_building1/14008_Moon_Building_Storage_Module_v2_L1.obj");
    moonBuilding2.LoadModel("models/moon_building2/14006_Moon_Building_Science_Module_v2_L1.obj");
    moonBuilding3.LoadModel("models/moon_building3/14007_Moon_Building_Engineering_Module_v2_L1.obj");
    moonBuilding4.LoadModel("models/moon_building4/14004_Moon_Building_Barracks_v2_L1.obj");
    moonTower.LoadModel("models/moon_tower/14005_Moon_Building_Communication_Relay_Tower_v2_L1.obj");
    rocket.LoadModel("models/rocket/12217_rocket_v1_l1.obj");
    cat.LoadModel("models/cat/12221_Cat_v1_l3.obj");
    wall.LoadModel("models/wall/wall.obj");
    stoneFloor.LoadModel("models/floor/ground.obj");
    cube.LoadModel("models/cube/cube.obj");
    fence.LoadModel("models/fence/13078_Wooden_Post_and_Rail_Fence_v1_l3.obj");
    mySkyBox.Load(faces);
}

void initDepthMapTexture() {
    glGenFramebuffers(1, &shadowMapFBO);
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 lightSpaceTransforms() {

    glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void bindShadows(gps::Shader &shader)
{
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightSpaceTransforms()));
}

//Version 4: Working
glm::mat4 positionMainFloor() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.5f));
    return tempModel;
}

glm::mat4 positionMainLeftWall() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(51.5f, 2.5f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return tempModel;
}

glm::mat4 positionMainBackWall() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, -49.0f));
    tempModel = glm::rotate(tempModel, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return tempModel;
}

glm::mat4 positionMainFrontWall() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, 49.0f));
    tempModel = glm::rotate(tempModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return tempModel;
}

glm::mat4 positionMainRightWall() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(-48.5f, 2.5f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return tempModel;
}

glm::mat4 positionMainFence() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(-46.0f, 0.5f, 15.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.05f, 0.05f, 0.05f));
    return tempModel;
}

glm::mat4 positionOtherMainFence() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(-46.0f, 0.5f, -20.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.05f, 0.05f, 0.05f));
    return tempModel;
}

void resetMainFence() {
    feLastX = -46.0f;
    feLastY = 0.5f;
    feLastZ = 15.0f;
}

void resetOtherMainFence() {
    feLastX = -46.0f;
    feLastY = 0.5f;
    feLastZ = -20.0f;
}

void resetLeftWall() {
    wLastX = 51.5f;
    wLastY = 2.5f;
    wLastZ = 0.0f;
}

void resetBackWall() {
    wLastX = 0.0f;
    wLastY = 2.5f;
    wLastZ = -49.0f;
}

void resetFrontWall() {
    wLastX = 0.0f;
    wLastY = 2.5f;
    wLastZ = 49.0f;
}

void resetRightWall() {
    wLastX = -48.5f;
    wLastY = 2.5f;
    wLastZ = 0.0f;
}

void resetLastFloor() {
    fLastX = 1.5f;
    fLastY = 0.5f;
    fLastZ = 0.5f;
}

glm::mat4 positionWalls(float xOffset, float zOffset, float rX, float rY, float rZ, float angle) {
    glm::mat4 tempModel = glm::mat4(1.0f);
    wLastX += xOffset;
    wLastZ += zOffset;
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(wLastX, wLastY, wLastZ));
    tempModel = glm::rotate(tempModel, glm::radians(angle), glm::vec3(rX, rY, rZ));
    return tempModel;
}

glm::mat4 positionFloors(float xOffset, float yOffset, float zOffset) {
    glm::mat4 tempModel = glm::mat4(1.0f);
    fLastX += xOffset;
    fLastY += yOffset;
    fLastZ += zOffset;
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(fLastX, fLastY, fLastZ));
    return tempModel;
}

glm::mat4 positionFences(float xOffset, float zOffset, float rX, float rY, float rZ, float angle) {
    glm::mat4 tempModel = glm::mat4(1.0f);
    feLastX += xOffset;
    feLastZ += zOffset;
    tempModel = glm::translate(glm::mat4(1.0f), glm::vec3(feLastX, feLastY, feLastZ));
    tempModel = glm::rotate(tempModel, glm::radians(angle), glm::vec3(rX, rY, rZ));
    tempModel = glm::scale(tempModel, glm::vec3(0.05f, 0.05f, 0.05f));
    return tempModel;
}

glm::mat4 positionCat() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(catX, catY, catZ));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(catAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.05f, 0.05f, 0.05f));
    return tempModel;
}

glm::mat4 positionMoon() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(25.0f, 650.0f, 15.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.000025f, 0.000025f, 0.000025f));
    tempModel = glm::rotate(tempModel, glm::radians(moonOrbit), glm::vec3(0.0f, 1.0f, 1.0f));
    moonOrbit += 0.1f;
    return tempModel;
}

glm::mat4 positionMoonB1() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(-4.0f, 0.5f, -35.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.025f, 0.025f, 0.025f));
    return tempModel;
}

glm::mat4 positionMoonB2() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(-30.0f, 0.5f, -30.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.03f, 0.03f, 0.03f));
    return tempModel;
}

glm::mat4 positionMoonB3() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(23.0f, 0.5f, -30.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.025f, 0.025f, 0.025f));
    return tempModel;
}

glm::mat4 positionMoonB3V2() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(43.0f, 0.5f, -29.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.025f, 0.025f, 0.025f));
    return tempModel;
}

glm::mat4 positionMoonB4() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(30.0f, 0.5f, 35.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.1f, 0.1f, 0.1f));
    return tempModel;
}

glm::mat4 positionMoonT() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(0.0f, 0.5f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3((0.025f, 0.025f, 0.025f)));
    return tempModel;
}

glm::mat4 positionCube() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    tempModel = glm::translate(tempModel, glm::vec3(0.0f, 30.0f, 0.0f));
    tempModel = glm::scale(tempModel, glm::vec3(0.5f, 0.5f, 0.5f));
    tempModel = glm::rotate(tempModel, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    return tempModel;
}

glm::mat4 positionRocket() {
    glm::mat4 tempModel = glm::mat4(1.0f);
    if (launch) {
        rocketY += rocketOffset;
        rocketOffset += 0.001f;
        if (!playedSound) {
            PlaySound(TEXT("rocket.wav"), NULL, SND_ASYNC);
            playedSound = true;
        }
    }
    tempModel = glm::translate(tempModel, glm::vec3(rocketX, rocketY, rocketZ));
    tempModel = glm::rotate(tempModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tempModel = glm::rotate(tempModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    tempModel = glm::scale(tempModel, glm::vec3((0.025f, 0.025f, 0.025f)));
    return tempModel;
}

void applyPointLight() {

    pointLightSource = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, 3.0f, -3.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    pointLightSource = view * pointLightSource;

    glUniform3fv(pointLightSourceLoc, 1, glm::value_ptr(pointLightSource));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void renderObject(gps::Shader &shader, gps::Model3D &obj3D, bool depthMapMode) {
    shader.useShaderProgram();

    if (!depthMapMode) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    else {
        glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(model));
    }

    obj3D.Draw(shader);
}

void genFloor(int n, int x, int z, gps::Shader& shader, bool depthMapMode) {
    resetLastFloor();

    for (int i = 0; i < n; i++) {
        model = positionFloors(floorOffset * x, 0.0f, floorOffset * z);
        renderObject(shader, stoneFloor, depthMapMode);
    }
}

void genWall(int n, int one, gps::Shader& shader, bool depthMapMode) {
    resetLeftWall();

    for (int i = 0; i < n; i++) {
        model = positionWalls(0.0f, wallOffset * one, 0.0f, 1.0f, 0.0f, -90.0f);
        renderObject(shader, wall, depthMapMode);
    }

    resetBackWall();

    for (int i = 0; i < n; i++) {
        model = positionWalls(wallOffset * one, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        renderObject(shader, wall, depthMapMode);
    }

    resetRightWall();

    for (int i = 0; i < n; i++) {
        model = positionWalls(0.0f, wallOffset * one, 0.0f, 1.0f, 0.0f, 90.0f);
        renderObject(shader, wall, depthMapMode);
    }

    resetFrontWall();

    for (int i = 0; i < n; i++) {
        model = positionWalls(wallOffset * one, 0.0f, 0.0f, 1.0f, 0.0f, 180.0f);
        renderObject(shader, wall, depthMapMode);
    }
}

void genFence(int n, int one, gps::Shader& shader, bool depthMapMode) {
    resetMainFence();

    for (int i = 0; i < n; i++) {
        model = positionFences(fenceOffset * one, 0.0f, 1.0f, 0.0f, 0.0f, -90.0f); //(x, z, rX, rY, rZ, angle) - rX = rotate around X
        renderObject(shader, fence, depthMapMode);
    }

    resetOtherMainFence();

    for (int i = 0; i < n; i++) {
        model = positionFences(fenceOffset * one, 0.0f, 1.0f, 0.0f, 0.0f, -90.0f); //(x, z, rX, rY, rZ, angle) - rX = rotate around X
        renderObject(shader, fence, depthMapMode);
    }
}

void createWall(gps::Shader& shader, bool depthMapMode) {
    genWall(13, 1, shader, depthMapMode);
    genWall(12, -1, shader, depthMapMode);
}

void createFence(gps::Shader& shader, bool depthMapMode) {
    genFence(7, 1, shader, depthMapMode);
}

void createGround(gps::Shader& shader, bool depthMapMode) {
    genFloor(2, 1, 0, shader, depthMapMode);
    genFloor(2, -1, 0, shader, depthMapMode);
    genFloor(2, 0, 1, shader, depthMapMode);
    genFloor(2, 0, -1, shader, depthMapMode);
    genFloor(2, -1, -1, shader, depthMapMode);
    genFloor(2, 1, 1, shader, depthMapMode);
    genFloor(2, -1, 1, shader, depthMapMode);
    genFloor(2, 1, -1, shader, depthMapMode);
    genFloor(1, 1, 2, shader, depthMapMode);
    genFloor(1, 2, 1, shader, depthMapMode);
    genFloor(1, -1, -2, shader, depthMapMode);
    genFloor(1, -2, -1, shader, depthMapMode);
    genFloor(1, -1, 2, shader, depthMapMode);
    genFloor(1, -2, 1, shader, depthMapMode);
    genFloor(1, 1, -2, shader, depthMapMode);
    genFloor(1, 2, -1, shader, depthMapMode);
}

void drawWorldObjects(gps::Shader shader, bool depthMapMode) {
    model = positionCat();
    renderObject(shader, cat, depthMapMode);

    model = positionMainFloor();
    renderObject(shader, stoneFloor, depthMapMode);

    model = positionMoon();
    renderObject(shader, moon, depthMapMode);

    model = positionMoonB1();
    renderObject(shader, moonBuilding1, depthMapMode);

    model = positionMoonB2();
    renderObject(shader, moonBuilding2, depthMapMode);

    model = positionMoonB3();
    renderObject(shader, moonBuilding3, depthMapMode);

    model = positionMoonB3V2();
    renderObject(shader, moonBuilding3, depthMapMode);

    model = positionMoonB4();
    renderObject(shader, moonBuilding4, depthMapMode);

    model = positionMoonT();
    renderObject(shader, moonTower, depthMapMode);

    model = positionRocket();
    renderObject(shader, rocket, depthMapMode);

    model = positionMainLeftWall();
    renderObject(shader, wall, depthMapMode);

    model = positionMainRightWall();
    renderObject(shader, wall, depthMapMode);

    model = positionMainBackWall();
    renderObject(shader, wall, depthMapMode);

    model = positionMainFrontWall();
    renderObject(shader, wall, depthMapMode);

    model = positionMainFence();
    renderObject(shader, fence, depthMapMode);

    model = positionOtherMainFence();
    renderObject(shader, fence, depthMapMode);

    createGround(shader, depthMapMode);
    createWall(shader, depthMapMode);
    createFence(shader, depthMapMode);
}

void drawLightCube(gps::Shader shader, bool depthMapMode) {
    shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    model = positionCube();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    cube.Draw(shader);
}

void renderDepthMap() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightSpaceTransforms()));
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawWorldObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {
    renderDepthMap();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    bindShadows(myBasicShader);

    applyPointLight();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightDirMatrix = glm::mat3(glm::inverseTranspose(view));

    glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    drawWorldObjects(myBasicShader, false);

    drawLightCube(lightShader, false);

    mySkyBox.Draw(skyboxShader, view, projection);
}

/*
void renderScene() {
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightSpaceTransforms()));
    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderWorldObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glViewport(0, 0, 1920, 1080);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
    glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightSpaceTransforms()));

    renderWorldObjects(myBasicShader, false);

    mySkyBox.Draw(skyboxShader, view, projection);//this needs to be the last
}
*/

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void calculateDeltaTime() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initializeSkyBoxFaces();
    initOpenGLState();

	initModels();

	initShaders();
	initUniforms();
    initDepthMapTexture();
    setWindowCallbacks();

    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        calculateDeltaTime();

        if (pause) {
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            processMovement();
        }

        processPause();
        renderScene();
    
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
        glCheckError();
	}

	cleanup();
    return EXIT_SUCCESS;
}