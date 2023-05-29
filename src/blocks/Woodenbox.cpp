#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/Texture.h>
#include <strug/blocks/Woodenbox.h>
#include <strug/ResourceManager.h>
#include <vector>

extern Strug::ResourceManager *game_resource_manager;

void Woodenbox::initialize(float scale)
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "non-pbr.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = game_resource_manager->getMesh("non-pbr/woodenbox/woodenbox_geometry.obj");
    Drawable *view = new Drawable(mesh, program);

    std::shared_ptr<Texture> diffuseMap = game_resource_manager->getTexture("non-pbr/woodenbox/woodenbox_diffuse.png");
    view->setUniform("uDiffuseMap", diffuseMap);

    std::shared_ptr<Texture> specularMap = game_resource_manager->getTexture("non-pbr/woodenbox/woodenbox_specular.png");
    view->setUniform("uSpecularMap", specularMap);

    addDrawable(view);
    initialized = true;
  }
}
