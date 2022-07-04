#pragma once

#include <strug/blocks/StrugObject.h>
#include <vector>

class Cube: public StrugObject
{
  public:
    virtual void initialize();
    void alterTop(float ky1, float ky2, float ky3, float ky4);

  private:
    void modifyVertexPosition(int triangleNum, int vertNum, float factor);
    void recalculateTriangleNormal(int triangleNum);
};
