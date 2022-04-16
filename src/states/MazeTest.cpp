#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <strug/states/MazeTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Cube.h>
#include <strug/blocks/Sphere.h>

#include <fcl/fcl.h>

#include <unordered_map>

typedef std::vector<int> Indices;
typedef std::unordered_map<Glade::Vector3i,Indices*> SpatialIndex;
typedef SpatialIndex::iterator SpatialIndexI;

static SpatialIndex spatialMeshIndex;

static void initSpatialIndex(Frank *frank)
{
  //TODO don't forget about transforming vertices with the object's Transform

  Drawable *view = frank->view;
  std::shared_ptr<Glade::Mesh> mesh = view->getMesh();

  for (unsigned i = 0; i < mesh->getVertexBufferSize(); i += 8) {
    Glade::Vector3i cellCoord;
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

typedef fcl::BVHModel<fcl::OBBRSSf> Model;

// perhaps I don't need these
static std::shared_ptr<Model> terrain;
static std::shared_ptr<Model> shovel;

static std::shared_ptr<fcl::CollisionObjectf> terrainCollider;
static std::shared_ptr<fcl::CollisionObjectf> shovelCollider;

static std::shared_ptr<fcl::CollisionObjectf> initCollisionModel(StrugObject *object)
{
  std::vector<fcl::Vector3f> vertices;
  std::vector<fcl::Triangle> triangles;

  Drawable *view = object->getDrawables()[0][0];
  float *gladeVertices = view->getMesh()->getVertices();
  unsigned short *gladeIndices = view->getMesh()->getIndices();

  for (int i = 0; i < view->getMesh()->getVertexBufferSize(); i += 8) {
    //log("V: %f %f %f", gladeVertices[i], gladeVertices[i + 1], gladeVertices[i + 2]);
    vertices.emplace_back(gladeVertices[i + 0], gladeVertices[i + 1], gladeVertices[i + 2]);
  }

  //log("");

  for (int i = 0; i < view->getMesh()->getIndexBufferSize(); i += 3) {
    //log("F: %d %d %d", gladeIndices[i], gladeIndices[i + 1], gladeIndices[i + 2]);
    triangles.emplace_back(gladeIndices[i], gladeIndices[i + 1], gladeIndices[i + 2]);
  }

  std::shared_ptr<Model> geom = std::make_shared<Model>();

  geom->beginModel();
  geom->addSubModel(vertices, triangles);
  geom->endModel();

  fcl::Matrix3f rot;
  fcl::Vector3f trans;

  rot.setIdentity();
  trans[0] = object->getTransform()->position->x;
  trans[1] = object->getTransform()->position->y;
  trans[2] = object->getTransform()->position->z;

  fcl::Transform3f pose = fcl::Transform3f::Identity();
  pose.linear() = rot;
  pose.translation() = trans;

  return std::make_shared<fcl::CollisionObjectf>(geom, pose);

}

MazeTest::MazeTest():
  State(),
  terrain(nullptr),
  digPressed(false),
  clearPressed(false),
  createPressed(false)
{}

MazeTest::~MazeTest()
{
}

static Glade::Vector3f vertexCoordsToSphericalShovel(StrugObject *shovel, float x, float y, float z)
{
  float inclination;
  float azimuth;
  float r;
  Transform::SharedVector shovelPosition = shovel->getTransform()->position;
  x -= shovelPosition->x;
  y -= shovelPosition->y;
  z -= shovelPosition->z;

  r = std::sqrt(x*x + y*y + z*z);
  inclination = std::acos(y/r);

  if (x > 0) {
    azimuth = std::atan(z/x);
  } else if (x < 0 && z >= 0) {
    azimuth = std::atan(z/x) + PI;
  } else if (x < 0 && z < 0) {
    azimuth = std::atan(z/x) - PI;
  } else if (x == 0 && z > 0) {
    azimuth =  PI / 2.0f;
  } else if (x == 0 && z < 0) {
    azimuth = -PI / 2.0f;
  } else if (x == 0 && z == 0) {
    azimuth = 0; // any
  } else {
    assert("Impossiblee!");
  }

  return Glade::Vector3f(inclination, azimuth, r);
}

float getDistanceBetweenTwoVerts(Glade::Vector3f &one, Glade::Vector3f &two)
{
  float x = one.x;
  float y = one.y;
  float z = one.z;

  x -= two.x;
  y -= two.y;
  z -= two.z;

  return std::sqrt(x*x + y*y + z*z);
}

bool isVertexShoveled(Frank *shovel, float* terrainVertices, unsigned vertIndex,/*out*/ float *distance)
{
  bool shoveled = false;

  Glade::Vector3f terrVert(terrainVertices[vertIndex*8 + 0], terrainVertices[vertIndex*8 + 1], terrainVertices[vertIndex*8 + 2]);

  Transform::SharedVector shovelPosition = shovel->getTransform()->position;
  *distance = getDistanceBetweenTwoVerts(terrVert, *shovelPosition);

  return (*distance < shovel->radius);
}

void MazeTest::createEntities()
{
  terrain = new Sphere();
  terrain->initialize();
  context->add(terrain);
  
  shovel = new Frank();
  shovel->initialize(2.0);
  context->add(shovel);
  shovel->getTransform()->position->y = 0.0f;
  shovel->getTransform()->position->x = 5.0f;
  shovel->getTransform()->position->z = -10.0f;

/*
  Frank *frank = new Frank();
  context->add(frank);
  frank->getTransform()->position->y = -7.7f;
*/
}

void MazeTest::init(Context &context)
{
  log("Init MazeTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  //initSpatialIndex(frank);

  /*
  terrainCollider = initCollisionModel(terrain);
  shovelCollider = initCollisionModel(shovel);

  fcl::CollisionRequest<float> request(1000, true);
  fcl::CollisionResult<float> result;
  fcl::collide<float>(terrainCollider.get(), shovelCollider.get(), request, result);

  if (result.isCollision()) {
    size_t numContacts = result.numContacts();
    log("Collision! Number of contacts: %d", numContacts);

    for (size_t i = 0; i < numContacts; ++i) {
      const fcl::Contact<float> &contact = result.getContact(i);
      //log("Terrain triangle in contact: %d", contact.b1);
      unsigned short *indices = terrain->view->getMesh()->getIndices();
      unsigned short tmp = indices[contact.b1 * 3 + 0];
      indices[contact.b1 * 3 + 0] = indices[contact.b1 * 3 + 1];
      indices[contact.b1 * 3 + 1] = tmp;
    }

    context.add(terrain);
  }
  */

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 4.0;

  context.setController(*this);
}

void MazeTest::dig()
{
  Transform::SharedVector shovelPosition = shovel->getTransform()->position;
  float *vertices = terrain->view->getMesh()->getVertices();
  unsigned short *indices = terrain->view->getMesh()->getIndices();

  std::vector<float> newVertices;
  std::vector<unsigned short> newIndices;

  unsigned newVertexNumber = 0;

  std::unordered_map<unsigned,unsigned> oldToNewIndices;

  for (unsigned i = 0; i < terrain->view->getMesh()->getIndexBufferSize(); i += 3) {
    for (int ii = 0; ii < 3; ii++) {
      unsigned oldVertIndex = indices[i + ii];
      float r;
      bool vertexShoveled = isVertexShoveled(shovel, vertices, oldVertIndex, &r);
      auto existingMapping = oldToNewIndices.find(oldVertIndex);
      bool alreadyThere = existingMapping != oldToNewIndices.end();
      unsigned newIndex = alreadyThere ? existingMapping->second : newVertexNumber;

      if (!alreadyThere) {
        if (vertexShoveled) {
          Glade::Vector3f fromShovelCenterToVertex(
            vertices[oldVertIndex*8 + 0] - shovelPosition->x,
            vertices[oldVertIndex*8 + 1] - shovelPosition->y,
            vertices[oldVertIndex*8 + 2] - shovelPosition->z
          );
          fromShovelCenterToVertex.normalize();

          newVertices.push_back(fromShovelCenterToVertex.x * shovel->radius + shovelPosition->x);
          newVertices.push_back(fromShovelCenterToVertex.y * shovel->radius + shovelPosition->y);
          newVertices.push_back(fromShovelCenterToVertex.z * shovel->radius + shovelPosition->z);
        } else {
          newVertices.push_back(vertices[oldVertIndex*8 + 0]);
          newVertices.push_back(vertices[oldVertIndex*8 + 1]);
          newVertices.push_back(vertices[oldVertIndex*8 + 2]);
        }

        newVertices.push_back(vertices[oldVertIndex*8 + 3]);
        newVertices.push_back(vertices[oldVertIndex*8 + 4]);
        newVertices.push_back(vertices[oldVertIndex*8 + 5]);
        newVertices.push_back(vertexShoveled ? 1. : 0);
        newVertices.push_back(vertexShoveled ? 1. : 0);

        oldToNewIndices[oldVertIndex] = newIndex;
        newVertexNumber++;
      }

      newIndices.push_back(newIndex);
    }
  }

  terrain->view->getMesh()->vertices = newVertices;
  terrain->view->getMesh()->indices = newIndices;

  context->add(terrain);
}

bool MazeTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId)
{
  //terrain->getTransform()->rotation->y = xPos * 0.001;
  //terrain->getTransform()->rotation->x = yPos * 0.001;
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool MazeTest::buttonPress(int controlId, int terminalId)
{
  if (controlId == 1) {
    log("> Hiding the shovel");
    shovel->toggleView(false);
  }

  if (controlId == 2) {
    log("> Showing the shovel");
    shovel->toggleView(true);
  }

  if (controlId == 3 && !digPressed) {
    log("> Dig!");
    digPressed = true;
    dig();
  }

  if (controlId == 4 && !clearPressed) {
    log("> Clear");
    clearPressed = true;
    context->clear();
  }

  if (controlId == 5 && !createPressed) {
    log("> Create");
    createPressed = true;
    createEntities();
  }

  if (controlId == 6) {
    log("> Left");
    shovel->getTransform()->position->x -= 0.1;
  }

  if (controlId == 7) {
    log("> Right");
    shovel->getTransform()->position->x += 0.1;
  }

  if (controlId == 8) {
    log("> Up");
    shovel->getTransform()->position->y += 0.1;
  }

  if (controlId == 9) {
    log("> Down");
    shovel->getTransform()->position->y -= 0.1;
  }

  return true;
}

bool MazeTest::buttonRelease(int controlId, int terminalId)
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

bool MazeTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;

  Glade::Vector3f nearPoint = context->getRenderer()->unprojectPoint(0, 0, 0);

  log("Near point: %f %f %f", nearPoint.x, nearPoint.y, nearPoint.z);

  Glade::Vector3f cameraPos = *context->getRenderer()->getCamera()->position;

  Glade::Vector3f dir(nearPoint.x, nearPoint.y, nearPoint.z);
  dir.subtract(cameraPos);
  dir.normalize();

  // for the smaller step:
  dir.x *= 0.5;
  dir.y *= 0.5;
  dir.z *= 0.5;

  log("Ray dir: %f %f %f", dir.x, dir.y, dir.z);

  Glade::Vector3f stepPoint(nearPoint.x, nearPoint.y, nearPoint.z);

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

    Glade::Vector3i cellCoord;
    cellCoord.x = std::floor(stepPoint.x);
    cellCoord.y = std::floor(stepPoint.y);
    cellCoord.z = std::floor(stepPoint.z);

    SpatialIndexI cell = spatialMeshIndex.find(cellCoord);

    if (cell != spatialMeshIndex.end()) {
      log("Shot at vertices:");

      for (int i = 0; i < cell->second->size(); i++) {
        unsigned vertexIndex = (*cell->second)[i];
        printf("%d ", vertexIndex);
        float *vertex = terrain->view->getMesh()->getVertices() + vertexIndex*8;
        vertex[2] -= 1;
      }
      printf("\n");

      context->add(terrain);
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
  delete terrain;
}

