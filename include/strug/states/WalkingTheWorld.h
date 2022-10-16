#pragma once

#include <glade/EventBus.h>
#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class VirtualContoller;

namespace Glade {
  class Vector2i;
}

class WalkingTheWorld: public State, VirtualController, Glade::EventListener
{
  private:
    Context *context;
    bool characterIsOnTheGround = false;

  public:
    WalkingTheWorld();
    ~WalkingTheWorld();
    
    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;
    void createEntities();
    void shoot();
    void reloadChunk(const Glade::Vector2i &chunkIndex);

    void onEvent(Glade::EventType type, void *payload) override;

    void initController() override {};
    bool buttonRelease(Glade::Key key, int terminalId) override;
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute = true) override;
};
