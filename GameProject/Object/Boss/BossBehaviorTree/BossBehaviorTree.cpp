#include "BossBehaviorTree.h"
#include "../../../BehaviorTree/Core/BTComposite.h"
#include "../../../BehaviorTree/Composites/BTSelector.h"
#include "../../../BehaviorTree/Composites/BTSequence.h"
#include "Actions/BTBossIdle.h"
#include "Actions/BTBossDash.h"
#include "Actions/BTBossShoot.h"
#include "Conditions/BTActionSelector.h"
#include "../Boss.h"
#include "../../Player/Player.h"
#include <fstream>
#include <unordered_map>

BossBehaviorTree::BossBehaviorTree(Boss* boss, Player* player) {
    // ブラックボードの初期化
    blackboard_ = std::make_unique<BTBlackboard>();
    blackboard_->SetBoss(boss);
    blackboard_->SetPlayer(player);
    blackboard_->SetInt("ActionCounter", 0);

    // ツリーの構築
    //BuildTree();
    LoadFromJSON("resources/Json/BossTree.json");
}

BossBehaviorTree::~BossBehaviorTree() = default;

void BossBehaviorTree::Update(float deltaTime) {
    if (!rootNode_) {
        return;
    }

    // ブラックボードにデルタータイムを設定
    blackboard_->SetDeltaTime(deltaTime);

    // 実行前に実行中ノード情報をクリア
    currentRunningNode_ = nullptr;

    // ルートノードを実行
    BTNodeStatus status = rootNode_->Execute(blackboard_.get());

    // 実行中ノードを検索
    FindRunningNodeRecursive(rootNode_);

    // 完了したらリセット
    if (status != BTNodeStatus::Running) {
        rootNode_->Reset();
    }
}

void BossBehaviorTree::Reset() {
    if (rootNode_) {
        rootNode_->Reset();
    }
    blackboard_->SetInt("ActionCounter", 0);
}

void BossBehaviorTree::SetPlayer(Player* player) {
    blackboard_->SetPlayer(player);
}

const std::string& BossBehaviorTree::GetCurrentNodeName() const {
    return currentNodeName_;
}

void BossBehaviorTree::BuildTree() {
    // ルートノードは行動ツリー
    rootNode_ = BuildActionTree();
}

BTNodePtr BossBehaviorTree::BuildActionTree() {
    // ルートシーケンス（Idle → Action を繰り返す）
    auto rootSequence = std::make_shared<BTSequence>();
    rootSequence->SetName("MainLoop");

    // 1. Idle（待機）
    auto idleNode = std::make_shared<BTBossIdle>();
    rootSequence->AddChild(idleNode);

    // 2. Action選択（DashかShootか）
    auto actionSelector = std::make_shared<BTSelector>();
    actionSelector->SetName("ActionSelector");

    // 2-1. Dashブランチ（偶数カウンター）
    auto dashSequence = std::make_shared<BTSequence>();
    dashSequence->SetName("DashSequence");

    auto dashCondition = std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Dash);
    auto dashAction = std::make_shared<BTBossDash>();

    dashSequence->AddChild(dashCondition);
    dashSequence->AddChild(dashAction);

    // 2-2. Shootブランチ（奇数カウンター）
    auto shootSequence = std::make_shared<BTSequence>();
    shootSequence->SetName("ShootSequence");

    auto shootCondition = std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Shoot);
    auto shootAction = std::make_shared<BTBossShoot>();

    shootSequence->AddChild(shootCondition);
    shootSequence->AddChild(shootAction);

    // アクション選択にブランチを追加
    actionSelector->AddChild(dashSequence);
    actionSelector->AddChild(shootSequence);

    // ルートシーケンスにアクション選択を追加
    rootSequence->AddChild(actionSelector);

    return rootSequence;
}

/// <summary>
/// ルートノードを外部から設定
/// </summary>
void BossBehaviorTree::SetRootNode(BTNodePtr rootNode) {
    if (rootNode) {
        rootNode_ = rootNode;
        // 既存のツリーをリセット
        Reset();
        currentNodeName_ = "External Tree";
    }
}

/// <summary>
/// JSONファイルからツリーを読み込み
/// </summary>
bool BossBehaviorTree::LoadFromJSON(const std::string& filepath) {
    try {
        // JSONファイルを読み込み
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json json;
        file >> json;
        file.close();

        // バージョンチェック
        if (!json.contains("version") || json["version"] != "1.0") {
            return false;
        }

        // ノードマップを作成（ID → ノード情報）
        std::unordered_map<int, nlohmann::json> nodeMap;
        if (json.contains("nodes")) {
            for (const auto& nodeJson : json["nodes"]) {
                int nodeId = nodeJson["id"];
                nodeMap[nodeId] = nodeJson;
            }
        }

        // リンク情報を取得
        std::vector<nlohmann::json> links;
        if (json.contains("links")) {
            links = json["links"].get<std::vector<nlohmann::json>>();
        }

        // ルートノードを探す（親リンクを持たないノード）
        int rootNodeId = -1;
        std::unordered_set<int> childNodeIds;
        for (const auto& link : links) {
            childNodeIds.insert(link["targetNodeId"].get<int>());
        }

        for (const auto& [nodeId, nodeJson] : nodeMap) {
            if (childNodeIds.find(nodeId) == childNodeIds.end()) {
                rootNodeId = nodeId;
                break;
            }
        }

        if (rootNodeId == -1 && !nodeMap.empty()) {
            // ルートが見つからない場合、最初のノードをルートとする
            rootNodeId = nodeMap.begin()->first;
        }

        if (rootNodeId == -1) {
            return false;
        }

        // ツリーを再帰的に構築
        std::unordered_set<int> visitedNodes;
        rootNode_ = BuildNodeFromJSON(nodeMap[rootNodeId], nodeMap, links, visitedNodes);

        if (!rootNode_) {
            return false;
        }

        // ツリーをリセット
        Reset();
        currentNodeName_ = "Loaded from JSON";

        return true;
    }
    catch (const std::exception&) {
        // エラー処理
        return false;
    }
}

