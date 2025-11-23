#pragma once
#include "../../../../BehaviorTree/Core/BTNode.h"
#include "../../../../BehaviorTree/Core/BTBlackboard.h"

/// <summary>
/// アクション選択条件ノード
/// ActionCounterの値に基づいて成功/失敗を返す
/// </summary>
class BTActionSelector : public BTNode {
public:
    /// <summary>
    /// 期待するアクションタイプ
    /// </summary>
    enum class ActionType {
        Dash = 0,   // 偶数の場合
        Shoot = 1   // 奇数の場合
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="type">期待するアクションタイプ</param>
    explicit BTActionSelector(ActionType type);

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTActionSelector() = default;

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
    // 期待するアクションタイプ
    ActionType expectedType_;
};