#include <glade/system.h>
#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/Chunked.h>
#include <strug/states/Craft.h>
#include <strug/blocks/Isosurface.h>
#include <glade/generation/Grid.h>

static const int CHUNKS_SIDE = 3;
static AdvancedMeshGenerator::TerrainGeneratorSettings terrainSettings;

Chunked::Chunked():
  State(),
  digging(false),
  growing(false),
  grid(nullptr),
  mItemToAdd(nullptr)
{}

Chunked::~Chunked()
{
}

void Chunked::addItem(GladeObject *itemToAdd)
{
  mItemToAdd = itemToAdd;
}

void Chunked::createEntities()
{
  for (int i = 0; i < CHUNKS_SIDE; i++) {
    for (int j = 0; j < CHUNKS_SIDE; j++) {
      Glade::Vector2i chunkIndex(i, j);
      Isosurface* surf = new Isosurface();
      grid->addChunk(i, j, surf);

      surf->initialize(chunkIndex, *grid, terrainSettings);
      context->add(surf);
    }
  }
}

void Chunked::loadAllItems()
{
  if (mItemToAdd) {
    mItemToAdd->getTransform()->position->x = context->getRenderer()->getCamera()->position->x;
    mItemToAdd->getTransform()->position->y = context->getRenderer()->getCamera()->position->y;
    mItemToAdd->getTransform()->position->z = context->getRenderer()->getCamera()->position->z;

    items.push_back(mItemToAdd);
    mItemToAdd = nullptr;
  }

  for (GladeObject *item: items)
    context->add(item);
}

void Chunked::init(Context &context)
{
  log("Init Chunked");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);

  perception = new Perception();
  context.renderer->setPerception(perception);

  grid = new Grid(60, 0.25);
  createEntities();
  loadAllItems();

  Glade::System::toggleMouseCursor(false);
  context.setController(*this);
}

bool Chunked::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  lastMousePos.x = xPos;
  lastMousePos.y = yPos;

  return true;
}

bool Chunked::buttonPress(Glade::Key controlId, int terminalId)
{
  VirtualController::buttonPress(controlId, terminalId);

  if (controlId == Glade::Key::GLADE_KEY_F) {
    context->requestStateChange(new Craft(this), true);
  }

  return true;
}

bool Chunked::buttonRelease(Glade::Key controlId, int terminalId)
{
  VirtualController::buttonRelease(controlId, terminalId);
  return true;
}

void Chunked::shoot()
{
  Glade::Vector3f nearPoint = context->getRenderer()->unprojectPoint(0, 0, 0);

  log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Glade::Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.1;
  dir.y *= 0.1;
  dir.z *= 0.1;

  log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);
  Grid::CellsI currentCell;

  Glade::Vector3i cellIndex;
  Glade::Vector2i chunkIndex; 
  std::pair<Glade::Vector2i, Glade::Vector3i> prevCellInfo;

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
      grid->addValueAtCell(cellIndex, digging ? 0.1 : -0.1);
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

bool Chunked::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
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

bool Chunked::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  digging = growing = false;

  return true;
}

void Chunked::reloadChunk(const Glade::Vector2i &chunkIndex)
{
  Isosurface* surf = (Isosurface *) grid->getChunk(chunkIndex.x, chunkIndex.y);

  if (surf) {
    surf->initialize(chunkIndex, *grid, terrainSettings);
    context->add(surf);
  }
}

void Chunked::applyRules(Context &context)
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

  if (digging || growing)
    shoot();
}

void Chunked::shutdown(Context &context)
{
  delete grid;
  grid = nullptr;

  context.renderer->setPerception(nullptr);
  delete perception;
  perception = nullptr;
}

void Chunked::suspend(Context &context)
{
  State::suspend(context);
}

void Chunked::wakeup(Context &context)
{
  State::wakeup(context);

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
  context.renderer->setPerception(perception);

  for (int i = 0; i < CHUNKS_SIDE; i++) {
    for (int j = 0; j < CHUNKS_SIDE; j++) {
      context.add(grid->getChunk(i, j));
    }
  }

  loadAllItems();

  Glade::System::toggleMouseCursor(false);
  Glade::System::setMouseCursorPosition(lastMousePos.x, lastMousePos.y);
  context.setController(*this);
}
