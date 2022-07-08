#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/MarchingCubes.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Isosurface.h>
#include <glade/render/meshes/CubeTerrainGenerator.h>

#include <unordered_map>

static Isosurface *surf = nullptr;

MarchingCubes::MarchingCubes():
  State(),
  terrain(nullptr),
  digPressed(false),
  clearPressed(false),
  createPressed(false)
{}

MarchingCubes::~MarchingCubes()
{
}

void MarchingCubes::createEntities()
{
  surf = new Isosurface();
  surf->initialize();
  context->add(surf);
}

void MarchingCubes::init(Context &context)
{
  log("Init MarchingCubes");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
}

void MarchingCubes::dig()
{
}

bool MarchingCubes::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool MarchingCubes::buttonPress(int controlId, int terminalId)
{
  return true;
}

bool MarchingCubes::buttonRelease(int controlId, int terminalId)
{
  return true;
}

bool MarchingCubes::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
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

  for (int i = 0; i < 100; i++) {
    Glade::Vector3i cellIndex = surf->grid.getCellIndexByCoords(stepPoint);
    currentCell = surf->grid.cells.find(cellIndex);

    if (currentCell == surf->grid.cells.end()) {
      break;
    }

    bool shotSolid = false;

    for (int j = 0; j < 8; j++) {
      if (currentCell->second.val[j] < 0.5) {
        shotSolid = true;
      }
    }

    surf->grid.addValueAt(cellIndex, 0.1);

    if (shotSolid)
      break;

    stepPoint.add(dir);
  }

  surf->initialize();
  context->add(surf);

  return true;
}

void MarchingCubes::applyRules(Context &context)
{
}

void MarchingCubes::shutdown(Context &context)
{
  delete terrain;
}

