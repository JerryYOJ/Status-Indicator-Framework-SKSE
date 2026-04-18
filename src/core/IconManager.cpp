#include "IconManager.h"

#include "../swfhelper/ImportData.h"

#include <algorithm>
#include <string>
#include <format>


constexpr float kProjectionZeroTolerance = 1e-5f;

static float CalculateAlpha(float distance, float fadeStart, float fadeMax)
{
	float alpha = 100.0f;

	if (distance > fadeStart) {
		alpha = ((distance - fadeMax) / (fadeStart - fadeMax)) * 100.0f;
	}

	return std::clamp(alpha, 0.0f, 100.0f);
}

static float CalculateScale(float depth, const Config::Settings& s)
{
	static float* g_fNear = reinterpret_cast<float*>(RELOCATION_ID(517032, 403540).address() + 0x40);
	static float* g_fFar  = reinterpret_cast<float*>(RELOCATION_ID(517032, 403540).address() + 0x44);

	const float linearizedDepth = (*g_fNear * *g_fFar) / (*g_fFar + depth * (*g_fNear - *g_fFar));
	const float scaleMult = std::clamp(linearizedDepth, s.scaleDepthNear, s.scaleDepthFar);
	return (((scaleMult - s.scaleDepthNear) * (s.scaleMin - s.scaleMax)) / (s.scaleDepthFar - s.scaleDepthNear)) + s.scaleMax;
}

static bool GetNodePosition(RE::Actor* actor, const char* nodeName, RE::NiPoint3& point)
{
	if (!actor || !nodeName || !nodeName[0]) {
		return false;
	}

	auto* object = actor->Get3D2();
	if (!object) {
		return false;
	}

	object = object->GetObjectByName(nodeName);
	if (!object) {
		return false;
	}

	point.x = object->world.translate.x;
	point.y = object->world.translate.y;
	point.z = object->world.translate.z;
	return true;
}

static bool GetTorsoPos(RE::Actor* actor, RE::NiPoint3& point)
{
	if (!actor) {
		return false;
	}

	auto* race = actor->GetRace();
	if (!race) {
		return false;
	}

	auto* bodyPartData = race->bodyPartData;
	if (!bodyPartData) {
		return false;
	}

	auto* bodyPart = bodyPartData->parts[0];
	if (!bodyPart) {
		return false;
	}

	return GetNodePosition(actor, bodyPart->targetName.c_str(), point);
}

static bool GetTargetPos(RE::TESObjectREFR* target, RE::NiPoint3& pos, bool getTorsoPos = false)
{
	if (!target || !target->Get3D2()) {
		return false;
	}

	if (target->formType == RE::FormType::ActorCharacter) {
		auto* actor = target->As<RE::Actor>();
		if (!actor) {
			return false;
		}

		if (!getTorsoPos || !GetTorsoPos(actor, pos)) {
			pos = target->GetLookingAtLocation();
		}
	} else {
		pos = target->GetLookingAtLocation();
	}

	return true;
}

static bool ProjectToHud(RE::FloatingQuestMarker* thiz, const RE::NiPoint3& worldPos, float& x, float& y, float& depth)
{
	static auto* g_worldToCamMatrix = reinterpret_cast<float(*)[4]>(RELOCATION_ID(519579, 406126).address());
	static RE::NiRect<float>* g_viewPort =
		reinterpret_cast<RE::NiRect<float>*>(RELOCATION_ID(519618, 406160).address());

	if (!thiz) {
		return false;
	}

	float normalizedX = 0.0f;
	float normalizedY = 0.0f;
	if (!RE::NiCamera::WorldPtToScreenPt3(g_worldToCamMatrix, *g_viewPort, worldPos, normalizedX, normalizedY, depth, kProjectionZeroTolerance) || depth < 0.0f) {
		return false;
	}

	const float hudX = ((thiz->pixelXMax - thiz->pixelXMin) * normalizedX) + thiz->pixelXMin;
	const float hudY = ((thiz->pixelYMax - thiz->pixelYMin) * (1.0f - normalizedY)) + thiz->pixelYMin;

	if (hudX < thiz->TopLeftCornerPos.x ||
		hudX > thiz->TopRightCornerPos.x ||
		hudY < thiz->TopLeftCornerPos_.y ||
		hudY > thiz->TopRightCornerPos_.y) {
		return false;
	}

	x = hudX - thiz->baseClipOffsetX;
	y = hudY - thiz->baseClipOffsetY;
	return true;
}

