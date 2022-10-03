#include <strug/states/WalkingTheWorld.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Isosurface.h>


#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <glade/generation/Grid.h>
#include <glade/system.h>

#include <unordered_map>

static Sphere *sphere= nullptr;
static Isosurface *terrain = nullptr;
static Grid* grid = nullptr;

static const float cellSize = 0.25;

static bool
  upPressed = false,
  downPressed = false,
  leftPressed = false,
  rightPressed = false,
  oPressed = false,
  lPressed = false;

WalkingTheWorld::WalkingTheWorld():
  State()
{}

WalkingTheWorld::~WalkingTheWorld()
{
}

void WalkingTheWorld::createEntities()
{
  sphere = new Sphere();
  sphere->initialize(cellSize);
  sphere->getTransform()->position->x = grid->chunkSizeCells/2 * grid->cellSize - 3;
  sphere->getTransform()->position->y = 20 * grid->cellSize;
  sphere->getTransform()->position->z = grid->chunkSizeCells/2 * grid->cellSize;
  context->add(sphere);

  // The size is 1 for now
  for (int i = 0; i < grid->getGridSizeChunks(); ++i) {
    for (int j = 0; j < grid->getGridSizeChunks(); ++j) {
      Glade::Vector2i chunkIndex(i, j);
      terrain = new Isosurface();
      grid->addChunk(i, j, terrain);

      terrain->initialize(chunkIndex, *grid, false);
      terrain->view->getMesh()->neverErase = true;
      context->add(terrain);
    }
  }
}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize, 1);

  createEntities();

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  context.renderer->getCamera()->position->z = 10.0;
  context.renderer->getCamera()->position->x = 10.0;
  context.renderer->getCamera()->position->y = 5.0;

  Glade::System::toggleMouseCursor(false);

  context.setController(*this);
}

void WalkingTheWorld::applyRules(Context &context)
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

  if (oPressed)
    sphere->getTransform()->position->y += sphereSpeed;

  if (lPressed)
    sphere->getTransform()->position->y -= sphereSpeed;
}

bool WalkingTheWorld::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool WalkingTheWorld::buttonPress(int controlId, int terminalId)
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
    case 10: oPressed = true;
       break;
    case 11: lPressed = true;
       break;
  }

  return true;
}

bool WalkingTheWorld::buttonRelease(int controlId, int terminalId)
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
    case 10: oPressed = false;
       break;
    case 11: lPressed = false;
       break;
  }

  return true;
}

bool WalkingTheWorld::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

bool WalkingTheWorld::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void WalkingTheWorld::shutdown(Context &context)
{

}

