#include <glade/render/Drawable.h>
#include <glade/render/ShaderProgram.h>
#include <strug/blocks/Cube.h>
#include <strug/ResourceManager.h>
#include <vector>

extern Strug::ResourceManager *game_resource_manager;

void Cube::initialize()
{
  if (!initialized ) {
    std::shared_ptr<ShaderProgram> program =
      game_resource_manager->getShaderProgram(
        "phong.vertex.glsl",
        "phong.fragment.glsl"
      );

    std::shared_ptr<Glade::Mesh> mesh = game_resource_manager->getMesh("geometry/cube.obj");
    view = new Drawable(mesh, program);

    for (int i = 0; i < mesh->getVertexBufferSize(); i += 3) {
      Glade::Vector3f vertex(mesh->vertices[i], mesh->vertices[i + 1], mesh->vertices[i + 2]);
      log("%f %f %f", vertex.x, vertex.y, vertex.z);
    }

    //log("Num vertex attrs: %d", mesh->vertices.size());

    addDrawable(view);
    initialized = true;
  }
}

void Cube::modifyVertexPosition(int triangleNum, int vertNum, float factor)
{
  std::shared_ptr<Glade::Mesh> mesh = view->getMesh();
  int vertAttrStartIndex = ((triangleNum - 1) * 3 + (vertNum - 1)) * 8;
  mesh->vertices[vertAttrStartIndex + 1] = (mesh->vertices[vertAttrStartIndex + 1] + 0.5) * factor - 0.5;
}

void Cube::recalculateTriangleNormal(int triangleNum)
{
  std::shared_ptr<Glade::Mesh> mesh = view->getMesh();
  int v0AttrStartIndex = ((triangleNum - 1) * 3 + 0) * 8;
  int v1AttrStartIndex = ((triangleNum - 1) * 3 + 1) * 8;
  int v2AttrStartIndex = ((triangleNum - 1) * 3 + 2) * 8;

  Glade::Vector3f v0(mesh->vertices[v0AttrStartIndex + 0], mesh->vertices[v0AttrStartIndex + 1], mesh->vertices[v0AttrStartIndex + 2]);
  Glade::Vector3f v1(mesh->vertices[v1AttrStartIndex + 0], mesh->vertices[v1AttrStartIndex + 1], mesh->vertices[v1AttrStartIndex + 2]);
  Glade::Vector3f v2(mesh->vertices[v2AttrStartIndex + 0], mesh->vertices[v2AttrStartIndex + 1], mesh->vertices[v2AttrStartIndex + 2]);

  Glade::Vector3f v01;
  v01.set(v1);
  v01.subtract(v0);

  Glade::Vector3f v02;
  v02.set(v2);
  v02.subtract(v0);

  Glade::Vector3f normal;
  v01.cross(v02, normal);
  normal.normalize();

  mesh->vertices[v0AttrStartIndex + 3] = normal.x;
  mesh->vertices[v0AttrStartIndex + 4] = normal.y;
  mesh->vertices[v0AttrStartIndex + 5] = normal.z;

  mesh->vertices[v1AttrStartIndex + 3] = normal.x;
  mesh->vertices[v1AttrStartIndex + 4] = normal.y;
  mesh->vertices[v1AttrStartIndex + 5] = normal.z;

  mesh->vertices[v2AttrStartIndex + 3] = normal.x;
  mesh->vertices[v2AttrStartIndex + 4] = normal.y;
  mesh->vertices[v2AttrStartIndex + 5] = normal.z;
}

void Cube::alterTop(float ky1, float ky2, float ky3, float ky4)
{
  // ky1 triangles: 1, 3, 4, 9, 10
  modifyVertexPosition(1, 3, ky1);
  modifyVertexPosition(3, 1, ky1);
  modifyVertexPosition(4, 1, ky1);
  modifyVertexPosition(9, 2, ky1);
  modifyVertexPosition(10, 3, ky1);

  // ky2 triangles: 3, 5, 6, 9
  modifyVertexPosition(3, 3, ky2);
  modifyVertexPosition(5, 1, ky2);
  modifyVertexPosition(6, 1, ky2);
  modifyVertexPosition(9, 3, ky2);

  // ky3 triangles: 3, 4, 6, 11, 12
  modifyVertexPosition(3, 2, ky3);
  modifyVertexPosition(4, 3, ky3);
  modifyVertexPosition(6, 2, ky3);
  modifyVertexPosition(11, 2, ky3);
  modifyVertexPosition(12, 3, ky3);

  // ky4 triangles: 1, 2, 4, 11
  modifyVertexPosition(1, 2, ky4);
  modifyVertexPosition(2, 3, ky4);
  modifyVertexPosition(4, 2, ky4);
  modifyVertexPosition(11, 3, ky4);

  // Triangle normals affected: 3, 4
  recalculateTriangleNormal(3);
  recalculateTriangleNormal(4);
}

