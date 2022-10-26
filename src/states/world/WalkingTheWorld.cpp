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

#include <imgui.h>

#include <pthread.h>
#include <unordered_map>
#include <algorithm>

static Frank *character= nullptr;
static Grid* grid = nullptr;
static const float cellSize = 0.25;
static const unsigned short CHUNK_GENERATION_RADIUS = 2;

struct GenerateNewChunksParameters {
  Context *context;
  Glade::Vector2i centralChunkIndex;
  bool autoDelete;
};

static void* generateNewChunks(void *p)
{
  GenerateNewChunksParameters *params = (GenerateNewChunksParameters*) p;
  Glade::Vector2i checkChunkIndex;

  for (int iinc = -CHUNK_GENERATION_RADIUS; iinc <= CHUNK_GENERATION_RADIUS; ++iinc) {
    for (int jinc = -CHUNK_GENERATION_RADIUS; jinc <= CHUNK_GENERATION_RADIUS; ++jinc) {
      checkChunkIndex.x = params->centralChunkIndex.x + iinc;
      checkChunkIndex.y = params->centralChunkIndex.y + jinc;

      if (!grid->getChunk(checkChunkIndex)) {
        Isosurface *chunk = new Isosurface();
        grid->addChunk(checkChunkIndex, chunk);

        chunk->initialize(checkChunkIndex, *grid, false);
        params->context->add(chunk);
      }
    }
  }

  if (params->autoDelete)
    delete params;

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

  regenerateTerrain();
}

void WalkingTheWorld::regenerateTerrain()
{
  grid->walkChunks([this](Grid::ChunksI &chunki) { context->remove(chunki->second); });
  grid->clear();

  generateNewChunksIfNeeded(true);
}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize);

  createEntities();

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  controller = new WorldController(context, character);
  controller->resetCameraAndCharacterPositions();

  Glade::System::toggleMouseCursor(false);

  context.setController(*controller);
}

void WalkingTheWorld::generateNewChunksIfNeeded(bool force)
{
  assert(grid);

  Glade::Vector2i centralChunkIndex;

  if (character) {
    Glade::Vector3i characterCellIndex = grid->pointToCellIndex(*character->getTransform()->position);
    centralChunkIndex = grid->cellIndexToChunkIndex(characterCellIndex);

    if (centralChunkIndex != lastCharacterChunkIndex) {
      log("Character enters chunk (%d, %d)", centralChunkIndex.x, centralChunkIndex.y);
      lastCharacterChunkIndex = centralChunkIndex;
    } else if (!force) {
      return;
    }
  }

  GenerateNewChunksParameters *params = new GenerateNewChunksParameters;
  params->context = context;
  params->centralChunkIndex = centralChunkIndex;
  params->autoDelete = true;

  pthread_t generateChunksThread;

  if (pthread_create(&generateChunksThread, NULL, generateNewChunks, params))
    log("Warning: failed to create thread for generating chunks");
}

void WalkingTheWorld::applyRules(Context &context)
{
  controller->update();

  {
    ImGui::Begin("Terrain parameters");

    //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

    if (ImGui::Button("Regenerate")) {
      log("Regenerating");

      regenerateTerrain();

      if (character)
        context.add(character);
    }

    ImGui::End();
  }

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
  assert(grid);
  grid->walkChunks([&context](Grid::ChunksI &chunki) { context.remove(chunki->second); });
  grid->clear();
  delete grid;
  grid = nullptr;

  assert(character);
  context.remove(character);
  delete character;
  character = nullptr;
}

