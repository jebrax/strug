#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class VirtualContoller;

class PhysicsTest: public State, VirtualController
{
  private:
    Context *context;

  public:
    PhysicsTest();
    ~PhysicsTest();
    
    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;
    void createEntities();
    void setupPhysics();

    void initController() override {};
    bool buttonPress(int controlId, int terminalId) override;
    bool buttonRelease(int controlId, int terminalId) override;
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute = true) override;
};
