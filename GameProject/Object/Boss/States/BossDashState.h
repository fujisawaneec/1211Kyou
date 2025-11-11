#pragma once

#include "BossStateBase.h"
#include "Vector3.h"

/// <summary>
/// ボスのダッシュ移動状態
/// </summary>
class BossDashState : public BossStateBase {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossDashState();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossDashState() override = default;

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
    const char* GetName() const override { return "Dash"; }

private:
    /// ダッシュ方向
    Vector3 dashDirection_;

    /// ダッシュ速度
    float dashSpeed_ = 60.0f;

    /// ダッシュ時間
    float dashDuration_ = 0.5f;

    /// ダッシュ開始位置
    Vector3 startPosition_;

    /// ダッシュ目標位置
    Vector3 targetPosition_;

    /// <summary>
    /// エリア内に収まる位置を計算
    /// </summary>
    /// <param name="position">調整前の位置</param>
    /// <returns>エリア内に収まる位置</returns>
    Vector3 ClampToArea(const Vector3& position);
};