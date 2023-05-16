#include "glade/generation/AdvancedMeshGenerator.h"
#include <strug/blocks/Isosurface.h>
#include <strug/ResourceManager.h>

#include <glade/generation/Grid.h>
#include <glade/physics/IsosurfaceCellPhysicalObject.h>
#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>

extern Strug::ResourceManager *game_resource_manager;

void Isosurface::InitializeDrawableAndPhysicalObject(std::shared_ptr<Glade::Mesh> mesh)
{
  std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
          "phong.vertex.glsl",
          "phong.fragment.glsl"
      );

  Drawable *view = new Drawable(mesh, program);
  addDrawable(view);

  PhysicalObject *phy = new IsosurfaceCellPhysicalObject(this);
  setPhysicalObject(*phy);

  Glade::Vector3f chunkPoint = mGrid.chunkPoint(mChunkIndex);
  getTransform()->position->x = chunkPoint.x;
  getTransform()->position->z = chunkPoint.z;

}

void Isosurface::InitializeWithIsoValues()
{
  std::shared_ptr<Glade::Mesh> mesh;

  if (initialized)
     mesh = getView()->getMesh();
  else
     mesh = std::make_shared<Glade::Mesh>();

  AdvancedMeshGenerator gen;
  gen.UpdateMeshVerticesFromGridIsoValues(mChunkIndex, mGrid, *mesh, mIsolevel);

  InitializeDrawableAndPhysicalObject(mesh);
  initialized = true;
}

void Isosurface::InitializeByGenerating(AdvancedMeshGenerator::TerrainGeneratorSettings &settings)
{
  std::shared_ptr<Glade::Mesh> mesh;

  if (initialized)
    mesh = getView()->getMesh();
  else
    mesh = std::make_shared<Glade::Mesh>();

  AdvancedMeshGenerator gen;

  if (mCraftingMode)
    settings.type = AdvancedMeshGenerator::CENTER_CELL_ONLY;

  mIsolevel = settings.isolevel;
  gen.mcGenChunk(mChunkIndex, mGrid, settings);
  gen.UpdateMeshVerticesFromGridIsoValues(mChunkIndex, mGrid, *mesh, mIsolevel);

  InitializeDrawableAndPhysicalObject(mesh);
  initialized = true;
}

void Isosurface::serialize(Glade::MemoryOutputStream &stream) {
  GladeObject::serialize(stream);
  stream.Write(mChunkIndex);
  stream.Write(mCraftingMode);
  stream.Write(mIsolevel);

  for (int i = 0; i < mGrid.chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < mGrid.chunkSizeCells; k++) {
        Glade::Vector3i relativeCellIndex(i, j, k);
        Glade::Vector3i absoluteCellIndex = mGrid.absoluteCellIndex(relativeCellIndex, mChunkIndex);
        Grid::Cell &cell = mGrid.getOrCreateCell(absoluteCellIndex);

        cell.serialize(stream);
      }
    }
  }

  log("Isosurface serialized size: %d", stream.GetWrittenSize());
}

void Isosurface::unserialize(Glade::MemoryInputStream &stream)
{
  GladeObject::unserialize(stream);
  stream.Read(mChunkIndex);
  stream.Read(mCraftingMode);
  stream.Read(mIsolevel);

  for (int i = 0; i < mGrid.chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < mGrid.chunkSizeCells; k++) {
        Glade::Vector3i relativeCellIndex(i, j, k);
        Glade::Vector3i absoluteCellIndex = mGrid.absoluteCellIndex(relativeCellIndex, mChunkIndex);
        Grid::Cell &cell = mGrid.getOrCreateCell(absoluteCellIndex);

        cell.unserialize(stream);
      }
    }
  }

  InitializeWithIsoValues();
}
