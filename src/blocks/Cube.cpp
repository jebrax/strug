#include <glade/render/Texture.h>
#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/TextureTransform.h>
#include <glade/render/meshes/Rectangle.h>
#include <glade/render/meshes/Cube.h>
#include <glade/render/meshes/DynamicMesh.h>
#include <glade/util/Path.h>
#include <strug/blocks/Cube.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

Cube::Cube(): initialized(false)
{
  setName("Some kind of object");
}

Cube::~Cube()
{
}

void Cube::initialize()
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    view = new Drawable(game_resource_manager->getMesh("geometry/cube.min.obj"), program);

    for (int i = 0; i < view->getMesh()->getVertexBufferSize(); i += 8) {
      log("V: %f %f %f", view->getMesh()->getVertices()[i], view->getMesh()->getVertices()[i + 1], view->getMesh()->getVertices()[i + 2]);
    }

    log("");

    for (int i = 0; i < view->getMesh()->getIndexBufferSize(); i += 3) {
      log("F: %d %d %d", view->getMesh()->getIndices()[i], view->getMesh()->getIndices()[i + 1], view->getMesh()->getIndices()[i + 2]);
    }

    view->getTransform()->setRotation(-1.52f, 0.0f, 1.52f);
    view->getTransform()->setScale(0.2f, 0.2f, 0.2f);
    
    addDrawable(view);
    initialized = true;
  }
}

