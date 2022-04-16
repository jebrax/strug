#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/render/meshes/DynamicMeshGenerator.h>
#include <strug/blocks/Frank.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Frank::initialize(float radius)
{
  StrugObject::initialize();

  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    this->radius = radius;

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    DynamicMeshGenerator gen;
    gen.generateHull(*mesh, radius, true);
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

