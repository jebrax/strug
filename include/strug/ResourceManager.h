#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glade/util/ResourceManager.h>
#include <glade/util/Path.h>
#include <glade/math/Vector.h>

namespace Strug
{
  class ResourceManager: public Glade::ResourceManager
  {
    public:
      ResourceManager(FileManager *file_manager);
  };
}
