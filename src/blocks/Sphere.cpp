#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/generation/MeshGenerator.h>
#include <strug/blocks/Sphere.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Sphere::initialize(float radius)
{
  StrugObject::initialize();
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    //view = new Drawable(game_resource_manager->getMesh("geometry/sphere.obj"), program);

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    MeshGenerator gen;
    gen.generateHull(*mesh, radius, true);
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

