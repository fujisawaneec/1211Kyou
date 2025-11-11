#pragma once
#include "BossStateBase.h"
#include "Vector3.h"

/// <summary>
/// ボスの射撃攻撃状態
/// </summary>
class BossShootState : public BossStateBase {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossShootState();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossShootState() override;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    void Enter(Boss* boss) override;

    /// <summary>
    /// 状態更新処理
    /// </summary>
    void Update(Boss* boss, float deltaTime) override;

    /// <summary>
    /// 状態終了時の処理
    /// </summary>
    void Exit(Boss* boss) override;

    /// <summary>
    /// 状態名の取得
    /// </summary>
    const char* GetName() const override { return "Shoot"; }

private:
    /// <summary>
    /// 弾を発射
    /// </summary>
    /// <param name="boss">ボス</param>
    void FireBullets(Boss* boss);

    /// <summary>
    /// 弾の発射方向を計算
    /// </summary>
    /// <param name="baseDirection">基準方向</param>
    /// <param name="angleOffset">角度オフセット（ラジアン）</param>
    /// <returns>発射方向</returns>
    Vector3 CalculateBulletDirection(const Vector3& baseDirection, float angleOffset);

private:
    /// <summary>
    /// 射撃前の準備時間
    /// </summary>
    float chargeTime_ = 0.5f;

    /// <summary>
    /// 射撃後の硬直時間
    /// </summary>
    float recoveryTime_ = 0.5f;

    /// <summary>
    /// 状態の総時間
    /// </summary>
    float totalDuration_ = 1.0f;

    /// <summary>
    /// 弾が発射済みかどうか
    /// </summary>
    bool hasFired_ = false;

    /// <summary>
    /// 弾の速度
    /// </summary>
    float bulletSpeed_ = 20.0f;

    /// <summary>
    /// 扇状発射の角度（ラジアン）
    /// </summary>
    float spreadAngle_ = 0.2618f; // 約15度
};