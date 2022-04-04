#pragma once

#include <glade/GladeObject.h>

class Cube: public GladeObject
{
  protected:
    bool initialized;
    Drawable *view;

  public:
    Cube();
    virtual ~Cube();

    virtual void initialize();
};
