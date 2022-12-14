#pragma once

#include <glade/generation/Grid.h>
#include <glade/Context.h>
#include <glade/generation/AdvancedMeshGenerator.h>

class GladeObject;

class ChunkManager
{
public:
  static constexpr int SLEEP_MILLISEC = 1000;
  static constexpr unsigned short CHUNK_GENERATION_RADIUS = 1;

  ChunkManager(Grid &grid, GladeObject *playerCharacter, Context &context):
    mContext(context),
    mGrid(grid),
    mPlayerCharacter(playerCharacter),
    mThreadHandle(0),
    mStopRequested(false),
    mClearRequested(false),
    mForceRegen(true)
  {
    mTerrainSettings.maxHeight = 1.0;
    mTerrainSettings.octaves = 6;
    mTerrainSettings.power = 4.0;
    mTerrainSettings.wavelength = 10.0;
  }

  void Run();
  void Stop();
  void RemoveAllTerrain(Context *context) { mClearRequested = true; }
  void SetPlayerCharacter(GladeObject *character);
  void GenerateNewChunks(Glade::Vector2i &centralChunkIndex);
  void ReloadChunk(const Glade::Vector2i &chunkIndex);
  void* Worker();

private:
  static void ThreadStart(ChunkManager *instance);

  Context &mContext;
  GladeObject *mPlayerCharacter;
  Grid mGrid;
  Glade::Vector2i mLastCharacterChunkIndex;
  bool mClearRequested, mForceRegen, mStopRequested;
  AdvancedMeshGenerator::TerrainGeneratorSettings mTerrainSettings;

  pthread_t mThreadHandle;
};
