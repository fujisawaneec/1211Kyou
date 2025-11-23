#pragma once
#include "../Core/BTComposite.h"
#include "../Core/BTBlackboard.h"

/// <summary>
/// セレクターノード（ORロジック）
/// 子ノードを順に実行し、最初に成功したところで停止
/// </summary>
class BTSelector : public BTComposite {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTSelector();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTSelector() = default;

    /// <summary>
    /// ノードの実行
    /// </summary>
    /// <param name="blackboard">ブラックボード</param>
    /// <returns>実行結果</returns>
    BTNodeStatus Execute(BTBlackboard* blackboard) override;
};