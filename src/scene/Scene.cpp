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

void Scene::onWindowShown(const int width, const int height) {
	resetFrameBuffer(width, height);
}

void Scene::onWindowResized(const int width, const int height) {
	resetFrameBuffer(width, height);
}

void Scene::resetFrameBuffer(const int width, const int height) {
	_entities.clear();
	_uniqueId = 0;

	// Cambia el tamaño de los pixeles teniendo encuenta lo mas pequeño ancho o alto
	int pixelSize = std::min(height, width) / 16;

	//Centra el FrameBuffer en medio lo mas grande posible 
	SDL_Rect frameBufferRect{
		.x = (width - pixelSize * 16) / 2,
		.y = (height - pixelSize * 16) / 2,
		.w = pixelSize * 16,
		.h = pixelSize * 16,
	};

	for (int i = 0; i < _frameBuffer.size(); ++i) {
		const SDL_Point pixelCoords{ i % 16 , i / 16 };

		// localiza los pixeles a partir de las coordenadas y propiedades del framebuffer
		const SDL_Point pixelPosition{
			.x = frameBufferRect.x + pixelCoords.x * pixelSize,
			.y = frameBufferRect.y + pixelCoords.y * pixelSize,
		};

		const SDL_Rect pixelRect = {
			.x = pixelPosition.x,
			.y = pixelPosition.y,
			.w = pixelSize,
			.h = pixelSize,
		};

		addEntity(Entity(consumeId(), "Pixel", pixelRect, _frameBuffer.at(i)));
	}
}

void Scene::initializeFrameBuffer() {
	ImColor Void = { 255,255,255,50 };
	ImColor Black = { 0,0,0,255 };
	ImColor White = { 255,255,255,255 };
	ImColor Green = { 28,148,134,255 };
	ImColor Red = { 206,52,52,255 };

	_frameBuffer = {
		Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,
		Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Black,Black,Void,Void,
		Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Black,Black,Green,Green,Black,Void,
		Void,Void,Void,Void,Void,Void,Void,Void,Black,Black,Green,Green,Green,Green,Black,Void,
		Void,Void,Void,Void,Void,Void,Void,Black,Green,Green,Black,Green,Black,Black,Void,Void,
		Void,Void,Void,Black,Black,Black,Black,Green,Black,Black,Black,Green,Black,Void,Void,Void,
		Void,Void,Black,Red,Red,Red,Red,Black,Black,Black,Green,Black,Void,Void,Void,Void,
		Void,Black,Red,Red,Red,Red,Red,Red,Black,Green,Black,Black,Void,Void,Void,Void,
		Void,Black,Red,Red,Red,Red,Red,Black,Red,Red,Red,Red,Black,Void,Void,Void,
		Void,Black,Red,White,Red,Red,Black,Red,Red,Red,Red,Red,Red,Black,Void,Void,
		Void,Black,Red,Red,White,White,Black,Red,Red,Red,Red,Red,Red,Black,Void,Void,
		Void,Void,Black,Red,Red,Red,Black,Red,White,Red,Red,Red,Red,Black,Void,Void,
		Void,Void,Void,Black,Black,Black,Black,Red,Red,White,White,Red,Red,Black,Void,Void,
		Void,Void,Void,Void,Void,Void,Void,Black,Red,Red,Red,Red,Black,Void,Void,Void,
		Void,Void,Void,Void,Void,Void,Void,Void,Black,Black,Black,Black,Void,Void,Void,Void,
		Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,Void,
	};
}

int Scene::consumeId() {
	return _uniqueId++;
}