#pragma once

#include <random>
#include <chrono>
#include <algorithm>

#include "REX/REX.h"
#include "Settings.h"
#include "Utils.h"

#include "ClibUtil/editorID.hpp"
// #include "CLibUtilsQTR/DrawDebug.hpp"

inline constexpr float M_PI_F = 3.14159265358979323846f;

enum TYPE : std::uint32_t {
    kMissile = 0,
    kFlame,
    kCone,
    kArrow,
    kBeam,
    kExplosion,
};

enum HazardIntensity { kNone = 0, kWeak, kMinor, kModerate, kStrong, kSevere, kExtreme };

struct HazardData {
    RE::ObjectRefHandle HazardHandle;
    HazardIntensity Intensity;
};

namespace Stormwalker {
    class Manager : public REX::Singleton<Manager> {
    public:
        Manager() {
            auto Stormwalker_ShockHazard01 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard01");
            auto Stormwalker_ShockHazard02 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard02");
            auto Stormwalker_ShockHazard03 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard03");
            auto Stormwalker_ShockHazard04 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard04");
            auto Stormwalker_ShockHazard05 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard05");
            auto Stormwalker_ShockHazard06 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Stormwalker_ShockHazard06");

            if (Stormwalker_ShockHazard01 && Stormwalker_ShockHazard02 && Stormwalker_ShockHazard03 &&
                Stormwalker_ShockHazard04 && Stormwalker_ShockHazard05 && Stormwalker_ShockHazard06) {
                ShockHazardForm01 = Stormwalker_ShockHazard01;
                ShockHazardForm02 = Stormwalker_ShockHazard02;
                ShockHazardForm03 = Stormwalker_ShockHazard03;
                ShockHazardForm04 = Stormwalker_ShockHazard04;
                ShockHazardForm05 = Stormwalker_ShockHazard05;
                ShockHazardForm06 = Stormwalker_ShockHazard06;
            } else {
                logger::error("Failed to find Shock Hazard form. Please ensure Stormwalker.esp is loaded.");
                Settings::GetSingleton()->ModActive = false;
            }
        };

        static float RandomFloat(float min, float max) {
            static thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_real_distribution<float> dist(min, max);
            return dist(rng);
        }

        static int RandomInt(int min, int max) {
            static thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_int_distribution<int> dist(min, max);
            return dist(rng);
        }

        HazardIntensity GetShockHazardIntensity(RE::TESObjectREFR* a_this) {
            HazardIntensity ret{HazardIntensity::kNone};
            auto baseObj = a_this->GetBaseObject();
            if (baseObj == ShockHazardForm01) {
                ret = HazardIntensity::kWeak;
            } else if (baseObj == ShockHazardForm02) {
                ret = HazardIntensity::kMinor;
            } else if (baseObj == ShockHazardForm03) {
                ret = HazardIntensity::kModerate;
            } else if (baseObj == ShockHazardForm04) {
                ret = HazardIntensity::kStrong;
            } else if (baseObj == ShockHazardForm05) {
                ret = HazardIntensity::kSevere;
            } else if (baseObj == ShockHazardForm06) {
                ret = HazardIntensity::kExtreme;
            }
            return ret;
        }

        void OnHazardLoad3D(RE::NiAVObject* a_obj, RE::TESObjectREFR* a_ref) {
            HazardIntensity hazInt = GetShockHazardIntensity(a_ref);
            if (hazInt != HazardIntensity::kNone) {
                ActiveShockHazards[a_ref->GetFormID()] = {a_ref->GetHandle(), hazInt};
                a_obj->local.rotate.entry[2][2] = 0.5f;
            }
        }

