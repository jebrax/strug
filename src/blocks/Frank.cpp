#include <glade/render/Texture.h>
#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/TextureTransform.h>
#include <glade/render/meshes/Rectangle.h>
#include <glade/render/meshes/Cube.h>
#include <glade/render/meshes/DynamicMesh.h>
#include <glade/render/meshes/DynamicMeshGenerator.h>
#include <glade/util/Path.h>
#include <strug/blocks/Frank.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

Frank::Frank(): initialized(false)
{
  setName("Some kind of object");
}

Frank::~Frank()
{
}

void Frank::initialize()
{
  if (!initialized) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    //view = new Drawable(game_resource_manager->getMesh("geometry/cube.min.obj"), program);

    std::shared_ptr<DynamicMesh> mesh = std::make_shared<DynamicMesh>();
    DynamicMeshGenerator gen;
    gen.generateHull(*mesh);
    view = new Drawable(mesh, program);

    addDrawable(view);
    initialized = true;
  }
}

