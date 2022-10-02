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
    StrugObject::initialize();

    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();

    getTransform()->position->x = chunkIndex.x * grid.chunkSizeCoords;
    getTransform()->position->z = chunkIndex.y * grid.chunkSizeCoords;

    if (crafting_mode) {
      getTransform()->position->x = grid.chunkSizeCoords / 2;
      getTransform()->position->y = 20 * grid.cellSize;
      getTransform()->position->z = grid.chunkSizeCoords / 2;
    }

    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5, true, crafting_mode ? AdvancedMeshGenerator::CENTER_CELL_ONLY : AdvancedMeshGenerator::ISOSURFACE_HEIGHTMAP);

    view = new Drawable(mesh, program);
    addDrawable(view);

    PhysicalObject *phy = new IsosurfaceCellPhysicalObject(this);
    setPhysicalObject(*phy);

    initialized = true;
  } else {
    std::shared_ptr<Glade::Mesh> mesh = view->getMesh();
    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5, false, crafting_mode ? AdvancedMeshGenerator::CENTER_CELL_ONLY : AdvancedMeshGenerator::ISOSURFACE_HEIGHTMAP);
  }
}

