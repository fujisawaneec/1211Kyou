#ifdef _DEBUG

#include "BossNodeEditor.h"
#include "BossNodeFactory.h"
#include "../BossBehaviorTree/BossBehaviorTree.h"
#include "../../../BehaviorTree/Core/BTComposite.h"
#include "../../../BehaviorTree/Core/BTBlackboard.h"
#include "../BossBehaviorTree/Conditions/BTActionSelector.h"
#include "DebugUIManager.h"
#include <imgui_internal.h>
#include <algorithm>
#include <queue>
#include <set>
#include <unordered_map>
#include <json.hpp>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

/// <summary>
/// コンストラクタ
/// </summary>
BossNodeEditor::BossNodeEditor()
    : editorContext_(nullptr)
    , editorConfig_(nullptr)
    , nextNodeId_(10000)    // BossNodeEditor専用: 10000番台
    , nextPinId_(20000)     // BossNodeEditor専用: 20000番台
    , nextLinkId_(30000)    // BossNodeEditor専用: 30000番台
    , isVisible_(false)
    , firstFrame_(true)
    , highlightedNodeId_(-1)
    , highlightStartTime_(0.0f) {
}

/// <summary>
/// デストラクタ
/// </summary>
BossNodeEditor::~BossNodeEditor() {
    Finalize();
}

/// <summary>
/// エディタの初期化
/// </summary>
void BossNodeEditor::Initialize() {
    // エディタコンフィグの作成
    editorConfig_ = new ed::Config();
    editorConfig_->SettingsFile = "resources/Json/BossNodeEditor.json";
    editorConfig_->NavigateButtonIndex = 1;        // マウス中ボタンでナビゲート
    editorConfig_->ContextMenuButtonIndex = 2;     // マウス右ボタンでコンテキストメニュー

    // エディタコンテキストの作成
    editorContext_ = ed::CreateEditor(editorConfig_);

    LoadFromJSON("resources/Json/BossTree.json");
}

/// <summary>
/// エディタの終了処理
/// </summary>
void BossNodeEditor::Finalize() {
    if (editorContext_) {
        ed::DestroyEditor(editorContext_);
        editorContext_ = nullptr;
    }
    if (editorConfig_) {
        delete editorConfig_;
        editorConfig_ = nullptr;
    }
    Clear();
}

/// <summary>
/// エディタの更新・描画（ImGuiウィンドウ内で呼ぶ）
/// </summary>
void BossNodeEditor::Update() {
    if (!isVisible_) return;

    // ImGuiウィンドウの開始
    if (ImGui::Begin("Boss Behavior Tree Editor", &isVisible_)) {
        // ツールバーの描画
        DrawToolbar();

        ImGui::Separator();

        // ノードエディタキャンバスの開始
        ed::SetCurrentEditor(editorContext_);
        ed::Begin("Boss Node Editor Canvas");

        // ノードの描画
        DrawNodes();

        // リンクの描画
        DrawLinks();

        // インタラクション処理
        HandleNodeCreation();
        HandleLinkCreation();
        HandleDeletion();

        // コンテキストメニュー
        // DrawContextMenu();

        // 選択ノードの取得（ed::End()前に実行必須）
        {
            int selectedCount = ed::GetSelectedObjectCount();
            if (selectedCount > 0) {
                std::vector<ed::NodeId> selectedNodes(selectedCount);
                int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), selectedCount);
                if (nodeCount > 0) {
                    selectedNodeId_ = static_cast<int>(selectedNodes[0].Get());
                } else {
                    selectedNodeId_ = -1;
                }
            } else {
                selectedNodeId_ = -1;
            }
        }

        ed::End();
        ed::SetCurrentEditor(nullptr);

        // ノードインスペクター
        DrawNodeInspector();

        // 初回フレームの処理完了
        if (firstFrame_) {
            firstFrame_ = false;
        }
    }
    ImGui::End();
}

/// <summary>
/// エディタのクリア
/// </summary>
void BossNodeEditor::Clear() {
    nodes_.clear();
    pins_.clear();
    links_.clear();
    runtimeNodeToEditorId_.clear();

    // ID範囲を初期値に戻す（ID競合を防ぐため範囲を分離）
    nextNodeId_ = 10000;    // BossNodeEditor専用: 10000番台
    nextPinId_ = 20000;     // BossNodeEditor専用: 20000番台
    nextLinkId_ = 30000;    // BossNodeEditor専用: 30000番台

    highlightedNodeId_ = -1;
    highlightStartTime_ = 0.0f;
    selectedNodeId_ = -1;
    firstFrame_ = true;
}

