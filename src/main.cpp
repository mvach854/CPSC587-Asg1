/**
 * Author:	Andrew Robert Owens
 * Email:	arowens [at] ucalgary.ca
 * Date:	January, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Copyright (c) 2017 - Please give credit to the author.
 *
 * File:	main.cpp
 *
 * Summary:
 *
 * This is a (very) basic program to
 * 1) load shaders from external files, and make a shader program
 * 2) make Vertex Array Object and Vertex Buffer Object for the quad
 *
 * take a look at the following sites for further readings:
 * opengl-tutorial.org -> The first triangle (New OpenGL, great start)
 * antongerdelan.net -> shaders pipeline explained
 * ogldev.atspace.co.uk -> good resource
 */

#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "ShaderTools.h"
#include "Vec3f.h"
#include "Vec3f_FileIO.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"
#include "Camera.h"
#include "ParametricCurve.h"

//==================== GLOBAL VARIABLES ====================//
/*	Put here for simplicity. Feel free to restructure into
*	appropriate classes or abstractions.
*/

// Drawing Program
GLuint basicProgramID;

// Data needed for Box
GLuint vaoID;
GLuint vertBufferID;
Mat4f M;

// Data needed for Track
GLuint line_vaoID;
GLuint line_vertBufferID;
Mat4f line_M;

// Data for plane
GLuint plane_vaoID;
GLuint plane_vertBufferID;
Mat4f plane_M;

// Only one camera so only one view and perspective matrix are needed.
Mat4f V;
Mat4f P;

// Only one thing is rendered at a time, so only need one MVP
// When drawing different objects, update M and MVP = M * V * P
Mat4f MVP;

// Camera and viewing Stuff
Camera camera;
int g_moveUpDown = 0;
int g_moveLeftRight = 0;
int g_moveBackForward = 0;
int g_rotateLeftRight = 0;
int g_rotateUpDown = 0;
int g_rotateRoll = 0;
float g_rotationSpeed = 0.015625;
float g_panningSpeed = 0.25;
bool g_cursorLocked;
float g_cursorX, g_cursorY;

bool g_play = false;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;
int FB_WIDTH = 800, FB_HEIGHT = 600;
float WIN_FOV = 60;
float WIN_NEAR = 0.01;
float WIN_FAR = 1000;

// Object for
ParametricCurve curve;

// Number of parametric points
int numPointsParametric;

// Control points container
VectorContainerVec3f controlPoints;

//==================== FUNCTION DECLARATIONS ====================//
void displayFunc();
void resizeFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void loadTrackToGPU();
void loadQuadGeometryToGPU();
void loadPlaneGeometryToGPU();
void reloadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();

void windowSetSizeFunc();
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowSetSizeFunc(GLFWwindow *window, int width, int height);
void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height);
void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void animateQuad(float t);
void moveCamera();
void reloadMVPUniform();
void reloadColorUniform(float r, float g, float b);
std::string GL_ERROR();
int main(int, char **);

//==================== FUNCTION DEFINITIONS ====================//