        void FrameUpdate(float a_delta) {
            auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            auto HazardsToRemove = std::vector<RE::FormID>{};

            for (auto HazardHandle : ActiveShockHazards) {
                if (auto Ref = HazardHandle.second.HazardHandle.get().get()) {
                    if (auto hazardRef = Ref->As<RE::Hazard>()) {
                        float remaningTime =
                            hazardRef->GetHazardRuntimeData().lifetime - hazardRef->GetHazardRuntimeData().age;

                        if (remaningTime < 5.0f) {
                            if (auto Obj3D = Ref->Get3D()) {
                                if (auto fade = Obj3D->AsFadeNode()) {
                                    fade->GetRuntimeData().currentFade = remaningTime / 5.0f;
                                }
                            }
                        }
                    }
                } else {
                    HazardsToRemove.push_back(HazardHandle.first);
                }
            }

            for (auto hazard : HazardsToRemove) {
                ActiveShockHazards.erase(hazard);
            }
        }

        // Based on po3's Splashes-of-Skyrim
        // https://github.com/powerof3/Splashes-of-Skyrim
        void ProjectileUpdate(RE::Projectile* a_projectile, TYPE type) {
            if (!a_projectile) {
                return;
            }
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            if (!a_projectile->IsDisabled() && !a_projectile->IsDeleted()) {
                auto damageType = Utils::GetProjectileType(a_projectile);
                if (damageType == ElementType::Shock) {
                    auto& runtimeData = a_projectile->GetProjectileRuntimeData();
                    RE::NiPoint3 startPos = a_projectile->GetPosition();

                    const auto waterHeight = Utils::get_water_height(startPos);

                    float projectileHeight = a_projectile->GetHeight();
                    if (projectileHeight <= 0.0f) {
                        projectileHeight = 1.0f;  // Default height if not set
                    }
                    if (type == kFlame || type == kBeam) {
                        RE::NiPoint3 endPos;
                        if (type == kFlame) {
                            RE::NiPoint3 linearVelocity;
                            a_projectile->GetLinearVelocity(linearVelocity);
                            linearVelocity.Unitize();

                            auto range = runtimeData.range;
                            endPos = startPos + linearVelocity * range;
                        } else {
                            const auto root = a_projectile->Get3D();
                            if (!root) {
                                return;
                            }
                            const auto beamEnd = root->GetObjectByName("BeamEnd");
                            if (!beamEnd) {
                                logger::warn("BeamEnd node not found for projectile {}", a_projectile->GetFormID());
                                return;
                            }
                            endPos = beamEnd->world.translate;
                        }

                        if (endPos.z != startPos.z) {
                            const auto t = (waterHeight - startPos.z) / (endPos.z - startPos.z);
                            endPos.x = (endPos.x - startPos.x) * t + startPos.x;
                            endPos.y = (endPos.y - startPos.y) * t + startPos.y;
                        }

                        if (!(waterHeight == -RE::NI_INFINITY)) {
                            const auto level = (waterHeight - endPos.z) / projectileHeight;
                            if (level >= 0.1f) {
                                endPos.z = waterHeight;
                                auto damage = Utils::GetDamageFromProjectile(a_projectile);
                                spawnElectroHazard(endPos, damage);
                            }
                        }
                    } else {
                        if (!(waterHeight == -RE::NI_INFINITY)) {
                            float treshold = a_projectile->GetHeight() + 50.0f;
                            if (startPos.z > waterHeight - treshold && startPos.z < waterHeight + treshold) {
                                RE::NiPoint3 spawnPos{startPos.x, startPos.y, waterHeight};
                                auto damage = Utils::GetDamageFromProjectile(a_projectile);
                                spawnElectroHazard(spawnPos, damage);
                                // if (it != lastProcessTime.end()) {
                                //     it->second = now;
                                // } else {
                                //     lastProcessTime[projFormID] = now;
                                // }
                            }
                        }
                    }
                }
            }
        }

        void ProjectileImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, RE::NiPoint3 ImpactPos) {
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }
            if (!a_proj) {
                return;
            }

