#include <strug/blocks/DebugObject.h>

void DebugObject::serialize(Glade::MemoryOutputStream &stream) {
  GladeObject::serialize(stream);
  //char buf[9504049];
  char buf[10];
  stream.Write(buf);
  log("DebugObject serialized size: %d", stream.GetWrittenSize());
}

void DebugObject::unserialize(Glade::MemoryInputStream &stream) {
  GladeObject::unserialize(stream);
}