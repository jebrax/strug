#include <sstream>

#include <glade/debug/log.h>
#include <glade/exception/GladeFileNotFoundException.h>
#include <strug/exception/StrugException.h>
#include <strug/ResourceManager.h>

namespace Strug
{
  ResourceManager::ResourceManager(FileManager *file_manager):
    Glade::ResourceManager(file_manager)
  {}
}

