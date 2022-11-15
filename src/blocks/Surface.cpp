#include <glade/generation/MeshGenerator.h>
#include <strug/blocks/Surface.h>
#include <strug/ResourceManager.h>

#include <glade/generation/Grid.h>
#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>

extern Strug::ResourceManager *game_resource_manager;

void Surface::initialize(const Glade::Vector2i &chunkIndex, Grid &grid, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings)
{
  MeshGenerator gen;

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

    gen.generate(*mesh, grid.chunkSizeCells, settings);

    Drawable *view = new Drawable(mesh, program);
    addDrawable(view);

    initialized = true;
  }
}

