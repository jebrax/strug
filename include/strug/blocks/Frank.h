#pragma once

#include <strug/blocks/StrugObject.h>

class Frank: public StrugObject
{
  public:
    float radius;

    void initialize(float radius);
};