/// <summary>
/// ツールバーの描画
/// </summary>
void BossNodeEditor::DrawToolbar() {
    // ノードの段階的オフセット用の静的変数
    static float nodeOffsetX = 100.0f;
    static float nodeOffsetY = 100.0f;

    // 選択されたノードタイプを保持する静的変数
    static int selectedNodeTypeIndex = 0;
    static std::string selectedNodeType = "BTSelector"; // デフォルト

    if (ImGui::Button("Save##bne_toolbar")) {
        SaveToJSON("resources/Json/BossTree.json");
    }
    ImGui::SameLine();

    if (ImGui::Button("Load##bne_toolbar")) {
        LoadFromJSON("resources/Json/BossTree.json");
    }
    ImGui::SameLine();

    if (ImGui::Button("Clear##bne_toolbar")) {
        Clear();
        nodeOffsetX = 100.0f;
        nodeOffsetY = 100.0f;
    }
    ImGui::SameLine();

    if (ImGui::Button("Build Runtime Tree##bne_toolbar")) {
        BTNodePtr runtimeTree = BuildRuntimeTree();
        if (runtimeTree) {
            ImGui::SameLine();
            ImGui::Text("Tree built successfully!");
        }
    }

    // ノード作成用のUIセクション
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    ImGui::Text("Node Type:");
    ImGui::SameLine();

    // 全ノードタイプを収集して統一されたドロップダウンを作成
    static std::vector<std::string> allNodeTypes;
    static std::vector<std::string> allDisplayNames;
    allNodeTypes.clear();
    allDisplayNames.clear();

    // Compositeノード
    auto compositeTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Composite);
    for (const auto& type : compositeTypes) {
        allNodeTypes.push_back(type);
        allDisplayNames.push_back("[Composite] " + BossNodeFactory::GetNodeDisplayName(type));
    }

    // Actionノード
    auto actionTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Action);
    for (const auto& type : actionTypes) {
        allNodeTypes.push_back(type);
        allDisplayNames.push_back("[Action] " + BossNodeFactory::GetNodeDisplayName(type));
    }

    // Conditionノード
    auto conditionTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Condition);
    for (const auto& type : conditionTypes) {
        allNodeTypes.push_back(type);
        allDisplayNames.push_back("[Condition] " + BossNodeFactory::GetNodeDisplayName(type));
    }

    // ドロップダウンリスト
    if (!allNodeTypes.empty()) {
        // 範囲チェック: 静的変数が範囲外の場合はリセット
        if (selectedNodeTypeIndex >= static_cast<int>(allDisplayNames.size())) {
            selectedNodeTypeIndex = 0;
        }

        // 現在選択されているノードの表示名
        const char* previewValue = selectedNodeTypeIndex < static_cast<int>(allDisplayNames.size())
            ? allDisplayNames[selectedNodeTypeIndex].c_str()
            : "Select Node Type...";

        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo("##NodeTypeCombo", previewValue)) {
            for (int i = 0; i < static_cast<int>(allDisplayNames.size()); i++) {
                bool isSelected = (selectedNodeTypeIndex == i);
                if (ImGui::Selectable(allDisplayNames[i].c_str(), isSelected)) {
                    selectedNodeTypeIndex = i;
                    selectedNodeType = allNodeTypes[i];
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        // Add Nodeボタン
        if (ImGui::Button("Add Node##bne_toolbar")) {
            ImVec2 centerPos = ImVec2(nodeOffsetX, nodeOffsetY);
            nodeOffsetX += 30.0f;
            nodeOffsetY += 20.0f;
            // 画面外に行かないようにリセット
            if (nodeOffsetX > 800.0f) nodeOffsetX = 100.0f;
            if (nodeOffsetY > 600.0f) nodeOffsetY = 100.0f;
            CreateNode(selectedNodeType, centerPos);
        }
    }
}

/// <summary>
/// ノードの描画
/// </summary>
void BossNodeEditor::DrawNodes() {
    for (const auto& node : nodes_) {
        DrawNode(node);
    }
}

/// <summary>
/// 個別ノードの描画
/// </summary>
void BossNodeEditor::DrawNode(const EditorNode& node) {
    // デフォルトのスタイル数を追跡
    int pushedColors = 2;  // NodeBg, NodeBorder
    int pushedVars = 2;    // NodeRounding, NodeBorderWidth

    // ノードの基本色を設定（少し暗めに）
    ImVec4 nodeColor = ImVec4(
        node.color.x * 0.7f,
        node.color.y * 0.7f,
        node.color.z * 0.7f,
        1.0f
    );

    // 実行中ノードのパルスエフェクト計算
    bool isHighlighted = (node.id == highlightedNodeId_);

    float pulseIntensity = 0.0f;
    float borderWidth = 1.5f;
    ImVec4 borderColor = ImVec4(0.31f, 0.31f, 0.31f, 1.0f); // ImColor(80, 80, 80)

    if (isHighlighted) {
        // 実行中ノード: パルスエフェクト
        float elapsed = static_cast<float>(ImGui::GetTime()) - highlightStartTime_;
        pulseIntensity = (sinf(elapsed * 6.0f) + 1.0f) * 0.5f; // 0.0～1.0で振動

        // ボーダー色を時間経過で変化（オレンジ～黄色）
        borderColor = ImVec4(
            1.0f,
            0.6f + pulseIntensity * 0.3f,
            0.2f + pulseIntensity * 0.3f,
            1.0f
        );
        borderWidth = 2.5f + pulseIntensity * 1.5f; // 2.5～4.0で変化

        // ノード背景色も少し明るく
        nodeColor = ImVec4(
            node.color.x * 0.7f + pulseIntensity * 0.1f,
            node.color.y * 0.7f + pulseIntensity * 0.1f,
            node.color.z * 0.7f + pulseIntensity * 0.1f,
            1.0f
        );
    }

    // スタイルを適用
    ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(nodeColor));
    ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(borderColor));
    ed::PushStyleVar(ed::StyleVar_NodeRounding, 5.0f);
    ed::PushStyleVar(ed::StyleVar_NodeBorderWidth, borderWidth);

    ed::BeginNode(node.id);

    ImGui::PushID(node.id);

    // ノード全体の幅を統一
    const float nodeWidth = 200.0f;
    const float barHeight = 24.0f;

    // ========== 入力ピンバー（上部） ==========
    if (!node.inputPinIds.empty()) {
        // バー背景を手動描画
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 barMin = ImGui::GetCursorScreenPos();
        ImVec2 barMax = ImVec2(barMin.x + nodeWidth, barMin.y + barHeight);
        drawList->AddRectFilled(barMin, barMax, IM_COL32(30, 30, 30, 255), 3.0f);

        // ピン領域を確保
        ImGui::Dummy(ImVec2(nodeWidth, barHeight));

        // 入力ピンを配置
        ImVec2 savedPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(savedPos.x, savedPos.y - barHeight + 4));

        for (int pinId : node.inputPinIds) {
            const EditorPin* pin = FindPinById(pinId);
            if (pin) {
                // 中央に配置（Dummy + SameLine方式）
                float textWidth = ImGui::CalcTextSize("Input").x;
                ImGui::Dummy(ImVec2((nodeWidth - textWidth) * 0.5f, 0));
                ImGui::SameLine(0, 0);
                DrawPin(*pin);
            }
        }

        ImGui::SetCursorPos(savedPos);
    }

    // ========== ノード本体（中央） ==========
    ImGui::Spacing();

    // ノード名を表示（中央揃え）
    const char* titleText = node.displayName.c_str();
    float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::Dummy(ImVec2((nodeWidth - titleWidth) * 0.5f, 0));
    ImGui::SameLine(0, 0);
    ImGui::Text("%s", titleText);

    // ノードタイプを小さく表示（中央揃え）
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::SetWindowFontScale(0.85f);
    float typeWidth = ImGui::CalcTextSize(node.nodeType.c_str()).x * 0.85f;
    ImGui::Dummy(ImVec2((nodeWidth - typeWidth) * 0.5f, 0));
    ImGui::SameLine(0, 0);
    ImGui::Text("%s", node.nodeType.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // ========== 出力ピンバー（下部） ==========
    if (!node.outputPinIds.empty()) {
        // バー背景を手動描画
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 barMin = ImGui::GetCursorScreenPos();
        ImVec2 barMax = ImVec2(barMin.x + nodeWidth, barMin.y + barHeight);
        drawList->AddRectFilled(barMin, barMax, IM_COL32(30, 30, 30, 255), 3.0f);

        // ピン領域を確保
        ImGui::Dummy(ImVec2(nodeWidth, barHeight));

        // 出力ピンを配置
        ImVec2 savedPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(savedPos.x, savedPos.y - barHeight + 4));

        int pinCount = static_cast<int>(node.outputPinIds.size());

        if (pinCount == 1 && !node.outputPinIds.empty()) {
            // 単一ピンは中央配置（範囲チェック追加）
            const EditorPin* pin = FindPinById(node.outputPinIds[0]);
            if (pin) {
                float textWidth = ImGui::CalcTextSize("Output").x;
                ImGui::Dummy(ImVec2((nodeWidth - textWidth) * 0.5f, 0));
                ImGui::SameLine(0, 0);
                DrawPin(*pin);
            }
        }
        else if (pinCount > 1 && node.outputPinIds.size() >= static_cast<size_t>(pinCount)) {
            // 複数ピンは横並び（範囲チェック追加）
            float spacing = nodeWidth / (pinCount + 1);

            for (int i = 0; i < pinCount && i < static_cast<int>(node.outputPinIds.size()); i++) {
                const EditorPin* pin = FindPinById(node.outputPinIds[i]);
                if (pin) {
                    float offset = spacing * (i + 1) - 20; // ピン幅を考慮
                    ImGui::Dummy(ImVec2(offset, 0));
                    ImGui::SameLine(0, 0);
                    DrawPin(*pin);
                    if (i < pinCount - 1) {
                        ImGui::SameLine();
                    }
                }
            }
        }

        ImGui::SetCursorPos(savedPos);
    }

    ImGui::PopID();

    ed::EndNode();

    // Push/Popの対応を正しく保つ（常に2色、2変数）
    ed::PopStyleVar(2);
    ed::PopStyleColor(2);

    // 初回フレームのみノード位置を設定（CreateNode時のデフォルト位置用）
    // LoadFromJSON時はLoadFromJSON内で設定するため、ここでは設定しない
    if (firstFrame_ && nodes_.size() <= 1 && (node.position.x != 0 || node.position.y != 0)) {
        ed::SetNodePosition(node.id, node.position);
    }
}

