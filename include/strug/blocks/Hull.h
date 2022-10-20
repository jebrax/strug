#pragma once

#include <glade/GladeObject.h>

class Hull: public GladeObject
{
  public:
    float radius;

    void initialize(float radius);
};
