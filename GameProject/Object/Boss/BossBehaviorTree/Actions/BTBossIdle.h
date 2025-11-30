#pragma once
#include "../../../../BehaviorTree/Core/BTNode.h"
#include "../../../../BehaviorTree/Core/BTBlackboard.h"

class Boss;

/// <summary>
/// ボスの待機アクションノード
/// </summary>
class BTBossIdle : public BTNode {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTBossIdle();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTBossIdle() = default;

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

    /// <summary>
    /// 待機時間の設定
    /// </summary>
    /// <param name="duration">待機時間</param>
    void SetIdleDuration(float duration) { idleDuration_ = duration; }

    /// <summary>
    /// 待機時間の取得
    /// </summary>
    /// <returns>待機時間</returns>
    float GetIdleDuration() const { return idleDuration_; }

private:
    /// <summary>
    /// プレイヤーの方向を向く処理
    /// </summary>
    /// <param name="boss">ボス</param>
    /// <param name="deltaTime">経過時間</param>
    void LookAtPlayer(Boss* boss, float deltaTime);


    // 待機時間（次の行動までの時間）
    float idleDuration_ = 2.0f;

    // 経過時間
    float elapsedTime_ = 0.0f;

    // 初回実行フラグ
    bool isFirstExecute_ = true;
};