void IconManager::InitializeAndAttachIcons(RE::FloatingQuestMarker* thiz)
{
	Config::ConfigManager::getInstance()->LoadConfigs();
	
	const auto poolSpecs = Config::ConfigManager::getInstance()->GetLoadedIcons();

	std::unordered_map<std::string, std::size_t> sourceIndex;
	std::vector<ImportData::loadReq> reqs;
	for (const auto& [label, spec] : poolSpecs) {
		auto [it, inserted] = sourceIndex.emplace(spec.source, reqs.size());
		if (inserted) reqs.push_back({ spec.source, {}, {} });
		reqs[it->second].exports.push_back(label);
	}

	ImportData::ImportResources(ImportData::GetMovieDefImpl(thiz->view->GetMovieDef()), reqs);

	if (!thiz->root.GetMember("FloatingQuestMarkerInstance", &_container) || !_container.IsDisplayObject())
		return;

	_pools.clear();

	for (const auto& [label, spec] : poolSpecs) {
		const std::size_t count = std::min(static_cast<std::size_t>(spec.maxInstances), kMaxPerLabel);
		auto& pool = _pools[label];
		for (std::size_t i = 0; i < count; ++i) {
			const auto name = std::format("sif_{}_{}", label, i);
			_container.AttachMovie(&pool[i], label.c_str(), name.c_str());
			HideIcon(pool[i]);
		}
	}
}

void IconManager::RenderIcons(RE::FloatingQuestMarker* thiz)
{
	// Hide all pool slots first
	for (auto& [label, pool] : _pools)
		for (auto& clip : pool)
			HideIcon(clip);

	auto* player        = RE::PlayerCharacter::GetSingleton();
	auto* configManager = Config::ConfigManager::getInstance();
	const auto& settings = configManager->GetSettings();

	// per-label slot counters, consumed across all actors this frame
	std::unordered_map<std::string, std::size_t> slotCounters;

	// Actors
	RE::ProcessLists::GetSingleton()->ForEachHighActor([&](RE::Actor* actor) -> RE::BSContainer::ForEachResult {
		if (!actor || actor->IsDeleted() || actor->IsDisabled())
			return RE::BSContainer::ForEachResult::kContinue;

		const auto icons = configManager->GetIcons(actor);
		if (icons.empty())
			return RE::BSContainer::ForEachResult::kContinue;

		RE::NiPoint3 worldPos;
		if (!GetTargetPos(actor, worldPos, false))
			return RE::BSContainer::ForEachResult::kContinue;

		worldPos.z += settings.actorOffsetZ;

		float x = 0.0f;
		float y = 0.0f;
		float depth = 0.0f;
		if (!ProjectToHud(thiz, worldPos, x, y, depth))
			return RE::BSContainer::ForEachResult::kContinue;

		const float distance = player->GetPosition().GetDistance(worldPos);

		// filter: collect (icon, slot) pairs, consuming counters immediately on inclusion
		std::vector<std::pair<const Config::IconData*, std::size_t>> renderable;
		for (const auto* iconData : icons) {
			if (distance > iconData->fadeMaxDistance)
				continue;
			if (!_pools.contains(iconData->label))
				continue;
			auto& slot = slotCounters[iconData->label];
			if (slot >= static_cast<std::size_t>(iconData->maxInstances))
				continue;
			renderable.emplace_back(iconData, slot++);
		}

		if (renderable.empty())
			return RE::BSContainer::ForEachResult::kContinue;

		// center the icon row horizontally on the projected position
		const float scaledSpacing = settings.iconSpacing * (CalculateScale(depth, settings) / settings.scaleMax);
		const float totalWidth = (static_cast<float>(renderable.size()) - 1.0f) * scaledSpacing;
		float iconX = x - totalWidth / 2.0f;

		for (const auto& [iconData, slot] : renderable) {
			ShowIcon(_pools[iconData->label][slot], iconX, y, distance, depth, iconData->fadeStartDistance, iconData->fadeMaxDistance, settings);
			iconX += scaledSpacing;
		}

		return RE::BSContainer::ForEachResult::kContinue;
	});

	//Load Zones
	for (const auto& handle : _cachedRefs) {
		auto ref = handle.get();
		if (!ref || ref->IsDeleted() || ref->IsDisabled())
			continue;

		const auto icons = configManager->GetIcons(ref.get());
		if (icons.empty())
			continue;

		RE::NiPoint3 worldPos;
		if (!GetTargetPos(ref.get(), worldPos))
			continue;

		const auto* baseObject = ref->GetBaseObject();
		worldPos.z += baseObject && baseObject->GetFormType() == RE::FormType::Door ?
			settings.markerOffsetZ :
			settings.genericOffsetZ;

		float x = 0.0f;
		float y = 0.0f;
		float depth = 0.0f;
		if (!ProjectToHud(thiz, worldPos, x, y, depth))
			continue;

		const float distance = player->GetPosition().GetDistance(worldPos);

		std::vector<std::pair<const Config::IconData*, std::size_t>> renderable;
		for (const auto* iconData : icons) {
			if (distance > iconData->fadeMaxDistance)
				continue;
			if (!_pools.contains(iconData->label))
				continue;
			auto& slot = slotCounters[iconData->label];
			if (slot >= static_cast<std::size_t>(iconData->maxInstances))
				continue;
			renderable.emplace_back(iconData, slot++);
		}

		if (renderable.empty())
			continue;

		const float scaledSpacing = settings.iconSpacing * (CalculateScale(depth, settings) / settings.scaleMax);
		const float totalWidth = (static_cast<float>(renderable.size()) - 1.0f) * scaledSpacing;
		float iconX = x - totalWidth / 2.0f;

		for (const auto& [iconData, slot] : renderable) {
			ShowIcon(_pools[iconData->label][slot], iconX, y, distance, depth, iconData->fadeStartDistance, iconData->fadeMaxDistance, settings);
			iconX += scaledSpacing;
		}
	}
}

