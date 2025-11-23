#ifdef _DEBUG

#include "BossNodeFactory.h"

// BehaviorTreeノードのインクルード
#include "../../../BehaviorTree/Composites/BTSelector.h"
#include "../../../BehaviorTree/Composites/BTSequence.h"
#include "../BossBehaviorTree/Actions/BTBossIdle.h"
#include "../BossBehaviorTree/Actions/BTBossDash.h"
#include "../BossBehaviorTree/Actions/BTBossShoot.h"
#include "../BossBehaviorTree/Conditions/BTActionSelector.h"

#include <typeinfo>
#include <algorithm>

// 静的メンバの定義
std::vector<BossNodeFactory::NodeTypeInfo> BossNodeFactory::nodeTypes_;
bool BossNodeFactory::initialized_ = false;

/// <summary>
/// ノードタイプ情報の初期化
/// </summary>
void BossNodeFactory::InitializeNodeTypes() {
    if (initialized_) return;

    // ノードタイプ情報の登録
    nodeTypes_ = {
        // ========== Composite ノード ==========
        {
            "BTSelector",
            "Selector",
            NodeCategory::Composite,
            ImVec4(0.8f, 0.4f, 0.2f, 1.0f),  // オレンジ
            true  // 子ノードを持てる
        },
        {
            "BTSequence",
            "Sequence",
            NodeCategory::Composite,
            ImVec4(0.2f, 0.6f, 0.8f, 1.0f),  // 青
            true  // 子ノードを持てる
        },

        // ========== Action ノード ==========
        {
            "BTBossIdle",
            "Idle",
            NodeCategory::Action,
            ImVec4(0.2f, 0.8f, 0.4f, 1.0f),  // 緑
            false  // 子ノードを持てない
        },
        {
            "BTBossDash",
            "Dash",
            NodeCategory::Action,
            ImVec4(0.3f, 0.8f, 0.5f, 1.0f),  // 明るい緑
            false
        },
        {
            "BTBossShoot",
            "Shoot",
            NodeCategory::Action,
            ImVec4(0.8f, 0.3f, 0.3f, 1.0f),  // 赤
            false
        },

        // ========== Condition ノード ==========
        {
            "BTActionSelector",
            "Action Selector",
            NodeCategory::Condition,
            ImVec4(0.8f, 0.8f, 0.2f, 1.0f),  // 黄色
            false
        }
    };

    initialized_ = true;
}

/// <summary>
/// 利用可能なノードタイプ一覧の取得
/// </summary>
std::vector<std::string> BossNodeFactory::GetAvailableNodeTypes() {
    InitializeNodeTypes();

    std::vector<std::string> types;
    for (const auto& nodeType : nodeTypes_) {
        types.push_back(nodeType.typeName);
    }
    return types;
}

/// <summary>
/// カテゴリごとのノードタイプ取得
/// </summary>
std::vector<std::string> BossNodeFactory::GetNodeTypesByCategory(NodeCategory category) {
    InitializeNodeTypes();

    std::vector<std::string> types;
    for (const auto& nodeType : nodeTypes_) {
        if (nodeType.category == category) {
            types.push_back(nodeType.typeName);
        }
    }
    return types;
}

/// <summary>
/// ノードの生成
/// </summary>
BTNodePtr BossNodeFactory::CreateNode(const std::string& nodeType) {
    // Compositeノード
    if (nodeType == "BTSelector") {
        return std::make_shared<BTSelector>();
    }
    else if (nodeType == "BTSequence") {
        return std::make_shared<BTSequence>();
    }
    // Actionノード（Boss/Player依存のため、ここでは生成しない）
    else if (nodeType == "BTBossIdle" ||
             nodeType == "BTBossDash" ||
             nodeType == "BTBossShoot") {
        // これらはCreateNodeWithDependenciesを使う必要がある
        return nullptr;
    }
    // Conditionノード
    else if (nodeType == "BTActionSelector") {
        // ActionSelectorも特定のアクションタイプが必要なため、ここでは基本的なものを作成
        return std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Dash);
    }

    return nullptr;
}

/// <summary>
/// Boss/Playerの依存関係を持つノードの生成
/// </summary>
BTNodePtr BossNodeFactory::CreateNodeWithDependencies(
    const std::string& nodeType,
    Boss* boss,
    Player* player) {

    // Compositeノード（依存関係なし）
    if (nodeType == "BTSelector") {
        return std::make_shared<BTSelector>();
    }
    else if (nodeType == "BTSequence") {
        return std::make_shared<BTSequence>();
    }
    // Actionノード（Boss/Player依存）
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
        // デフォルトでDashタイプを作成（後で編集可能にする）
        return std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Dash);
    }

    return nullptr;
}

/// <summary>
/// ノードタイプの取得（逆引き）
/// </summary>
std::string BossNodeFactory::GetNodeType(const BTNodePtr& node) {
    if (!node) return "";

    // RTTIを使用してタイプを判定
    // 注意: この方法はtypeid().name()の結果がコンパイラ依存
    const std::type_info& typeInfo = typeid(*node);

    // 各タイプと比較
    if (typeInfo == typeid(BTSelector)) return "BTSelector";
    if (typeInfo == typeid(BTSequence)) return "BTSequence";
    if (typeInfo == typeid(BTBossIdle)) return "BTBossIdle";
    if (typeInfo == typeid(BTBossDash)) return "BTBossDash";
    if (typeInfo == typeid(BTBossShoot)) return "BTBossShoot";
    if (typeInfo == typeid(BTActionSelector)) return "BTActionSelector";

    return "";
}

/// <summary>
/// ノードタイプ情報の取得
/// </summary>
BossNodeFactory::NodeTypeInfo BossNodeFactory::GetNodeTypeInfo(const std::string& nodeType) {
    InitializeNodeTypes();

    for (const auto& info : nodeTypes_) {
        if (info.typeName == nodeType) {
            return info;
        }
    }

    // デフォルト値を返す
    NodeTypeInfo defaultInfo;
    defaultInfo.typeName = nodeType;
    defaultInfo.displayName = nodeType;
    defaultInfo.category = NodeCategory::Action;
    defaultInfo.color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    defaultInfo.isComposite = false;
    return defaultInfo;
}

/// <summary>
/// ノードの表示名を取得
/// </summary>
std::string BossNodeFactory::GetNodeDisplayName(const std::string& nodeType) {
    NodeTypeInfo info = GetNodeTypeInfo(nodeType);
    return info.displayName;
}

/// <summary>
/// ノードの色を取得
/// </summary>
ImVec4 BossNodeFactory::GetNodeColor(const std::string& nodeType) {
    NodeTypeInfo info = GetNodeTypeInfo(nodeType);
    return info.color;
}

/// <summary>
/// コンポジットノードかどうか判定
/// </summary>
bool BossNodeFactory::IsCompositeNode(const std::string& nodeType) {
    NodeTypeInfo info = GetNodeTypeInfo(nodeType);
    return info.isComposite;
}

/// <summary>
/// ノードカテゴリを取得
/// </summary>
BossNodeFactory::NodeCategory BossNodeFactory::GetNodeCategory(const std::string& nodeType) {
    NodeTypeInfo info = GetNodeTypeInfo(nodeType);
    return info.category;
}

#endif // _DEBUG