/// <summary>
/// ピンの描画（改善版）
/// </summary>
void BossNodeEditor::DrawPin(const EditorPin& pin) {
    // ピンのカラー設定
    ImColor pinColor = pin.isInput ? ImColor(150, 150, 200) : ImColor(150, 200, 150);
    ImColor borderColor = ImColor(200, 200, 200, 200);

    // スタイルをプッシュ
    ed::PushStyleColor(ed::StyleColor_PinRect, pinColor);
    ed::PushStyleColor(ed::StyleColor_PinRectBorder, borderColor);

    ed::BeginPin(pin.id, pin.isInput ? ed::PinKind::Input : ed::PinKind::Output);

    ImGui::PushID(pin.id);

    // ピンの矩形領域開始位置を記録
    ImVec2 pinRectMin = ImGui::GetCursorScreenPos();

    // ピンテキストを表示
    ImGui::Text("    ");


    // ピンの矩形領域終了位置を記録
    ImVec2 pinRectMax = ImVec2(
        ImGui::GetItemRectMax().x,
        ImGui::GetItemRectMax().y
    );

    // ピンの矩形領域を定義（クリック判定用）
    ed::PinRect(pinRectMin, pinRectMax);

    // ピンアイコンの配置位置を設定
    // 入力ピンは左側（0.0f）、出力ピンは右側（1.0f）に配置
    ImVec2 alignment = pin.isInput ? ImVec2(0.5f, 0.0f) : ImVec2(0.5f, 1.0f);
    ed::PinPivotAlignment(alignment);
    ed::PinPivotSize(ImVec2(0, 0)); // 自動サイズ

    ImGui::PopID();

    ed::EndPin();

    // スタイルをポップ
    ed::PopStyleColor(2);
}

/// <summary>
/// リンクの描画
/// </summary>
void BossNodeEditor::DrawLinks() {
    for (const auto& link : links_) {
        ed::Link(link.id, link.startPinId, link.endPinId, ImColor(200, 200, 200), 2.0f);
    }
}

/// <summary>
/// ノード作成処理
/// </summary>
void BossNodeEditor::HandleNodeCreation() {
    // HandleLinkCreation内のBeginCreateと競合しないように、
    // この関数は現在空実装のままとする。
    // ノード作成はコンテキストメニューから行う。
}

/// <summary>
/// リンク作成処理（仮実装）
/// </summary>
void BossNodeEditor::HandleLinkCreation() {
    if (ed::BeginCreate()) {
        ed::PinId inputPinId, outputPinId;

        if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
            // ピンの検証
            EditorPin* inputPin = FindPinById(static_cast<int>(inputPinId.Get()));
            EditorPin* outputPin = FindPinById(static_cast<int>(outputPinId.Get()));

            if (inputPin && outputPin) {
                // 入力と出力が逆の場合は入れ替える
                if (inputPin->isInput == false && outputPin->isInput == true) {
                    std::swap(inputPin, outputPin);
                    std::swap(inputPinId, outputPinId);
                }

                // リンク作成の検証
                bool canCreateLink = inputPin->isInput != outputPin->isInput;

                // 循環参照チェック
                if (canCreateLink) {
                    canCreateLink = !HasCyclicDependency(outputPin->nodeId, inputPin->nodeId);
                }

                if (canCreateLink && ed::AcceptNewItem()) {
                    // リンク作成
                    EditorLink newLink;
                    newLink.id = nextLinkId_++;
                    newLink.startPinId = static_cast<int>(outputPinId.Get());
                    newLink.endPinId = static_cast<int>(inputPinId.Get());
                    newLink.startNodeId = outputPin->nodeId;
                    newLink.endNodeId = inputPin->nodeId;

                    links_.push_back(newLink);
                }
                else if (!canCreateLink) {
                    // リンク作成を拒否（赤色で表示）
                    ed::RejectNewItem(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);
                }
            }
        }
    }
    ed::EndCreate();
}

