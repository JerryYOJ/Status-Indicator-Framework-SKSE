#pragma once

#include "../config/ConfigManager.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

class IconManager : public SINGLETON<IconManager> {
public:
	void InitializeAndAttachIcons(RE::FloatingQuestMarker* thiz);
	void RenderIcons(RE::FloatingQuestMarker* thiz);
	void ShowIcon(RE::GFxValue& clip, float x, float y, float distance, float depth, float fadeStart, float fadeMax, const Config::Settings& settings);
	void HideIcon(RE::GFxValue& clip);
	void AddCachedRef(RE::TESObjectREFR* ref);
	void RemoveCachedRef(const RE::ObjectRefHandle& handle);

private:
	// max simultaneous instances of a single exported symbol
	static constexpr std::size_t kMaxPerLabel = 48;

	// label → fixed pool of kMaxPerLabel pre-attached clips
	std::unordered_map<std::string, std::array<RE::GFxValue, kMaxPerLabel>> _pools;
	RE::GFxValue _container;

	std::vector<RE::ObjectRefHandle>               _cachedRefs;
	std::unordered_map<std::uint32_t, std::size_t> _indexByHandle;
};
