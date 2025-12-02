#pragma once

#include "Projectile.h"
#include "../../../GameProject/Collision/CollisionTypeIdDef.h"
#include <memory>
#include <string>

class EmitterManager;
class ModelManager;
class BossBulletCollider;

/// <summary>
/// ボスの弾クラス
/// </summary>
class BossBullet : public Projectile {
    //=========================================================================================
    // 定数
    //=========================================================================================
private:
    static constexpr float kDamage = 10.0f;             ///< ダメージ量
    static constexpr float kLifetime = 5.0f;            ///< 弾の寿命（秒）
    static constexpr float kRotationSpeedMin = -10.0f;  ///< 回転速度の最小値
    static constexpr float kRotationSpeedMax = 10.0f;   ///< 回転速度の最大値
    static constexpr uint32_t kIdResetThreshold = 10000;///< ID リセット閾値
    static constexpr float kInitialScale = 0.0f;        ///< 初期スケール
    static constexpr float kYBoundaryMin = -10.0f;      ///< Y座標の下限
    static constexpr float kYBoundaryMax = 50.0f;       ///< Y座標の上限

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossBullet(EmitterManager* emittermanager);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossBullet() override;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="position">初期位置</param>
    /// <param name="velocity">初期速度</param>
    void Initialize(const Vector3& position, const Vector3& velocity) override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// コリジョンタイプIDを取得
    /// </summary>
    CollisionTypeId GetTypeId() const { return CollisionTypeId::BOSS_ATTACK; }

    /// <summary>
    /// コライダーを取得
    /// </summary>
    BossBulletCollider* GetCollider() const { return collider_.get(); }

private:
    // モデルを設定
    void SetModel();

private:
    // エフェクト用の回転速度
    Vector3 rotationSpeed_;

    // パーティクルタイマー
    float particleTimer_ = 0.0f;

    // パーティクル生成間隔
    float particleInterval_ = 0.05f;

    // 専用コライダー
    std::unique_ptr<BossBulletCollider> collider_;

    // エミッタマネージャャーへのポインタ
    EmitterManager* emitterManager_ = nullptr;

    // エミッターの名前
    std::string bulletEmitterName_ = "";
    std::string explodeEmitterName_ = "";

    // id
    static uint32_t id;
};