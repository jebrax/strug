#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/generation/Grid.h>
#include <strug/blocks/Isosurface.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Isosurface::initialize(const Glade::Vector2i &chunkIndex, Grid &grid, bool empty)
{
  CubeTerrainGenerator gen;

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

    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5);
    //gen.genIsoCube(grid, *mesh, 0.5);

    view = new Drawable(mesh, program);
    addDrawable(view);
    initialized = true;
  } else {
    std::shared_ptr<Glade::Mesh> mesh = view->getMesh();
    gen.mcGenChunk(chunkIndex, grid, *mesh, 0.5, false);
    //gen.genIsoCube(grid, *mesh, 0.5);
  }
}

