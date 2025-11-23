#pragma once
#include "BTNode.h"
#include <vector>

/// <summary>
/// コンポジットノードの基底クラス
/// 複数の子ノードを持つノード
/// </summary>
class BTComposite : public BTNode {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTComposite() = default;

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTComposite() = default;

    /// <summary>
    /// 子ノードの追加
    /// </summary>
    /// <param name="child">追加する子ノード</param>
    void AddChild(BTNodePtr child);

    /// <summary>
    /// 子ノードの削除
    /// </summary>
    /// <param name="child">削除する子ノード</param>
    void RemoveChild(BTNodePtr child);

    /// <summary>
    /// 子ノードのクリア
    /// </summary>
    void ClearChildren();

    /// <summary>
    /// 子ノードの取得
    /// </summary>
    /// <returns>子ノードのリスト</returns>
    const std::vector<BTNodePtr>& GetChildren() const { return children_; }

    /// <summary>
    /// 子ノードの数を取得
    /// </summary>
    /// <returns>子ノードの数</returns>
    size_t GetChildCount() const { return children_.size(); }

    /// <summary>
    /// ノードのリセット
    /// </summary>
    void Reset() override;

protected:
    // 子ノードのリスト
    std::vector<BTNodePtr> children_;

    // 現在実行中の子ノードのインデックス
    size_t currentChildIndex_ = 0;
};