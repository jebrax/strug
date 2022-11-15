#pragma once

#include <glade/GladeObject.h>
#include <glade/generation/AdvancedMeshGenerator.h>

class Grid;

class Isosurface: public GladeObject
{
  public:
    Isosurface(): GladeObject() {}
    virtual void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, AdvancedMeshGenerator::TerrainGeneratorSettings &settings, bool crafting_mode = false);
};
