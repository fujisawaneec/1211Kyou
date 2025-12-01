#pragma once
#include "../../../BehaviorTree/Core/BTNode.h"
#include "../../../BehaviorTree/Core/BTBlackboard.h"
#include <memory>
#include <json.hpp>
#include <unordered_set>

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

    /// <summary>
    /// ルートノードの取得（エディタ用）
    /// </summary>
    /// <returns>ルートノード</returns>
    BTNodePtr GetRootNode() const { return rootNode_; }

    /// <summary>
    /// ルートノードを外部から設定
    /// </summary>
    /// <param name="rootNode">新しいルートノード</param>
    void SetRootNode(BTNodePtr rootNode);

    /// <summary>
    /// ブラックボードの取得（ノード初期化用）
    /// </summary>
    /// <returns>ブラックボード</returns>
    BTBlackboard* GetBlackboard() const { return blackboard_.get(); }

    /// <summary>
    /// JSONファイルからツリーを読み込み
    /// </summary>
    /// <param name="filepath">JSONファイルのパス</param>
    /// <returns>成功したらtrue</returns>
    bool LoadFromJSON(const std::string& filepath);

    /// <summary>
    /// 現在実行中のノードを取得
    /// </summary>
    /// <returns>実行中のノード（なければnullptr）</returns>
    BTNodePtr GetCurrentRunningNode() const { return currentRunningNode_; }

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

    /// <summary>
    /// JSONからノードツリーを再帰的に構築
    /// </summary>
    /// <param name="nodeJson">ノードのJSON</param>
    /// <param name="nodeMap">全ノードのマップ</param>
    /// <param name="links">リンク情報</param>
    /// <param name="visitedNodes">訪問済みノードセット</param>
    /// <returns>構築したノード</returns>
    BTNodePtr BuildNodeFromJSON(const nlohmann::json& nodeJson,
                                const std::unordered_map<int, nlohmann::json>& nodeMap,
                                const std::vector<nlohmann::json>& links,
                                std::unordered_set<int>& visitedNodes);

    /// <summary>
    /// 実行中のノードを再帰的に検索
    /// </summary>
    /// <param name="node">検索開始ノード</param>
    void FindRunningNodeRecursive(const BTNodePtr& node);

    // ルートノード
    BTNodePtr rootNode_;

    // ブラックボード
    std::unique_ptr<BTBlackboard> blackboard_;

    // 現在のノード名
    std::string currentNodeName_;

    // 実行中ノード追跡用
    BTNodePtr currentRunningNode_;
};