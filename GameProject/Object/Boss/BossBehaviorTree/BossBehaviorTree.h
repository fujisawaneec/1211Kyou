#pragma once
#include "../../../BehaviorTree/Core/BTNode.h"
#include "../../../BehaviorTree/Core/BTBlackboard.h"
#include <memory>

class Boss;
class Player;

/// <summary>
/// ボス用ビヘイビアツリー
/// </summary>
class BossBehaviorTree {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    /// <param name="player">プレイヤーのポインタ</param>
    BossBehaviorTree(Boss* boss, Player* player);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossBehaviorTree();

    /// <summary>
    /// ビヘイビアツリーの更新
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// ビヘイビアツリーのリセット
    /// </summary>
    void Reset();

    /// <summary>
    /// プレイヤーの設定
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    void SetPlayer(Player* player);

    /// <summary>
    /// デバッグ情報の取得
    /// </summary>
    /// <returns>現在実行中のノード名</returns>
    const std::string& GetCurrentNodeName() const;

private:
    /// <summary>
    /// ビヘイビアツリーの構築
    /// </summary>
    void BuildTree();

    /// <summary>
    /// 行動ツリーの構築（Idle → Dash/Shoot の選択）
    /// </summary>
    /// <returns>構築したノード</returns>
    BTNodePtr BuildActionTree();

    // ルートノード
    BTNodePtr rootNode_;

    // ブラックボード
    std::unique_ptr<BTBlackboard> blackboard_;

    // 現在のノード名（デバッグ用）
    std::string currentNodeName_;
};