#include <algorithm>
#include <strug/states/world/WalkingTheWorld.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Isosurface.h>
#include <strug/states/world/WorldController.h>

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

WalkingTheWorld::WalkingTheWorld():
  State()
{}

WalkingTheWorld::~WalkingTheWorld()
{}

void WalkingTheWorld::createEntities()
{
  sphere = new Sphere();
  sphere->initialize(cellSize);
  context->add(sphere);

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

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize, 2);

  createEntities();

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  controller = new WorldController(context, sphere);
  controller->resetCameraAndCharacterPositions();

  Glade::System::toggleMouseCursor(false);

  context.setController(*controller);
}

void WalkingTheWorld::applyRules(Context &context)
{
  controller->update();

  if (controller->isShootButtonDown())
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
      grid->addValueAtCell(cellInfo.second, controller->isDigButtonDown()? 0.1 : -0.1);
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

void WalkingTheWorld::shutdown(Context &context)
{
}

