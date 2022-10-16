#include <algorithm>
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
static bool flyMode = false;
static bool needsToReleaseJumpButtonFirst = false;
static bool digging = false;
static bool growing = false;

static float gravityAcceleration = -0.08;
static float jumpAcceleration = 0.4;
static float verticalSpeed = 0.0;

static const float cellSize = 0.25;

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
  sphere->getTransform()->position->x = (float) grid->chunkSizeCells/2 * grid->cellSize;
  sphere->getTransform()->position->y = 20 * grid->cellSize;
  sphere->getTransform()->position->z = (float) grid->chunkSizeCells/2 * grid->cellSize;
  context->add(sphere);

  // The size is 1 for now
  for (int i = 0; i < grid->getGridSizeChunks(); ++i) {
    for (int j = 0; j < grid->getGridSizeChunks(); ++j) {
      Glade::Vector2i chunkIndex(i, j);
      terrain = new Isosurface();
      grid->addChunk(i, j, terrain);

      terrain->initialize(chunkIndex, *grid, false);
      context->add(terrain);
    }
  }
}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");
  this->context = &context;
  context.eventBus.registerListener(Glade::EventType::GLADE_COLLISION_EVENT, this);

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize, 2);

  createEntities();

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  context.renderer->getCamera()->position->z = 4.0;
  context.renderer->getCamera()->position->x = 2.0;
  context.renderer->getCamera()->position->y = 3.0;

  Glade::System::toggleMouseCursor(false);

  context.setController(*this);
}

void WalkingTheWorld::applyRules(Context &context)
{
  float forward = 0.0, strafe = 0.0, fly = 0.0;

  if (isKeyPressed(Glade::Key::GLADE_KEY_W))
    forward = -0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_S))
    forward = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_A))
    strafe = -0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_D))
    strafe = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_SPACE))
    fly = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_X))
    fly = -0.1;

  Transform *camera = context.getRenderer()->getCamera();
  float zModifier = forward * cos(camera->rotation->y) + strafe * sin(camera->rotation->y);
  float xModifier = -forward * sin(camera->rotation->y) + strafe * cos(camera->rotation->y);
  float yModifier = forward * sin(camera->rotation->x) + fly;

  camera->position->z += zModifier;
  camera->position->y += yModifier;
  camera->position->x += xModifier;
  float sphereSpeed = 0.06;

  if (isKeyPressed(Glade::Key::GLADE_KEY_LEFT))
    sphere->getTransform()->position->x -= sphereSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_RIGHT))
    sphere->getTransform()->position->x += sphereSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_UP))
    sphere->getTransform()->position->z -= sphereSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_DOWN))
    sphere->getTransform()->position->z += sphereSpeed;

  if (flyMode) {
    if (isKeyPressed(Glade::Key::GLADE_KEY_O))
      sphere->getTransform()->position->y += sphereSpeed;

    if (isKeyPressed(Glade::Key::GLADE_KEY_L))
      sphere->getTransform()->position->y -= sphereSpeed;
  } else {
    // gravity and jumping
    if (isKeyPressed(Glade::GLADE_KEY_F) && characterIsOnTheGround && !needsToReleaseJumpButtonFirst) {
        verticalSpeed = jumpAcceleration;
        characterIsOnTheGround = false;
        needsToReleaseJumpButtonFirst = true;
    }

    verticalSpeed += gravityAcceleration;
    verticalSpeed = std::clamp(verticalSpeed, -0.1f, 0.5f);
    sphere->getTransform()->position->y += verticalSpeed;
  }

  if (digging || growing)
    shoot();
}

void WalkingTheWorld::shoot()
{
  Glade::Vector3f nearPoint = context->getRenderer()->unprojectPoint(0, 0, 0);

  //log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Glade::Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.1;
  dir.y *= 0.1;
  dir.z *= 0.1;

  //log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);
  Grid::CellsI currentCell;

  std::pair<Glade::Vector2i, Glade::Vector3i> cellInfo;
  std::pair<Glade::Vector2i, Glade::Vector3i> prevCellInfo;

  for (int i = 0; i < 100; i++) {
    prevCellInfo = cellInfo;
    cellInfo = grid->getCellIndexByCoords(stepPoint);
    currentCell = grid->cells.find(cellInfo.second);

    if (currentCell == grid->cells.end()) {
      break;
    }

    bool shotSolid = false;

    for (int j = 0; j < 8; j++) {
      if (currentCell->second.val[j] < 0.5) {
        shotSolid = true;
        break;
      }
    }

    if (shotSolid) {
      grid->addValueAtCell(cellInfo.second, digging ? 0.1 : -0.1);
      break;
    }

    stepPoint.add(dir);
  }

  reloadChunk(cellInfo.first);

  std::vector<Glade::Vector2i> adjacentChunks;
  grid->getAdjacentChunks(cellInfo.second, adjacentChunks);

  for (const Glade::Vector2i &chunkIndex: adjacentChunks) {
    //log("Adj chunk (%d, %d)", chunkIndex.x, chunkIndex.y);
    reloadChunk(chunkIndex);
  }

}

void WalkingTheWorld::reloadChunk(const Glade::Vector2i &chunkIndex)
{
  Isosurface* surf = (Isosurface *) grid->getChunk(chunkIndex.x, chunkIndex.y);

  if (surf) {
    surf->initialize(chunkIndex, *grid);
    context->add(surf);
  }
}


void WalkingTheWorld::onEvent(Glade::EventType type, void *payload) {
  characterIsOnTheGround = true;
}

bool WalkingTheWorld::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool WalkingTheWorld::buttonRelease(Glade::Key key, int terminalId) {
  VirtualController::buttonRelease(key, terminalId);

  if (key == Glade::Key::GLADE_KEY_F) {
    needsToReleaseJumpButtonFirst = false;
  }

  return true;
}

bool WalkingTheWorld::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  switch (controlId) {
    case 0: digging = true;
            growing = false;
            break;
    case 1: growing = true;
            digging = false;
            break;
  }

  return true;
}

bool WalkingTheWorld::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  digging = growing = false;

  return true;
}

void WalkingTheWorld::shutdown(Context &context)
{

}

