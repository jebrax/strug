#include <strug/states/world/WalkingTheWorld.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Isosurface.h>
#include <strug/blocks/DebugObject.h>
#include <strug/states/world/WorldController.h>
#include <strug/states/world/ChunkManager.h>
#include <strug/controls/StrugKeyMapper.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/generation/Grid.h>
#include <glade/system.h>

#include <unordered_map>

WalkingTheWorld::WalkingTheWorld():
  State(),
  mChunkManager(nullptr)
{}

WalkingTheWorld::~WalkingTheWorld()
{}

GladeObject* WalkingTheWorld::CreateEntityByTypeId(unsigned int type, const unsigned int *forceId, bool replicate)
{
  GladeObject *entity = nullptr;

  switch ((ObjectType) type) {
    case ObjectType::CHARACTER: {
      Frank *character = new Frank(forceId);
      character->initialize(0.26);
      mContext->add(character, replicate);

      entity = character;
      break;
    }
    case ObjectType::ISOSURFACE_CHUNK: {
      Isosurface *chunk = new Isosurface(forceId, *mGrid);

      mContext->add(chunk, replicate);
      entity = chunk;
      break;
    }
    case ObjectType::GENERIC: {
      DebugObject *obj = new DebugObject();

      mContext->add(obj, replicate);
      entity = obj;
      break;
    }
  }

  return entity;
}

void WalkingTheWorld::onEntityReplicated(GladeObject *entity)
{
  if (mContext->networkManager->isServer())
    return;

  if (entity->getType() == ObjectType::CHARACTER) {
    Frank *character = (Frank*) entity;

    unsigned int currentUserId = mContext->networkManager->getCurrentUserId();
    if (character->mUserId == currentUserId) {
      log("Player entity received, creating controller");
      WorldController *controller = new WorldController(*mContext);
      controller->SetKeyMapper(new StrugKeyMapper());
      controller->setCharacter(character);
      mCharacterControllers[currentUserId] = controller;
      mContext->setController(*controller);
    }
  }

  if (entity->getType() == ObjectType::ISOSURFACE_CHUNK) {
    Isosurface *surface = (Isosurface*) entity;
    mGrid->addChunk(surface->GetChunkIndex(), surface);
  }
}

void WalkingTheWorld::onEntityUnserialized(GladeObject *entity) {
  // Objects whos mesh may be updated must be re-added to the context
  // in order for the mesh to be updated in the video memory
  if (entity->getType() == ObjectType::ISOSURFACE_CHUNK) {
    mContext->add(entity);
  }
}

void WalkingTheWorld::onUserLoggedIn(unsigned int userId)
{
  if (mContext->networkManager->isServer()) {
    log("Adding object");
    Frank* character = (Frank*) CreateEntityByTypeId(ObjectType::CHARACTER, nullptr, true);
    character->mUserId = userId;

    WorldController *controller = new WorldController(*mContext);
    controller->setCharacter(character);
    controller->resetCameraAndCharacterPositions();
    mCharacterControllers[userId] = controller;
    mChunkManager->AddCharacter(character);
  }
}

void WalkingTheWorld::onClientInputReceived(unsigned int userId, unsigned int inputAction, bool isPressed)
{
  auto entry = mCharacterControllers.find(userId);

  if (entry == mCharacterControllers.end()) {
    log("Warning: network input message for user id %d for which there is no character controller", userId);
    return;
  }

  entry->second->UpdateInputActionState(inputAction, isPressed);
}

void WalkingTheWorld::init(Context &context)
{
  log("Init WalkingTheWorld");

  this->mContext = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);

  mGrid = new Grid(60, 0.25);

  context.getCollisionDetector()->setSpatialIndex(mGrid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  Glade::System::toggleMouseCursor(false);

  if (context.networkManager->isServer()) {
    // Create character for the server player (if it's not a dedicated server)
    Frank *playerCharacter = nullptr;
    playerCharacter = (Frank *) CreateEntityByTypeId(ObjectType::CHARACTER, nullptr, true);
    WorldController *controller = new WorldController(context);
    controller->SetKeyMapper(new StrugKeyMapper());
    controller->setCharacter(playerCharacter);
    mCharacterControllers[context.networkManager->getCurrentUserId()] = controller;

    controller->resetCameraAndCharacterPositions();
    context.setController(*controller);

    mChunkManager = new ChunkManager(*mGrid, context);
    mChunkManager->AddCharacter(playerCharacter);
    mChunkManager->Run();
  }

  CreateEntityByTypeId(ObjectType::GENERIC, nullptr, true);

  if (!context.networkManager->isServer())
    context.networkManager->connectToServer();
}

void WalkingTheWorld::applyRules(Context &context)
{
  for (auto entry: mCharacterControllers) {
    entry.second->update();
  }

  //if (entry->second->isShootButtonDown())
  //  shoot();
}

void WalkingTheWorld::shoot()
{
  if (!mChunkManager) {
    log("No chunk manager");
    return;
  }

  Glade::Vector3f nearPoint = mContext->getRenderer()->unprojectPoint(0, 0, 0);

  //log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Glade::Vector3f cameraPos = *mContext->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.1;
  dir.y *= 0.1;
  dir.z *= 0.1;

  //log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);
  Grid::Cell* currentCell = nullptr;

  Glade::Vector3i cellIndex;
  Glade::Vector2i chunkIndex;

  for (int i = 0; i < 100; i++) {
    cellIndex = mGrid->pointToCellIndex(stepPoint);
    chunkIndex = mGrid->cellIndexToChunkIndex(cellIndex);
    currentCell = mGrid->getCell(cellIndex);

    if (!currentCell) {
      break;
    }

    bool shotSolid = false;

    for (int j = 0; j < 8; j++) {
      if (currentCell->val[j] < 0.5) {
        shotSolid = true;
        break;
      }
    }

    if (shotSolid) {
      auto entry = mCharacterControllers.find(mContext->networkManager->getCurrentUserId());
      if (entry == mCharacterControllers.end())
        return;

      mGrid->addValueAtCell(cellIndex, entry->second->isDigButtonDown() ? 0.1 : -0.1);
      break;
    }

    stepPoint.add(dir);
  }

  mChunkManager->ReloadChunk(chunkIndex);

  std::vector<Glade::Vector2i> adjacentChunks;
  mGrid->getAdjacentChunks(cellIndex, adjacentChunks);

  for (const Glade::Vector2i &chunkIndex: adjacentChunks) {
    //log("Adj chunk (%d, %d)", chunkIndex.x, chunkIndex.y);
    mChunkManager->ReloadChunk(chunkIndex);
  }
}

void WalkingTheWorld::shutdown(Context &context)
{
  assert(mGrid);
  mGrid->walkChunks([&context](Grid::ChunksI &chunki) { context.remove(chunki->second); });
  mGrid->clear();
  delete mGrid;
  mGrid = nullptr;

  if (mChunkManager) {
    mChunkManager->Stop();
    delete mChunkManager;
    mChunkManager = nullptr;
  }

  // TODO delete all entities
}