/// <summary>
/// ノードタイプからインスタンスを作成
/// </summary>
BTNodePtr BossBehaviorTree::CreateNodeByType(const std::string& nodeType) {
    // Compositeノード
    if (nodeType == "BTSelector") {
        return std::make_shared<BTSelector>();
    }
    else if (nodeType == "BTSequence") {
        return std::make_shared<BTSequence>();
    }
    // Actionノード
    else if (nodeType == "BTBossIdle") {
        return std::make_shared<BTBossIdle>();
    }
    else if (nodeType == "BTBossDash") {
        return std::make_shared<BTBossDash>();
    }
    else if (nodeType == "BTBossShoot") {
        return std::make_shared<BTBossShoot>();
    }
    // Conditionノード
    else if (nodeType == "BTActionSelector") {
        // デフォルトでDashタイプ（後でパラメータで上書き）
        return std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Dash);
    }

    return nullptr;
}

/// <summary>
/// JSONからノードツリーを再帰的に構築
/// </summary>
BTNodePtr BossBehaviorTree::BuildNodeFromJSON(
    const nlohmann::json& nodeJson,
    const std::unordered_map<int, nlohmann::json>& nodeMap,
    const std::vector<nlohmann::json>& links,
    std::unordered_set<int>& visitedNodes) {

    if (nodeJson.empty()) return nullptr;

    int nodeId = nodeJson["id"];

    // 循環参照を防ぐ
    if (visitedNodes.find(nodeId) != visitedNodes.end()) {
        return nullptr;
    }
    visitedNodes.insert(nodeId);

    // ノードを作成
    std::string nodeType = nodeJson["type"];
    BTNodePtr node = CreateNodeByType(nodeType);

    if (!node) {
        return nullptr;
    }

    // パラメータを適用
    if (nodeJson.contains("parameters") && !nodeJson["parameters"].is_null()) {
        nlohmann::json params = nodeJson["parameters"];

        // BTActionSelectorの場合
        if (nodeType == "BTActionSelector" && params.contains("actionType")) {
            auto actionSelector = std::dynamic_pointer_cast<BTActionSelector>(node);
            if (actionSelector) {
                int actionType = params["actionType"];
                actionSelector->SetActionType(static_cast<BTActionSelector::ActionType>(actionType));
            }
        }
        // BTBossIdleの場合
        else if (nodeType == "BTBossIdle") {
            auto idleNode = std::dynamic_pointer_cast<BTBossIdle>(node);
            if (idleNode && params.contains("idleDuration")) {
                idleNode->SetIdleDuration(params["idleDuration"]);
            }
        }
        // BTBossDashの場合
        else if (nodeType == "BTBossDash") {
            auto dashNode = std::dynamic_pointer_cast<BTBossDash>(node);
            if (dashNode) {
                if (params.contains("dashSpeed")) dashNode->SetDashSpeed(params["dashSpeed"]);
                if (params.contains("dashDuration")) dashNode->SetDashDuration(params["dashDuration"]);
            }
        }
        // BTBossShootの場合
        else if (nodeType == "BTBossShoot") {
            auto shootNode = std::dynamic_pointer_cast<BTBossShoot>(node);
            if (shootNode) {
                if (params.contains("chargeTime")) shootNode->SetChargeTime(params["chargeTime"]);
                if (params.contains("bulletSpeed")) shootNode->SetBulletSpeed(params["bulletSpeed"]);
                if (params.contains("spreadAngle")) shootNode->SetSpreadAngle(params["spreadAngle"]);
                if (params.contains("recoveryTime")) shootNode->SetRecoveryTime(params["recoveryTime"]);
            }
        }
    }

    // 表示名を設定（オプション）
    if (nodeJson.contains("displayName")) {
        node->SetName(nodeJson["displayName"]);
    }

    // コンポジットノードの場合、子ノードを追加
    bool isComposite = (nodeType == "BTSelector" || nodeType == "BTSequence");
    if (isComposite) {
        auto compositeNode = std::dynamic_pointer_cast<BTComposite>(node);
        if (compositeNode) {
            // このノードの子ノードIDを収集
            std::vector<int> childIds;
            for (const auto& link : links) {
                if (link["sourceNodeId"] == nodeId) {
                    childIds.push_back(link["targetNodeId"]);
                }
            }

            // 子ノードを再帰的に構築
            for (int childId : childIds) {
                auto childIt = nodeMap.find(childId);
                if (childIt != nodeMap.end()) {
                    BTNodePtr childNode = BuildNodeFromJSON(childIt->second, nodeMap, links, visitedNodes);
                    if (childNode) {
                        compositeNode->AddChild(childNode);
                    }
                }
            }
        }
    }

    return node;
}

/// <summary>
/// 実行中のノードを再帰的に検索
/// </summary>
void BossBehaviorTree::FindRunningNodeRecursive(const BTNodePtr& node) {
    if (!node || !node->IsRunning()) {
        return;
    }

    // 現在実行中のノードを更新
    currentRunningNode_ = node;

    // コンポジットノードの場合、子ノードも探索
    auto composite = std::dynamic_pointer_cast<BTComposite>(node);
    if (composite) {
        const auto& children = composite->GetChildren();
        for (const auto& child : children) {
            if (child && child->IsRunning()) {
                // 実行中の子ノードを再帰的に探索
                FindRunningNodeRecursive(child);
                break;  // 最初に見つかった実行中の子ノードのみ処理
            }
        }
    }
}