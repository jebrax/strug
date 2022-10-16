#include <strug/states/world/WorldController.h>

#include <glade/controls/VirtualController.h>
#include <glade/math/Transform.h>
#include <glade/Context.h>

WorldController::WorldController(Context &context, GladeObject *character):
    context(context),
    character(character)
{
  context.eventBus.registerListener(Glade::EventType::GLADE_COLLISION_EVENT, this);
}

void WorldController::onEvent(Glade::EventType type, void *payload) {
  characterIsOnTheGround = true;
}

bool WorldController::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  context.getRenderer()->getCamera()->rotation->y = xPos * 0.001;
  context.getRenderer()->getCamera()->rotation->x = yPos * 0.001;

  return true;
}

void WorldController::update()
{
  updateCamera();
  updateCharacter();
}

void WorldController::updateCamera()
{
  float forward = 0.0, strafe = 0.0, fly = 0.0;

  if (isKeyPressed(Glade::Key::GLADE_KEY_W))
    forward = -0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_S))
    forward = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_A))
    strafe = -0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_D))
    strafe = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_SPACE))
    fly = 0.1;

  if (isKeyPressed(Glade::Key::GLADE_KEY_X))
    fly = -0.1;

  Transform *camera = context.getRenderer()->getCamera();
  float zModifier = forward * cos(camera->rotation->y) + strafe * sin(camera->rotation->y);
  float xModifier = -forward * sin(camera->rotation->y) + strafe * cos(camera->rotation->y);
  float yModifier = forward * sin(camera->rotation->x) + fly;

  camera->position->z += zModifier;
  camera->position->y += yModifier;
  camera->position->x += xModifier;
}

void WorldController::updateCharacter()
{
  float characterSpeed = 0.06;

  if (isKeyPressed(Glade::Key::GLADE_KEY_LEFT))
    character->getTransform()->position->x -= characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_RIGHT))
    character->getTransform()->position->x += characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_UP))
    character->getTransform()->position->z -= characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_DOWN))
    character->getTransform()->position->z += characterSpeed;

  if (flyMode) {
    if (isKeyPressed(Glade::Key::GLADE_KEY_O))
      character->getTransform()->position->y += characterSpeed;

    if (isKeyPressed(Glade::Key::GLADE_KEY_L))
      character->getTransform()->position->y -= characterSpeed;
  } else {
    // gravity and jumping
    if (isKeyPressed(Glade::GLADE_KEY_F) && characterIsOnTheGround && !needsToReleaseJumpButtonFirst) {
        verticalSpeed = jumpAcceleration;
        characterIsOnTheGround = false;
        needsToReleaseJumpButtonFirst = true;
    }

    verticalSpeed += gravityAcceleration;
    verticalSpeed = std::clamp(verticalSpeed, -0.1f, 0.5f);
    character->getTransform()->position->y += verticalSpeed;
  }
}

bool WorldController::buttonPress(Glade::Key key, int terminalId) {
  VirtualController::buttonPress(key, terminalId);
  return true;
}

bool WorldController::buttonRelease(Glade::Key key, int terminalId) {
  VirtualController::buttonRelease(key, terminalId);

  if (key == Glade::Key::GLADE_KEY_F) {
    needsToReleaseJumpButtonFirst = false;
  }

  return true;
}

bool WorldController::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
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

bool WorldController::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  digging = growing = false;

  return true;
}

