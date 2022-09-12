#include <glade/system.h>
#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <glade/physics/BulletTriangleMeshDecorator.h>
#include <strug/states/PhysicsTest.h>
#include <strug/blocks/StrugObject.h>
#include <strug/blocks/Cube.h>
#include <strug/blocks/Triangle.h>
#include <strug/blocks/Isosurface.h>
#include <glade/generation/Grid.h>

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <unordered_map>

static Cube *cube= nullptr;;
static Isosurface *terrain= nullptr;;
//static Cube *terrain= nullptr;;
//static Triangle *terrain= nullptr;;
static Grid* grid= nullptr;;
static btDiscreteDynamicsWorld* dynamicsWorld= nullptr;;
static btKinematicCharacterController* character= nullptr;;
static btPairCachingGhostObject* m_ghostObject = nullptr;

static bool
  upPressed = false,
  downPressed = false,
  leftPressed = false,
  rightPressed = false;

//keep track of the shapes, we release memory at exit.
//make sure to re-use collision shapes among rigid bodies whenever possible!
btAlignedObjectArray<btCollisionShape*> collisionShapes;

PhysicsTest::PhysicsTest():
  State()
{}

PhysicsTest::~PhysicsTest()
{
}

void PhysicsTest::createEntities()
{
  cube = new Cube();
  cube->initialize();
  this->context->add(cube);

  /*
  //terrain = new Cube();
  terrain = new Triangle();
  terrain->initialize();
  terrain->view->getMesh()->neverErase = true;
  terrain->view->getMesh()->bulletMeshDecorator = new BulletTriangleMeshDecorator(terrain->view->getMesh());
  this->context->add(terrain);
  */

  Glade::Vector2i chunkIndex(0, 0);
  terrain = new Isosurface();
  terrain->initialize(chunkIndex, *grid);
  terrain->view->getMesh()->neverErase = true;
  terrain->view->getMesh()->bulletMeshDecorator = new BulletTriangleMeshDecorator(terrain->view->getMesh());
  context->add(terrain);

  //terrain->getTransform()->scale->x = 2.0f;
  //terrain->getTransform()->scale->y = 2.0f;
  //terrain->getTransform()->scale->z = 2.0f;
}

void PhysicsTest::setupPhysics()
{
  // collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
  btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

  // use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
  btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

  // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  //btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

  btVector3 worldMin(-1000,-1000,-1000);
  btVector3 worldMax(1000,1000,1000);
  btAxisSweep3* sweepBP = new btAxisSweep3(worldMin,worldMax);
  btBroadphaseInterface* overlappingPairCache = sweepBP;

  // the default constraint solver. For parallel processing you can use a different solver
  // (see Extras/BulletMultiThreaded)
  btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
  dynamicsWorld->setGravity(btVector3(0,-1000,0));

  // Character controller
  {
    btTransform startTransform;
    startTransform.setIdentity ();
    //startTransform.setOrigin (btVector3(0.0, 4.0, 0.0));
    startTransform.setOrigin (btVector3(10, 15, 7));


    m_ghostObject = new btPairCachingGhostObject();
    m_ghostObject->setWorldTransform(startTransform);
    sweepBP->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    btScalar characterHeight=1.75;
    btScalar characterWidth =1.75;
    btConvexShape* capsule = new btCapsuleShape(characterWidth,characterHeight);
    m_ghostObject->setCollisionShape (capsule);
    m_ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);

    btScalar stepHeight = btScalar(0.35);
    character = new btKinematicCharacterController (m_ghostObject,capsule,stepHeight);
    character->setGravity(btVector3(0,-1000,0));

    dynamicsWorld->addCollisionObject(m_ghostObject,btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
    dynamicsWorld->addAction(character);
  }

  //the ground
  {
    BulletTriangleMeshDecorator* meshInterface = (BulletTriangleMeshDecorator*) terrain->view->getMesh()->getBulletMeshInterface();
    btCollisionShape* groundShape = new btBvhTriangleMeshShape(meshInterface, false);
    //btCollisionShape* groundShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
    //btCollisionShape* groundShape = new btSphereShape(1.0));

    collisionShapes.push_back(groundShape);

    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -3, 0));

    btScalar mass(0.);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
      groundShape->calculateLocalInertia(mass, localInertia);

    //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    rbInfo.m_friction = 1;
    btRigidBody* body = new btRigidBody(rbInfo);

    //add the body to the dynamics world
    dynamicsWorld->addRigidBody(body);
  }
/*
  {
    //create a dynamic rigidbody

    btCollisionShape* colShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
    //btCollisionShape* colShape = new btSphereShape(0.5);
    collisionShapes.push_back(colShape);

    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar mass(10.f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
      colShape->calculateLocalInertia(mass, localInertia);

    startTransform.setOrigin(btVector3(10, 15, 5));
    //startTransform.setOrigin(btVector3(-0.1, 0, -0.1));

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    rbInfo.m_friction = 1;
    btRigidBody* body = new btRigidBody(rbInfo);

    dynamicsWorld->addRigidBody(body);
  }
*/
}

void PhysicsTest::init(Context &context)
{
  log("Init PhysicsTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  grid = new Grid(60, 0.25);

  createEntities();
  setupPhysics();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);
  //context.renderer->getCamera()->position->z = 5.0;
  context.renderer->getCamera()->position->z = 10.0;
  context.renderer->getCamera()->position->x = 10.0;
  context.renderer->getCamera()->position->y = 5.0;

  Glade::System::toggleMouseCursor(false);

  context.setController(*this);
}

void PhysicsTest::applyRules(Context &context)
{
  float delta = context.physicsTimer.getDeltaTime();
  dynamicsWorld->stepSimulation(delta, 10, 0.0001);
  //dynamicsWorld->stepSimulation(0.001, 10, 0.0001);

  //print positions of all objects
  for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
  {
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
    btRigidBody* body = btRigidBody::upcast(obj);
    btTransform trans;

    if (body && body->getMotionState())
      body->getMotionState()->getWorldTransform(trans);
    else
      trans = obj->getWorldTransform();
/*
    if (j == 0) {
      terrain->getTransform()->position->x = trans.getOrigin().getX();
      terrain->getTransform()->position->y = trans.getOrigin().getY();
      terrain->getTransform()->position->z = trans.getOrigin().getZ();
    }
*/
  }

// Character controller
  btTransform xform;
  xform = m_ghostObject->getWorldTransform ();

  btVector3 forwardDir = xform.getBasis()[2];
  btVector3 upDir = xform.getBasis()[1];
  btVector3 strafeDir = xform.getBasis()[0];
  forwardDir.normalize ();
  upDir.normalize ();
  strafeDir.normalize ();

  btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
  btScalar walkVelocity = btScalar(1.1) * 4.0; // 4 km/h -> 1.1 m/s
  btScalar walkSpeed = walkVelocity * delta;

  if (upPressed)
    walkDirection += forwardDir;

  if (downPressed)
    walkDirection -= forwardDir;

  character->setWalkDirection(walkDirection*walkSpeed);

  cube->getTransform()->position->x = xform.getOrigin().getX();
  cube->getTransform()->position->y = xform.getOrigin().getY();
  cube->getTransform()->position->z = xform.getOrigin().getZ();
}

bool PhysicsTest::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context->getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context->getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

bool PhysicsTest::buttonPress(int controlId, int terminalId)
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

bool PhysicsTest::buttonRelease(int controlId, int terminalId)
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

bool PhysicsTest::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

bool PhysicsTest::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  return true;
}

void PhysicsTest::shutdown(Context &context)
{

}

