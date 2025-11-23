#pragma once

#ifdef _DEBUG

#include <string>
#include <vector>
#include <memory>
#include <imgui.h>
#include "../../../BehaviorTree/Core/BTNode.h"

class Boss;
class Player;

/// <summary>
/// ボスノードの生成ファクトリ
/// ノードタイプ文字列から実際のBTNodeインスタンスを生成
/// </summary>
class BossNodeFactory {
public:
    /// <summary>
    /// ノードカテゴリ
    /// </summary>
    enum class NodeCategory {
        Composite,      // 複合ノード（Selector, Sequence）
        Action,         // アクションノード（Idle, Dash, Shoot）
        Condition,      // 条件ノード（ActionSelector）
        Decorator       // デコレータノード（将来の拡張用）
    };

    /// <summary>
    /// ノードタイプ情報
    /// </summary>
    struct NodeTypeInfo {
        std::string typeName;           // ノードタイプ名（"BTSelector"等）
        std::string displayName;        // 表示名（"セレクター"等）
        NodeCategory category;          // カテゴリ
        ImVec4 color;                  // ノードカラー
        bool isComposite;              // 子ノードを持てるか
    };

    /// <summary>
    /// 利用可能なノードタイプ一覧の取得
    /// </summary>
    /// <returns>利用可能なノードタイプ名のリスト</returns>
    static std::vector<std::string> GetAvailableNodeTypes();

    /// <summary>
    /// カテゴリごとのノードタイプ取得
    /// </summary>
    /// <param name="category">カテゴリ</param>
    /// <returns>該当カテゴリのノードタイプリスト</returns>
    static std::vector<std::string> GetNodeTypesByCategory(NodeCategory category);

    /// <summary>
    /// ノードの生成
    /// </summary>
    /// <param name="nodeType">ノードタイプ名（"BTSelector", "BTSequence"等）</param>
    /// <returns>生成されたノード（失敗時はnullptr）</returns>
    static BTNodePtr CreateNode(const std::string& nodeType);

    /// <summary>
    /// Boss/Playerの依存関係を持つノードの生成
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <param name="boss">ボスのポインタ（必要な場合）</param>
    /// <param name="player">プレイヤーのポインタ（必要な場合）</param>
    /// <returns>生成されたノード（失敗時はnullptr）</returns>
    static BTNodePtr CreateNodeWithDependencies(
        const std::string& nodeType,
        Boss* boss = nullptr,
        Player* player = nullptr
    );

    /// <summary>
    /// ノードタイプの取得（逆引き）
    /// </summary>
    /// <param name="node">BTNodeインスタンス</param>
    /// <returns>ノードタイプ名</returns>
    static std::string GetNodeType(const BTNodePtr& node);

    /// <summary>
    /// ノードタイプ情報の取得
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <returns>ノードタイプ情報</returns>
    static NodeTypeInfo GetNodeTypeInfo(const std::string& nodeType);

    /// <summary>
    /// ノードの表示名を取得
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <returns>表示用の名前</returns>
    static std::string GetNodeDisplayName(const std::string& nodeType);

    /// <summary>
    /// ノードの色を取得（エディタ表示用）
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <returns>ノードの色（ImVec4）</returns>
    static ImVec4 GetNodeColor(const std::string& nodeType);

    /// <summary>
    /// コンポジットノードかどうか判定
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <returns>コンポジット（子を持てる）ならtrue</returns>
    static bool IsCompositeNode(const std::string& nodeType);

    /// <summary>
    /// ノードカテゴリを取得
    /// </summary>
    /// <param name="nodeType">ノードタイプ名</param>
    /// <returns>ノードカテゴリ</returns>
    static NodeCategory GetNodeCategory(const std::string& nodeType);

private:
    /// <summary>
    /// ノードタイプ情報の初期化（静的）
    /// </summary>
    static void InitializeNodeTypes();

    /// <summary>
    /// ノードタイプ情報のマップ
    /// </summary>
    static std::vector<NodeTypeInfo> nodeTypes_;

    /// <summary>
    /// 初期化フラグ
    /// </summary>
    static bool initialized_;
};

#endif // _DEBUG