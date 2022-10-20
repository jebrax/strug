#pragma once

#include <glade/GladeObject.h>
#include <vector>

class Frank: public GladeObject
{
  public:
    void initialize(float scale = 1.0);
    bool toggleCollisionShapeView();

  private:
    Drawable *collisionShapeView;
};

