#pragma once

#include <glade/GladeObject.h>

class Sphere: public GladeObject
{
  public:
    Sphere(const unsigned int *force_id): GladeObject(force_id) {}
    virtual void initialize(float radius);
};
