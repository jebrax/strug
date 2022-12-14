#pragma once

#include <glade/GladeObject.h>
#include <glade/generation/AdvancedMeshGenerator.h>
#include <strug/states/world/WalkingTheWorld.h>

class Grid;

class Surface: public GladeObject
{
  public:
    Surface(const unsigned int *force_id): GladeObject(force_id) {}
    void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings);
    unsigned int getType() override { return  WalkingTheWorld::ObjectType::SURFACE_CHUNK; }
};
