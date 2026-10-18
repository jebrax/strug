#pragma once

#include <glade/State.h>
#include <glade/controls/VirtualController.h>
#include <glade/debug/log.h>

class Context;
class Hull;
class WorldController;

class CubeTest: public State
{
  private:
    Context *context;
    bool rotateIsDown;
    WorldController *mController;

  public:
    CubeTest();
    ~CubeTest();

    void init(Context &context) override;
    void shutdown(Context &context) override;
    void applyRules(Context &context) override;
    void createEntities();

    GladeObject *createEntityByTypeId(unsigned int type, const unsigned int *force_id = nullptr, bool replicate = true) override;
};
