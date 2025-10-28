#ifdef _DEBUG

#include "CameraAnimationEditor.h"
#include "CameraAnimationTimeline.h"
#include "CameraAnimationCurveEditor.h"
#include "CameraAnimationHistory.h"
#include "CameraSystem/CameraAnimationController.h"
#include <imgui.h>
// #include <imgui_internal.h> // 不要な場合はコメントアウト
#include <algorithm>
#include <sstream>

CameraAnimationEditor::CameraAnimationEditor() {
    // コンポーネントの初期化は Initialize で行う
}

CameraAnimationEditor::~CameraAnimationEditor() {
    // unique_ptr が自動的にクリーンアップ
}

void CameraAnimationEditor::Initialize(CameraAnimation* animation, Camera* camera) {
    animation_ = animation;
    camera_ = camera;

    // コンポーネントの初期化
    timeline_ = std::make_unique<CameraAnimationTimeline>();
    timeline_->Initialize(animation);

    curveEditor_ = std::make_unique<CameraAnimationCurveEditor>();
    curveEditor_->Initialize(animation);

    history_ = std::make_unique<CameraAnimationHistory>();
    history_->Initialize(animation);
}

void CameraAnimationEditor::Initialize(CameraAnimationController* controller, Camera* camera) {
    controller_ = controller;
    camera_ = camera;

    // 現在のアニメーションを取得
    animation_ = controller ? controller->GetCurrentAnimation() : nullptr;

    // コンポーネントの初期化
    if (animation_) {
        timeline_ = std::make_unique<CameraAnimationTimeline>();
        timeline_->Initialize(animation_);

        curveEditor_ = std::make_unique<CameraAnimationCurveEditor>();
        curveEditor_->Initialize(animation_);

        history_ = std::make_unique<CameraAnimationHistory>();
        history_->Initialize(animation_);
    }
}

void CameraAnimationEditor::Draw() {
    if (!isOpen_ || !animation_ || !camera_) {
        return;
    }

    // メインウィンドウ
    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Camera Animation Editor", &isOpen_, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    // ショートカット処理
    ProcessShortcuts();

    // メニューバー
    DrawMenuBar();

    // アニメーション選択UI（コントローラー経由の場合のみ）
    if (controller_) {
        DrawAnimationSelector();
    }

    // レイアウトに応じた描画
    switch (layoutMode_) {
        case LayoutMode::COMPACT:
            DrawPlaybackControls();
            DrawTimelinePanel();
            DrawKeyframeList();
            break;

        case LayoutMode::STANDARD:
            // 左右分割レイアウト
            {
                float availWidth = ImGui::GetContentRegionAvail().x;
                float leftWidth = availWidth - inspectorPanelWidth_ - 10.0f;

                // 左側：タイムラインとカーブエディター
                if (ImGui::BeginChild("LeftPanel", ImVec2(leftWidth, 0), false)) {
                    DrawPlaybackControls();
                    DrawTimelinePanel();

                    if (ImGui::CollapsingHeader("Curve Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
                        DrawCurveEditorPanel();
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // 右側：インスペクター
                if (ImGui::BeginChild("RightPanel", ImVec2(inspectorPanelWidth_, 0), false)) {
                    DrawInspectorPanel();
                }
                ImGui::EndChild();
            }
            break;

        case LayoutMode::ADVANCED:
            // 高度なレイアウト（全機能表示）
            {
                // 上部：再生コントロールとツール
                DrawPlaybackControls();

                ImGui::Separator();

                // 中央：タイムラインとカーブエディター
                float availHeight = ImGui::GetContentRegionAvail().y - 25.0f; // ステータスバー分を引く
                float topHeight = availHeight * 0.4f;
                float bottomHeight = availHeight * 0.6f;

                // タイムライン
                if (ImGui::BeginChild("TimelineSection", ImVec2(0, topHeight), true)) {
                    DrawTimelinePanel();
                }
                ImGui::EndChild();

                // 下部を2分割
                float availWidth = ImGui::GetContentRegionAvail().x;
                float leftWidth = availWidth * 0.7f;
                float rightWidth = availWidth * 0.3f;

                // 左：カーブエディター
                if (ImGui::BeginChild("CurveSection", ImVec2(leftWidth - 5, bottomHeight), true)) {
                    ImGui::Text("Curve Editor");
                    ImGui::Separator();
                    DrawCurveEditorPanel();
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // 右：インスペクター
                if (ImGui::BeginChild("InspectorSection", ImVec2(rightWidth - 5, bottomHeight), true)) {
                    ImGui::Text("Inspector");
                    ImGui::Separator();
                    DrawInspectorPanel();
                }
                ImGui::EndChild();
            }
            break;

        case LayoutMode::CUSTOM:
            // カスタムレイアウト（ユーザー設定）
            DrawPlaybackControls();
            DrawTimelinePanel();
            break;
    }

    // ステータスバー
    DrawStatusBar();

    ImGui::End();

    // プレビューウィンドウ（別ウィンドウ）- 無効化
    // if (previewMode_ != PreviewMode::NONE) {
    //     ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    //     if (ImGui::Begin("Camera Preview", nullptr)) {
    //         DrawPreviewPanel();
    //     }
    //     ImGui::End();
    // }
}

void CameraAnimationEditor::Update(float deltaTime) {
    if (!animation_ || !camera_) {
        return;
    }

    // タイムラインの更新
    if (timeline_) {
        timeline_->Update(deltaTime);
    }

    // スクラブ再生中の処理
    if (isScrubbing_) {
        animation_->SetCurrentTime(scrubTime_);
    }
}

void CameraAnimationEditor::ProcessShortcuts() {
    // Ctrl+Z: Undo
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        Undo();
    }

    // Ctrl+Y: Redo
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        Redo();
    }

    // Ctrl+C: Copy
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C)) {
        CopySelectedKeyframes();
    }

    // Ctrl+V: Paste
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_V)) {
        PasteKeyframes();
    }

    // Delete: Delete selected keyframes
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        DeleteSelectedKeyframes();
    }

    // Space: Play/Pause
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        if (animation_->GetPlayState() == CameraAnimation::PlayState::PLAYING) {
            animation_->Pause();
        } else {
            animation_->Play();
        }
    }

    // Escape: Deselect all
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        selectedKeyframes_.clear();
    }
}

