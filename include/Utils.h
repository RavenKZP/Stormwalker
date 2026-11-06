#pragma once

#include "REX/REX.h"
#include "CLibUtilsQTR/Tasker.hpp"

#include <shared_mutex>

enum ElementType {
    Unknown = -1,
    Neutral = 0,
    Fire,
    Shock,
    Cold,
    Water,
};

class DetectionCache : public REX::Singleton<DetectionCache> {
public:
    std::optional<ElementType> Get(RE::FormID id) {
        std::shared_lock<std::shared_mutex> readLock(cacheMutex_);
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void Add(RE::FormID id, ElementType type) {
        std::unique_lock<std::shared_mutex> writeLock(cacheMutex_);
        cache_[id] = type;
    }

private:
    std::unordered_map<RE::FormID, ElementType> cache_;
    mutable std::shared_mutex cacheMutex_;
};

namespace Utils {

    std::string ToLower(std::string s);

    float GetDamageFromProjectile(RE::Projectile* proj);
    ElementType GetProjectileType(RE::Projectile* proj);
    ElementType GetExplosionType(RE::Explosion* exp);

    // Returns: {water_height}
    float get_water_height(const RE::NiPoint3& a_pos);

    // Returns: {actual_radius, water_height}
    std::pair<float, float> get_explosion_water_radius(const RE::Explosion* a_explosion, float base_radius);
}