#pragma once

#include <glade/GladeObject.h>

class Grid;

class Isosurface: public GladeObject
{
  public:
    Isosurface(): GladeObject() {}
    virtual void initialize(const Glade::Vector2i &chunkIndex, Grid &grid, bool crafting_mode = false);
};