void CameraAnimationEditor::DrawMenuBar() {
    if (!ImGui::BeginMenuBar()) {
        return;
    }

    // File menu
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Animation", "Ctrl+N")) {
            animation_->ClearKeyframes();
            selectedKeyframes_.clear();
        }

        if (ImGui::MenuItem("Load...", "Ctrl+O")) {
            // TODO: ファイルダイアログを開く
        }

        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            animation_->SaveToJson(animation_->GetAnimationName() + ".json");
        }

        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
            // TODO: ファイルダイアログを開く
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Close", "Alt+F4")) {
            Close();
        }

        ImGui::EndMenu();
    }

    // Edit menu
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, history_ && history_->CanUndo())) {
            Undo();
        }

        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, history_ && history_->CanRedo())) {
            Redo();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Copy", "Ctrl+C", false, !selectedKeyframes_.empty())) {
            CopySelectedKeyframes();
        }

        if (ImGui::MenuItem("Paste", "Ctrl+V", false, !clipboard_.empty())) {
            PasteKeyframes();
        }

        if (ImGui::MenuItem("Delete", "Del", false, !selectedKeyframes_.empty())) {
            DeleteSelectedKeyframes();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Select All", "Ctrl+A")) {
            selectedKeyframes_.clear();
            for (size_t i = 0; i < animation_->GetKeyframeCount(); ++i) {
                selectedKeyframes_.push_back(static_cast<int>(i));
            }
        }

        ImGui::EndMenu();
    }

    // View menu
    if (ImGui::BeginMenu("View")) {
        if (ImGui::BeginMenu("Layout")) {
            if (ImGui::MenuItem("Compact", nullptr, layoutMode_ == LayoutMode::COMPACT)) {
                layoutMode_ = LayoutMode::COMPACT;
            }
            if (ImGui::MenuItem("Standard", nullptr, layoutMode_ == LayoutMode::STANDARD)) {
                layoutMode_ = LayoutMode::STANDARD;
            }
            if (ImGui::MenuItem("Advanced", nullptr, layoutMode_ == LayoutMode::ADVANCED)) {
                layoutMode_ = LayoutMode::ADVANCED;
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::MenuItem("Show Motion Path", nullptr, &showMotionPath_);
        ImGui::MenuItem("Show Ghost Frames", nullptr, &showGhostFrames_);
        ImGui::MenuItem("Show Time Ruler", nullptr, &showTimeRuler_);
        ImGui::MenuItem("Show Property Tracks", nullptr, &showPropertyTracks_);

        ImGui::EndMenu();
    }

    // Animation menu
    if (ImGui::BeginMenu("Animation")) {
        // キーフレーム操作
        if (ImGui::MenuItem("Add Keyframe", "A", false, camera_ && animation_)) {
            if (camera_ && animation_) {
                float currentTime = animation_->GetCurrentTime();
                CameraKeyframe newKf;
                newKf.time = currentTime;
                newKf.position = camera_->GetTranslate();
                newKf.rotation = camera_->GetRotate();
                newKf.fov = camera_->GetFovY();
                newKf.interpolation = CameraKeyframe::InterpolationType::LINEAR;

                animation_->AddKeyframe(newKf);
                if (history_) {
                    history_->RecordAdd(animation_->GetKeyframeCount() - 1);
                }
            }
        }

        if (ImGui::MenuItem("Delete Selected", "Delete", false, !selectedKeyframes_.empty())) {
            DeleteSelectedKeyframes();
        }

        if (ImGui::MenuItem("Clear All Keyframes", nullptr, false, animation_ && animation_->GetKeyframeCount() > 0)) {
            if (animation_) {
                animation_->ClearKeyframes();
                selectedKeyframes_.clear();
            }
        }

        ImGui::Separator();

        ImGui::MenuItem("Auto Keyframe", nullptr, &autoKeyframe_);
        ImGui::MenuItem("Enable Grid Snap", nullptr, &enableGridSnap_);

        ImGui::Separator();

        if (ImGui::BeginMenu("Grid Snap Interval")) {
            if (ImGui::MenuItem("0.1 sec", nullptr, gridSnapInterval_ == 0.1f)) {
                gridSnapInterval_ = 0.1f;
            }
            if (ImGui::MenuItem("0.25 sec", nullptr, gridSnapInterval_ == 0.25f)) {
                gridSnapInterval_ = 0.25f;
            }
            if (ImGui::MenuItem("0.5 sec", nullptr, gridSnapInterval_ == 0.5f)) {
                gridSnapInterval_ = 0.5f;
            }
            if (ImGui::MenuItem("1.0 sec", nullptr, gridSnapInterval_ == 1.0f)) {
                gridSnapInterval_ = 1.0f;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    // Help menu
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Shortcuts")) {
            ImGui::OpenPopup("ShortcutsPopup");
        }
        if (ImGui::MenuItem("About")) {
            ImGui::OpenPopup("AboutPopup");
        }
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

    // ポップアップ
    if (ImGui::BeginPopupModal("ShortcutsPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::Separator();
        ImGui::Text("Space: Play/Pause");
        ImGui::Text("Ctrl+Z: Undo");
        ImGui::Text("Ctrl+Y: Redo");
        ImGui::Text("Ctrl+C: Copy");
        ImGui::Text("Ctrl+V: Paste");
        ImGui::Text("Delete: Delete Selected");
        ImGui::Text("1-5: Change Edit Mode");
        ImGui::Separator();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


void CameraAnimationEditor::DrawPlaybackControls() {
    if (!animation_) {
        ImGui::Text("No animation loaded");
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 8));

    // 再生ボタン
    if (animation_->GetPlayState() == CameraAnimation::PlayState::PLAYING) {
        if (ImGui::Button("||", ImVec2(40, 0))) {
            // コントローラー経由で呼び出し（isActive_フラグを更新するため）
            if (controller_) {
                controller_->Pause();
            } else {
                animation_->Pause();
            }
        }
    } else {
        if (ImGui::Button(">", ImVec2(40, 0))) {
            // コントローラー経由で呼び出し（isActive_フラグを更新するため）
            if (controller_) {
                controller_->Play();
            } else {
                animation_->Play();
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("[]", ImVec2(40, 0))) {
        // コントローラー経由で呼び出し（isActive_フラグを更新するため）
        if (controller_) {
            controller_->Stop();
        } else {
            animation_->Stop();
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("|<", ImVec2(40, 0))) {
        animation_->SetCurrentTime(0.0f);
    }

    ImGui::SameLine();
    if (ImGui::Button(">|", ImVec2(40, 0))) {
        animation_->SetCurrentTime(animation_->GetDuration());
    }

    // タイムディスプレイ
    ImGui::SameLine();
    ImGui::Text("Time: %.2f / %.2f", animation_->GetCurrentTime(), animation_->GetDuration());

    // ループトグル
    ImGui::SameLine();
    bool isLooping = animation_->IsLooping();
    if (ImGui::Checkbox("Loop", &isLooping)) {
        animation_->SetLooping(isLooping);
    }

    // 再生速度
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    static float playSpeed = 1.0f;
    if (ImGui::DragFloat("Speed", &playSpeed, 0.01f, -2.0f, 2.0f, "%.2fx")) {
        animation_->SetPlaySpeed(playSpeed);
    }

    ImGui::PopStyleVar();
}

void CameraAnimationEditor::DrawTimelinePanel() {
    if (!animation_) {
        ImGui::Text("No animation loaded");
        return;
    }

    if (timeline_) {
        // タイムラインコンポーネントに描画を委譲
        timeline_->Draw(timelineZoom_, timelineOffset_);

        // 選択状態を同期
        selectedKeyframes_ = timeline_->GetSelectedKeyframes();
        hoveredKeyframe_ = timeline_->GetHoveredKeyframe();
        isDragging_ = timeline_->IsDragging();
    }
}

void CameraAnimationEditor::DrawInspectorPanel() {
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (!animation_) {
        ImGui::TextDisabled("No animation loaded");
        return;
    }

    // 新規追加：キーフレーム追加セクション
    if (ImGui::CollapsingHeader("Add New Keyframe", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float newKeyTime = 0.0f;
        ImGui::DragFloat("Time (seconds)", &newKeyTime, 0.01f, 0.0f, 10.0f);

        if (ImGui::Button("Add from Current Camera")) {
            if (camera_) {
                CameraKeyframe newKf;
                newKf.time = enableGridSnap_ ? SnapToGrid(newKeyTime) : newKeyTime;
                newKf.position = camera_->GetTranslate();
                newKf.rotation = camera_->GetRotate();
                newKf.fov = camera_->GetFovY();
                newKf.interpolation = CameraKeyframe::InterpolationType::LINEAR;

                animation_->AddKeyframe(newKf);
                if (history_) {
                    history_->RecordAdd(animation_->GetKeyframeCount() - 1);
                }

                // 時間を次のポイントに自動で進める
                newKeyTime = newKeyTime + 1.0f;
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Camera not available");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Add Default")) {
            CameraKeyframe defaultKf;
            defaultKf.time = enableGridSnap_ ? SnapToGrid(newKeyTime) : newKeyTime;
            defaultKf.position = Vector3(0.0f, 5.0f, -10.0f);
            defaultKf.rotation = Vector3(0.2f, 0.0f, 0.0f);
            defaultKf.fov = 45.0f * 3.14159265f / 180.0f;
            defaultKf.interpolation = CameraKeyframe::InterpolationType::LINEAR;

            animation_->AddKeyframe(defaultKf);
            if (history_) {
                history_->RecordAdd(animation_->GetKeyframeCount() - 1);
            }

            newKeyTime = newKeyTime + 1.0f;
        }

        ImGui::Separator();
    }

    // 選択中のキーフレーム情報
    if (selectedKeyframes_.empty()) {
        ImGui::TextDisabled("No keyframe selected");
        return;
    }

    if (selectedKeyframes_.size() == 1) {
        // 単一選択
        int idx = selectedKeyframes_[0];
        if (idx >= 0 && idx < static_cast<int>(animation_->GetKeyframeCount())) {
            CameraKeyframe kf = animation_->GetKeyframe(idx);
            bool changed = false;

            ImGui::Text("Keyframe %d", idx);
            ImGui::Separator();

            // 時間
            if (ImGui::DragFloat("Time", &kf.time, 0.01f, 0.0f, animation_->GetDuration())) {
                if (enableGridSnap_) {
                    kf.time = SnapToGrid(kf.time);
                }
                changed = true;
            }

            // 位置
            if (ImGui::DragFloat3("Position", &kf.position.x, 0.1f)) {
                changed = true;
            }

            // 回転（度単位）
            Vector3 rotDeg = {
                kf.rotation.x * 180.0f / 3.14159265f,
                kf.rotation.y * 180.0f / 3.14159265f,
                kf.rotation.z * 180.0f / 3.14159265f
            };
            if (ImGui::DragFloat3("Rotation", &rotDeg.x, 1.0f)) {
                kf.rotation = {
                    rotDeg.x * 3.14159265f / 180.0f,
                    rotDeg.y * 3.14159265f / 180.0f,
                    rotDeg.z * 3.14159265f / 180.0f
                };
                changed = true;
            }

            // FOV（度単位）
            float fovDeg = kf.fov * 180.0f / 3.14159265f;
            if (ImGui::DragFloat("FOV", &fovDeg, 0.5f, 10.0f, 120.0f)) {
                kf.fov = fovDeg * 3.14159265f / 180.0f;
                changed = true;
            }

            // 補間タイプ
            const char* interpTypes[] = { "Linear", "Ease In", "Ease Out", "Ease In-Out" };
            int currentType = static_cast<int>(kf.interpolation);
            if (ImGui::Combo("Interpolation", &currentType, interpTypes, 4)) {
                kf.interpolation = static_cast<CameraKeyframe::InterpolationType>(currentType);
                changed = true;
            }

            if (changed) {
                // 履歴に記録してから編集
                if (history_) {
                    history_->RecordEdit(idx, animation_->GetKeyframe(idx), kf);
                }
                animation_->EditKeyframe(idx, kf);
            }

            // 現在のカメラ状態を適用ボタン
            if (ImGui::Button("Apply Current Camera")) {
                if (camera_) {
                    kf.position = camera_->GetTranslate();
                    kf.rotation = camera_->GetRotate();
                    kf.fov = camera_->GetFovY();
                    animation_->EditKeyframe(idx, kf);
                }
            }

            // 削除ボタン（赤色で強調）
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("Delete Keyframe", ImVec2(-1, 0))) {
                DeleteSelectedKeyframes();
            }
            ImGui::PopStyleColor(3);
        }
    } else {
        // 複数選択
        ImGui::Text("%d keyframes selected", static_cast<int>(selectedKeyframes_.size()));
        ImGui::Separator();

        // 一括操作
        static Vector3 offsetPos = { 0, 0, 0 };
        if (ImGui::DragFloat3("Offset Position", &offsetPos.x, 0.1f)) {
            // 適用ボタンで実行
        }

        if (ImGui::Button("Apply Offset")) {
            for (int idx : selectedKeyframes_) {
                if (idx >= 0 && idx < static_cast<int>(animation_->GetKeyframeCount())) {
                    CameraKeyframe kf = animation_->GetKeyframe(idx);
                    kf.position.x += offsetPos.x;
                    kf.position.y += offsetPos.y;
                    kf.position.z += offsetPos.z;
                    animation_->EditKeyframe(idx, kf);
                }
            }
            offsetPos = { 0, 0, 0 };
        }

        // 削除ボタン（複数選択時、赤色で強調）
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Delete Selected Keyframes", ImVec2(-1, 0))) {
            DeleteSelectedKeyframes();
        }
        ImGui::PopStyleColor(3);
    }
}

void CameraAnimationEditor::DrawCurveEditorPanel() {
    if (curveEditor_) {
        curveEditor_->Draw(selectedKeyframes_);
    }
}

void CameraAnimationEditor::DrawPreviewPanel() {
    // プレビューのレンダリングをここに実装
    // 実際の3Dビューポートの描画は複雑なため、簡略化
    ImGui::Text("Camera Preview");
    ImGui::Separator();

    if (showMotionPath_) {
        ImGui::Text("Motion Path: ON");
    }

    if (showGhostFrames_) {
        ImGui::Text("Ghost Frames: %d", ghostFrameCount_);
    }

    // カメラ位置情報
    if (camera_) {
        Vector3 pos = camera_->GetTranslate();
        Vector3 rot = camera_->GetRotate();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Rotation: (%.1f, %.1f, %.1f)",
            rot.x * 180.0f / 3.14159265f,
            rot.y * 180.0f / 3.14159265f,
            rot.z * 180.0f / 3.14159265f);
    }
}

void CameraAnimationEditor::DrawStatusBar() {
    ImGui::Separator();

    if (!animation_) {
        ImGui::Text("Status: No animation loaded - Add keyframes to begin");
        return;
    }

    // ステータス情報
    const char* modeNames[] = { "Select", "Move", "Scale", "Add", "Delete", "Scrub" };
    const char* modeHelp[] = {
        "Click keyframes to select",
        "Drag selected keyframes to move",
        "Drag to scale timing",
        "Double-click timeline to add keyframe",
        "Select and press Delete key",
        "Drag to preview animation"
    };

    // 統計情報
    ImGui::Text("Keyframes: %zu | Selected: %zu | Snap: %s (%.2fs)",
        animation_->GetKeyframeCount(),
        selectedKeyframes_.size(),
        enableGridSnap_ ? "ON" : "OFF",
        gridSnapInterval_);
}

void CameraAnimationEditor::DrawKeyframeList() {
    ImGui::Text("Keyframes");
    ImGui::Separator();

    if (ImGui::BeginChild("KeyframeListScroll", ImVec2(0, 150), true)) {
        for (size_t i = 0; i < animation_->GetKeyframeCount(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            const CameraKeyframe& kf = animation_->GetKeyframe(i);

            // 選択状態をチェック
            bool isSelected = std::find(selectedKeyframes_.begin(),
                                       selectedKeyframes_.end(),
                                       static_cast<int>(i)) != selectedKeyframes_.end();

            char label[128];
            snprintf(label, sizeof(label), "KF %zu: %.2fs", i, kf.time);

            if (ImGui::Selectable(label, isSelected)) {
                if (ImGui::GetIO().KeyCtrl) {
                    // Ctrl+クリックで複数選択
                    if (isSelected) {
                        selectedKeyframes_.erase(
                            std::remove(selectedKeyframes_.begin(),
                                      selectedKeyframes_.end(),
                                      static_cast<int>(i)),
                            selectedKeyframes_.end());
                    } else {
                        selectedKeyframes_.push_back(static_cast<int>(i));
                    }
                } else {
                    // 通常クリックで単一選択
                    selectedKeyframes_.clear();
                    selectedKeyframes_.push_back(static_cast<int>(i));
                }
            }

            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

float CameraAnimationEditor::SnapToGrid(float time) const {
    if (!enableGridSnap_) {
        return time;
    }
    return std::round(time / gridSnapInterval_) * gridSnapInterval_;
}

void CameraAnimationEditor::CopySelectedKeyframes() {
    clipboard_.clear();
    for (int idx : selectedKeyframes_) {
        if (idx >= 0 && idx < static_cast<int>(animation_->GetKeyframeCount())) {
            clipboard_.push_back(animation_->GetKeyframe(idx));
        }
    }
}

void CameraAnimationEditor::PasteKeyframes() {
    if (clipboard_.empty()) {
        return;
    }

    float currentTime = animation_->GetCurrentTime();
    float minTime = clipboard_[0].time;

    // クリップボード内の最小時間を基準にペースト
    for (const auto& kf : clipboard_) {
        CameraKeyframe newKf = kf;
        newKf.time = currentTime + (kf.time - minTime);

        if (history_) {
            history_->RecordAdd(animation_->GetKeyframeCount());
        }
        animation_->AddKeyframe(newKf);
    }
}

void CameraAnimationEditor::DeleteSelectedKeyframes() {
    if (selectedKeyframes_.empty()) {
        return;
    }

    // インデックスを降順にソート（後ろから削除するため）
    std::sort(selectedKeyframes_.rbegin(), selectedKeyframes_.rend());

    for (int idx : selectedKeyframes_) {
        if (idx >= 0 && idx < static_cast<int>(animation_->GetKeyframeCount())) {
            if (history_) {
                history_->RecordDelete(idx, animation_->GetKeyframe(idx));
            }
            animation_->RemoveKeyframe(idx);
        }
    }

    selectedKeyframes_.clear();
}

void CameraAnimationEditor::Undo() {
    if (history_ && history_->CanUndo()) {
        history_->Undo();
    }
}

void CameraAnimationEditor::Redo() {
    if (history_ && history_->CanRedo()) {
        history_->Redo();
    }
}

void CameraAnimationEditor::DrawAnimationSelector() {
    if (!controller_) return;

    ImGui::Separator();

    // アニメーション選択コンボボックス
    auto animList = controller_->GetAnimationList();
    std::string currentName = controller_->GetCurrentAnimationName();

    ImGui::Text("Animation:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::BeginCombo("##AnimSelect", currentName.c_str())) {
        for (const auto& name : animList) {
            bool isSelected = (name == currentName);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                if (controller_->SwitchAnimation(name)) {
                    // アニメーション切り替え成功
                    animation_ = controller_->GetCurrentAnimation();

                    // コンポーネントを再初期化
                    if (animation_) {
                        timeline_->Initialize(animation_);
                        curveEditor_->Initialize(animation_);
                        history_->Initialize(animation_);
                    }
                }
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // 新規作成ボタン
    if (ImGui::Button("New")) {
        ImGui::OpenPopup("NewAnimation");
    }

    ImGui::SameLine();

    // 複製ボタン
    if (ImGui::Button("Duplicate")) {
        std::string newName = currentName + "_copy";
        if (controller_->DuplicateAnimation(currentName, newName)) {
            controller_->SwitchAnimation(newName);
            animation_ = controller_->GetCurrentAnimation();
        }
    }

    ImGui::SameLine();

    // 削除ボタン
    if (currentName != "Default") {
        if (ImGui::Button("Delete")) {
            ImGui::OpenPopup("DeleteAnimation");
        }
    }

    ImGui::SameLine();

    // 保存ボタン
    if (ImGui::Button("Save")) {
        // TODO: ファイル選択ダイアログ実装
        std::string fileName = currentName + ".json";
        controller_->SaveAnimationToFile(currentName, fileName);
    }

    ImGui::SameLine();

    // 読み込みボタン
    if (ImGui::Button("Load")) {
        ImGui::OpenPopup("LoadAnimation");
    }

    // 新規作成ダイアログ
    if (ImGui::BeginPopup("NewAnimation")) {
        static char nameBuf[128] = "NewAnimation";
        ImGui::Text("Animation Name:");
        ImGui::InputText("##Name", nameBuf, sizeof(nameBuf));

        if (ImGui::Button("Create")) {
            if (controller_->CreateAnimation(nameBuf)) {
                controller_->SwitchAnimation(nameBuf);
                animation_ = controller_->GetCurrentAnimation();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // 削除確認ダイアログ
    if (ImGui::BeginPopup("DeleteAnimation")) {
        ImGui::Text("Delete animation '%s'?", currentName.c_str());
        ImGui::Text("This action cannot be undone.");

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            controller_->DeleteAnimation(currentName);
            animation_ = controller_->GetCurrentAnimation();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // 読み込みダイアログ
    if (ImGui::BeginPopup("LoadAnimation")) {
        static char nameBuf[128] = "LoadedAnimation";
        static char pathBuf[256] = "resources/CameraAnimations/";

        ImGui::Text("Animation Name:");
        ImGui::InputText("##LoadName", nameBuf, sizeof(nameBuf));

        ImGui::Text("File Path:");
        ImGui::InputText("##LoadPath", pathBuf, sizeof(pathBuf));

        if (ImGui::Button("Load")) {
            if (controller_->LoadAnimationFromFile(pathBuf, nameBuf)) {
                controller_->SwitchAnimation(nameBuf);
                animation_ = controller_->GetCurrentAnimation();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();
}

#endif // _DEBUG