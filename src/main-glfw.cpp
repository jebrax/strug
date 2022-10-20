#include <strug/ResourceManager.h>
#include <strug/controls/StrugController.h>
#include <strug/exception/StrugException.h>
#include <strug/states/world/WalkingTheWorld.h>
#include <strug/states/Chunked.h>
#include <strug/states/Craft.h>

#include <glade/exception/GladeException.h>
#include <glade/debug/log.h>
#include <glade/debug/crashHandler.h>
#include <glade/render/GladeRenderer.h>
#include <glade/Context.h>
#include <glade/State.h>
#include <glade/util/Path.h>
#include <glade/util/DesktopFileManager.h>
#include <glade/util/ResourceManager.h>
#include <glade/controls/VirtualController.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <sys/syslimits.h>
#include <mach-o/dyld.h>

#include <fstream>
#include <cmath>
#include <cstdlib>

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720

GLFWwindow *window = nullptr;

FileManager *file_manager = NULL;
Glade::ResourceManager *resource_manager;
Strug::ResourceManager *game_resource_manager;

Context *gameContext = NULL;

namespace Glade {
  namespace System {
    void getViewportSize(unsigned int *width, unsigned int *height) {
      *width = VIEWPORT_WIDTH;
      *height = VIEWPORT_HEIGHT;
    }

    void toggleMouseCursor(bool enable) {
      glfwSetInputMode(window, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    void setMouseCursorPosition(double x, double y) {
      glfwSetCursorPos(window, x, y);
    }
  }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  VirtualController *controller = gameContext->getController();
  controller->pointerMove(xpos, ypos, 0, 0, 0);
}

static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  VirtualController *controller = gameContext->getController();
  controller->pointerMove(xoffset, yoffset, 0, 1, 0, false);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  int gladeControlId;

  if (button == GLFW_MOUSE_BUTTON_LEFT)
    gladeControlId = 0;
  if (button == GLFW_MOUSE_BUTTON_RIGHT)
    gladeControlId = 1;
  if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    gladeControlId = 2;
  }

  VirtualController *controller = gameContext->getController();

  double x, y;
  glfwGetCursorPos(window, &x, &y);

  if (action == GLFW_PRESS)
    controller->pointerDown(x, y, 0, gladeControlId, 0);
  if (action == GLFW_RELEASE)
    controller->pointerUp(x, y, 0, gladeControlId, 0);
}

static Path determineAssetsDirectory()
{
  char* executablePath = nullptr;
  uint32_t size = 0;

  assert(_NSGetExecutablePath(executablePath, &size) < 0);
  executablePath = (char*) malloc(size);
  assert(_NSGetExecutablePath(executablePath, &size) == 0);

  Path assetsDir(executablePath);
  assetsDir = assetsDir.base();
  assetsDir.append("assets");

  log("Assets directory: %s", assetsDir.cString());
 
  return assetsDir;
}

void processInput(GLFWwindow* window)
{
  VirtualController *controller = gameContext->getController();

  if (controller == nullptr)
    return;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_W, 0);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_W, 0);

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_S, 0);

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_S, 0);

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_A, 0);

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_A, 0);

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_D, 0);

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_D, 0);

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_SPACE, 0);

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_SPACE, 0);

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_X, 0);

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_X, 0);

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_G, 0);

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_G, 0);

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_F, 0);

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_F, 0);

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_T, 0);

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_T, 0);

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_R, 0);

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_R, 0);

  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_C, 0);

  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_C, 0);

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_LEFT, 0);

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_LEFT, 0);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_RIGHT, 0);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_RIGHT, 0);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_UP, 0);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_UP, 0);

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_DOWN, 0);

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_DOWN, 0);

  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_O, 0);

  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_O, 0);

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_L, 0);

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_L, 0);

  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_1, 0);

  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_1, 0);

  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_2, 0);

  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_2, 0);

  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_3, 0);

  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_3, 0);

  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_4, 0);

  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_4, 0);

  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    controller->buttonPress(Glade::Key::GLADE_KEY_5, 0);

  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
    controller->buttonRelease(Glade::Key::GLADE_KEY_5, 0);
}

int main()
{ 
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "GLFW OpenGL", NULL, NULL);
  glfwMakeContextCurrent(window);

  log("OpenGL version: %s", glGetString(GL_VERSION));

  glewInit();

  Path assetsDir = determineAssetsDirectory();

  // Set resource manager pointers
  file_manager = new DesktopFileManager(assetsDir);
  resource_manager = new Strug::ResourceManager(file_manager);
  game_resource_manager = (Strug::ResourceManager *) resource_manager;
 
  // Create game context and renderer
  Glade::Renderer renderer;
  renderer.onSurfaceCreated();
  renderer.onSurfaceChanged(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  gameContext = new Context(&renderer);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, mouse_scroll_callback);

  // glfwSetKeyCallback(window, key_callback);

  gameContext->requestStateChange(new WalkingTheWorld());
 
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

