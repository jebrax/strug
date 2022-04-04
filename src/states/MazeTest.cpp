#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/MazeTest.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Cube.h>

#include <fcl/fcl.h>

#include <unordered_map>

typedef std::vector<int> Indices;
typedef std::unordered_map<Vector3i,Indices*> SpatialIndex;
typedef SpatialIndex::iterator SpatialIndexI;

static SpatialIndex spatialMeshIndex;

static void initSpatialIndex(Frank *frank)
{
  //TODO don't forget about transforming vertices with the object's Transform

  Drawable *view = frank->view;
  std::shared_ptr<Glade::Mesh> mesh = view->getMesh();

  for (unsigned i = 0; i < mesh->getVertexBufferSize(); i += 8) {
    Vector3i cellCoord;
    cellCoord.x = std::floor(mesh->getVertices()[i + 0]);
    cellCoord.y = std::floor(mesh->getVertices()[i + 1]);
    cellCoord.z = std::floor(mesh->getVertices()[i + 2]);

    Indices *indicesVector = nullptr;
    SpatialIndexI cell = spatialMeshIndex.find(cellCoord);

    if (cell == spatialMeshIndex.end()) {
      indicesVector = new Indices();
      spatialMeshIndex.insert({cellCoord, indicesVector});
    } else {
      indicesVector = cell->second;
    }

    indicesVector->push_back(i / 8);
  }
}

MazeTest::MazeTest():
  State(),
  frank(nullptr)
{}

MazeTest::~MazeTest()
{
}

void MazeTest::init(Context &context)
{
  log("Init MazeTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);

  frank = new Frank();
  frank->initialize();
  context.add(frank);

  typedef fcl::BVHModel<fcl::OBBRSSf> Model;
  std::shared_ptr<Model> geom = std::make_shared<Model>();

  initSpatialIndex(frank);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->y = 4.0;

  context.setController(*this);
}

bool MazeTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId)
{
  //frank->getTransform()->rotation->y = xPos * 0.001;
  //frank->getTransform()->rotation->x = yPos * 0.001;
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool MazeTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  Vector3f nearPoint = context->getRenderer()->unprojectPoint(0, 0, 0);

  log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.5;
  dir.y *= 0.5;
  dir.z *= 0.5;

  log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);

  for (int i = 0; i < 50; i++) {
    /*
    Cube *another = new Cube();
    //Frank *another = new Frank();
    another->initialize();
    another->getTransform()->position->x = stepPoint.x;
    another->getTransform()->position->y = stepPoint.y;
    another->getTransform()->position->z = stepPoint.z;
    context->add(another);
    */

    Vector3i cellCoord;
    cellCoord.x = std::floor(stepPoint.x);
    cellCoord.y = std::floor(stepPoint.y);
    cellCoord.z = std::floor(stepPoint.z);

    SpatialIndexI cell = spatialMeshIndex.find(cellCoord);

    if (cell != spatialMeshIndex.end()) {
      log("Shot at vertices:");

      for (int i = 0; i < cell->second->size(); i++) {
        unsigned vertexIndex = (*cell->second)[i];
        printf("%d ", vertexIndex);
        float *vertex = frank->view->getMesh()->getVertices() + vertexIndex*8;
        vertex[2] -= 1;
      }
      printf("\n");

      context->add(frank);
      break;
    }

    stepPoint.add(dir);
  }

  return true;
}

void MazeTest::applyRules(Context &context)
{
}

void MazeTest::shutdown(Context &context)
{
  delete frank;
}

