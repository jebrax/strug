#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class StrugObject;
class Chunked;
class VirtualContoller;

namespace Glade {
  class Vector2i;
}

class Craft: public State, VirtualController
{
  private:
    Context *context;
    Chunked *mMainState;

    bool digging, growing;
    bool rotateIsDown, firstMove;
    float xCursorPosNormalized, yCursorPosNormalized;
    float xPosRotationDelta, yPosRotationDelta;
    float xPosRotationLast, yPosRotationLast;

    void shoot();
    void createEntities();
    void reloadChunk(const Glade::Vector2i &chunkIndex);
  public:
    Craft(Chunked *mainState = nullptr);
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

