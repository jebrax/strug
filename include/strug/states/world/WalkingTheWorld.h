#pragma once

#include <glade/EventBus.h>
#include <glade/State.h>
#include <glade/debug/log.h>

class Context;
class WorldController;

namespace Glade {
  class Vector2i;
}

class WalkingTheWorld: public State 
{
public:
  WalkingTheWorld();
  ~WalkingTheWorld();
  
  void init(Context &context) override;
  void shutdown(Context &context) override;
  void applyRules(Context &context) override;

private:
  void createEntities();
  void shoot();
  void reloadChunk(const Glade::Vector2i &chunkIndex);
  void generateNewChunks();

  Context *context;
  WorldController *controller;
};

