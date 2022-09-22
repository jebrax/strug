#pragma once

#include <strug/blocks/StrugObject.h>

class Sphere: public StrugObject
{
  public:
    virtual void initialize(float radius);
    float radius;
};
