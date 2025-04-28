#include "Scene.h"

#include "EngineGetter.h"
#include "LoggerGetter.h"
#include <format>


namespace Const
{
	constexpr SDL_Rect DefaultRect{ 500, 200, 100, 100 };
	constexpr ImColor DefaultColor{ 120, 200, 120, 255 };
	constexpr SDL_Point NewEntityOffset{ 50, 50 };
}

Scene::Scene() {
	_backgroundColor = ImColor(0.45f, 0.55f, 0.60f, 1.00f);

	initializeFrameBuffer();
}

void Scene::addEntity(Entity entity) {
	_pendingEntities.push(std::make_unique<Entity>(std::move(entity)));
}

void Scene::cloneEntity(const std::unique_ptr<Entity>& entity) {
	Entity clone = entity->clone(consumeId());
	clone.addPosition(Const::NewEntityOffset);
	addEntity(clone);
}

void Scene::update() {
	std::erase_if(_entities, [](const auto& entity) { return entity->getRemoved(); });

	while (!_pendingEntities.empty()) {
		_entities.push_back(std::move(_pendingEntities.front()));
		_pendingEntities.pop();
	}

	for (const auto& entity : _entities) {
		entity->update();
	}
}

void Scene::render() const {
	const auto& renderer = Engine::Get().getRenderer();

	const ImGuiIO& io = ImGui::GetIO();
	SDL_RenderSetScale(renderer.get(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);

	renderer.setColor(_backgroundColor);
	SDL_RenderClear(renderer.get());

	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
	for (const auto& entity : _entities) {
		entity->render();
	}

	SDL_RenderCopy(renderer.get(), _frameBuffer.get(), nullptr, nullptr);
}

void Scene::renderImGui() {
	ImGui::ColorEdit3("Background", &_backgroundColor.Value.x);

	auto [x, y] = Engine::Get().getWindow().getSize();
	const auto text = std::format("Scene width and height: {} x {}", x, y);
	ImGui::Text("%s", text.c_str());

	ImGui::Separator();
	if (!ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen)) return;

	const auto cloneColor = static_cast<ImVec4>(ImColor{ 25, 100, 25, 255 });
	const auto removeColor = static_cast<ImVec4>(ImColor{ 128, 30, 10, 255 });

	for (const auto& entity : _entities) {
		std::string nameId = entity->getNameId();

		const bool showEntity = ImGui::TreeNode(nameId.c_str());

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, cloneColor);
		ImGui::SmallButton("Clone");
		if (ImGui::IsItemClicked()) {
			Log::Debug(std::format("{} Clone Button Clicked", nameId));
			cloneEntity(entity);
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, removeColor);
		ImGui::SmallButton("Remove");
		if (ImGui::IsItemClicked()) {
			Log::Debug(std::format("{} Remove Button Clicked", nameId));
			entity->remove();
		}
		ImGui::PopStyleColor();

		if (showEntity) {
			entity->renderImGui();
			ImGui::TreePop();
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Scene::onWindowShown(const int width, const int height) {
	resetFrameBuffer(width, height);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Scene::onWindowResized(const int width, const int height) {
	resetFrameBuffer(width, height);
}

void Scene::resetFrameBuffer(const int width, const int height) {
	_frameBuffer = std::unique_ptr<SDL_Texture, TextureDestroyer>(
		SDL_CreateTexture(Engine::Get().getRenderer().get(),
			SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
			width, height));

	void* pixels;
	int pitch; // The number of bytes in a row of pixel data
	SDL_LockTexture(_frameBuffer.get(), nullptr, &pixels, &pitch);
	const auto pixelData = static_cast<Uint32*>(pixels);

	Log::Debug(std::format("pitch: {}", pitch));

	// TODO: Set the proper frame buffer size
	const int frameBufferSize = height * width;
	for (int i = 0; i < frameBufferSize; ++i) {
		// TODO: Find the pixel coordinates for each index
		const SDL_Point pixelCoords{
			.x = i % 1200,
			.y = i / 1200,
		};

		int division_x=num_division * pixelcoord
		// TODO: Make a red gradient in the 'x' axis and a green gradient in the 'y' axis
		ImColor color{
			pixelCoords.x *  255/ 1200,
			 0,
			 0,
			 255,
		};

		pixelData[i] = static_cast<ImU32>(color);
	}

	SDL_UnlockTexture(_frameBuffer.get());
}

int Scene::consumeId() {
	return _uniqueId++;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Scene::initializeFrameBuffer() {}

void Scene::TextureDestroyer::operator()(SDL_Texture* texture) const {
	SDL_DestroyTexture(texture);
}
