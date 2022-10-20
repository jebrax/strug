#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/Minecraft.h>
#include <strug/blocks/Cube.h>
#include <glade/generation/AdvancedMeshGenerator.h>

#include <unordered_map>

struct Block
{
  bool bisected = false;
  float ky1;
  float ky2;
  float ky3;
  float ky4;

  Cube *gladeObject = nullptr;
};

typedef std::unordered_map<Glade::Vector3i,Block> SpatialIndex;
typedef SpatialIndex::iterator SpatialIndexI;

static SpatialIndex spatialIndex;

Minecraft::Minecraft():
  State(),
  terrain(nullptr),
  digPressed(false),
  clearPressed(false),
  createPressed(false)
{}

Minecraft::~Minecraft()
{
}

void Minecraft::createEntities()
{
  AdvancedMeshGenerator gen;

  auto createBlock = [this](Glade::Vector3i &gridCoord, float y1, float y2, float y3, float y4) {
    Block block;
    block.gladeObject = new Cube();
    block.gladeObject->initialize();
    block.bisected = true;
    block.ky1 = std::clamp(y1 - gridCoord.y, 0.0f, 1.0f);
    block.ky2 = std::clamp(y2 - gridCoord.y, 0.0f, 1.0f);
    block.ky3 = std::clamp(y3 - gridCoord.y, 0.0f, 1.0f);
    block.ky4 = std::clamp(y4 - gridCoord.y, 0.0f, 1.0f);

    spatialIndex[gridCoord] = block;
    gridCoord.y--;

    for (; gridCoord.y >= 0; gridCoord.y--) {
      block.gladeObject = new Cube();
      block.gladeObject->initialize();
      block.bisected = false;
      spatialIndex[gridCoord] = block;
    }
  };

  gen.generate(createBlock, Grid::CHUNK_HEIGHT);

  for (SpatialIndexI i = spatialIndex.begin(); i != spatialIndex.end(); i++) {
    int x = i->first.x, y = i->first.y, z = i->first.z;

    bool hidden = true;
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x + 1, y, z)) != spatialIndex.end());
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x - 1, y, z)) != spatialIndex.end());
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x, y + 1, z)) != spatialIndex.end());
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x, y - 1, z)) != spatialIndex.end());
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x, y, z - 1)) != spatialIndex.end());
    hidden = hidden && (spatialIndex.find(Glade::Vector3i(x, y, z + 1)) != spatialIndex.end());

    if (!hidden || true) {
      i->second.gladeObject->getTransform()->position->x = x;
      i->second.gladeObject->getTransform()->position->y = y;
      i->second.gladeObject->getTransform()->position->z = z;

      if (i->second.bisected)
        i->second.gladeObject->alterTop(i->second.ky1, i->second.ky2, i->second.ky3, i->second.ky4);

      this->context->add(i->second.gladeObject);
    }
  }
}

void Minecraft::init(Context &context)
{
  log("Init Minecraft");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
}

void Minecraft::dig()
{
}

bool Minecraft::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  //terrain->getTransform()->rotation->y = xPos * 0.001;
  //terrain->getTransform()->rotation->x = yPos * 0.001;
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool Minecraft::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  Glade::Vector3f nearPoint = context->getRenderer()->unprojectPoint(0, 0, 0);

  log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Glade::Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.05;
  dir.y *= 0.05;
  dir.z *= 0.05;

  log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);
  SpatialIndexI currentCell;

  for (int i = 0; i < 50; i++) {
    Cube *another = new Cube();
    //Hull *another = new Hull();
    //another->initialize();
    //another->getTransform()->position->x = stepPoint.x;
    //another->getTransform()->position->y = stepPoint.y;
    //another->getTransform()->position->z = stepPoint.z;
    //context->add(another);

    Glade::Vector3i cellCoord;
    cellCoord.x = std::floor(stepPoint.x);
    cellCoord.y = std::floor(stepPoint.y);
    cellCoord.z = std::floor(stepPoint.z);

    currentCell = spatialIndex.find(cellCoord);

    if (currentCell != spatialIndex.end()) {
      context->remove(currentCell->second.gladeObject);
      currentCell->second.gladeObject = nullptr;
      spatialIndex.erase(currentCell);
      break;
    }

    stepPoint.add(dir);
  }

  return true;
}

void Minecraft::applyRules(Context &context)
{
}

void Minecraft::shutdown(Context &context)
{
  delete terrain;
}

