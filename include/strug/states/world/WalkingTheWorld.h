#pragma once

#include <glade/EventBus.h>
#include <glade/State.h>
#include <glade/math/Vector.h>
#include <glade/debug/log.h>

class Context;
class WorldController;
class GladeObject;

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
  void onEntityReplicated(GladeObject *entity) override;
  void onUserLoggedIn(unsigned int userId) override;

private:
  void createEntities();
  void shoot();
  void reloadChunk(const Glade::Vector2i &chunkIndex);
  void generateNewChunksIfNeeded(bool force = false);
  void regenerateTerrain();

  Context *context;
  WorldController *controller;
  Glade::Vector2i lastCharacterChunkIndex;
  unsigned int mUserId;
};
