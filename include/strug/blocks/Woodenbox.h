#pragma once

#include <glade/GladeObject.h>
#include <vector>

class Woodenbox: public GladeObject
{
  public:
    Woodenbox(const unsigned int *force_id): GladeObject(force_id) {}
    virtual void initialize(float scale = 1.0);
};

