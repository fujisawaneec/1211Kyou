#pragma once

#include "SphereCollider.h"
#include "BossBullet.h"
#include <unordered_set>

class BossBullet;
class Player;

/// <summary>
/// ボスの弾専用コライダー
/// MeleeAttackColliderと同じ設計パターンで実装
/// </summary>
class BossBulletCollider : public SphereCollider {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="owner">所有者（BossBullet）</param>
    BossBulletCollider(BossBullet* owner);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossBulletCollider() override = default;

    /// <summary>
    /// 衝突開始時の処理
    /// </summary>
    /// <param name="other">衝突相手のコライダー</param>
    void OnCollisionEnter(Collider* other) override;

    /// <summary>
    /// 衝突継続時の処理
    /// </summary>
    /// <param name="other">衝突相手のコライダー</param>
    void OnCollisionStay(Collider* other) override;

    /// <summary>
    /// 衝突終了時の処理
    /// </summary>
    /// <param name="other">衝突相手のコライダー</param>
    void OnCollisionExit(Collider* other) override;

    /// <summary>
    /// リセット
    /// </summary>
    void Reset();

    /// <summary>
    /// ヒットしたプレイヤーを取得
    /// </summary>
    Player* GetHitPlayer() const { return hitPlayer_; }

private:
    // 所有者（BossBullet）への参照
    BossBullet* owner_ = nullptr;

    // ヒット済みターゲット（多重ヒット防止用）
    std::unordered_set<void*> hitTargets_;

    // 現在ヒットしているプレイヤー
    Player* hitPlayer_ = nullptr;

    // ダメージを与えたかどうか
    bool hasDealtDamage_ = false;
};