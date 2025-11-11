#include "BossBulletCollider.h"
#include "../Player/Player.h"
#include "../../Collision/CollisionTypeIdDef.h"
#include "CollisionManager.h"

BossBulletCollider::BossBulletCollider(BossBullet* owner)
    : owner_(owner) {
}

void BossBulletCollider::OnCollisionEnter(Collider* other) {
    if (!owner_ || !owner_->IsActive()) {
        return;
    }

    // プレイヤーとの衝突判定
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::PLAYER)) {
        // 多重ヒット防止チェック
        void* targetPtr = other->GetOwner();
        if (hitTargets_.find(targetPtr) != hitTargets_.end()) {
            return;  // 既にヒット済み
        }

        // プレイヤーの取得
        Player* player = static_cast<Player*>(other->GetOwner());
        if (player) {
            hitPlayer_ = player;
            hitTargets_.insert(targetPtr);

            // プレイヤーにダメージを与える
            float currentHp = player->GetHp();
            player->SetHp(currentHp - owner_->GetDamage());
            hasDealtDamage_ = true;

            // 弾を非アクティブ化
            owner_->SetActive(false);

            // TODO: ヒットエフェクト生成
        }
    }
}

void BossBulletCollider::OnCollisionStay(Collider* other) {
    // BossBulletは一度の衝突で消えるため、Stayは基本的に使用しない
    // 必要に応じて実装可能
}

void BossBulletCollider::OnCollisionExit(Collider* other) {
    // 衝突終了処理（必要に応じて実装）
    if (other->GetOwner() == hitPlayer_) {
        hitPlayer_ = nullptr;
    }
}

void BossBulletCollider::Reset() {
    hitTargets_.clear();
    hitPlayer_ = nullptr;
    hasDealtDamage_ = false;
}