            if (!a_proj->IsDisabled() && !a_proj->IsDeleted()) {
                auto damageType = Utils::GetProjectileType(a_proj);
                if (damageType == ElementType::Shock) {
                    RE::NiPoint3 PositionOfImpact = ImpactPos;

                    if (a_ref) {
                        if (a_ref->IsActor()) {
                            PositionOfImpact = a_ref->GetPosition();
                        }
                    }

                    const auto waterHeight = Utils::get_water_height(PositionOfImpact);
                    if (waterHeight > PositionOfImpact.z) {
                        auto SpawnPos = PositionOfImpact;
                        SpawnPos.z = waterHeight;
                        float damage = Utils::GetDamageFromProjectile(a_proj);
                        spawnElectroHazard(SpawnPos, damage);
                    }
                }
            }
        }

        // void RemoveProjectile(RE::Projectile* a_projectile) { lastProcessTime.erase(a_projectile->GetFormID()); }

        void ProcessExplosion(RE::Explosion* a_explosion) {
            if (!a_explosion) {
                return;
            }
            // auto start = std::chrono::high_resolution_clock::now();
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            auto damageType = Utils::GetExplosionType(a_explosion);
            if (damageType == ElementType::Shock) {
                auto spawnPos = a_explosion->GetPosition();
                auto& explosionRuntimeData = a_explosion->GetExplosionRuntimeData();
                float radius = std::max(explosionRuntimeData.radius, 50.0f);

                auto [waterAffectedRadius, waterHeight] = Utils::get_explosion_water_radius(a_explosion, radius);

                if (waterHeight != -RE::NI_INFINITY && waterAffectedRadius > 0.0f) {
                    spawnPos.z = waterHeight;

                    spawnElectroHazard(spawnPos, waterAffectedRadius / 5.0f);
                }

                if (waterAffectedRadius > 250.0f) {
                    int hazardsToSpawn = waterAffectedRadius / 50.0f;
                    for (int i = 0; i < hazardsToSpawn; i++) {
                        auto RandomSpawnPos = spawnPos;
                        RandomSpawnPos.x += RandomFloat(-waterAffectedRadius, waterAffectedRadius);
                        RandomSpawnPos.y += RandomFloat(-waterAffectedRadius, waterAffectedRadius);
                        auto RandomPosWaterHeight = Utils::get_water_height(RandomSpawnPos);
                        if (RandomPosWaterHeight == waterHeight) {
                            float damage = waterAffectedRadius / 5.0f;
                            damage += RandomFloat(-waterAffectedRadius * 0.1f, waterAffectedRadius * 0.1f);
                            spawnElectroHazard(RandomSpawnPos, damage);
                        }
                    }
                }
            }
        }

    private:
        void spawnElectroHazard(RE::NiPoint3 SpawnPos, float damage) {
            SKSE::GetTaskInterface()->AddTask([this, SpawnPos, damage]() {

                if (damage == 0.0f) {
                    return;
                }

                float LandHght;
                if (RE::TES::GetSingleton()->GetLandHeight(SpawnPos, LandHght)) {
                    if (SpawnPos.z < LandHght) {
                        return;
                    }
                }

                HazardIntensity intens = HazardIntensity::kNone;
                RE::TESBoundObject* objToSpawn;

                constexpr float maxDamage = 150.0f;

                if (damage > maxDamage) {
                    intens = HazardIntensity::kExtreme;
                    objToSpawn = ShockHazardForm06;
                } else if (damage > maxDamage * 0.8f) {
                    intens = HazardIntensity::kSevere;
                    objToSpawn = ShockHazardForm05;
                } else if (damage > maxDamage * 0.6f) {
                    intens = HazardIntensity::kStrong;
                    objToSpawn = ShockHazardForm04;
                } else if (damage > maxDamage * 0.4f) {
                    intens = HazardIntensity::kModerate;
                    objToSpawn = ShockHazardForm03;
                } else if (damage > maxDamage * 0.2f) {
                    intens = HazardIntensity::kMinor;
                    objToSpawn = ShockHazardForm02;
                } else {
                    intens = HazardIntensity::kWeak;
                    objToSpawn = ShockHazardForm01;
                }

                if (!objToSpawn) {
                    logger::error("Shock Hazard object is null");
                    return;
                }

                for (auto& ActiveShockHazard : ActiveShockHazards) {
                    if (auto Ref = ActiveShockHazard.second.HazardHandle.get().get()) {
                        if (ActiveShockHazard.second.Intensity >= intens) {
                            auto Pos = Ref->GetPosition();
                            float distance = SpawnPos.GetDistance(Pos);
                            if (distance < 100.0f) {
                                if (auto hazardRef = Ref->As<RE::Hazard>()) {
                                    hazardRef->GetHazardRuntimeData().age =
                                        std::max(0.0f, hazardRef->GetHazardRuntimeData().age - (damage * 0.1f));
                                }
                                return;
                            }
                        }
                    }
                }

                auto spawnedRef = RE::PlayerCharacter::GetSingleton()->PlaceObjectAtMe(objToSpawn, false);
                if (!spawnedRef) {
                    logger::error("Failed to spawn Shock Hazard");
                    return;
                }
                // Random rotation around Z-axis (yaw) to add visual variety
                float yaw = RandomFloat(-M_PI_F, M_PI_F);

                auto FinalPos = SpawnPos;
                FinalPos.z -= 10.0f;

                spawnedRef->SetPosition(FinalPos);
                spawnedRef->SetAngle(RE::NiPoint3{0.0f, 0.0f, yaw});

                constexpr float minScale = 2.0f;
                constexpr float maxScale = 4.0f;

                const auto value = static_cast<float>(intens) - 1.0f;
                const auto t = value / (static_cast<float>(HazardIntensity::kExtreme) - 1.0f);

                auto scale = std::lerp(minScale, maxScale, t);
                scale += RandomFloat(-scale * 0.1f, scale * 0.1f);
                spawnedRef->SetScale(scale);

                auto spawnedHazard = spawnedRef->As<RE::Hazard>();
                float lifetime = std::clamp(damage, 5.0f, 30.0f);
                lifetime += RandomFloat(-lifetime * 0.1f, lifetime * 0.1f);
                spawnedHazard->GetHazardRuntimeData().lifetime = lifetime;

                ActiveShockHazards[spawnedRef->formID] = {spawnedRef->GetHandle(), intens};

                // Kill fishes near!
                constexpr float minRadius = 350.0f;
                constexpr float maxRadius = 750.0f;

                auto radius = std::lerp(minRadius, maxRadius, t);
                RE::TES::GetSingleton()->ForEachReferenceInRange(
                    FinalPos, radius, [this, FinalPos](RE::TESObjectREFR* ref) {
                        if (ref) {
                            if (ref->GetPositionZ() < FinalPos.z) {
                                auto set = Settings::GetSingleton();
                                auto baseObj = ref->GetBaseObject();
                                if (auto it = set->FishSwaps.find(baseObj->GetFormID()); it != set->FishSwaps.end()) {
                                    auto newBase = it->second;
                                    if (newBase) {
                                        ref->Disable();
                                        ref->Enable(false);
                                        ref->data.objectReference = newBase;
                                    }
                                }
                            }
                        }
                        return RE::BSContainer::ForEachResult::kContinue;
                    });
            });
        }

        // std::unordered_map<RE::FormID, float> lastProcessTime;

        std::unordered_map<RE::FormID, HazardData> ActiveShockHazards;

        RE::BGSHazard* ShockHazardForm01;
        RE::BGSHazard* ShockHazardForm02;
        RE::BGSHazard* ShockHazardForm03;
        RE::BGSHazard* ShockHazardForm04;
        RE::BGSHazard* ShockHazardForm05;
        RE::BGSHazard* ShockHazardForm06;
    };
}