/// <summary>
/// 削除処理（仮実装）
/// </summary>
void BossNodeEditor::HandleDeletion() {
    if (ed::BeginDelete()) {
        // リンクの削除
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId)) {
            if (ed::AcceptDeletedItem()) {
                links_.erase(
                    std::remove_if(links_.begin(), links_.end(),
                        [deletedLinkId](const EditorLink& link) {
                            return link.id == static_cast<int>(deletedLinkId.Get());
                        }),
                    links_.end()
                );
            }
        }

        // ノードの削除
        ed::NodeId deletedNodeId;
        while (ed::QueryDeletedNode(&deletedNodeId)) {
            if (ed::AcceptDeletedItem()) {
                int nodeId = static_cast<int>(deletedNodeId.Get());

                // ノードに関連するリンクを削除
                links_.erase(
                    std::remove_if(links_.begin(), links_.end(),
                        [nodeId](const EditorLink& link) {
                            return link.startNodeId == nodeId || link.endNodeId == nodeId;
                        }),
                    links_.end()
                );

                // ノードのピンを削除
                EditorNode* node = FindNodeById(nodeId);
                if (node) {
                    for (int pinId : node->inputPinIds) {
                        pins_.erase(
                            std::remove_if(pins_.begin(), pins_.end(),
                                [pinId](const EditorPin& pin) {
                                    return pin.id == pinId;
                                }),
                            pins_.end()
                        );
                    }
                    for (int pinId : node->outputPinIds) {
                        pins_.erase(
                            std::remove_if(pins_.begin(), pins_.end(),
                                [pinId](const EditorPin& pin) {
                                    return pin.id == pinId;
                                }),
                            pins_.end()
                        );
                    }
                }

                // ノード自体を削除
                nodes_.erase(
                    std::remove_if(nodes_.begin(), nodes_.end(),
                        [nodeId](const EditorNode& node) {
                            return node.id == nodeId;
                        }),
                    nodes_.end()
                );
            }
        }
    }
    ed::EndDelete();
}

