#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <stdio.h>
#include <math.h>

#include <glade/exception/GladeException.h>
#include <glade/debug/log.h>
#include <glade/debug/crashHandler.h>
#include <glade/render/GladeRenderer.h>
#include <glade/Context.h>
#include <glade/State.h>
#include <glade/util/DesktopFileManager.h>
#include <glade/util/ResourceManager.h>
#include <strug/ResourceManager.h>
#include <strug/controls/StrugController.h>
#include <strug/exception/StrugException.h>
#include <strug/states/MazeTest.h>
#include <strug/states/Minecraft.h>
#include <strug/states/CubeTest.h>
#include <strug/states/MarchingCubes.h>
#include <strug/states/Chunked.h>

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720

// FIXME global resource manager is shit. Make it a part of context (Game Context probably should extend Glade Context)
FileManager *file_manager = NULL;
Glade::ResourceManager *resource_manager;
Strug::ResourceManager *game_resource_manager;

Context *gameContext = NULL;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  static double initXpos = xpos, initYpos = ypos;

  VirtualController *controller = gameContext->getController();
  controller->pointerMove(xpos - initXpos, ypos - initYpos, 0, 0, 0);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    VirtualController *controller = gameContext->getController();
    controller->pointerDown(0, 0, 0, 0, 0);
  }
}

void processInput(GLFWwindow* window)
{
  Transform *camera = gameContext->getRenderer()->getCamera();

  if (!camera)
    return;

  float forward = 0.0, strafe = 0.0, fly = 0.0;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    forward = -0.1;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    forward = 0.1;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    strafe = -0.1;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    strafe = 0.1;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    fly = 0.1;
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    fly = -0.1;

  VirtualController *controller = gameContext->getController();

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    controller->buttonPress(1, 0);

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    controller->buttonPress(2, 0);

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    controller->buttonPress(3, 0);

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
    controller->buttonRelease(3, 0);

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    controller->buttonPress(4, 0);

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    controller->buttonRelease(4, 0);

  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    controller->buttonPress(5, 0);

  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
    controller->buttonRelease(5, 0);

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    controller->buttonPress(6, 0);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    controller->buttonPress(7, 0);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    controller->buttonPress(8, 0);

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    controller->buttonPress(9, 0);

  float zModifier = forward * cos(camera->rotation->y) + strafe * sin(camera->rotation->y);
  float xModifier = -forward * sin(camera->rotation->y) + strafe * cos(camera->rotation->y);
  float yModifier = forward * sin(camera->rotation->x) + fly;

  camera->position->z += zModifier;
  camera->position->y += yModifier;
  camera->position->x += xModifier;
}

int main()
{ 
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow *window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "GLFW OpenGL", NULL, NULL);
  glfwMakeContextCurrent(window);

  log("OpenGL version: %s", glGetString(GL_VERSION));

  glewInit();

  const char assetsDir[200] = "/Users/jebrax/sourcery/sources/strug/build/strug/assets";
  log("Assets directory: %s", assetsDir);
 
  // Set resource manager pointers
  file_manager = new DesktopFileManager(assetsDir);
  resource_manager = new Strug::ResourceManager(file_manager);
  game_resource_manager = (Strug::ResourceManager *) resource_manager;
 
  // Create game context and renderer
  Glade::Renderer renderer;
  renderer.onSurfaceCreated();
  renderer.onSurfaceChanged(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  gameContext = new Context(&renderer);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // glfwSetKeyCallback(window, key_callback);

  gameContext->requestStateChange(std::unique_ptr<State>(new Chunked()));
 
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    processInput(window);
    gameContext->processRequests();
    renderer.onDrawFrame();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  if (gameContext != NULL) {
    delete gameContext;
    gameContext = NULL;
  }
  
  if (resource_manager != NULL) {
    delete resource_manager;
    resource_manager = NULL;
    
    if (game_resource_manager != NULL) {
      game_resource_manager = NULL;
    }
  }
  
  if (file_manager != NULL) {
    delete file_manager;
    file_manager = NULL;
  }

  log("Program is finishing");
}

