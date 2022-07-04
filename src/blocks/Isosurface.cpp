#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/render/meshes/CubeTerrainGenerator.h>
#include <strug/blocks/Isosurface.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Isosurface::initialize()
{
  StrugObject::initialize();

  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    CubeTerrainGenerator gen;
    gen.marchingCubes(*mesh);
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