/// <summary>
/// コンテキストメニューの描画
/// </summary>
void BossNodeEditor::DrawContextMenu() {
    // エディタコンテキストを一時停止（ImGuiポップアップのため）
    ed::Suspend();

    // 背景右クリックメニュー
    if (ed::ShowBackgroundContextMenu()) {
        ImGui::OpenPopup("CreateNodeMenu");
    }

    // ノード右クリックメニュー
    ed::NodeId contextNodeId;
    if (ed::ShowNodeContextMenu(&contextNodeId)) {
        ImGui::OpenPopup("NodeContextMenu");
        ImGui::SetNextWindowSize(ImVec2(200, 0));
    }

    // ノード作成メニュー
    if (ImGui::BeginPopup("CreateNodeMenu")) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ed::ScreenToCanvas(mousePos);

        ImGui::Text("Create Node");
        ImGui::Separator();

        // カテゴリごとにノードを表示
        // Compositeノード
        if (ImGui::BeginMenu("Composite Nodes")) {
            auto compositeTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Composite);
            for (const auto& nodeType : compositeTypes) {
                std::string displayName = BossNodeFactory::GetNodeDisplayName(nodeType);
                if (ImGui::MenuItem(displayName.c_str())) {
                    CreateNode(nodeType, canvasPos);
                }
            }
            ImGui::EndMenu();
        }

        // Actionノード
        if (ImGui::BeginMenu("Action Nodes")) {
            auto actionTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Action);
            for (const auto& nodeType : actionTypes) {
                std::string displayName = BossNodeFactory::GetNodeDisplayName(nodeType);
                if (ImGui::MenuItem(displayName.c_str())) {
                    CreateNode(nodeType, canvasPos);
                }
            }
            ImGui::EndMenu();
        }

        // Conditionノード
        if (ImGui::BeginMenu("Condition Nodes")) {
            auto conditionTypes = BossNodeFactory::GetNodeTypesByCategory(BossNodeFactory::NodeCategory::Condition);
            for (const auto& nodeType : conditionTypes) {
                std::string displayName = BossNodeFactory::GetNodeDisplayName(nodeType);
                if (ImGui::MenuItem(displayName.c_str())) {
                    CreateNode(nodeType, canvasPos);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    // ノード個別メニュー
    if (ImGui::BeginPopup("NodeContextMenu")) {
        if (ImGui::MenuItem("Delete")) {
            // 削除は既にHandleDeletion()で処理されるので、ここでは何もしない
            // または、削除フラグを立てる処理を追加
        }
        ImGui::EndPopup();
    }

    // エディタコンテキストを再開
    ed::Resume();
}

/// <summary>
/// ノードインスペクターの描画
/// </summary>
void BossNodeEditor::DrawNodeInspector() {
    // インスペクターパネル（別ウィンドウとして表示）
    if (!ImGui::Begin("Node Inspector##BNE")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Inspector");
    ImGui::Separator();

    if (selectedNodeId_ < 0) {
        ImGui::TextDisabled("No node selected");
        ImGui::End();
        return;
    }

    EditorNode* node = FindNodeById(selectedNodeId_);
    if (!node) {
        ImGui::TextDisabled("Invalid selection");
        ImGui::End();
        return;
    }

    // 基本情報
    ImGui::Text("ID: %d", node->id);
    ImGui::Text("Type: %s", node->nodeType.c_str());

    // 表示名編集
    char nameBuf[256];
    strncpy_s(nameBuf, node->displayName.c_str(), sizeof(nameBuf) - 1);
    if (ImGui::InputText("Name##inspector", nameBuf, sizeof(nameBuf))) {
        node->displayName = nameBuf;
    }

    ImGui::Separator();
    ImGui::Text("Parameters");

    // インスペクターでパラメータ編集
    if (node->runtimeNode) {
        auto inspector = CreateNodeInspector(node->runtimeNode);
        if (inspector) {
            inspector->DrawUI();
        } else {
            ImGui::TextDisabled("No editable parameters");
        }
    } else {
        ImGui::TextDisabled("No runtime node");
    }

    ImGui::End();
}

/// <summary>
/// JSONから読み込み
/// </summary>
bool BossNodeEditor::LoadFromJSON(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            DebugUIManager::GetInstance()->AddLog(
                "[BossNodeEditor] Failed to open file for reading: " + filepath,
                DebugUIManager::LogType::Error);
            return false;
        }

        nlohmann::json json;
        file >> json;
        file.close();

        // バージョンチェック
        if (!json.contains("version") || json["version"] != "1.0") {
            DebugUIManager::GetInstance()->AddLog(
                "[BossNodeEditor] Unsupported file version",
                DebugUIManager::LogType::Error);
            return false;
        }

        // 既存のエディタをクリア
        Clear();

        // IDマッピング（古いID → 新しいID）
        std::unordered_map<int, int> oldToNewNodeIdMap;
        std::unordered_map<int, int> oldToNewPinIdMap;

        // ノードを復元
        if (json.contains("nodes")) {
            for (const auto& nodeJson : json["nodes"]) {
                int oldId = nodeJson["id"];
                std::string nodeType = nodeJson["type"];
                std::string displayName = nodeJson.value("displayName", nodeType);
                ImVec2 position(
                    nodeJson["position"]["x"],
                    nodeJson["position"]["y"]
                );

                // ノードを作成（IDは自動生成される）
                int newId = CreateNodeWithId(nextNodeId_++, nodeType, position);
                if (newId != -1) {
                    oldToNewNodeIdMap[oldId] = newId;

                    // 表示名を設定
                    auto* node = FindNodeById(newId);
                    if (node) {
                        node->displayName = displayName;

                        // パラメータを適用
                        if (nodeJson.contains("parameters")) {
                            ApplyNodeParameters(*node, nodeJson["parameters"]);
                        }

                        // ピンIDのマッピングも作成
                        // 入力ピン
                        for (size_t i = 0; i < node->inputPinIds.size(); ++i) {
                            // 元のピンIDは保存されていないので、順序で対応付け
                            // TODO: より正確なピンID管理が必要な場合は、JSONにピン情報も保存
                        }
                    }
                }
            }
        }

        // リンクを復元
        if (json.contains("links")) {
            for (const auto& linkJson : json["links"]) {
                int oldSourceNodeId = linkJson["sourceNodeId"];
                int oldTargetNodeId = linkJson["targetNodeId"];

                // 新しいノードIDに変換
                auto sourceIt = oldToNewNodeIdMap.find(oldSourceNodeId);
                auto targetIt = oldToNewNodeIdMap.find(oldTargetNodeId);

                if (sourceIt != oldToNewNodeIdMap.end() &&
                    targetIt != oldToNewNodeIdMap.end()) {

                    int newSourceNodeId = sourceIt->second;
                    int newTargetNodeId = targetIt->second;

                    auto* sourceNode = FindNodeById(newSourceNodeId);
                    auto* targetNode = FindNodeById(newTargetNodeId);

                    if (sourceNode && targetNode &&
                        !sourceNode->outputPinIds.empty() &&
                        !targetNode->inputPinIds.empty()) {

                        // リンクを作成（最初の出力ピンと最初の入力ピンを接続）
                        EditorLink link;
                        link.id = nextLinkId_++;
                        link.startPinId = sourceNode->outputPinIds[0];
                        link.endPinId = targetNode->inputPinIds[0];
                        link.startNodeId = newSourceNodeId;
                        link.endNodeId = newTargetNodeId;

                        links_.push_back(link);
                    }
                }
            }
        }

        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Successfully loaded from: " + filepath,
            DebugUIManager::LogType::Info);
        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Loaded " + std::to_string(nodes_.size()) + " nodes and " + std::to_string(links_.size()) + " links",
            DebugUIManager::LogType::Info);
        return true;
    }
    catch (const std::exception& e) {
        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Failed to load JSON: " + std::string(e.what()),
            DebugUIManager::LogType::Error);
        return false;
    }
}

/// <summary>
/// JSONに保存
/// </summary>
bool BossNodeEditor::SaveToJSON(const std::string& filepath) {
    try {
        nlohmann::json json;

        // バージョン情報
        json["version"] = "1.0";

        // メタデータ
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;

        // Windows環境用のlocaltime_s使用
        struct tm timeinfo;
#ifdef _WIN32
        localtime_s(&timeinfo, &time_t);
#else
        localtime_r(&time_t, &timeinfo);
#endif
        ss << std::put_time(&timeinfo, "%Y-%m-%dT%H:%M:%S");

        json["metadata"]["name"] = "Boss Behavior Tree";
        json["metadata"]["created"] = ss.str();
        json["metadata"]["modified"] = ss.str();

        // ノード情報を保存
        json["nodes"] = nlohmann::json::array();
        for (const auto& node : nodes_) {
            nlohmann::json nodeJson;
            nodeJson["id"] = node.id;
            nodeJson["type"] = node.nodeType;
            nodeJson["displayName"] = node.displayName;
            nodeJson["position"]["x"] = node.position.x;
            nodeJson["position"]["y"] = node.position.y;

            // ノードパラメータの保存
            nodeJson["parameters"] = ExtractNodeParameters(node);

            json["nodes"].push_back(nodeJson);
        }

        // リンク情報を保存
        json["links"] = nlohmann::json::array();
        for (const auto& link : links_) {
            nlohmann::json linkJson;
            linkJson["id"] = link.id;
            linkJson["sourceNodeId"] = link.startNodeId;
            linkJson["targetNodeId"] = link.endNodeId;
            linkJson["sourcePinId"] = link.startPinId;
            linkJson["targetPinId"] = link.endPinId;
            json["links"].push_back(linkJson);
        }

        // ディレクトリが存在しない場合は作成
        std::filesystem::path filePath(filepath);
        std::filesystem::create_directories(filePath.parent_path());

        // ファイルに書き出し
        std::ofstream file(filepath);
        if (!file.is_open()) {
            DebugUIManager::GetInstance()->AddLog(
                "[BossNodeEditor] Failed to open file for writing: " + filepath,
                DebugUIManager::LogType::Error);
            return false;
        }

        file << json.dump(2); // インデント2で整形
        file.close();

        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Successfully saved to: " + filepath,
            DebugUIManager::LogType::Info);
        return true;
    }
    catch (const std::exception& e) {
        // エラーログ
        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Failed to save JSON: " + std::string(e.what()),
            DebugUIManager::LogType::Error);
        return false;
    }
}

