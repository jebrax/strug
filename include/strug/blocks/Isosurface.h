#pragma once

#include <glade/GladeObject.h>
#include <glade/generation/AdvancedMeshGenerator.h>
#include <strug/states/world/WalkingTheWorld.h>

class Grid;

class Isosurface: public GladeObject
{
public:
  Isosurface(const unsigned int *forcedId, Grid& grid, bool craftingMode = false, const Glade::Vector2i &chunkIndex = Glade::Vector2i()):
    GladeObject(forcedId),
    mGrid(grid),
    mChunkIndex(chunkIndex),
    mCraftingMode(craftingMode) {}

  void InitializeByGenerating(AdvancedMeshGenerator::TerrainGeneratorSettings &settings);
  void InitializeWithIsoValues();

  unsigned int getType() override { return  WalkingTheWorld::ObjectType::ISOSURFACE_CHUNK; }

  const Glade::Vector2i& GetChunkIndex() {
    return mChunkIndex;
  }

  void serialize(Glade::MemoryOutputStream &stream) override;
  void unserialize(Glade::MemoryInputStream &stream) override;

private:
  void InitializeDrawableAndPhysicalObject(std::shared_ptr<Glade::Mesh> mesh);

  Grid& mGrid;
  Glade::Vector2i mChunkIndex;
  bool mCraftingMode;
  float mIsolevel;
};
