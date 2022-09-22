#include <strug/states/CollisionTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Cube.h>
#include <strug/blocks/Isosurface.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <glade/generation/Grid.h>
#include <glade/system.h>

#include <ccd/ccd.h>
#include <ccd/quat.h>

#include <unordered_map>

static Sphere *sphere= nullptr;
static Isosurface *terrain = nullptr;
static Cube* cube = nullptr;
static Cube* cube2 = nullptr;
static Grid* grid = nullptr;

static const float cellSize = 0.25;

static bool
  upPressed = false,
  downPressed = false,
  leftPressed = false,
  rightPressed = false;

static void meshSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  StrugObject *sobj= (StrugObject*) obj;

  ccd_quat_t q, qx, qy, qz, qinv;
  ccd_vec3_t axisx, axisy, axisz;
  ccdVec3Set(&axisx, 1, 0, 0);
  ccdVec3Set(&axisy, 0, 1, 0);
  ccdVec3Set(&axisz, 0, 0, 1);

  ccdQuatSetAngleAxis(&qx, sobj->getTransform()->rotation->x, &axisx);
  ccdQuatSetAngleAxis(&qy, sobj->getTransform()->rotation->y, &axisy);
  ccdQuatSetAngleAxis(&qz, sobj->getTransform()->rotation->z, &axisz);

  ccdQuatCopy(&q, &qx);
  ccdQuatMul(&q, &qy);
  ccdQuatMul(&q, &qz);

  ccdQuatInvert2(&qinv, &q);

  ccd_vec3_t dir_rotated;
  ccdVec3Set(&dir_rotated, ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));
  ccdQuatRotVec(&dir_rotated, &qinv);

  Glade::Vector3f direction(ccdVec3X(&dir_rotated), ccdVec3Y(&dir_rotated), ccdVec3Z(&dir_rotated));
  direction.normalize();

  std::shared_ptr<Glade::Mesh> mesh = sobj->view->getMesh();
  Glade::Vector3f vertex(mesh->vertices[0], mesh->vertices[1], mesh->vertices[2]);
  float maxDot = vertex.dot(direction);
  int maxi = 0;
  //log("Box vertex: %f, %f, %f", vertex.x, vertex.y, vertex.z);

  for (int i = 8; i < mesh->getVertexBufferSize(); i += 8) {
    Glade::Vector3f vertex(mesh->vertices[i], mesh->vertices[i + 1], mesh->vertices[i + 2]);
    //log("Box vertex: %f, %f, %f", vertex.x, vertex.y, vertex.z);
    float dot = vertex.dot(direction);

    if (dot > maxDot) {
      maxDot = dot;
      maxi = i;
    }
  }

  ccdVec3Set(point, mesh->vertices[maxi + 0], mesh->vertices[maxi + 1], mesh->vertices[maxi + 2]);

  ccdQuatRotVec(point, &q);

  ccd_vec3_t translation;
  ccdVec3Set(&translation, sobj->getTransform()->position->x, sobj->getTransform()->position->y, sobj->getTransform()->position->z);
  ccdVec3Add(point, &translation);
}

static void sphereSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  Sphere *sphere = (Sphere*) obj;
  Glade::Vector3f direction(ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));

  direction.normalize();
  direction.scale(sphere->radius);

  Glade::Vector3f resultPoint;
  resultPoint.add(*sphere->getTransform()->position.get());
  resultPoint.add(direction);

  ccdVec3Set(point, resultPoint.x, resultPoint.y, resultPoint.z);
}

CollisionTest::CollisionTest():
  State()
{}

CollisionTest::~CollisionTest()
{
}

void CollisionTest::createEntities()
{
  sphere = new Sphere();
  sphere->initialize(cellSize);
  this->context->add(sphere);

  cube = new Cube();
  cube->initialize();
  cube->getTransform()->rotation->y = 0.5;
  cube->getTransform()->position->z = 5;
  this->context->add(cube);

  cube2 = new Cube();
  cube2->initialize();
  cube2->getTransform()->rotation->y = 0.5;
  cube2->getTransform()->position->z = 5;
  cube2->getTransform()->position->x = 0.5;
  this->context->add(cube2);

  Glade::Vector2i chunkIndex(0, 0);
  terrain = new Isosurface();
  terrain->initialize(chunkIndex, *grid);
  terrain->view->getMesh()->neverErase = true;
  context->add(terrain);
}