/// <summary>
/// 実行時ツリーを構築
/// </summary>
BTNodePtr BossNodeEditor::BuildRuntimeTree() {
    // ノードが存在しない場合
    if (nodes_.empty()) {
        return nullptr;
    }

    // ルートノードを探す
    int rootId = FindRootNodeId();
    if (rootId == -1) {
        // ルートが見つからない場合、最初のノードをルートとする（フォールバック）
        rootId = nodes_[0].id;
    }

    // 再帰的にツリーを構築
    BTNodePtr rootNode;
    BuildRuntimeTreeRecursive(rootId, rootNode);

    return rootNode;
}

/// <summary>
/// BossBehaviorTreeからツリーをインポート
/// </summary>
void BossNodeEditor::ImportFromBehaviorTree(BossBehaviorTree* tree) {
    if (!tree) return;

    // 既存のエディタをクリア
    Clear();

    // ルートノードを取得
    BTNodePtr rootNode = tree->GetRootNode();
    if (!rootNode) {
        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] No root node found in BossBehaviorTree",
            DebugUIManager::LogType::Error);
        return;
    }

    // ノードを再帰的にインポート
    ImportNodeRecursive(rootNode, ImVec2(400, 100), 0);

    DebugUIManager::GetInstance()->AddLog(
        "[BossNodeEditor] Successfully imported BossBehaviorTree",
        DebugUIManager::LogType::Info);
    DebugUIManager::GetInstance()->AddLog(
        "[BossNodeEditor] Imported " + std::to_string(nodes_.size()) + " nodes",
        DebugUIManager::LogType::Info);
}

/// <summary>
/// 現在実行中のノードをハイライト表示（パルスエフェクト付き）
/// </summary>
void BossNodeEditor::HighlightRunningNode(const BTNodePtr& nodePtr) {
    if (!nodePtr) {
        highlightedNodeId_ = -1;
        return;
    }

    EditorNode* editorNode = FindNodeByRuntimeNode(nodePtr);
    if (editorNode) {
        // ハイライトノードを更新
        if (highlightedNodeId_ != editorNode->id) {
            highlightedNodeId_ = editorNode->id;
            highlightStartTime_ = static_cast<float>(ImGui::GetTime());
        }
    }
}

// ==========================================
// ヘルパー関数の実装
// ==========================================

