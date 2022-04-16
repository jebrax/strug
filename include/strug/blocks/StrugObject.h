#pragma once

#include <glade/GladeObject.h>

class StrugObject: public GladeObject
{
  protected:
    bool initialized;

  public:
    StrugObject(): initialized(false)
    {
      setName("Some kind of object");
    }

    virtual ~StrugObject() {}

    virtual void initialize() {};

    Drawable *view;
};
