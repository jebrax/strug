#include <glade/system.h>
#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/generation/Grid.h>
#include <strug/states/Craft.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Isosurface.h>

#include <unordered_map>

#define GRID_CELL_SIZE_COORDS 0.25
#define GRID_CELLS_IN_A_CHUNK 60

typedef std::unordered_map<Glade::Vector2i, Isosurface*> ChunksMap;
typedef ChunksMap::iterator ChunksMapI;

static const int CHUNKS_SIDE = 1;
static Grid grid(GRID_CELLS_IN_A_CHUNK, GRID_CELL_SIZE_COORDS);

static ChunksMap chunks;

Craft::Craft():
  State(),
  digging(false),
  growing(false),
  rotateIsDown(false),
  xCursorPosNormalized(0),
  yCursorPosNormalized(0)
{}

Craft::~Craft()
{
}

void Craft::createEntities()
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

void Craft::init(Context &context)
{
  log("Init Craft");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
  pointerMove(0,0,0,0,0);
}

bool Craft::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  unsigned int viewportWidth, viewportHeight;
  Glade::System::getViewportSize(&viewportWidth, &viewportHeight);

  if (rotateIsDown && controlId == 0) {
    if (firstMove) {
      xPosRotationDelta = 0;
      yPosRotationDelta = 0;
      firstMove = false;
    } else {
      xPosRotationDelta = xPos - xPosRotationLast;
      yPosRotationDelta = yPos - yPosRotationLast;
    }
  }

  if (controlId == 0) {
    xPosRotationLast = xPos;
    yPosRotationLast = yPos;

    xCursorPosNormalized =  (xPos / viewportWidth * 2 - 1);
    yCursorPosNormalized = -(yPos / viewportHeight * 2 - 1);
  }

  static float phi = 0.0, theta = 0.0;
  static float r = 4.0f;
  static float originX = GRID_CELLS_IN_A_CHUNK * GRID_CELL_SIZE_COORDS / 2,
               originY = 40 * GRID_CELL_SIZE_COORDS / 2,
               originZ = GRID_CELLS_IN_A_CHUNK * GRID_CELL_SIZE_COORDS / 2;

  if (controlId == 0 || controlId == 1) {
    float x, y, z;

    if (controlId == 0 && rotateIsDown) {
      phi += yPosRotationDelta * 0.001;
      theta += xPosRotationDelta * 0.001;
    }

    if (controlId == 1 && !rotateIsDown) {
      r += yPos * 0.1;
    }

    y = cos(phi);
    x = cos(theta) * sin(phi);
    z = sin(theta) * sin(phi);
    x *= r; y *= r; z *= r;

    context->getRenderer()->getCamera()->position->x = x + originX;
    context->getRenderer()->getCamera()->position->y = y + originY;
    context->getRenderer()->getCamera()->position->z = z + originZ;

    context->getRenderer()->getCamera()->rotation->x = phi + PI / 2.0;
    context->getRenderer()->getCamera()->rotation->y = theta + PI / 2.0;
   }

  return true;
}

bool Craft::buttonPress(int controlId, int terminalId)
{
  return true;
}

bool Craft::buttonRelease(int controlId, int terminalId)
{
  return true;
}

void Craft::shoot()
{
  // near plane point (or should I say ray that goes through the near plane and the screen center)
  Glade::Vector3f towardsPoint = context->getRenderer()->unprojectPoint(xCursorPosNormalized, yCursorPosNormalized, 0);

  //log("Towards point: %f %f %f", towardsPoint.x, towardsPoint.y, towardsPoint.z);

  Glade::Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(towardsPoint.x, towardsPoint.y, towardsPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.1;
  dir.y *= 0.1;
  dir.z *= 0.1;

  //log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(towardsPoint.x, towardsPoint.y, towardsPoint.z);
  Grid::CellsI currentCell;

  std::pair<Glade::Vector2i, Glade::Vector3i> cellInfo;
  std::pair<Glade::Vector2i, Glade::Vector3i> prevCellInfo = grid.getCellIndexByCoords(stepPoint);

  for (int i = 0; i < 200; i++) {
    // this might not always work because step is much less then a cell size
    cellInfo = grid.getCellIndexByCoords(stepPoint);
    currentCell = grid.cells.find(cellInfo.second);

    if (currentCell == grid.cells.end()) {
      log("NO CELL");
      break;
    }

    bool shotSolid = true;

    for (int j = 0; j < 8; j++) {
      // this is not the best way, because prevCell gains value together with the currentCell
      if (currentCell->second.val[j] > 0.2) {
        shotSolid = false;
        break;
      }
    }

    if (shotSolid) {
      grid.addValueAtCell(prevCellInfo.second, digging ? 0.18 : -0.18);
      break;
    }

    prevCellInfo = cellInfo;
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

bool Craft::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  switch (controlId) {
    case 0: if (rotateIsDown)
              break;
            digging = true;
            growing = false;
            break;
    case 1: if (rotateIsDown)
              break;
            growing = true;
            digging = false;
            break;
    case 2: rotateIsDown = true;
            firstMove = true;
            Glade::System::toggleMouseCursor(false);
            break;
  }

  return true;
}

bool Craft::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  if (controlId == 2) {
    rotateIsDown = false;
    Glade::System::toggleMouseCursor(true);
  } else {
    digging = growing = false;
  }

  return true;
}

void Craft::reloadChunk(const Glade::Vector2i &chunkIndex)
{
  ChunksMapI chunk = chunks.find(chunkIndex);

  if (chunk != chunks.end()) {
    Isosurface* surf = chunk->second;
    surf->initialize(chunkIndex, grid);
    context->add(surf);
  }
}

void Craft::applyRules(Context &context)
{
  if (digging || growing)
    shoot();
}

void Craft::shutdown(Context &context)
{
}