/// <summary>
/// IDでノードを検索
/// </summary>
BossNodeEditor::EditorNode* BossNodeEditor::FindNodeById(int nodeId) {
    for (auto& node : nodes_) {
        if (node.id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

/// <summary>
/// IDでノードを検索（const版）
/// </summary>
const BossNodeEditor::EditorNode* BossNodeEditor::FindNodeById(int nodeId) const {
    for (const auto& node : nodes_) {
        if (node.id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

/// <summary>
/// ランタイムノードでエディタノードを検索
/// </summary>
BossNodeEditor::EditorNode* BossNodeEditor::FindNodeByRuntimeNode(const BTNodePtr& node) {
    if (!node) return nullptr;

    auto it = runtimeNodeToEditorId_.find(node.get());
    if (it != runtimeNodeToEditorId_.end()) {
        return FindNodeById(it->second);
    }
    return nullptr;
}

/// <summary>
/// IDでピンを検索
/// </summary>
BossNodeEditor::EditorPin* BossNodeEditor::FindPinById(int pinId) {
    for (auto& pin : pins_) {
        if (pin.id == pinId) {
            return &pin;
        }
    }
    return nullptr;
}

/// <summary>
/// IDでピンを検索（const版）
/// </summary>
const BossNodeEditor::EditorPin* BossNodeEditor::FindPinById(int pinId) const {
    for (const auto& pin : pins_) {
        if (pin.id == pinId) {
            return &pin;
        }
    }
    return nullptr;
}

/// <summary>
/// IDでリンクを検索
/// </summary>
BossNodeEditor::EditorLink* BossNodeEditor::FindLinkById(int linkId) {
    for (auto& link : links_) {
        if (link.id == linkId) {
            return &link;
        }
    }
    return nullptr;
}

/// <summary>
/// ルートノードIDを検索
/// </summary>
int BossNodeEditor::FindRootNodeId() const {
    // 入力リンクを持たないノードを探す
    for (const auto& node : nodes_) {
        bool hasInputLink = false;

        // このノードが何かのリンクの終点になっているか確認
        for (const auto& link : links_) {
            if (link.endNodeId == node.id) {
                hasInputLink = true;
                break;
            }
        }

        // 入力リンクがないノードがルート
        if (!hasInputLink) {
            return node.id;
        }
    }

    // ルートノードが見つからない場合
    return -1;
}

/// <summary>
/// 子ノードIDリストを取得
/// </summary>
std::vector<int> BossNodeEditor::GetChildNodeIds(int parentNodeId) const {
    std::vector<int> childIds;

    // 指定ノードから出ているリンクを探す
    for (const auto& link : links_) {
        if (link.startNodeId == parentNodeId) {
            childIds.push_back(link.endNodeId);
        }
    }

    return childIds;
}

/// <summary>
/// 循環参照チェック
/// </summary>
bool BossNodeEditor::HasCyclicDependency(int startNodeId, int endNodeId) const {
    // startNodeIdからendNodeIdへのリンクを作成した場合に、
    // endNodeIdからstartNodeIdへのパスが存在するかチェック

    // BFS（幅優先探索）で循環参照をチェック
    std::set<int> visited;
    std::queue<int> queue;
    queue.push(endNodeId);

    while (!queue.empty()) {
        int currentId = queue.front();
        queue.pop();

        // startNodeIdに到達した場合、循環参照が発生
        if (currentId == startNodeId) {
            return true;
        }

        // 既に訪問済みの場合はスキップ
        if (visited.find(currentId) != visited.end()) {
            continue;
        }
        visited.insert(currentId);

        // 現在のノードの子ノードをキューに追加
        std::vector<int> childIds = GetChildNodeIds(currentId);
        for (int childId : childIds) {
            queue.push(childId);
        }
    }

    // 循環参照なし
    return false;
}

/// <summary>
/// ノード作成
/// </summary>
void BossNodeEditor::CreateNode(const std::string& nodeType, const ImVec2& position) {
    CreateNodeWithId(nextNodeId_++, nodeType, position);
}

/// <summary>
/// 指定IDでノード作成
/// </summary>
int BossNodeEditor::CreateNodeWithId(int nodeId, const std::string& nodeType, const ImVec2& position) {
    // エディタノードを作成
    EditorNode newNode;
    newNode.id = nodeId;
    newNode.position = position;
    newNode.nodeType = nodeType;
    newNode.displayName = BossNodeFactory::GetNodeDisplayName(nodeType);
    newNode.color = BossNodeFactory::GetNodeColor(nodeType);

    // ランタイムノードを作成（依存関係なしで試みる）
    newNode.runtimeNode = BossNodeFactory::CreateNode(nodeType);
    if (!newNode.runtimeNode) {
        // 依存関係が必要な場合は、とりあえずnullptrでも作成
        // 後でSetDependenciesのようなメソッドで設定可能にする
        newNode.runtimeNode = BossNodeFactory::CreateNodeWithDependencies(nodeType, nullptr, nullptr);
    }

    // 入力ピンを作成（全ノードは親を持てる）
    EditorPin inputPin;
    inputPin.id = nextPinId_++;
    inputPin.nodeId = nodeId;
    inputPin.isInput = true;
    inputPin.name = "In";
    pins_.push_back(inputPin);
    newNode.inputPinIds.push_back(inputPin.id);

    // 出力ピンを作成（コンポジットノードのみ）
    if (BossNodeFactory::IsCompositeNode(nodeType)) {
        EditorPin outputPin;
        outputPin.id = nextPinId_++;
        outputPin.nodeId = nodeId;
        outputPin.isInput = false;
        outputPin.name = "Out";
        pins_.push_back(outputPin);
        newNode.outputPinIds.push_back(outputPin.id);
    }

    // ノードを追加
    nodes_.push_back(newNode);

    // ランタイムノードとエディタIDのマッピング
    if (newNode.runtimeNode) {
        runtimeNodeToEditorId_[newNode.runtimeNode.get()] = nodeId;
    }

    // ノード位置を設定
    //ed::SetNodePosition(nodeId, position);

    return nodeId;
}

/// <summary>
/// 再帰的にランタイムツリーを構築
/// </summary>
void BossNodeEditor::BuildRuntimeTreeRecursive(int nodeId, BTNodePtr& outNode) {
    // エディタノードを取得
    EditorNode* editorNode = FindNodeById(nodeId);
    if (!editorNode || !editorNode->runtimeNode) {
        return;
    }

    // このノードのランタイムインスタンスを設定
    outNode = editorNode->runtimeNode;

    // コンポジットノードの場合、子ノードを追加
    if (BossNodeFactory::IsCompositeNode(editorNode->nodeType)) {
        auto compositeNode = std::dynamic_pointer_cast<BTComposite>(outNode);
        if (compositeNode) {
            // 既存の子をクリア
            compositeNode->ClearChildren();

            // このノードの子ノードIDを取得
            std::vector<int> childIds = GetChildNodeIds(nodeId);

            // 各子ノードを再帰的に構築
            for (int childId : childIds) {
                BTNodePtr childNode;
                BuildRuntimeTreeRecursive(childId, childNode);
                if (childNode) {
                    compositeNode->AddChild(childNode);
                }
            }
        }
    }
}

/// <summary>
/// ノードパラメータを抽出
/// </summary>
nlohmann::json BossNodeEditor::ExtractNodeParameters(const EditorNode& node) {
    if (!node.runtimeNode) return {};

    auto inspector = CreateNodeInspector(node.runtimeNode);
    return inspector ? inspector->ExtractParams() : nlohmann::json{};
}

/// <summary>
/// ノードパラメータを適用
/// </summary>
void BossNodeEditor::ApplyNodeParameters(EditorNode& node, const nlohmann::json& params) {
    if (!node.runtimeNode || params.empty()) return;

    auto inspector = CreateNodeInspector(node.runtimeNode);
    if (inspector) {
        inspector->ApplyParams(params);
    }
}

/// <summary>
/// ノードを再帰的にインポート
/// </summary>
int BossNodeEditor::ImportNodeRecursive(const BTNodePtr& btNode, const ImVec2& position, int depth) {
    if (!btNode) return -1;

    // ノードタイプを取得
    std::string nodeType = BossNodeFactory::GetNodeType(btNode);
    if (nodeType.empty()) {
        DebugUIManager::GetInstance()->AddLog(
            "[BossNodeEditor] Unknown node type during import",
            DebugUIManager::LogType::Warning);
        return -1;
    }

    // エディタノードを作成
    int nodeId = CreateNodeWithId(nextNodeId_++, nodeType, position);
    if (nodeId == -1) {
        return -1;
    }

    auto* editorNode = FindNodeById(nodeId);
    if (!editorNode) {
        return -1;
    }

    // ランタイムノードとのマッピングを保存
    editorNode->runtimeNode = btNode;
    runtimeNodeToEditorId_[btNode.get()] = nodeId;

    // BTActionSelectorの場合、パラメータをコピー
    if (nodeType == "BTActionSelector") {
        auto srcSelector = std::dynamic_pointer_cast<BTActionSelector>(btNode);
        auto dstSelector = std::dynamic_pointer_cast<BTActionSelector>(editorNode->runtimeNode);
        if (srcSelector && dstSelector) {
            dstSelector->SetActionType(srcSelector->GetActionType());
        }
    }

    // コンポジットノードの場合、子ノードもインポート
    auto composite = std::dynamic_pointer_cast<BTComposite>(btNode);
    if (composite) {
        const auto& children = composite->GetChildren();
        float childOffsetX = 200.0f;
        float childOffsetY = 150.0f;
        float startX = position.x - (children.size() - 1) * childOffsetX * 0.5f;

        for (size_t i = 0; i < children.size(); ++i) {
            ImVec2 childPos(startX + i * childOffsetX, position.y + childOffsetY);
            int childId = ImportNodeRecursive(children[i], childPos, depth + 1);

            // リンクを作成
            if (childId != -1) {
                auto* childNode = FindNodeById(childId);
                if (childNode && !editorNode->outputPinIds.empty() && !childNode->inputPinIds.empty()) {
                    EditorLink link;
                    link.id = nextLinkId_++;
                    link.startPinId = editorNode->outputPinIds[0];
                    link.endPinId = childNode->inputPinIds[0];
                    link.startNodeId = nodeId;
                    link.endNodeId = childId;
                    links_.push_back(link);
                }
            }
        }
    }

    // ノード位置を設定
    ed::SetNodePosition(nodeId, position);

    return nodeId;
}

/// <summary>
/// リンク作成ヘルパー
/// </summary>
bool BossNodeEditor::CreateLink(int sourceNodeId, int targetNodeId) {
    auto* sourceNode = FindNodeById(sourceNodeId);
    auto* targetNode = FindNodeById(targetNodeId);

    if (!sourceNode || !targetNode) {
        return false;
    }

    if (sourceNode->outputPinIds.empty() || targetNode->inputPinIds.empty()) {
        return false;
    }

    // リンクを作成
    EditorLink link;
    link.id = nextLinkId_++;
    link.startPinId = sourceNode->outputPinIds[0];
    link.endPinId = targetNode->inputPinIds[0];
    link.startNodeId = sourceNodeId;
    link.endNodeId = targetNodeId;

    links_.push_back(link);
    return true;
}

/// <summary>
/// デフォルトツリーの作成（BuildActionTreeと同じ構造）
/// </summary>
void BossNodeEditor::CreateDefaultTree() {
    // 既存のノードをクリア
    Clear();

    // ノード作成
    // 1. Root Sequence (MainLoop)
    int rootId = CreateNodeWithId(nextNodeId_++, "BTSequence", ImVec2(400, 100));
    auto* rootNode = FindNodeById(rootId);
    if (rootNode) {
        rootNode->displayName = "MainLoop";
    }

    // 2. BTBossIdle
    int idleId = CreateNodeWithId(nextNodeId_++, "BTBossIdle", ImVec2(250, 250));
    auto* idleNode = FindNodeById(idleId);
    if (idleNode) {
        idleNode->displayName = "Idle";
    }

    // 3. Action Selector
    int selectorId = CreateNodeWithId(nextNodeId_++, "BTSelector", ImVec2(550, 250));
    auto* selectorNode = FindNodeById(selectorId);
    if (selectorNode) {
        selectorNode->displayName = "ActionSelector";
    }

    // 4. Dash Sequence
    int dashSeqId = CreateNodeWithId(nextNodeId_++, "BTSequence", ImVec2(350, 400));
    auto* dashSeqNode = FindNodeById(dashSeqId);
    if (dashSeqNode) {
        dashSeqNode->displayName = "DashSequence";
    }

    // 5. Dash Condition (BTActionSelector with ActionType::Dash)
    int dashCondId = CreateNodeWithId(nextNodeId_++, "BTActionSelector", ImVec2(250, 550));
    auto* dashCondNode = FindNodeById(dashCondId);
    if (dashCondNode) {
        dashCondNode->displayName = "DashCondition";
        // ActionTypeをDashに設定
        if (dashCondNode->runtimeNode) {
            auto selector = std::dynamic_pointer_cast<BTActionSelector>(dashCondNode->runtimeNode);
            if (selector) {
                selector->SetActionType(BTActionSelector::ActionType::Dash);
            }
        }
    }

    // 6. Dash Action
    int dashActId = CreateNodeWithId(nextNodeId_++, "BTBossDash", ImVec2(450, 550));
    auto* dashActNode = FindNodeById(dashActId);
    if (dashActNode) {
        dashActNode->displayName = "DashAction";
    }

    // 7. Shoot Sequence
    int shootSeqId = CreateNodeWithId(nextNodeId_++, "BTSequence", ImVec2(750, 400));
    auto* shootSeqNode = FindNodeById(shootSeqId);
    if (shootSeqNode) {
        shootSeqNode->displayName = "ShootSequence";
    }

    // 8. Shoot Condition (BTActionSelector with ActionType::Shoot)
    int shootCondId = CreateNodeWithId(nextNodeId_++, "BTActionSelector", ImVec2(650, 550));
    auto* shootCondNode = FindNodeById(shootCondId);
    if (shootCondNode) {
        shootCondNode->displayName = "ShootCondition";
        // ActionTypeをShootに設定
        if (shootCondNode->runtimeNode) {
            auto selector = std::dynamic_pointer_cast<BTActionSelector>(shootCondNode->runtimeNode);
            if (selector) {
                selector->SetActionType(BTActionSelector::ActionType::Shoot);
            }
        }
    }

    // 9. Shoot Action
    int shootActId = CreateNodeWithId(nextNodeId_++, "BTBossShoot", ImVec2(850, 550));
    auto* shootActNode = FindNodeById(shootActId);
    if (shootActNode) {
        shootActNode->displayName = "ShootAction";
    }

    // リンク作成
    // MainLoop -> Idle
    CreateLink(rootId, idleId);
    // MainLoop -> ActionSelector
    CreateLink(rootId, selectorId);
    // ActionSelector -> DashSequence
    CreateLink(selectorId, dashSeqId);
    // ActionSelector -> ShootSequence
    CreateLink(selectorId, shootSeqId);
    // DashSequence -> DashCondition
    CreateLink(dashSeqId, dashCondId);
    // DashSequence -> DashAction
    CreateLink(dashSeqId, dashActId);
    // ShootSequence -> ShootCondition
    CreateLink(shootSeqId, shootCondId);
    // ShootSequence -> ShootAction
    CreateLink(shootSeqId, shootActId);

    // 自動的にJSONに保存
    SaveToJSON("resources/Json/BossTree.json");

    DebugUIManager::GetInstance()->AddLog(
        "[BossNodeEditor] Default tree created and saved to BossTree.json",
        DebugUIManager::LogType::Info);
}

#endif // _DEBUG