void displayFunc() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use our shader
  glUseProgram(basicProgramID);

  // ===== DRAW QUAD ====== //
  MVP = P * V * M;
  reloadMVPUniform();
  reloadColorUniform(1, 0, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(vaoID);
  // Draw Quads, start at vertex 0, draw 20 of them (for a quad with 5 sides)
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 20);

  // ==== DRAW LINE ===== //
  MVP = P * V * line_M;
  reloadMVPUniform();
  reloadColorUniform(0, 1, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(line_vaoID);
  // Draw lines
  glDrawArrays(GL_LINE_LOOP, 0, numPointsParametric); // Number of defined vertices for track

  // ===== DRAW PLANE ====== //
  MVP = P * V * plane_M;
  reloadMVPUniform();
  reloadColorUniform(0, 1, 0);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(plane_vaoID);
  // Draw Quads, start at vertex 0, draw 20 of them (for a quad with 5 sides)
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void animateQuad(Vec3f position) {
  M = IdentityMatrix();

//  M = RotateAboutYMatrix(100 * position.y());

//  float s = (std::sin(t) + 1.f) / 2.f;
//  float x = (1 - s) * (10) + s * (-10);

//  M = TranslateMatrix(x, 0, 0) * M;
  M = TranslateMatrix(position.x(), position.y(), position.z()) * M;

  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void loadQuadGeometryToGPU() {
  // Just basic layout of floats, for a quad
  // 3 floats per vertex, 4 vertices
  std::vector<Vec3f> verts;
  verts.push_back(Vec3f(-1, -1, 0));
  verts.push_back(Vec3f(-1, 1, 0));
  verts.push_back(Vec3f(1, -1, 0));
  verts.push_back(Vec3f(1, 1, 0));

  verts.push_back(Vec3f(1, 1, 0));
  verts.push_back(Vec3f(1, 0, 0));
  verts.push_back(Vec3f(1, 1, 1));
  verts.push_back(Vec3f(1, 0, 1));

  verts.push_back(Vec3f(-1, 1, 0));
  verts.push_back(Vec3f(-1, 1, 1));
  verts.push_back(Vec3f(1, -1, 0));
  verts.push_back(Vec3f(1, -1, 1));

  verts.push_back(Vec3f(-1, -1, 1));
  verts.push_back(Vec3f(-1, 1, 1));
  verts.push_back(Vec3f(1, 1, 1));
  verts.push_back(Vec3f(1, -1, 1));

  verts.push_back(Vec3f(1, -1, 0));
  verts.push_back(Vec3f(1, -1, 1));
  verts.push_back(Vec3f(1, -1, 0));
  verts.push_back(Vec3f(1, -1, 1));

  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vec3f) * 20, // byte size of Vec3f, 20 of them
               verts.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

// Edited by: Manorie Vachon
void loadTrackToGPU() {
  curve.setCurve(controlPoints);

  std::vector<Vec3f> curvePoints;
  float deltaU = 0.0001;
  for (float u = 0.f; u <= 1.f; u+= deltaU) {
    curvePoints.push_back(curve.getCurvePoint(u));
  }

  numPointsParametric = curvePoints.size();

  glBindBuffer(GL_ARRAY_BUFFER, line_vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vec3f) * numPointsParametric, // byte size of Vec3f * number of points
               curvePoints.data(),      // pointer (Vec3f*) to contents of curvePoints
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void loadPlaneGeometryToGPU() {
  std::vector<Vec3f> verts;
  verts.push_back(Vec3f(30, -1, 10));
  verts.push_back(Vec3f(30, -10, -2));
  verts.push_back(Vec3f(-60, -1, 10));
  verts.push_back(Vec3f(-60, -10, -2));

  glBindBuffer(GL_ARRAY_BUFFER, plane_vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vec3f) * 4, // byte size of Vec3f, 20 of them
               verts.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void setupVAO() {
  glBindVertexArray(vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(line_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, line_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(0); // reset to default

  glBindVertexArray(plane_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, plane_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(0); // reset to default
}

void reloadProjectionMatrix() {
  // Perspective Only

  // field of view angle 60 degrees
  // window aspect ratio
  // near Z plane > 0
  // far Z plane

  P = PerspectiveProjection(WIN_FOV, // FOV
                            static_cast<float>(WIN_WIDTH) /
                                WIN_HEIGHT, // Aspect
                            WIN_NEAR,       // near plane
                            WIN_FAR);       // far plane depth
}

void loadModelViewMatrix() {
  M = IdentityMatrix();
  line_M = IdentityMatrix();
  plane_M = IdentityMatrix();
  // view doesn't change, but if it did you would use this
  V = camera.lookatMatrix();
}

void reloadViewMatrix() { V = camera.lookatMatrix(); }

void setupModelViewProjectionTransform() {
  MVP = P * V * M; // transforms vertices from right to left (odd huh?)
}

void reloadMVPUniform() {
  GLint id = glGetUniformLocation(basicProgramID, "MVP");

  glUseProgram(basicProgramID);
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     MVP.data() // pointer to data in Mat4f
                     );
}

void reloadColorUniform(float r, float g, float b) {
  GLint id = glGetUniformLocation(basicProgramID, "inputColor");

  glUseProgram(basicProgramID);
  glUniform3f(id, // ID in basic_vs.glsl
              r, g, b);
}

void generateIDs() {
  // shader ID from OpenGL
  std::string vsSource = loadShaderStringfromFile("./shaders/basic_vs.glsl");
  std::string fsSource = loadShaderStringfromFile("./shaders/basic_fs.glsl");
  basicProgramID = CreateShaderProgram(vsSource, fsSource);

  // VAO and buffer IDs given from OpenGL
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vertBufferID);
  glGenVertexArrays(1, &line_vaoID);
  glGenBuffers(1, &line_vertBufferID);
  glGenVertexArrays(1, &plane_vaoID);
  glGenBuffers(1, &plane_vertBufferID);
}

void deleteIDs() {
  glDeleteProgram(basicProgramID);

  glDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &vertBufferID);
  glDeleteVertexArrays(1, &line_vaoID);
  glDeleteBuffers(1, &line_vertBufferID);
  glDeleteVertexArrays(1, &plane_vaoID);
  glDeleteBuffers(1, &plane_vertBufferID);
}

void init() {
  glEnable(GL_DEPTH_TEST);
  glPointSize(50);

  camera = Camera(Vec3f{0, 0, 40}, Vec3f{0, 0, -1}, Vec3f{0, 1, 0});

  // SETUP SHADERS, BUFFERS, VAOs

  generateIDs();
  setupVAO();
  loadQuadGeometryToGPU();
  loadTrackToGPU();
  loadPlaneGeometryToGPU();

  loadModelViewMatrix();
  reloadProjectionMatrix();
//  M = IdentityMatrix();
  M = TranslateMatrix(2.f, 2.f, 0.f) * M;
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

int main(int argc, char **argv) {
  GLFWwindow *window;

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window =
      glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "CPSC 587 Assignment 1", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSetSizeFunc);
  glfwSetFramebufferSizeCallback(window, windowSetFramebufferSizeFunc);
  glfwSetKeyCallback(window, windowKeyFunc);
  glfwSetCursorPosCallback(window, windowMouseMotionFunc);
  glfwSetMouseButtonCallback(window, windowMouseButtonFunc);

  glfwGetFramebufferSize(window, &WIN_WIDTH, &WIN_HEIGHT);

  // initialize glad
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialise GLAD" << std::endl;
    return -1;
  }

  std::cout << "GL Version: :" << glGetString(GL_VERSION) << std::endl;
  std::cout << GL_ERROR() << std::endl;

  // Read in points from file
  std::string file( "track2.txt" );

	loadVec3fFromFile(controlPoints, file);

	for( auto & v : controlPoints )
	{
		std::cout << v << std::endl;
	}

  // ialize all the geometry, and load it once to the GPU
  init(); // our own initialize stuff func

  float s = 0.f; // this deltaS is the distance we want to travel along the curve
  float deltaT = 0.05f; // change in time
  float v = 0.f; // velocity, as determined by physics
  Vec3f currentPos = curve.getPosition(s);
  Vec3f tangent;
  Vec3f acc;
  Vec3f binormal;

  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window)) {

    if (g_play) {
      tangent = curve.tangent(s);
      v = curve.getVelocity(s);
      acc = ((v*v) * curve.curvature(s)) + Vec3f(0.f, 9.81f, 0.f) +
            (curve.tanAcc(s, deltaT) * curve.tangent(s));
      binormal = tangent.crossProduct(acc);
      s += v * deltaT;

      s = curve.wrap(s);

      currentPos = curve.getPosition(s);

      animateQuad(currentPos);
    }

    displayFunc();
    moveCamera();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up after loop
  deleteIDs();

  return 0;
}

//==================== CALLBACK FUNCTIONS ====================//

void windowSetSizeFunc(GLFWwindow *window, int width, int height) {
  WIN_WIDTH = width;
  WIN_HEIGHT = height;

  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height) {
  FB_WIDTH = width;
  FB_HEIGHT = height;

  glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}

void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      g_cursorLocked = GL_TRUE;
    } else {
      g_cursorLocked = GL_FALSE;
    }
  }
}