void IconManager::ShowIcon(RE::GFxValue& clip, float x, float y, float distance, float depth, float fadeStart, float fadeMax, const Config::Settings& settings)
{
	if (!clip.IsDisplayObject()) {
		return;
	}

	const float scale = CalculateScale(depth, settings);

	RE::GFxValue::DisplayInfo displayInfo;
	clip.GetDisplayInfo(&displayInfo);
	displayInfo.SetX(x);
	displayInfo.SetY(y);
	displayInfo.SetRotation(0.0);
	displayInfo.SetAlpha(CalculateAlpha(distance, fadeStart, fadeMax));
	displayInfo.SetScale(scale, scale);
	clip.SetDisplayInfo(displayInfo);
}

void IconManager::HideIcon(RE::GFxValue& clip)
{
	if (!clip.IsDisplayObject()) {
		return;
	}

	RE::GFxValue::DisplayInfo displayInfo;
	clip.GetDisplayInfo(&displayInfo);
	displayInfo.SetAlpha(0.0);
	clip.SetDisplayInfo(displayInfo);
}

void IconManager::AddCachedRef(RE::TESObjectREFR* ref)
{
	if (!ref) {
		return;
	}
	const auto handle = ref->GetHandle();
	if (!handle) {
		return;
	}

	const auto key = handle.native_handle();
	if (_indexByHandle.find(key) != _indexByHandle.end()) {
		return;
	}

	_indexByHandle[key] = _cachedRefs.size();
	_cachedRefs.push_back(handle);
}

void IconManager::RemoveCachedRef(const RE::ObjectRefHandle& handle)
{
	if (!handle) {
		return;
	}

	const auto key = handle.native_handle();
	const auto it = _indexByHandle.find(key);
	if (it == _indexByHandle.end()) {
		return;
	}

	const auto index = it->second;
	const auto lastIndex = _cachedRefs.size() - 1;
	if (index != lastIndex) {
		_cachedRefs[index] = _cachedRefs[lastIndex];
		_indexByHandle[_cachedRefs[index].native_handle()] = index;
	}

	_cachedRefs.pop_back();
	_indexByHandle.erase(it);
}
