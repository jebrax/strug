#pragma once

#include <strug/blocks/StrugObject.h>
#include <glade/generation/CubeTerrainGenerator.h>

class Grid;

class Isosurface: public StrugObject
{
  public:
    Isosurface(): StrugObject() {}
    virtual void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, bool empty = false);
};
