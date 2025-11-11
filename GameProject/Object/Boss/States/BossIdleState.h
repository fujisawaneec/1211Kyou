#pragma once

#include "BossStateBase.h"

/// <summary>
/// ボスの待機状態
/// </summary>
class BossIdleState : public BossStateBase {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossIdleState();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossIdleState() override = default;

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
    const char* GetName() const override { return "Idle"; }

private:
    /// <summary>
    /// 待機時間（次の行動までの時間）
    /// </summary>
    float idleDuration_ = 2.0f;

    /// <summary>
    /// 次のアクションタイプ（0: ダッシュ、1: 射撃）
    /// </summary>
    int nextActionType_ = 0;

    /// <summary>
    /// アクションカウンター（交互に行動するため）
    /// </summary>
    static int actionCounter_;
};