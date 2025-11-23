#pragma once
#include <memory>
#include <vector>
#include <string>

class BTBlackboard;

/// <summary>
/// ビヘイビアツリーのノード状態
/// </summary>
enum class BTNodeStatus {
    /// <summary>
    /// 成功
    /// </summary>
    Success,

    /// <summary>
    /// 失敗
    /// </summary>
    Failure,

    /// <summary>
    /// 実行中
    /// </summary>
    Running
};

/// <summary>
/// ビヘイビアツリーノードの基底クラス
/// </summary>
class BTNode {
public:
    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTNode() = default;

    /// <summary>
    /// ノードの実行
    /// </summary>
    /// <param name="blackboard">ブラックボード</param>
    /// <returns>実行結果</returns>
    virtual BTNodeStatus Execute(BTBlackboard* blackboard) = 0;

    /// <summary>
    /// ノードのリセット
    /// </summary>
    virtual void Reset() {
        status_ = BTNodeStatus::Failure;
        isRunning_ = false;
    }

    /// <summary>
    /// ノード名の取得
    /// </summary>
    /// <returns>ノード名</returns>
    const std::string& GetName() const { return name_; }

    /// <summary>
    /// ノード名の設定
    /// </summary>
    /// <param name="name">ノード名</param>
    void SetName(const std::string& name) { name_ = name; }

    /// <summary>
    /// 現在の状態を取得
    /// </summary>
    /// <returns>現在の状態</returns>
    BTNodeStatus GetStatus() const { return status_; }

    /// <summary>
    /// 実行中かどうか
    /// </summary>
    /// <returns>実行中の場合true</returns>
    bool IsRunning() const { return isRunning_; }

protected:
    // 現在の状態
    BTNodeStatus status_ = BTNodeStatus::Failure;

    // 実行中フラグ
    bool isRunning_ = false;

    // ノード名
    std::string name_ = "BTNode";
};

/// <summary>
/// ビヘイビアツリーノードのスマートポインタ型定義
/// </summary>
using BTNodePtr = std::shared_ptr<BTNode>;