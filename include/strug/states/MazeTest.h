#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class Frank;
class VirtualContoller;

class MazeTest: public State, VirtualController
{
  private:
    StrugObject *terrain;
    Frank *shovel;
    Context *context;

    bool digPressed;
    bool clearPressed;
    bool createPressed;

  public:
    MazeTest();
    ~MazeTest();
    
    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;
    void dig();
    void createEntities();

    void initController() override {};
    bool buttonPress(int controlId, int terminalId) override;
    bool buttonRelease(int controlId, int terminalId) override;
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override { return false; }
    bool pointerMove(float axisX, float axisY, float axisZ, int controlId, int terminalId, bool isAbsolute = true) override;
};
