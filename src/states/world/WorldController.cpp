#include <strug/states/world/WorldController.h>
#include <strug/blocks/Frank.h>

#include <glade/controls/VirtualController.h>
#include <glade/math/Transform.h>
#include <glade/Context.h>
#include <glade/system.h>

WorldController::WorldController(Context &context, GladeObject *character):
    context(context),
    character(character)
{
  context.eventBus.registerListener(Glade::EventType::GLADE_COLLISION_EVENT, this);
}

void WorldController::resetCameraAndCharacterPositions()
{
  context.renderer->getCamera()->position->z = 4.0f;
  context.renderer->getCamera()->position->x = 2.0f;
  context.renderer->getCamera()->position->y = 3.0f;

  character->getTransform()->position->x = 7.5f;
  character->getTransform()->position->y = 5.0f;
  character->getTransform()->position->z = 7.5f;
}

void WorldController::onEvent(Glade::EventType type, void *payload) {
  characterIsOnTheGround = true;
}

void WorldController::setCameraMode(CameraMode mode)
{
  cameraMode = mode;

  if (cameraMode == CameraMode::THIRD_PERSON) {
    Glade::System::toggleMouseCursor(true);
    phi = -PI/4;
    thirdPersonCameraMouseMove(0,0,0,0); // this reset third person camera parameters
    updateThirdPersonCamera();
  }

  if (cameraMode == CameraMode::FIRST_PERSON) {
    context.getRenderer()->getCamera()->rotation->x = 0;
    updateFirstPersonCamera();
  }

  Glade::System::toggleMouseCursor(cameraMode == CameraMode::THIRD_PERSON);
}

bool WorldController::pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute)
{
  if (cameraMode == CameraMode::FREE) {
    context.getRenderer()->getCamera()->rotation->y = xPos * 0.001;
    context.getRenderer()->getCamera()->rotation->x = yPos * 0.001;
  } else if (cameraMode == CameraMode::FIRST_PERSON) {
    context.getRenderer()->getCamera()->rotation->x = yPos * 0.001;
    character->getTransform()->rotation->y = xPos * 0.001;
  } else {
    thirdPersonCameraMouseMove(xPos, yPos, zPos, controlId);
  }

  return true;
}

void WorldController::thirdPersonCameraMouseMove(float xPos, float yPos, float zPos, int controlId)
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

  if (controlId == 0 && rotateIsDown) {
    phi += yPosRotationDelta * 0.001;
    theta += xPosRotationDelta * 0.001;
  }

  if (controlId == 1 && !rotateIsDown) {
    r -= yPos * 0.1;
  }

  y = cos(phi);
  x = cos(theta) * sin(phi);
  z = sin(theta) * sin(phi);
  x *= r; y *= r; z *= r;
}

void WorldController::update()
{
  updateCharacter();

  if (cameraMode == CameraMode::FREE) {
    updateFreeCamera();
  } else if (cameraMode == CameraMode::THIRD_PERSON) {
    updateThirdPersonCamera();
  } else if (cameraMode == CameraMode::FIRST_PERSON) {
    updateFirstPersonCamera();
  }
}

void WorldController::updateFirstPersonCamera()
{
  float originX = character->getTransform()->position->x,
        originY = character->getTransform()->position->y,
        originZ = character->getTransform()->position->z;

  float rotY =  character->getTransform()->rotation->y;

  context.getRenderer()->getCamera()->position->x = originX;
  context.getRenderer()->getCamera()->position->y = originY + CHARACTER_HEIGHT;
  context.getRenderer()->getCamera()->position->z = originZ;

  context.getRenderer()->getCamera()->rotation->y = rotY;
}

void WorldController::updateThirdPersonCamera()
{
  float originX = character->getTransform()->position->x,
        originY = character->getTransform()->position->y,
        originZ = character->getTransform()->position->z;

  context.getRenderer()->getCamera()->position->x = x + originX;
  context.getRenderer()->getCamera()->position->y = y + originY;
  context.getRenderer()->getCamera()->position->z = z + originZ;

  context.getRenderer()->getCamera()->rotation->x = phi + PI / 2.0;
  context.getRenderer()->getCamera()->rotation->y = theta + PI / 2.0;
}

