#include "Hooks.h"

#include "Manager.h"
#include "Settings.h"

namespace Hooks {

    void UpdatePlayerHook::Update(RE::Actor* a_this, float a_delta) {
        Update_(a_this, a_delta);

        static auto* set = Settings::GetSingleton();
        if (!set->ModActive) {
            return;
        }

        Stormwalker::Manager::GetSingleton()->FrameUpdate(a_delta);
    }

    RE::NiAVObject* HazardLoadHook::Load3D(RE::TESObjectREFR* a_this, bool a_backgroundLoading) {
        auto res = Load3D_(a_this, a_backgroundLoading);
        Stormwalker::Manager::GetSingleton()->OnHazardLoad3D(res, a_this);
        return res;
    }

    // Projectile Updates
    void MissileProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kMissile);
        Update_(a_proj, a_delta);
    }

    void BeamProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kBeam);
        Update_(a_proj, a_delta);
    }

    void FlameProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kFlame);
        Update_(a_proj, a_delta);
    }

    void GrenadeProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kExplosion);
        Update_(a_proj, a_delta);
    }

    void ConeProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kCone);
        Update_(a_proj, a_delta);
    }

    void ArrowProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Stormwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kArrow);
        Update_(a_proj, a_delta);
    }

    // Projectile Impacts
    void MissileProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                      const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                      std::int32_t a_arg6, std::uint32_t a_arg7) {
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void BeamProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                   const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                                   std::uint32_t a_arg7) {
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void FlameProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                    const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                    std::int32_t a_arg6, std::uint32_t a_arg7) {
        a_proj->GetProjectileRuntimeData().linearVelocity = a_targetLoc - a_proj->GetPosition();
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void GrenadeProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                      const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                      std::int32_t a_arg6, std::uint32_t a_arg7) {
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void ConeProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                   const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                                   std::uint32_t a_arg7) {
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void ArrowProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                    const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                    std::int32_t a_arg6, std::uint32_t a_arg7) {
        Stormwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref, a_targetLoc);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    // Explosions
    void ExplosionHook::Initialize(RE::Explosion* a_this) {
        Stormwalker::Manager::GetSingleton()->ProcessExplosion(a_this);
        Initialize_(a_this);
    }
}  // namespace Hooks