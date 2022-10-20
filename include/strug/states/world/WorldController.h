#pragma once

#include <glade/EventBus.h>
#include <glade/controls/VirtualController.h>

class Context;
class GladeObject;

class WorldController: public VirtualController, Glade::EventListener
{
public:
  enum CameraMode {
    FREE,
    THIRD_PERSON,
    FIRST_PERSON
  };

  WorldController(Context &context, GladeObject *character);

  bool isShootButtonDown() { return digging || growing; }
  bool isDigButtonDown() { return digging; }

  void onEvent(Glade::EventType type, void *payload) override;

  bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute) override;
  bool buttonPress(Glade::Key key, int terminalId) override;
  bool buttonRelease(Glade::Key key, int terminalId) override;
  bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
  bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;

  // Character and camera controller functionality
  void update();
  void resetCameraAndCharacterPositions();
  void setCameraMode(CameraMode mode);

private:
  void updateFreeCamera();
  void updateThirdPersonCamera();
  void thirdPersonCameraMouseMove(float xPos, float yPos, float zPos, int controlId);
  void updateCharacter();

  static constexpr float gravityAcceleration = -0.08;
  static constexpr float jumpAcceleration = 0.4;

  bool flyMode = false;
  bool characterIsOnTheGround = false;
  bool digging = false, growing = false;
  float verticalSpeed = 0.0;

  // Third person camera controller variables
  bool rotateIsDown = false, firstMove = true;
  float xCursorPosNormalized, yCursorPosNormalized;
  float xPosRotationDelta, yPosRotationDelta;
  float xPosRotationLast, yPosRotationLast;
  float phi = 0.0, theta = 0.0;
  float r = 4.0f;
  float x = 0.0, y = 0.0, z = 0.0;


  CameraMode cameraMode = CameraMode::FREE;
  GladeObject *character;
  Context &context;
};
