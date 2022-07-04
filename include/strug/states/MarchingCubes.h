#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class VirtualContoller;

class MarchingCubes: public State, VirtualController
{
  private:
    StrugObject *terrain;
    Context *context;

    bool digPressed;
    bool clearPressed;
    bool createPressed;

  public:
    MarchingCubes();
    ~MarchingCubes();
    
    void init(Context &context);
    void shutdown(Context &context);
    void applyRules(Context &context);
    void dig();
    void createEntities();

    void initController() {};
    bool buttonPress(int controlId, int terminalId);
    bool buttonRelease(int controlId, int terminalId);
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId);
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) { return false; }
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId);
};