void WorldController::updateFreeCamera()
{
  float cameraSpeed = 0.1;
  float forward = 0.0, strafe = 0.0, fly = 0.0;

  if (isKeyPressed(Glade::Key::GLADE_KEY_UP))
    forward = -cameraSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_DOWN))
    forward = cameraSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_LEFT))
    strafe = -cameraSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_RIGHT))
    strafe = cameraSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_O))
    fly = cameraSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_L))
    fly = -cameraSpeed;

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
  float characterSpeed = 0.1;
  float forward = 0.0, strafe = 0.0, fly = 0.0;

  if (isKeyPressed(Glade::Key::GLADE_KEY_W))
    forward = -characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_S))
    forward = characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_A))
    strafe = -characterSpeed;

  if (isKeyPressed(Glade::Key::GLADE_KEY_D))
    strafe = characterSpeed;

  Transform *charTransform = character->getTransform();
  float zModifier = forward * cos(charTransform->rotation->y) + strafe * sin(charTransform->rotation->y);
  float xModifier = -forward * sin(charTransform->rotation->y) + strafe * cos(charTransform->rotation->y);
  float yModifier = forward * sin(charTransform->rotation->x) + fly;

  charTransform->position->z += zModifier;
  charTransform->position->y += yModifier;
  charTransform->position->x += xModifier;

  if (flyMode) {
    if (isKeyPressed(Glade::Key::GLADE_KEY_SPACE))
      character->getTransform()->position->y += characterSpeed;

    if (isKeyPressed(Glade::Key::GLADE_KEY_X))
      character->getTransform()->position->y -= characterSpeed;
  } else {
    // gravity
    verticalSpeed += GRAVITY_ACCELERATION;
    verticalSpeed = std::clamp(verticalSpeed, -0.1f, 0.5f);
    character->getTransform()->position->y += verticalSpeed;
  }
}

bool WorldController::buttonPress(Glade::Key key, int terminalId) {
  bool changedState = VirtualController::buttonPress(key, terminalId);

  if (!changedState)
    return false;

  if (key == Glade::Key::GLADE_KEY_1) {
    flyMode = !flyMode;
    log("Flying mode is %s", flyMode ? "ON" : "OFF");

    if (flyMode)
      characterIsOnTheGround = false;
  }

  if (key == Glade::Key::GLADE_KEY_2) {
    context.enableCollisionDetector = !context.enableCollisionDetector;
    log("Collisions are %s", context.enableCollisionDetector ? "ON" : "OFF");
  }

  if (key == Glade::Key::GLADE_KEY_3) {
    resetCameraAndCharacterPositions();
    log("Camera and character positions were reset");
  }

  if (key == Glade::Key::GLADE_KEY_4) {
    setCameraMode((CameraMode) (cameraMode + 1));

    if (cameraMode == CameraMode::_ENUM_LAST_VALUE)
      setCameraMode(cameraMode = (CameraMode) 0);
  }

  if (key == Glade::Key::GLADE_KEY_5) {
    bool enabled = ((Frank*)character)->toggleCollisionShapeView();
    log("%s collision shapes", enabled ? "SHOWING " : "HIDING ");
  }

  if (key == Glade::Key::GLADE_KEY_SPACE && characterIsOnTheGround) {
      verticalSpeed = JUMP_ACCELERATION;
      characterIsOnTheGround = false;
  }

  return changedState;
}

bool WorldController::buttonRelease(Glade::Key key, int terminalId) {
  VirtualController::buttonRelease(key, terminalId);

  return true;
}

bool WorldController::pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  switch (controlId) {
    case 0: if (cameraMode == CameraMode::THIRD_PERSON)
              break;  
            digging = true;
            growing = false;
            break;
    case 1: if (cameraMode == CameraMode::THIRD_PERSON)
              break;
            growing = true;
            digging = false;
            break;
    case 2: if (cameraMode != CameraMode::THIRD_PERSON)
              break;
            rotateIsDown = true;
            firstMove = true;
            Glade::System::toggleMouseCursor(false);
            break;
  }

  return true;
}

bool WorldController::pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId)
{
  if (controlId == 2 && CameraMode::THIRD_PERSON) {
    rotateIsDown = false;
    Glade::System::toggleMouseCursor(true);
  } else {
    digging = growing = false;
  }

  return true;
}

