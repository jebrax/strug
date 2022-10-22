#include <strug/states/CollisionTest.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Cube.h>
#include <strug/blocks/Isosurface.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/physics/SpherePhysicalObject.h>
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
static std::vector<GladeObject*> terrainObjects;

static const float cellSize = 0.25;

static bool
  upPressed = false,
  downPressed = false,
  leftPressed = false,
  rightPressed = false,
  oPressed = false,
  lPressed = false;

static void meshSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  GladeObject *sobj= (GladeObject*) obj;

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

  std::shared_ptr<Glade::Mesh> mesh = sobj->getView()->getMesh();
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

static void isosurfaceCellSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  Grid::Cell *cell= (Grid::Cell*) obj;

  ccd_quat_t q, qx, qy, qz, qinv;
  ccd_vec3_t axisx, axisy, axisz;
  ccdVec3Set(&axisx, 1, 0, 0);
  ccdVec3Set(&axisy, 0, 1, 0);
  ccdVec3Set(&axisz, 0, 0, 1);

  ccdQuatSetAngleAxis(&qx, terrain->getTransform()->rotation->x, &axisx);
  ccdQuatSetAngleAxis(&qy, terrain->getTransform()->rotation->y, &axisy);
  ccdQuatSetAngleAxis(&qz, terrain->getTransform()->rotation->z, &axisz);

  ccdQuatCopy(&q, &qx);
  ccdQuatMul(&q, &qy);
  ccdQuatMul(&q, &qz);

  ccdQuatInvert2(&qinv, &q);

  ccd_vec3_t dir_rotated;
  ccdVec3Set(&dir_rotated, ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));
  ccdQuatRotVec(&dir_rotated, &qinv);

  Glade::Vector3f direction(ccdVec3X(&dir_rotated), ccdVec3Y(&dir_rotated), ccdVec3Z(&dir_rotated));
  direction.normalize();

  int startingIndex = cell->startingVertexIndex * 8;
  //log("Starting index: %d, num verts: %d * 8", startingIndex, cell->numVertices);

  std::shared_ptr<Glade::Mesh> mesh = terrain->getView()->getMesh();
  Glade::Vector3f vertex(mesh->vertices[startingIndex], mesh->vertices[startingIndex + 1], mesh->vertices[startingIndex + 2]);
  float maxDot = vertex.dot(direction);
  int maxi = startingIndex;
  //log("Box vertex: %f, %f, %f", vertex.x, vertex.y, vertex.z);

  for (int i = startingIndex + 8; i < startingIndex + cell->numVertices * 8; i += 8) {
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
  ccdVec3Set(&translation, terrain->getTransform()->position->x, terrain->getTransform()->position->y, terrain->getTransform()->position->z);
  ccdVec3Add(point, &translation);
}

static void sphereSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  Sphere *sphere = (Sphere*) obj;
  Glade::Vector3f direction(ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));

  direction.normalize();
  SpherePhysicalObject *phy = (SpherePhysicalObject *) sphere->getPhysicalObject();
  direction.scale(phy->radius);

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
  sphere->getTransform()->position->x = (float) grid->chunkSizeCells/2 * grid->cellSize - 3;
  sphere->getTransform()->position->y = 20 * grid->cellSize;
  sphere->getTransform()->position->z = (float) grid->chunkSizeCells/2 * grid->cellSize;
  context->add(sphere);

  cube = new Cube();
  cube->initialize();
  cube->getTransform()->rotation->y = 0.5;
  cube->getTransform()->position->z = 5;
  context->add(cube);
  terrainObjects.push_back(cube);

  cube2 = new Cube();
  cube2->initialize();
  cube2->getTransform()->rotation->y = 0.5;
  cube2->getTransform()->position->z = 5;
  cube2->getTransform()->position->x = 1;
  context->add(cube2);
  terrainObjects.push_back(cube2);

  Glade::Vector2i chunkIndex(0, 0);
  terrain = new Isosurface();
  terrain->initialize(chunkIndex, *grid, false);
  terrain->getView()->getMesh()->neverErase = true;
  context->add(terrain);
  //terrainObjects.push_back(terrain);
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

  if (oPressed)
    sphere->getTransform()->position->y += sphereSpeed;

  if (lPressed)
    sphere->getTransform()->position->y -= sphereSpeed;


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

  int intersect;

  for (const GladeObject* terrainObject: terrainObjects) {
    intersect = ccdGJKPenetration(terrainObject, sphere, &ccd, &depth, &dir, &pos);
    if (intersect >= 0) {
      CollisionInfo collision;
      collision.depth = depth;
      collision.dir = dir;
      collisions.push_back(collision);
    }
  }

  ccd.support1 = isosurfaceCellSupportFunction;

  for (int i = 0; i < grid->chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < grid->chunkSizeCells; k++) {
        Glade::Vector3i centerCellCoord(i, j, k);
        auto celli = grid->cells.find(centerCellCoord);

        if (celli == grid->cells.end()) {
          //log("No cell here");
          continue;
        }

        Grid::Cell &cell = celli->second;

        if (cell.numVertices <= 0) {
          //log("No vertices in this cell");
          continue;
        }

        intersect = ccdGJKPenetration(&cell, sphere, &ccd, &depth, &dir, &pos);

        if (intersect >= 0) {
          CollisionInfo collision;
          collision.depth = depth;
          collision.dir = dir;
          collisions.push_back(collision);
        }
      }
    }
  }

  Glade::Vector3f separation;

  for (const CollisionInfo& collision: collisions) {
    Glade::Vector3f partialSeparation(ccdVec3X(&collision.dir), ccdVec3Y(&collision.dir), ccdVec3Z(&collision.dir));
    partialSeparation.scale(collision.depth);
    separation.add(partialSeparation);
  }

  {
    //log("Penetration depth: %f, Separation dir: %f %f %f", depth, ccdVec3X(&dir), ccdVec3Y(&dir), ccdVec3Z(&dir));
    Glade::Vector3f separationDir(separation);
    separationDir.normalize();

    Glade::Vector3f toPrevPosition = prevPosition;
    toPrevPosition.subtract(*sphere->getTransform()->position);
    toPrevPosition.normalize();

    float dot = toPrevPosition.dot(separationDir);

    float sumDepth = separation.magnitude();

    if (dot > 0.71) { // stick/slide threshold is about 45 degrees
      // stick
      float pushDistance = sumDepth / dot;
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

