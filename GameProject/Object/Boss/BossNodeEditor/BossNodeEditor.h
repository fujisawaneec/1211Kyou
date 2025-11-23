#pragma once

#ifdef _DEBUG

#include <imgui.h>
#include <../../../../TakoEngine/project/externals/imgui-node-editor/imgui_node_editor.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <json.hpp>
#include "../../../BehaviorTree/Core/BTNode.h"

// 名前空間エイリアス
namespace ed = ax::NodeEditor;

class BossBehaviorTree;
class Boss;
class Player;

/// <summary>
/// ボス用ビヘイビアツリーノードエディタ
/// imgui-node-editorを直接使用してビヘイビアツリーを視覚的に編集
/// </summary>
class BossNodeEditor {
public:
    /// <summary>
    /// エディタノードデータ（エディタ専用メタデータ）
    /// </summary>
    struct EditorNode {
        int id;                              // エディタ固有ID（10000番台）
        ImVec2 position;                     // エディタ上の位置
        std::string nodeType;                // ノードタイプ名（"BTSelector", "BTSequence"等）
        std::string displayName;             // 表示名
        BTNodePtr runtimeNode;               // 実際の実行ノード
        std::vector<int> inputPinIds;        // 入力ピンID（親接続用）
        std::vector<int> outputPinIds;       // 出力ピンID（子接続用）
        ImVec4 color;                        // ノードカラー
    };

    /// <summary>
    /// エディタリンクデータ
    /// </summary>
    struct EditorLink {
        int id;                              // リンク固有ID（30000番台）
        int startPinId;                      // 開始ピンID
        int endPinId;                        // 終了ピンID
        int startNodeId;                     // 開始ノードID
        int endNodeId;                       // 終了ノードID
    };

    /// <summary>
    /// エディタピンデータ
    /// </summary>
    struct EditorPin {
        int id;                              // ピン固有ID（20000番台）
        int nodeId;                          // 所属ノードID
        bool isInput;                        // 入力ピンかどうか
        std::string name;                    // ピン名
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossNodeEditor();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossNodeEditor();

    /// <summary>
    /// エディタの初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// エディタの更新・描画（ImGuiウィンドウ内で呼ぶ）
    /// </summary>
    void Update();

    /// <summary>
    /// エディタの終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// エディタの表示切り替え
    /// </summary>
    /// <param name="visible">表示するかどうか</param>
    void SetVisible(bool visible) { isVisible_ = visible; }

    /// <summary>
    /// エディタの表示状態取得
    /// </summary>
    /// <returns>表示中ならtrue</returns>
    bool IsVisible() const { return isVisible_; }

    /// <summary>
    /// ツリーをJSONから読み込み
    /// </summary>
    /// <param name="filepath">JSONファイルパス</param>
    /// <returns>成功したらtrue</returns>
    bool LoadFromJSON(const std::string& filepath);

    /// <summary>
    /// ツリーをJSONに保存
    /// </summary>
    /// <param name="filepath">JSONファイルパス</param>
    /// <returns>成功したらtrue</returns>
    bool SaveToJSON(const std::string& filepath);

    /// <summary>
    /// 実行時ツリーを構築（BossBehaviorTreeに渡す用）
    /// </summary>
    /// <returns>ルートノード</returns>
    BTNodePtr BuildRuntimeTree();

    /// <summary>
    /// BossBehaviorTreeからツリーをインポート
    /// </summary>
    /// <param name="tree">インポート元のツリー</param>
    void ImportFromBehaviorTree(BossBehaviorTree* tree);

    /// <summary>
    /// 構築したツリーをBossBehaviorTreeに適用
    /// </summary>
    /// <param name="behaviorTree">適用先のBehaviorTree</param>
    /// <returns>成功したらtrue</returns>
    bool ApplyToBehaviorTree(BossBehaviorTree* behaviorTree);

    /// <summary>
    /// 現在実行中のノードをハイライト表示（デバッグ用）
    /// </summary>
    /// <param name="nodePtr">実行中のノード</param>
    void HighlightRunningNode(const BTNodePtr& nodePtr);

    /// <summary>
    /// エディタのクリア
    /// </summary>
    void Clear();

    /// <summary>
    /// デフォルトツリーの作成（BuildActionTreeと同じ構造）
    /// </summary>
    void CreateDefaultTree();

private:
    // ax::NodeEditor コンテキスト
    ed::EditorContext* editorContext_;
    ed::Config* editorConfig_;

    // エディタデータ
    std::vector<EditorNode> nodes_;
    std::vector<EditorLink> links_;
    std::vector<EditorPin> pins_;

    // ID管理（ID範囲を分離して競合を防ぐ）
    int nextNodeId_;    // 10000番台
    int nextLinkId_;    // 30000番台
    int nextPinId_;     // 20000番台

    // エディタ状態
    bool isVisible_;
    bool firstFrame_;
    int highlightedNodeId_;  // 現在ハイライト中のノードID（実行デバッグ用）

    // 内部処理
    void DrawNodes();
    void DrawNode(const EditorNode& node);
    void DrawLinks();
    void DrawPin(const EditorPin& pin);
    void HandleNodeCreation();
    void HandleLinkCreation();
    void HandleDeletion();
    void DrawContextMenu();
    void DrawNodeInspector();
    void DrawToolbar();

    // ノード作成
    void CreateNode(const std::string& nodeType, const ImVec2& position);
    int CreateNodeWithId(int nodeId, const std::string& nodeType, const ImVec2& position);

    // リンク作成ヘルパー
    bool CreateLink(int sourceNodeId, int targetNodeId);

    // ヘルパー関数
    EditorNode* FindNodeById(int nodeId);
    const EditorNode* FindNodeById(int nodeId) const;
    EditorNode* FindNodeByRuntimeNode(const BTNodePtr& node);
    EditorPin* FindPinById(int pinId);
    const EditorPin* FindPinById(int pinId) const;
    EditorLink* FindLinkById(int linkId);

    int FindRootNodeId() const;
    void BuildRuntimeTreeRecursive(int nodeId, BTNodePtr& outNode);
    bool HasCyclicDependency(int startNodeId, int endNodeId) const;
    std::vector<int> GetChildNodeIds(int parentNodeId) const;

    // ノードパラメータの保存・復元
    nlohmann::json ExtractNodeParameters(const EditorNode& node);
    void ApplyNodeParameters(EditorNode& node, const nlohmann::json& params);

    // ImportFromBehaviorTree用ヘルパー
    int ImportNodeRecursive(const BTNodePtr& btNode, const ImVec2& position, int depth);

    // 依存関係設定
    void SetupNodeDependencies(BTNodePtr node, BTBlackboard* blackboard);

    // ノード・ピンIDマッピング管理
    std::unordered_map<BTNode*, int> runtimeNodeToEditorId_;
};

#endif // _DEBUG