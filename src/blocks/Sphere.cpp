#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/meshes/DynamicMeshGenerator.h>
#include <strug/blocks/Sphere.h>
#include <strug/ResourceManager.h>

extern Strug::ResourceManager *game_resource_manager;

void Sphere::initialize()
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    //view = new Drawable(game_resource_manager->getMesh("geometry/sphere.obj"), program);

    std::shared_ptr<Glade::Mesh> mesh = std::make_shared<Glade::Mesh>();
    DynamicMeshGenerator gen;
    //gen.generateHull(*mesh, 5.0, true);
    gen.generate(*mesh);
    view = new Drawable(mesh, program);

    /*
    for (int i = 0; i < view->getMesh()->getVertexBufferSize(); i += 8) {
      log("V: %f %f %f", view->getMesh()->getVertices()[i], view->getMesh()->getVertices()[i + 1], view->getMesh()->getVertices()[i + 2]);
    }

    log("");

    for (int i = 0; i < view->getMesh()->getIndexBufferSize(); i += 3) {
      log("F: %d %d %d", view->getMesh()->getIndices()[i], view->getMesh()->getIndices()[i + 1], view->getMesh()->getIndices()[i + 2]);
    }
    */

    addDrawable(view);
    initialized = true;
  }
}

