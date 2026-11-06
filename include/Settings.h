#pragma once

#include <unordered_set>

#include "REX/REX.h"

class Settings : public REX::Singleton<Settings> {
public:
    Settings() { LoadSettings(); };

    void LoadSettings();
    void LoadFishSwaps();

    // Kill Switch
    bool ModActive = true;

    std::vector<std::string> neutralSources;
    std::vector<std::string> electricSources;

    std::unordered_map<RE::FormID, RE::TESBoundObject*> FishSwaps;
};