#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class Frank;
class VirtualContoller;

class MazeTest: public State, VirtualController
{
  private:
    Frank *frank;
    Context *context;

  public:
    MazeTest();
    ~MazeTest();
    
    void init(Context &context);
    void shutdown(Context &context);
    void applyRules(Context &context);

    void initController() {};
    bool buttonPress(int controlId, int terminalId) { return false; }
    bool buttonRelease(int controlId, int terminalId) { return false; }
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId);
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) { return false; }
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId);
};
