#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class Frank;
class VirtualContoller;

class CubeTest: public State, VirtualController
{
  private:
    Context *context;
    bool rotateIsDown;

  public:
    CubeTest();
    ~CubeTest();
    
    void init(Context &context);
    void shutdown(Context &context);
    void applyRules(Context &context);
    void createEntities();

    void initController() {};
    bool buttonPress(int controlId, int terminalId);
    bool buttonRelease(int controlId, int terminalId);
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId);
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId);
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute = false);
};
