#include <strug/blocks/Isosurface.h>
#include <strug/ResourceManager.h>

#include <glade/generation/AdvancedMeshGenerator.h>
#include <glade/generation/Grid.h>
#include <glade/physics/IsosurfaceCellPhysicalObject.h>
#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>

extern Strug::ResourceManager *game_resource_manager;

void Isosurface::initialize(const Glade::Vector2i &chunkIndex, Grid &grid, bool crafting_mode)
{
  AdvancedMeshGenerator gen;

  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();

    Glade::Vector3f chunkPoint = grid.chunkPoint(chunkIndex);
    getTransform()->position->x = chunkPoint.x;
    getTransform()->position->z = chunkPoint.z;

    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5, true, crafting_mode ? AdvancedMeshGenerator::CENTER_CELL_ONLY : AdvancedMeshGenerator::ISOSURFACE_HEIGHTMAP);

    Drawable *view = new Drawable(mesh, program);
    addDrawable(view);

    PhysicalObject *phy = new IsosurfaceCellPhysicalObject(this);
    setPhysicalObject(*phy);

    initialized = true;
  } else {
    std::shared_ptr<Glade::Mesh> mesh = getView()->getMesh();
    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5, false, crafting_mode ? AdvancedMeshGenerator::CENTER_CELL_ONLY : AdvancedMeshGenerator::ISOSURFACE_HEIGHTMAP);
  }
}

