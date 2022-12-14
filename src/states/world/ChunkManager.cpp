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
        Isosurface *chunk = new Isosurface(&GladeObject::unusedEntityId);
        mGrid.addChunk(checkChunkIndex, chunk);

        chunk->initialize(checkChunkIndex, mGrid, mTerrainSettings, false);
        mContext.add(chunk);
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

    if (mPlayerCharacter) {
      Glade::Vector2i centralChunkIndex;
      Glade::Vector3i characterCellIndex = mGrid.pointToCellIndex(*mPlayerCharacter->getTransform()->position);
      centralChunkIndex = mGrid.cellIndexToChunkIndex(characterCellIndex);
      bool characterMovedToANewChunk = (centralChunkIndex != mLastCharacterChunkIndex);

      if (characterMovedToANewChunk) {
        log("Character enters chunk (%d, %d)", centralChunkIndex.x, centralChunkIndex.y);
        mLastCharacterChunkIndex = centralChunkIndex;
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
    surf->initialize(chunkIndex, mGrid, mTerrainSettings);
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

void ChunkManager::SetPlayerCharacter(GladeObject *character)
{
  if (character == nullptr) {
    mPlayerCharacter = character;
    return;
  }

  if (mPlayerCharacter == nullptr)
    mForceRegen = true;

  mPlayerCharacter = character;
  Glade::Vector3i characterCellIndex = mGrid.pointToCellIndex(*mPlayerCharacter->getTransform()->position);
  mLastCharacterChunkIndex = mGrid.cellIndexToChunkIndex(characterCellIndex);
}
