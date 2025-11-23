#pragma once
#include "../Core/BTComposite.h"
#include "../Core/BTBlackboard.h"

/// <summary>
/// シーケンスノード（ANDロジック）
/// 全ての子ノードが成功するまで順に実行
/// </summary>
class BTSequence : public BTComposite {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTSequence();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTSequence() = default;

    /// <summary>
    /// ノードの実行
    /// </summary>
    /// <param name="blackboard">ブラックボード</param>
    /// <returns>実行結果</returns>
    BTNodeStatus Execute(BTBlackboard* blackboard) override;
};