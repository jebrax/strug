#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>
#include <glade/math/Vector.h>
#include <unordered_map>
#include <vector>

class Context;
class StrugObject;
class VirtualContoller;
class Isosurface;
class Grid;
class Perception;

typedef std::unordered_map<Glade::Vector2i, Isosurface*> ChunksMap;
typedef ChunksMap::iterator ChunksMapI;

class Chunked: public State, public VirtualController
{
  private:
    Context *context;
    Perception *perception;
    StrugObject *mItemToAdd;

    bool digging, growing;
    Glade::Vector2f lastMousePos;

    Grid *grid;

    std::vector<StrugObject*> items;

    void shoot();
    void createEntities();
    void reloadChunk(const Glade::Vector2i &chunkIndex);
    void loadAllItems();

  public:
    Chunked();
    ~Chunked();

    void addItem(StrugObject *itemToAdd);
    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;
    void suspend(Context &context) override;
    void wakeup(Context &context) override;

    void initController() override {};
    bool buttonPress(Glade::Key controlId, int terminalId) override;
    bool buttonRelease(Glade::Key controlId, int terminalId) override;
    bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) override;
    bool pointerMove(float xPos, float yPos, float zPos, int controlId, int terminalId, bool isAbsolute = true) override;
};

