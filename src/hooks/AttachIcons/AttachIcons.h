#pragma once

class AttachIcons {

public:
    static void Install();
protected:
    static void InitializeMarkerData(RE::FloatingQuestMarker* thiz);

private:
    AttachIcons() = delete;
    AttachIcons(const AttachIcons&) = delete;
    AttachIcons(AttachIcons&&) = delete;
    ~AttachIcons() = delete;

    AttachIcons& operator=(const AttachIcons&) = delete;
    AttachIcons& operator=(AttachIcons&&) = delete;

    inline static REL::Relocation<decltype(InitializeMarkerData)> _InitializeMarkerData;
};
