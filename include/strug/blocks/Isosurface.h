#pragma once

#include <strug/blocks/StrugObject.h>
#include <glade/render/meshes/CubeTerrainGenerator.h>

class Isosurface: public StrugObject
{
  public:
    Isosurface(): StrugObject(), grid(60, 0.25) {}
    virtual void initialize();

    Grid grid;
};
