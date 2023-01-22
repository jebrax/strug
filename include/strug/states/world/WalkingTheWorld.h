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

  GladeObject *CreateEntityByTypeId(unsigned int type, const unsigned int *force_id = nullptr, bool replicate = true) override;
  void onClientInputReceived(unsigned int userId, unsigned int inputAction, bool isPressed) override;
  void onEntityReplicated(GladeObject *entity) override;
  void onEntityUnserialized(GladeObject *entity) override;
  void onUserLoggedIn(unsigned int userId) override;

private:
  typedef std::unordered_map<unsigned int, WorldController*> CharacterControllers;
  void shoot();

  Grid* mGrid = nullptr;
  Context *mContext;
  CharacterControllers mCharacterControllers;
  ChunkManager *mChunkManager;
};
