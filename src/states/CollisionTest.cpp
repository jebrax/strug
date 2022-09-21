#include <glade/system.h>
#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <strug/states/CollisionTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Isosurface.h>
#include <glade/generation/Grid.h>

#include <unordered_map>

static Sphere *sphere= nullptr;
static Isosurface *terrain = nullptr;
static Grid* grid = nullptr;

static const float cellSize = 0.25;

static bool
  upPressed = false,
  downPressed = false,
  leftPressed = false,
  rightPressed = false;

CollisionTest::CollisionTest():
  State()
{}

CollisionTest::~CollisionTest()
{
}

void CollisionTest::createEntities()
{
  sphere = new Sphere();
  sphere->initialize(cellSize);
  this->context->add(sphere);

  Glade::Vector2i chunkIndex(0, 0);
  terrain = new Isosurface();
  terrain->initialize(chunkIndex, *grid);
  terrain->view->getMesh()->neverErase = true;
  context->add(terrain);
}

void CollisionTest::init(Context &context)
{
  log("Init CollisionTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize);

  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 5.0;
  context.renderer->getCamera()->position->z = 10.0;
  context.renderer->getCamera()->position->x = 10.0;
  context.renderer->getCamera()->position->y = 5.0;

  Glade::System::toggleMouseCursor(false);

  context.setController(*this);
}

void CollisionTest::applyRules(Context &context)
{
  float sphereSpeed = 0.06;
  if (leftPressed)
    sphere->getTransform()->position->x -= sphereSpeed;

  if (rightPressed)
    sphere->getTransform()->position->x += sphereSpeed;

  if (upPressed)
    sphere->getTransform()->position->z -= sphereSpeed;

  if (downPressed)
    sphere->getTransform()->position->z += sphereSpeed;
}

bool CollisionTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool CollisionTest::buttonPress(int controlId, int terminalId)
{
  switch (controlId) {
    case 6: leftPressed = true;
       break;
    case 7: rightPressed = true;
       break;
    case 8: upPressed = true;
       break;
    case 9: downPressed = true;
       break;
  }

  return true;
}

bool CollisionTest::buttonRelease(int controlId, int terminalId)
{
  switch (controlId) {
    case 6: leftPressed = false;
       break;
    case 7: rightPressed = false;
       break;
    case 8: upPressed = false;
       break;
    case 9: downPressed = false;
       break;
  }

  return true;
}

bool CollisionTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

bool CollisionTest::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void CollisionTest::shutdown(Context &context)
{

}

