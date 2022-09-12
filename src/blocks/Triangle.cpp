#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <strug/blocks/Triangle.h>
#include <strug/ResourceManager.h>
#include <vector>

extern Strug::ResourceManager *game_resource_manager;

void Triangle::initialize()
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = game_resource_manager->getMesh("geometry/triangle-horizontal.obj");
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