void windowMouseMotionFunc(GLFWwindow *window, double x, double y) {
  if (g_cursorLocked) {
    float deltaX = (x - g_cursorX) * 0.01;
    float deltaY = (y - g_cursorY) * 0.01;
    camera.rotateAroundFocus(deltaX, deltaY);

    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }

  g_cursorX = x;
  g_cursorY = y;
}

void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  bool set = action != GLFW_RELEASE && GLFW_REPEAT;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_W:
    g_moveBackForward = set ? 1 : 0;
    break;
  case GLFW_KEY_S:
    g_moveBackForward = set ? -1 : 0;
    break;
  case GLFW_KEY_A:
    g_moveLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_D:
    g_moveLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_Q:
    g_moveUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_E:
    g_moveUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_UP:
    g_rotateUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_DOWN:
    g_rotateUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_LEFT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? -1 : 0;
    else
      g_rotateLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_RIGHT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? 1 : 0;
    else
      g_rotateLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_SPACE:
    g_play = set ? !g_play : g_play;
    break;
  case GLFW_KEY_LEFT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 0.5;
    } else {
      g_panningSpeed *= 0.5;
    }
    break;
  case GLFW_KEY_RIGHT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 1.5;
    } else {
      g_panningSpeed *= 1.5;
    }
    break;
  default:
    break;
  }
}

//==================== OPENGL HELPER FUNCTIONS ====================//

void moveCamera() {
  Vec3f dir;

  if (g_moveBackForward) {
    dir += Vec3f(0, 0, g_moveBackForward * g_panningSpeed);
  }
  if (g_moveLeftRight) {
    dir += Vec3f(g_moveLeftRight * g_panningSpeed, 0, 0);
  }
  if (g_moveUpDown) {
    dir += Vec3f(0, g_moveUpDown * g_panningSpeed, 0);
  }

  if (g_rotateUpDown) {
    camera.rotateUpDown(g_rotateUpDown * g_rotationSpeed);
  }
  if (g_rotateLeftRight) {
    camera.rotateLeftRight(g_rotateLeftRight * g_rotationSpeed);
  }
  if (g_rotateRoll) {
    camera.rotateRoll(g_rotateRoll * g_rotationSpeed);
  }

  if (g_moveUpDown || g_moveLeftRight || g_moveBackForward ||
      g_rotateLeftRight || g_rotateUpDown || g_rotateRoll) {
    camera.move(dir);
    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }
}

std::string GL_ERROR() {
  GLenum code = glGetError();

  switch (code) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  default:
    return "Non Valid Error Code";
  }
}
