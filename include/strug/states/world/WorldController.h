#pragma once

#include <glade/EventBus.h>
#include <glade/controls/VirtualController.h>

class Context;
class GladeObject;

class WorldController: public VirtualController, Glade::EventListener
{
public:
  enum CameraMode {
    FREE = 0,
    THIRD_PERSON,
    FIRST_PERSON,
    ENUM_LAST_VALUE
  };

  explicit WorldController(Context &context);

  void setCharacter(GladeObject* character);
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

  bool flyMode = false;

private:
  void updateFreeCamera();
  void updateThirdPersonCamera();
  void thirdPersonCameraMouseMove(float xPos, float yPos, float zPos, int controlId);
  void updateFirstPersonCamera();
  void updateCharacter();

  static constexpr float GRAVITY_ACCELERATION = -0.08;
  static constexpr float JUMP_ACCELERATION = 0.4;
  static constexpr float CHARACTER_HEIGHT = 0.2;

  bool characterIsOnTheGround = false;
  bool digging = false, growing = false;
  float verticalSpeed = 0.0;

  // Third person camera controller variables
  bool rotateIsDown = false;
  double lastXPos, lastYPos;
  float cameraOwnPhi= 0.0, cameraOwnTheta= 0.0;
  float r = 4.0f;

  CameraMode cameraMode = CameraMode::FREE;
  GladeObject *character;
  Context &context;
};
