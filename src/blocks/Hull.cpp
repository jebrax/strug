#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/generation/MeshGenerator.h>
#include <strug/blocks/Hull.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Hull::initialize(float radius)
{
  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    this->radius = radius;

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    MeshGenerator gen;
    gen.generateHull(*mesh, radius, true);
    //gen.generate(*mesh);
    Drawable *view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

