#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/physics/SpherePhysicalObject.h>
#include <glade/generation/MeshGenerator.h>
#include <strug/blocks/Frank.h>
#include <strug/ResourceManager.h>
#include <vector>

extern Strug::ResourceManager *game_resource_manager;

void Frank::initialize(float scale)
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = game_resource_manager->getMesh("geometry/cube.obj");
    Drawable *view = new Drawable(mesh, program);

    view->getTransform()->scale->x = scale;
    view->getTransform()->scale->y = scale;
    view->getTransform()->scale->z = scale;

    PhysicalObject *phy = new SpherePhysicalObject(this, scale, PhysicalObject::Type::KINEMATIC);
    setPhysicalObject(*phy);

    addDrawable(view);

    // TODO get a sphere asset, why generating it?
    //view = new Drawable(game_resource_manager->getMesh("geometry/sphere.obj"), program);

    MeshGenerator gen;
    std::shared_ptr<Glade::Mesh> colShapeMesh = std::make_shared<Glade::Mesh>();
    gen.generateHull(*colShapeMesh, scale, true);
    collisionShapeView = new Drawable(colShapeMesh, program);
    addDrawable(collisionShapeView);
    toggleCollisionShape();

    initialized = true;
  }
}

bool Frank::toggleCollisionShapeView()
{
  bool isEnabled = isViewEnabled(collisionShapeView);
  toggleView(*collisionShapeView, !isEnabled);

  return !isEnabled;
}

