#include <strug/states/world/WalkingTheWorld.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Isosurface.h>
#include <strug/states/world/WorldController.h>
#include <strug/states/world/ChunkManager.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/generation/Grid.h>
#include <glade/system.h>

#include <unordered_map>

WalkingTheWorld::WalkingTheWorld():
  State()
{}

WalkingTheWorld::~WalkingTheWorld()
{}

GladeObject* WalkingTheWorld::CreateEntityByTypeId(unsigned int type, const unsigned int *forceId)
{
  GladeObject *entity = nullptr;

  switch ((ObjectType) type) {
    case ObjectType::CHARACTER:
      Frank *character = new Frank(forceId);
      character->initialize(0.26);
      context->add(character);

      entity = character;
      break;
  }

  return entity;
}

void WalkingTheWorld::onEntityReplicated(GladeObject *entity)
{
  if (context->networkManager->isServer())
    return;

  if (entity->getType() == ObjectType::CHARACTER) {
    Frank *character = (Frank*) entity;

    if (character->mUserId == context->networkManager->getCurrentUserId()) {
      assert(grid);
      controller->setCharacter(character);
      mChunkManager->SetPlayerCharacter(character);
    }
  }
}

void WalkingTheWorld::onUserLoggedIn(unsigned int userId)
{
  if (context->networkManager->isServer()) {
    log("Adding object");
    Frank* character = (Frank*) CreateEntityByTypeId(ObjectType::CHARACTER);
    character->mUserId = userId;
    context->networkManager->addObject(character);
  }
}

void WalkingTheWorld::onClientInputReceived(unsigned int userId, unsigned int gladeKeyCode, bool isPressed)
{

}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");

  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);

  grid = new Grid(60, 0.25);

  Frank *playerCharacter = nullptr;
  if (context.networkManager->isServer()) {
    playerCharacter = (Frank *) CreateEntityByTypeId(ObjectType::CHARACTER, nullptr);
    context.networkManager->addObject(playerCharacter);
  }

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  controller = new WorldController(context);
  controller->setCharacter(playerCharacter);
  controller->resetCameraAndCharacterPositions();

  Glade::System::toggleMouseCursor(false);

  if (!context.networkManager->isServer())
    context.networkManager->connectToServer();

  context.setController(*controller);

  mChunkManager = new ChunkManager(*grid, playerCharacter, context);
  mChunkManager->Run();
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

  mChunkManager->ReloadChunk(chunkIndex);

  std::vector<Glade::Vector2i> adjacentChunks;
  grid->getAdjacentChunks(cellIndex, adjacentChunks);

  for (const Glade::Vector2i &chunkIndex: adjacentChunks) {
    //log("Adj chunk (%d, %d)", chunkIndex.x, chunkIndex.y);
    mChunkManager->ReloadChunk(chunkIndex);
  }
}

void WalkingTheWorld::shutdown(Context &context)
{
  assert(grid);
  grid->walkChunks([&context](Grid::ChunksI &chunki) { context.remove(chunki->second); });
  grid->clear();
  delete grid;
  grid = nullptr;

  mChunkManager->Stop();
  delete mChunkManager;
  mChunkManager = nullptr;

  // TODO delete all entities
}

