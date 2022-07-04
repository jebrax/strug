#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/CubeTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Cube.h>

#include <unordered_map>

static Cube* cube;

CubeTest::CubeTest():
  State(),
  terrain(nullptr),
  digPressed(false),
  clearPressed(false),
  createPressed(false)
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
  //context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
}

void CubeTest::dig()
{
}

bool CubeTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId)
{
  //terrain->getTransform()->rotation->y = xPos * 0.001;
  //terrain->getTransform()->rotation->x = yPos * 0.001;
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool CubeTest::buttonPress(int controlId, int terminalId)
{
  return true;
}

bool CubeTest::buttonRelease(int controlId, int terminalId)
{
  if (controlId == 3) {
    digPressed = false;
  }

  if (controlId == 4) {
    clearPressed = false;
  }

  if (controlId == 5) {
    createPressed = false;
  }

  return true;
}

bool CubeTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void CubeTest::applyRules(Context &context)
{
}

void CubeTest::shutdown(Context &context)
{
  delete terrain;
}

