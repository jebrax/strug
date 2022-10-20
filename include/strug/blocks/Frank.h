#pragma once

#include <glade/GladeObject.h>

class Frank: public GladeObject
{
  public:
    float radius;

    void initialize(float radius);
};
