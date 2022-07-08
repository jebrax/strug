#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>
#include <strug/blocks/Isosurface.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Isosurface::initialize()
{
  StrugObject::initialize();
  CubeTerrainGenerator gen;

  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    gen.marchingCubes(grid, *mesh, 0.5);
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  } else {
    std::shared_ptr<Glade::Mesh> mesh = view->getMesh();
    gen.marchingCubes(grid, *mesh, 0.5, false);
  }
}

