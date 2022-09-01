#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class VirtualContoller;

namespace Glade {
  class Vector2i;
}

class Craft: public State, VirtualController
{
  private:
    StrugObject *terrain;
    Context *context;

    bool digging, growing;
    bool rotateIsDown, firstMove;
    float xCursorPosNormalized, yCursorPosNormalized;
    float xPosRotationDelta, yPosRotationDelta;
    float xPosRotationLast, yPosRotationLast;

    void shoot();
    void createEntities();
    void reloadChunk(const Glade::Vector2i &chunkIndex);
  public:
    Craft();
    ~Craft();
    
    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;

    void initController() override {};
    bool buttonPress(int controlId, int terminalId) override;
    bool buttonRelease(int controlId, int terminalId) override;
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute = true) override;
};

