#pragma once

#include <glade/GladeObject.h>

class Sphere: public GladeObject
{
  public:
    virtual void initialize(float radius);
};
