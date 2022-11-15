#pragma once

#include <glade/GladeObject.h>
#include <glade/generation/AdvancedMeshGenerator.h>

class Grid;

class Surface: public GladeObject
{
  public:
    Surface(): GladeObject() {}
    void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings);
};
