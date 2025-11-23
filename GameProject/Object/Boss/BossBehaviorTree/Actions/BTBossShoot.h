#pragma once
#include "../../../../BehaviorTree/Core/BTNode.h"
#include "../../../../BehaviorTree/Core/BTBlackboard.h"
#include "Vector3.h"

class Boss;

/// <summary>
/// ボスの射撃アクションノード
/// </summary>
class BTBossShoot : public BTNode {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTBossShoot();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTBossShoot() = default;

    /// <summary>
    /// ノードの実行
    /// </summary>
    /// <param name="blackboard">ブラックボード</param>
    /// <returns>実行結果</returns>
    BTNodeStatus Execute(BTBlackboard* blackboard) override;

    /// <summary>
    /// ノードのリセット
    /// </summary>
    void Reset() override;

private:
    /// <summary>
    /// 射撃パラメータの初期化
    /// </summary>
    /// <param name="boss">ボス</param>
    void InitializeShoot(Boss* boss);

    /// <summary>
    /// プレイヤーを狙う処理
    /// </summary>
    /// <param name="boss">ボス</param>
    /// <param name="deltaTime">経過時間</param>
    void AimAtPlayer(Boss* boss, float deltaTime);

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

    // 射撃前の準備時間
    float chargeTime_ = 0.5f;

    // 射撃後の硬直時間
    float recoveryTime_ = 0.5f;

    // 状態の総時間
    float totalDuration_ = 1.0f;

    // 弾が発射済みかどうか
    bool hasFired_ = false;

    // 弾の速度
    float bulletSpeed_ = 20.0f;

    // 扇状発射の角度（ラジアン）
    float spreadAngle_ = 0.2618f; // 約15度

    // 経過時間
    float elapsedTime_ = 0.0f;

    // 初回実行フラグ
    bool isFirstExecute_ = true;
};