#pragma once

#include <glade/GladeObject.h>

class DebugObject: public GladeObject
{
public:
  DebugObject():
    GladeObject(nullptr) {}

  void serialize(Glade::MemoryOutputStream &stream) override;
  void unserialize(Glade::MemoryInputStream &stream) override;
};
