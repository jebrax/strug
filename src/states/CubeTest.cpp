#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <strug/states/CubeTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Cube.h>

#include <unordered_map>

static Cube* cube;

CubeTest::CubeTest():
  State(),
  rotateIsDown(false)
{}

CubeTest::~CubeTest()
{
}

void CubeTest::createEntities()
{
  cube = new Cube();
  cube->initialize();
  this->context->add(cube);

  cube->alterTop(0.5, 1.0, 1.0, 1.0);
  this->context->add(cube);
}

void CubeTest::init(Context &context)
{
  log("Init CubeTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  context.setController(*this);
  pointerMove(0,0,0,0,0);
}

bool CubeTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  static float phi = 0.0, theta = 0.0;
  static float r = 4.0f;

  if (controlId == 0 || controlId == 1) {
    float x, y, z;

    if (controlId == 0 && rotateIsDown) {
      phi = yPos * 0.001;
      theta = xPos * 0.001;
    }

    if (controlId == 1) {
      r += yPos * 0.1;
    }

    y = cos(phi);
    x = cos(theta) * sin(phi);
    z = sin(theta) * sin(phi);
    x *= r; y *= r; z *= r;

    context->getRenderer()->getCamera()->position->x = x;
    context->getRenderer()->getCamera()->position->y = y;
    context->getRenderer()->getCamera()->position->z = z;

    context->getRenderer()->getCamera()->rotation->x = phi + PI / 2.0;
    context->getRenderer()->getCamera()->rotation->y = theta + PI / 2.0;
   }

  return true;
}

bool CubeTest::buttonPress(int controlId, int terminalId)
{
  return true;
}

bool CubeTest::buttonRelease(int controlId, int terminalId)
{
  return true;
}

bool CubeTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  if (controlId == 2) {
    rotateIsDown = true;
  }

  return true;
}

bool CubeTest::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  if (controlId == 2) {
    rotateIsDown = false;
  }

  return true;
}

void CubeTest::applyRules(Context &context)
{
}

void CubeTest::shutdown(Context &context)
{

}

