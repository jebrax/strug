#include <strug/states/world/ChunkManager.h>
#include <strug/blocks/Isosurface.h>

#include <pthread.h>
#include <unistd.h>

typedef void * (*THREADFUNCPTR)(void *);

void ChunkManager::GenerateNewChunks(Glade::Vector2i &centralChunkIndex)
{
  Glade::Vector2i checkChunkIndex;

  for (int iinc = -ChunkManager::CHUNK_GENERATION_RADIUS; iinc <= ChunkManager::CHUNK_GENERATION_RADIUS; ++iinc) {
    for (int jinc = -ChunkManager::CHUNK_GENERATION_RADIUS; jinc <= ChunkManager::CHUNK_GENERATION_RADIUS; ++jinc) {
      checkChunkIndex.x = centralChunkIndex.x + iinc;
      checkChunkIndex.y = centralChunkIndex.y + jinc;

      if (!mGrid.getChunk(checkChunkIndex)) {
        log("Generating chunk %d, %d", checkChunkIndex.x, checkChunkIndex.y);
        Isosurface *chunk = new Isosurface(nullptr, mGrid, false, checkChunkIndex);
        mGrid.addChunk(checkChunkIndex, chunk);
        chunk->InitializeByGenerating(mTerrainSettings);
        mContext.add(chunk, true);
      }
    }
  }
}

void* ChunkManager::Worker()
{
  while (!mStopRequested) {
    if (mClearRequested) {
      mGrid.walkChunks([this](Grid::ChunksI &chunki) { mContext.remove(chunki->second); });
      mGrid.clear();

      mForceRegen = true;
      mClearRequested = false;
    }

    for (auto& character: mPlayerCharacters) {
      Glade::Vector2i centralChunkIndex;
      Glade::Vector3i characterCellIndex = mGrid.pointToCellIndex(*character->getTransform()->position);
      centralChunkIndex = mGrid.cellIndexToChunkIndex(characterCellIndex);
      auto entry = mLastCharacterChunkIndices.find(character);
      bool characterMovedToANewChunk = (centralChunkIndex != entry->second);

      if (characterMovedToANewChunk) {
        log("Character enters chunk (%d, %d)", centralChunkIndex.x, centralChunkIndex.y);
        entry->second = centralChunkIndex;
      }

      if (characterMovedToANewChunk || mForceRegen) {
        GenerateNewChunks(centralChunkIndex);
      }

      mForceRegen = false;
    }

   usleep(1000 * SLEEP_MILLISEC);
  }
}

void ChunkManager::ReloadChunk(const Glade::Vector2i &chunkIndex)
{
  Isosurface* surf = (Isosurface *) mGrid.getChunk(chunkIndex.x, chunkIndex.y);

  if (surf) {
    surf->InitializeWithIsoValues();
    mContext.add(surf);
  }
}


void ChunkManager::ThreadStart(ChunkManager *instance)
{
  instance->Worker();
}

void ChunkManager::Run()
{
  if (pthread_create(&mThreadHandle, NULL, (THREADFUNCPTR) &ThreadStart, this))
    log("Warning: failed to create thread for generating chunks");
}

void ChunkManager::Stop()
{
  if (mThreadHandle) {
    mStopRequested = true;
    pthread_join(mThreadHandle, NULL);
  }
}

void ChunkManager::AddCharacter(GladeObject *character)
{
  if (character == nullptr)
    return;

  if (mPlayerCharacters.empty())
    mForceRegen = true;

  mPlayerCharacters.push_back(character);
  Glade::Vector3i characterCellIndex = mGrid.pointToCellIndex(*character->getTransform()->position);
  mLastCharacterChunkIndices[character] = mGrid.cellIndexToChunkIndex(characterCellIndex);
}
