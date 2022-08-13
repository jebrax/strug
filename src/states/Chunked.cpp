#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/Chunked.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Isosurface.h>
#include <glade/generation/Grid.h>

#include <unordered_map>

typedef std::unordered_map<Glade::Vector2i, Isosurface*> ChunksMap;
typedef ChunksMap::iterator ChunksMapI;

static const int CHUNKS_SIDE = 3;
static Grid grid(60, 0.25);

static ChunksMap chunks;

Chunked::Chunked():
  State(),
  digging(false),
  growing(false)
{}

Chunked::~Chunked()
{
}

void Chunked::createEntities()
{
  for (int i = 0; i < CHUNKS_SIDE; i++) {
    for (int j = 0; j < CHUNKS_SIDE; j++) {
      Glade::Vector2i chunkIndex(i, j);
      Isosurface* surf = new Isosurface();
      surf->initialize(chunkIndex, grid, false);
      context->add(surf);
      chunks[chunkIndex] = surf;
    }
  }
}

void Chunked::init(Context &context)
{
  log("Init Chunked");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
}

bool Chunked::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool Chunked::buttonPress(int controlId, int terminalId)
{
  return true;
}

bool Chunked::buttonRelease(int controlId, int terminalId)
{
  return true;
}

void Chunked::dig()
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
    cellInfo = grid.getCellIndexByCoords(stepPoint);
    currentCell = grid.cells.find(cellInfo.second);

    if (currentCell == grid.cells.end()) {
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
      grid.addValueAtCellPerCubeVertex(cellInfo.second, stepPoint, 0.008);
      //grid.addValueAtCell(cellInfo.second, 0.1, -1);
      break;
    }

    stepPoint.add(dir);
  }

  reloadChunk(cellInfo.first);

  std::vector<Glade::Vector2i> adjacentChunks;
  grid.getAdjacentChunks(cellInfo.second, adjacentChunks);

  for (const Glade::Vector2i &chunkIndex: adjacentChunks) {
    //log("Adj chunk (%d, %d)", chunkIndex.x, chunkIndex.y);
    reloadChunk(chunkIndex);
  }

}

void Chunked::grow()
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
    cellInfo = grid.getCellIndexByCoords(stepPoint);
    currentCell = grid.cells.find(cellInfo.second);

    if (currentCell == grid.cells.end()) {
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
      grid.addValueAtCellPerCubeVertex(cellInfo.second, stepPoint, -0.008);
      //grid.addValueAtCell(cellInfo.second, -0.1, -1);
      break;
    }

    stepPoint.add(dir);
  }

  reloadChunk(cellInfo.first);

  std::vector<Glade::Vector2i> adjacentChunks;
  grid.getAdjacentChunks(cellInfo.second, adjacentChunks);

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
    surf->initialize(chunkIndex, grid);
    context->add(surf);
  }
}

void Chunked::applyRules(Context &context)
{
  if (digging)
    dig();

  if (growing)
    grow();
}

void Chunked::shutdown(Context &context)
{
}

