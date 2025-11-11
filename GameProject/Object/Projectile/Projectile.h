#pragma once

#include "Transform.h"
#include "Vector3.h"
#include <memory>

class Object3d;
class Model;

/// <summary>
/// プロジェクタイル（弾）基底クラス
/// 移動と生存時間の管理のみを担当
/// 衝突判定は派生クラスで実装
/// </summary>
class Projectile {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    Projectile();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~Projectile();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="position">初期位置</param>
    /// <param name="velocity">初期速度</param>
    virtual void Initialize(const Vector3& position, const Vector3& velocity);

    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    virtual void Update(float deltaTime);

    /// <summary>
    /// 描画
    /// </summary>
    virtual void Draw();

    /// <summary>
    /// アクティブかどうか
    /// </summary>
    bool IsActive() const { return isActive_; }

    /// <summary>
    /// アクティブ状態を設定
    /// </summary>
    void SetActive(bool active) { isActive_ = active; }

    /// <summary>
    /// ダメージ量を取得
    /// </summary>
    float GetDamage() const { return damage_; }

    /// <summary>
    /// ダメージ量を設定
    /// </summary>
    void SetDamage(float damage) { damage_ = damage; }

    /// <summary>
    /// 速度を取得
    /// </summary>
    const Vector3& GetVelocity() const { return velocity_; }

    /// <summary>
    /// 速度を設定
    /// </summary>
    void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

    /// <summary>
    /// Transformを取得
    /// </summary>
    const Transform& GetTransform() const { return transform_; }

    /// <summary>
    /// Transform参照を取得（コライダー設定用）
    /// </summary>
    Transform* GetTransformPtr() { return &transform_; }

    /// <summary>
    /// モデルを取得
    /// </summary>
    Object3d* GetModel() const { return model_.get(); }

protected:
    /// <summary>
    /// 生存時間を更新
    /// </summary>
    void UpdateLifetime(float deltaTime);

    /// <summary>
    /// 移動処理
    /// </summary>
    virtual void Move(float deltaTime);

protected:
    /// <summary>
    /// 3Dモデルオブジェクト（描画用）
    /// </summary>
    std::unique_ptr<Object3d> model_;

    /// <summary>
    /// 座標変換情報
    /// </summary>
    Transform transform_{};

    /// <summary>
    /// アクティブフラグ
    /// </summary>
    bool isActive_ = false;

    /// <summary>
    /// 速度ベクトル
    /// </summary>
    Vector3 velocity_;

    /// <summary>
    /// ダメージ量
    /// </summary>
    float damage_ = 10.0f;

    /// <summary>
    /// 生存時間
    /// </summary>
    float lifeTime_ = 5.0f;

    /// <summary>
    /// 経過時間
    /// </summary>
    float elapsedTime_ = 0.0f;
};