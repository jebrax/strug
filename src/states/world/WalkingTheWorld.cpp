#include <strug/states/world/WalkingTheWorld.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Isosurface.h>
#include <strug/states/world/WorldController.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <glade/generation/Grid.h>
#include <glade/system.h>

#include <pthread.h>
#include <unordered_map>
#include <algorithm>

static Frank *character= nullptr;
static Isosurface *terrain = nullptr;
static Grid* grid = nullptr;
static const float cellSize = 0.25;
static const unsigned short CHUNK_GENERATION_RADIUS = 1;

static void* generateNewChunks(void *contextParam)
{
  Context *context = (Context*) contextParam;

  Glade::Vector3i characterCellIndex = grid->pointToCellIndex(*character->getTransform()->position);
  Glade::Vector2i characterChunkIndex = grid->cellIndexToChunkIndex(characterCellIndex);
  Glade::Vector2i checkChunkIndex;

  for (int iinc = -CHUNK_GENERATION_RADIUS; iinc <= CHUNK_GENERATION_RADIUS; ++iinc) {
    for (int jinc = -CHUNK_GENERATION_RADIUS; jinc <= CHUNK_GENERATION_RADIUS; ++jinc) {
      checkChunkIndex.x = characterChunkIndex.x + iinc;
      checkChunkIndex.y = characterChunkIndex.y + jinc;

      if (!grid->getChunk(checkChunkIndex)) {
        Isosurface *chunk = new Isosurface(); // deallocate
        grid->addChunk(checkChunkIndex, chunk);

        chunk->initialize(checkChunkIndex, *grid, false);
        context->add(chunk);
      }
    }
  }

  return NULL;
}

WalkingTheWorld::WalkingTheWorld():
  State()
{}

WalkingTheWorld::~WalkingTheWorld()
{}

void WalkingTheWorld::createEntities()
{
  character = new Frank();
  character->initialize(cellSize);
  context->add(character);
  Glade::Vector3i characterCellIndex = grid->pointToCellIndex(*character->getTransform()->position);
  lastCharacterChunkIndex = grid->cellIndexToChunkIndex(characterCellIndex);

  Glade::Vector2i chunkIndex(0, 0);
  terrain = new Isosurface(); // deallocate
  grid->addChunk(0, 0, terrain);

  terrain->initialize(chunkIndex, *grid, false);
  context->add(terrain);
}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize);

  createEntities();
  generateNewChunks(&context);

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  controller = new WorldController(context, character);
  controller->resetCameraAndCharacterPositions();

  Glade::System::toggleMouseCursor(false);

  context.setController(*controller);
}

void WalkingTheWorld::generateNewChunksIfNeeded()
{
  Glade::Vector3i characterCellIndex = grid->pointToCellIndex(*character->getTransform()->position);
  Glade::Vector2i characterChunkIndex = grid->cellIndexToChunkIndex(characterCellIndex);

  if (characterChunkIndex == lastCharacterChunkIndex)
    return;

  log("Character enters chunk (%d, %d)", characterChunkIndex.x, characterChunkIndex.y);

  pthread_t generateChunksThread;

  if (pthread_create(&generateChunksThread, NULL, generateNewChunks, context)) {
    log("Warning: failed to create thread for generating chunks");
    return;
  }

  lastCharacterChunkIndex = characterChunkIndex;
}

void WalkingTheWorld::applyRules(Context &context)
{
  controller->update();

  generateNewChunksIfNeeded();

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

  Glade::Vector3i cellIndex;
  Glade::Vector2i chunkIndex;

  for (int i = 0; i < 100; i++) {
    cellIndex = grid->pointToCellIndex(stepPoint);
    chunkIndex = grid->cellIndexToChunkIndex(cellIndex);
    currentCell = grid->cells.find(cellIndex);

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
      grid->addValueAtCell(cellIndex, controller->isDigButtonDown()? 0.1 : -0.1);
      break;
    }

    stepPoint.add(dir);
  }

  reloadChunk(chunkIndex);

  std::vector<Glade::Vector2i> adjacentChunks;
  grid->getAdjacentChunks(cellIndex, adjacentChunks);

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

