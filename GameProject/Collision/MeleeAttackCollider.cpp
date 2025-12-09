#include "MeleeAttackCollider.h"
#include "../Object/Player/Player.h"
#include "../Object/Boss/Boss.h"
#include "CollisionTypeIdDef.h"
#include "GlobalVariables.h"
#include "../CameraSystem/CameraManager.h"

MeleeAttackCollider::MeleeAttackCollider(Player* player)
    : player_(player) {
    // GlobalVariablesから値を取得
    GlobalVariables* gv = GlobalVariables::GetInstance();
    attackDamage_ = gv->GetValueFloat("MeleeAttack", "AttackDamage");

    SetTypeID(static_cast<uint32_t>(CollisionTypeId::PLAYER_ATTACK));
    SetActive(false);
}

void MeleeAttackCollider::OnCollisionEnter(Collider* other) {
    if (!other) return;

    uint32_t typeID = other->GetTypeID();

    if (typeID == static_cast<uint32_t>(CollisionTypeId::BOSS)) {
        Boss* enemy = static_cast<Boss*>(other->GetOwner());
        if (enemy) {

            enemy->OnHit(attackDamage_, 1.0f);

            // カメラシェイク発動（攻撃ヒット時は軽めに）
            CameraManager::GetInstance()->StartShake(0.3f);

            if (!detectedEnemy_) {
                detectedEnemy_ = enemy;
            }
        }
    }
}

void MeleeAttackCollider::OnCollisionStay(Collider* other) {
    if (!other) return;

#ifdef _DEBUG
    collisionCount_++;
#endif

    uint32_t typeID = other->GetTypeID();

    if (typeID == static_cast<uint32_t>(CollisionTypeId::BOSS)) {
        Boss* enemy = static_cast<Boss*>(other->GetOwner());
        if (enemy && !detectedEnemy_) {
            detectedEnemy_ = enemy;
            if (canDamage)
            {
                enemy->OnHit(attackDamage_, 1.0f);

                // カメラシェイク発動（攻撃ヒット時は軽めに）
                CameraManager::GetInstance()->StartShake(0.3f);

                canDamage = false;
            }
        }
    }
}

void MeleeAttackCollider::Reset() {
    hitEnemies_.clear();
    detectedEnemy_ = nullptr;
#ifdef _DEBUG
    collisionCount_ = 0;
#endif
}

void MeleeAttackCollider::Damage()
{
    canDamage = true;
}

bool MeleeAttackCollider::HasHitEnemy(uint32_t enemyId) const {
    return hitEnemies_.find(enemyId) != hitEnemies_.end();
}