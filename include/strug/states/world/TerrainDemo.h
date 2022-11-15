#pragma once

#include <glade/EventBus.h>
#include <glade/State.h>
#include <glade/math/Vector.h>
#include <glade/debug/log.h>

class Context;
class WorldController;

class TerrainDemo: public State 
{
public:
  TerrainDemo();
  ~TerrainDemo();
  
  void init(Context &context) override;
  void shutdown(Context &context) override;
  void applyRules(Context &context) override;

private:
  void createEntities();
  void regenerateTerrain();

  Context *context;
  WorldController *controller;
  Glade::Vector2i lastCharacterChunkIndex;
};

