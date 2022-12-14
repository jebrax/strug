#pragma once

#include <glade/GladeObject.h>
#include <glade/generation/AdvancedMeshGenerator.h>
#include <strug/states/world/WalkingTheWorld.h>

class Grid;

class Isosurface: public GladeObject
{
  public:
    Isosurface(const unsigned int *force_id): GladeObject(force_id) {}
    virtual void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, AdvancedMeshGenerator::TerrainGeneratorSettings &settings, bool crafting_mode = false);
    unsigned int getType() override { return  WalkingTheWorld::ObjectType::ISOSURFACE_CHUNK; }
};
