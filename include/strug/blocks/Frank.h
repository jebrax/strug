#pragma once

#include <glade/GladeObject.h>
#include <strug/states/world/WalkingTheWorld.h>
#include <vector>

class Frank: public GladeObject
{
public:
  explicit Frank(const unsigned int *forceObjectId):
    GladeObject(forceObjectId),
    mUserId(0),
    collisionShapeView(nullptr)
    {}

  void initialize(float scale = 1.0);
  bool toggleCollisionShapeView();
  unsigned int getType() override { return  WalkingTheWorld::ObjectType::CHARACTER; }

  void serialize(Glade::MemoryOutputStream &stream) override {
    GladeObject::serialize(stream);
    stream.Write(mUserId);
  }

  void unserialize(Glade::MemoryInputStream &stream) override {
    GladeObject::unserialize(stream);
    stream.Read(mUserId);
  }

  unsigned int mUserId;

private:
  Drawable *collisionShapeView;
};
