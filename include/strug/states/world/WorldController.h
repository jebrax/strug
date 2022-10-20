#pragma once

#include <glade/EventBus.h>
#include <glade/controls/VirtualController.h>

class Context;
class GladeObject;

class WorldController: public VirtualController, Glade::EventListener
{
public:
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

private:
  void updateCamera();
  void updateCharacter();

  static constexpr float gravityAcceleration = -0.08;
  static constexpr float jumpAcceleration = 0.4;

  bool flyMode = false;
  bool characterIsOnTheGround = false;
  bool digging = false, growing = false;
  float verticalSpeed = 0.0;

  GladeObject *character;
  Context &context;
};
