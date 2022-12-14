#include <strug/states/world/TerrainDemo.h>
#include <strug/blocks/Sphere.h>
#include <strug/blocks/Frank.h>
#include <strug/blocks/Surface.h>
#include <strug/states/world/WorldController.h>

#include <glade/Context.h>
#include <glade/render/Perception.h>
#include <glade/controls/VirtualController.h>
#include <glade/math/util.h>
#include <glade/generation/Grid.h>
#include <glade/generation/AdvancedMeshGenerator.h>
#include <glade/system.h>

#include <imgui.h>

#include <pthread.h>
#include <unordered_map>
#include <algorithm>

static Frank *character= nullptr;
static Grid* grid = nullptr;
static const float cellSize = 0.25;
static std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> octavesSettings;

TerrainDemo::TerrainDemo():
  State()
{}

TerrainDemo::~TerrainDemo()
{}

void TerrainDemo::createEntities()
{
  character = new Frank(&GladeObject::unusedEntityId);
  character->initialize(cellSize);
  context->add(character);
  Glade::Vector3i characterCellIndex = grid->pointToCellIndex(*character->getTransform()->position);
  lastCharacterChunkIndex = grid->cellIndexToChunkIndex(characterCellIndex);

  regenerateTerrain();
}

void TerrainDemo::regenerateTerrain()
{
  grid->walkChunks([this](Grid::ChunksI &chunki) { context->remove(chunki->second); });
  grid->clear();

  Glade::Vector2i checkChunkIndex;

  log("Generating chunk %d, %d", checkChunkIndex.x, checkChunkIndex.y);

  if (!grid->getChunk(checkChunkIndex)) {
    Surface *chunk = new Surface(&GladeObject::unusedEntityId);
    grid->addChunk(checkChunkIndex, chunk);

    chunk->initialize(checkChunkIndex, *grid, octavesSettings);
    context->add(chunk);
  }
}

void TerrainDemo::init(Context &context)
{
  log("Init TerrainDemo");
  this->context = &context;

  context.renderer->setBackgroundColor(0.2f, 0.1f, 0.5f);
  context.renderer->setSceneProjectionMode(Glade::Renderer::PERSPECTIVE);
 
  AdvancedMeshGenerator::TerrainGeneratorSettings settings;

  settings.maxHeight = 15.0;
  settings.power = 4.0;
  settings.wavelength = 10.0;
  octavesSettings.push_back(settings);

  grid = new Grid(160, cellSize);

  createEntities();

  context.getCollisionDetector()->setSpatialIndex(grid);

  Perception *perception = new Perception();
  context.renderer->setPerception(perception);

  controller = new WorldController(context);
  controller->setCharacter(character);

  character->getTransform()->position->x = grid->chunkSizeCells / 2.0;
  character->getTransform()->position->y = 0.0f;
  character->getTransform()->position->z = -grid->chunkSizeCells / 2.0;

  character->toggleView(false);

  controller->setCameraMode(WorldController::CameraMode::THIRD_PERSON);
  controller->flyMode = true;

  Glade::System::toggleMouseCursor(false);

  context.setController(*controller);
}

void TerrainDemo::applyRules(Context &context)
{
  controller->update();

  int i = 0;
  for (auto &settings: octavesSettings) {
    std::string windowName = std::string("Octave ") + std::to_string(i);
    ImGui::Begin(windowName.c_str());
    bool changed = false;

    changed |= ImGui::SliderFloat("Max height", &settings.maxHeight, 0.0f, 100.0f);
    changed |= ImGui::SliderFloat("Power", &settings.power, 0.0f, 10.0f);
    changed |= ImGui::SliderFloat("Wavelength", &settings.wavelength, 0.01f, 90.0f);

    if (i == 0) {
      if (ImGui::Button("Add octave")) {
        AdvancedMeshGenerator::TerrainGeneratorSettings newSettings;
        newSettings.maxHeight = 0.0;
        newSettings.power = 1.0;
        newSettings.wavelength = 40.0;
        octavesSettings.push_back(newSettings);
      }
    }

    if (changed) {
      log("Regenerating");

      regenerateTerrain();

      if (character)
        context.add(character);
    }

    ImGui::End();
    i++;
  }
}

void TerrainDemo::shutdown(Context &context)
{
  assert(grid);
  grid->walkChunks([&context](Grid::ChunksI &chunki) { context.remove(chunki->second); });
  grid->clear();
  delete grid;
  grid = nullptr;

  assert(character);
  context.remove(character);
  delete character;
  character = nullptr;
}

