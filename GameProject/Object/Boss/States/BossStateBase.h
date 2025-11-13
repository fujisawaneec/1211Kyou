#pragma once

class Boss;

/// <summary>
/// ボスの状態基底クラス
/// </summary>
class BossStateBase {
public:
    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BossStateBase() = default;

    /// <summary>
    /// 状態の初期化
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    virtual void Enter(Boss* boss) = 0;

    /// <summary>
    /// 状態の更新
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    virtual void Update(Boss* boss, float deltaTime) = 0;

    /// <summary>
    /// 状態の終了処理
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    virtual void Exit(Boss* boss) = 0;

    /// <summary>
    /// 状態名の取得
    /// </summary>
    /// <returns>状態名</returns>
    virtual const char* GetName() const = 0;

    /// <summary>
    /// 状態タイマーの取得
    /// </summary>
    /// <returns>状態タイマー</returns>
    float GetStateTimer() const { return stateTimer_; }

protected:
    /// <summary>
    /// 状態タイマー
    /// </summary>
    float stateTimer_ = 0.0f;
};