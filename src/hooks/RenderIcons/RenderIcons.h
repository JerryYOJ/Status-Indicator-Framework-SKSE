#pragma once

class RenderIcons {
public:
    static void Install();

protected:
    static void Update(RE::FloatingQuestMarker* thiz);

private:
    RenderIcons() = delete;
    RenderIcons(const RenderIcons&) = delete;
    RenderIcons(RenderIcons&&) = delete;
    ~RenderIcons() = delete;

    RenderIcons& operator=(const RenderIcons&) = delete;
    RenderIcons& operator=(RenderIcons&&) = delete;

    inline static REL::Relocation<decltype(Update)> _Update;
};
