#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <strug/states/CubeTest.h>
#include <strug/blocks/Woodenbox.h>
#include <strug/states/world/WorldController.h>
#include <strug/controls/StrugKeyMapper.h>

#include <unordered_map>

static Woodenbox* cube;

CubeTest::CubeTest():
  State(),
  mController(nullptr)
{}

CubeTest::~CubeTest()
{
}

void CubeTest::createEntities()
{
  cube = new Woodenbox(&GladeObject::unusedEntityId);
  cube->initialize();
  this->context->add(cube);
}

void CubeTest::init(Context &context)
{
  log("Init CubeTest");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  createEntities();

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  mController = new WorldController(context);
  mController->SetKeyMapper(new StrugKeyMapper());
  mController->setCharacter(cube);
  context.setController(*mController);
}

void CubeTest::applyRules(Context &context)
{
  if (mController)
    mController->update();
}

void CubeTest::shutdown(Context &context)
{

}

GladeObject *CubeTest::CreateEntityByTypeId(unsigned int type, const unsigned int *force_id, bool replicate)
{
  return nullptr;
}
