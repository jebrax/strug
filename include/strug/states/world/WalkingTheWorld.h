#pragma once

#include <glade/EventBus.h>
#include <glade/State.h>
#include <glade/math/Vector.h>
#include <glade/debug/log.h>

class Context;
class WorldController;
class GladeObject;
class ChunkManager;
class Grid;

class WalkingTheWorld: public State 
{
public:
  WalkingTheWorld();
  ~WalkingTheWorld();

  void init(Context &context) override;
  void shutdown(Context &context) override;
  void applyRules(Context &context) override;

  enum ObjectType {
    GENERIC = 0,
    CHARACTER,
    ISOSURFACE_CHUNK,
    SURFACE_CHUNK
  };

  GladeObject *CreateEntityByTypeId(unsigned int type, const unsigned int *force_id = nullptr) override;
  void onClientInputReceived(unsigned int userId, unsigned int gladeKeyCode, bool isPressed) override;
  void onEntityReplicated(GladeObject *entity) override;
  void onUserLoggedIn(unsigned int userId) override;

private:
  void shoot();

  Grid* grid = nullptr;
  Context *context;
  WorldController *controller;
  ChunkManager *mChunkManager;
};