void CollisionTest::init(Context &context)
{
  log("Init CollisionTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, cellSize);

  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 5.0;
  context.renderer->getCamera()->position->z = 10.0;
  context.renderer->getCamera()->position->x = 10.0;
  context.renderer->getCamera()->position->y = 5.0;

  Glade::System::toggleMouseCursor(false);

  context.setController(*this);
}

void CollisionTest::applyRules(Context &context)
{
  float sphereSpeed = 0.06;
  Glade::Vector3f prevPosition;
  prevPosition = *sphere->getTransform()->position;

  if (leftPressed)
    sphere->getTransform()->position->x -= sphereSpeed;

  if (rightPressed)
    sphere->getTransform()->position->x += sphereSpeed;

  if (upPressed)
    sphere->getTransform()->position->z -= sphereSpeed;

  if (downPressed)
    sphere->getTransform()->position->z += sphereSpeed;

  ccd_t ccd;
  CCD_INIT(&ccd); // initialize ccd_t struct

  // set up ccd_t struct
  ccd.support1       = meshSupportFunction; // support function for first object
  ccd.support2       = sphereSupportFunction; // support function for second object
  ccd.max_iterations = 100;     // maximal number of iterations
  ccd.epa_tolerance  = 0.0001;  // maximal tolerance fro EPA part

  typedef struct {
    ccd_vec3_t dir;
    ccd_real_t depth;
  } CollisionInfo;

  std::vector<CollisionInfo> collisions;

  ccd_real_t depth;
  ccd_vec3_t dir, pos;

  int intersect = ccdGJKPenetration(cube, sphere, &ccd, &depth, &dir, &pos);
  if (intersect >= 0) {
    CollisionInfo collision;
    collision.depth = depth;
    collision.dir = dir;
    collisions.push_back(collision);
  }

  /*
  intersect = ccdGJKPenetration(cube2, sphere, &ccd, &depth, &dir, &pos);
  if (intersect >= 0) {
    CollisionInfo collision;
    collision.depth = depth;
    collision.dir = dir;
    collisions.push_back(collision);
  }
  */

  {
    //log("Penetration depth: %f, Separation dir: %f %f %f", depth, ccdVec3X(&dir), ccdVec3Y(&dir), ccdVec3Z(&dir));
    Glade::Vector3f separation(ccdVec3X(&dir), ccdVec3Y(&dir), ccdVec3Z(&dir));

    Glade::Vector3f toPrevPosition = prevPosition;
    toPrevPosition.subtract(*sphere->getTransform()->position);
    toPrevPosition.normalize();

    float dot = toPrevPosition.dot(separation);

    separation.scale(depth);

    if (dot > 0.71) { // stick/slide threshold is about 45 degrees
      // stick
      float pushDistance = depth / dot;
      Glade::Vector3f pushVector(toPrevPosition);
      pushVector.scale(pushDistance);
      sphere->getTransform()->position->add(pushVector);
    } else {
      // slide
      sphere->getTransform()->position->add(separation);
    }
  }
}

bool CollisionTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool CollisionTest::buttonPress(int controlId, int terminalId)
{
  switch (controlId) {
    case 6: leftPressed = true;
       break;
    case 7: rightPressed = true;
       break;
    case 8: upPressed = true;
       break;
    case 9: downPressed = true;
       break;
  }

  return true;
}

bool CollisionTest::buttonRelease(int controlId, int terminalId)
{
  switch (controlId) {
    case 6: leftPressed = false;
       break;
    case 7: rightPressed = false;
       break;
    case 8: upPressed = false;
       break;
    case 9: downPressed = false;
       break;
  }

  return true;
}

bool CollisionTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

bool CollisionTest::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void CollisionTest::shutdown(Context &context)
{

}

