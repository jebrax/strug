#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/MarchingCubes.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Isosurface.h>
#include <glade/render/meshes/CubeTerrainGenerator.h>

#include <unordered_map>

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
  Isosurface *surf = new Isosurface();
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
  //terrain->getTransform()->rotation->y = xPos * 0.001;
  //terrain->getTransform()->rotation->x = yPos * 0.001;
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
  if (controlId == 3) {
    digPressed = false;
  }

  if (controlId == 4) {
    clearPressed = false;
  }

  if (controlId == 5) {
    createPressed = false;
  }

  return true;
}

bool MarchingCubes::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void MarchingCubes::applyRules(Context &context)
{
}

void MarchingCubes::shutdown(Context &context)
{
  delete terrain;
}

