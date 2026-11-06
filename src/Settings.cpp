#include "Settings.h"

#include "Utils.h"

static void LoadPatternsFromFile(const std::filesystem::path& path, std::vector<std::string>& outPatterns) {
    std::ifstream in(path);
    if (!in.is_open()) {
        logger::warn("Failed to open pattern config '{}'", path.string());
        return;
    }
    logger::info("Loading patterns from '{}'", path.string());
    std::string line;
    while (std::getline(in, line)) {
        // trim whitespace
        auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            continue;
        }
        auto last = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(first, last - first + 1);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }
        outPatterns.push_back(Utils::ToLower(trimmed));  // store lowercase
    }
}

static void LoadAllPatterns(const std::filesystem::path& folder, std::vector<std::string>& outPatterns) {
    if (!std::filesystem::exists(folder)) {
        return;
    }
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        LoadPatternsFromFile(entry.path(), outPatterns);
    }
}

void Settings::LoadFishSwaps() {

    const std::filesystem::path folderPath = "Data\\SKSE\\Plugins\\Stormwalker\\FishSwaps";

    if (!std::filesystem::exists(folderPath)) {
        return;
    }

    auto trim = [](std::string value) {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return std::string{};
        }

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    };

    for (auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            logger::warn("Failed to open FishSwaps file: {}", entry.path().string());
            continue;
        }

        logger::info("Loading fish swaps from '{}'", entry.path().string());

        std::string line;
        std::size_t lineNumber = 0;

        while (std::getline(file, line)) {
            ++lineNumber;

            if (const auto comment = line.find('#'); comment != std::string::npos) {
                line.erase(comment);
            }

            line = trim(line);
            if (line.empty()) {
                continue;
            }

            const auto separator = line.find('|');
            if (separator == std::string::npos || line.find('|', separator + 1) != std::string::npos) {
                logger::warn("Invalid FishSwap entry in {} at line {}: {}", entry.path().string(), lineNumber, line);
                continue;
            }

            const auto firstEditorID = trim(line.substr(0, separator));
            const auto secondEditorID = trim(line.substr(separator + 1));

            if (firstEditorID.empty() || secondEditorID.empty()) {
                logger::warn("Empty EditorID in {} at line {}", entry.path().string(), lineNumber);
                continue;
            }

            auto first = RE::TESForm::LookupByEditorID<RE::TESBoundObject>(firstEditorID);
            auto second = RE::TESForm::LookupByEditorID<RE::TESBoundObject>(secondEditorID);

            if (!first || !second) {
                logger::warn("Could not resolve FishSwap in {} at line {}: {}|{}", entry.path().string(), lineNumber,
                             firstEditorID, secondEditorID);
                continue;
            }

            FishSwaps.emplace(first->GetFormID(), second);
        }
    }
}

void Settings::LoadSettings() {

    LoadAllPatterns("Data\\SKSE\\Plugins\\Stormwalker\\NeutralSources", neutralSources);
    LoadAllPatterns("Data\\SKSE\\Plugins\\Stormwalker\\ElectricSources", electricSources);

    LoadFishSwaps();

    logger::info("Loaded {} neutral source patterns", neutralSources.size());
    logger::info("Loaded {} electric source patterns", electricSources.size());

    logger::info("Loaded {} fish swaps", FishSwaps.size());
}