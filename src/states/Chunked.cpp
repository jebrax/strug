#include <glade/system.h>
#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/Chunked.h>
#include <strug/states/Craft.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Isosurface.h>
#include <glade/generation/Grid.h>

static const int CHUNKS_SIDE = 3;

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

void Chunked::addItem(StrugObject *itemToAdd)
{
  mItemToAdd = itemToAdd;
}

void Chunked::createEntities()
{
  for (int i = 0; i < CHUNKS_SIDE; i++) {
    for (int j = 0; j < CHUNKS_SIDE; j++) {
      Glade::Vector2i chunkIndex(i, j);
      Isosurface* surf = new Isosurface();
      surf->initialize(chunkIndex, *grid);
      context->add(surf);
      chunks[chunkIndex] = surf;
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

  for (StrugObject *item: items)
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

bool Chunked::buttonPress(int controlId, int terminalId)
{
  if (controlId == 2) {
    context->requestStateChange(new Craft(this), true);
  }

  return true;
}

bool Chunked::buttonRelease(int controlId, int terminalId)
{
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
      grid->addValueAtCell(cellInfo.second, 0.1);
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
  ChunksMapI chunk = chunks.find(chunkIndex);

  if (chunk != chunks.end()) {
    Isosurface* surf = chunk->second;
    surf->initialize(chunkIndex, *grid);
    context->add(surf);
  }
}

void Chunked::applyRules(Context &context)
{
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
      Glade::Vector2i chunkIndex(i, j);
      context.add(chunks[chunkIndex]);
    }
  }

  loadAllItems();

  Glade::System::toggleMouseCursor(false);
  Glade::System::setMouseCursorPosition(lastMousePos.x, lastMousePos.y);
  context.setController(*this);
}
