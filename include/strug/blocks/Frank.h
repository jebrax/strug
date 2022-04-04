#pragma once

#include <glade/GladeObject.h>

class Frank: public GladeObject
{
  public:
    bool initialized;
    Drawable *view;

  public:
    Frank();
    virtual ~Frank();

    virtual void initialize();
};
