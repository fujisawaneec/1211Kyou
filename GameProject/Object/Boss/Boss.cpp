#include "Boss.h"

#include <algorithm>
#include "Object3d.h"
#include "OBBCollider.h"
#include "CollisionManager.h"
#include "../../Collision/CollisionTypeIdDef.h"
#include "FrameTimer.h"
#include "Sprite.h"
#include "WinApp.h"
#include "States/BossStateMachine.h"
#include "States/BossIdleState.h"
#include "States/BossDashState.h"
#include "States/BossShootState.h"
#include "BossBehaviorTree/BossBehaviorTree.h"

#ifdef _DEBUG
#include "ImGuiManager.h"
#include "BossNodeEditor/BossNodeEditor.h"
#endif

Boss::Boss()
{
}

Boss::~Boss()
{
}

void Boss::Initialize()
{
    model_ = std::make_unique<Object3d>();
    model_->Initialize();
    model_->SetModel("white_cube.gltf");
    model_->SetMaterialColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

    transform_.translate = Vector3(0.0f, 2.5f, 10.0f);
    transform_.rotate = Vector3(0.0f, 0.0f, 0.0f);
    transform_.scale = Vector3(1.0f, 1.0f, 1.0f);

    model_->SetTransform(transform_);

    // HPバースプライトの初期化
    hpBarSprite1_ = std::make_unique<Sprite>();
    hpBarSprite1_->Initialize("white.png");
    hpBarSize1_ = Vector2(500.0f, 30.0f);
    hpBarSprite1_->SetSize(hpBarSize1_);
    hpBarSprite1_->SetColor({ 0.5f, 0.5f, 1.0f, 1.0f });
    hpBarSprite1_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));

    hpBarSprite2_ = std::make_unique<Sprite>();
    hpBarSprite2_->Initialize("white.png");
    hpBarSize2_ = Vector2(500.0f, 30.0f);
    hpBarSprite2_->SetSize(hpBarSize2_);
    hpBarSprite2_->SetColor({ 1.0f, 0.3f, 0.3f, 1.0f });
    hpBarSprite2_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));

    hpBarBGSprite_ = std::make_unique<Sprite>();
    hpBarBGSprite_->Initialize("white.png");
    hpBarBGSprite_->SetSize(hpBarSize1_);
    hpBarBGSprite_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));

    // Colliderの初期化
    bodyCollider_ = std::make_unique<OBBCollider>();
    bodyCollider_->SetTransform(&transform_);
    bodyCollider_->SetSize(Vector3(3.2f, 3.2f, 3.2f));
    bodyCollider_->SetOffset(Vector3(0.0f, 0.0f, 0.0f));
    bodyCollider_->SetTypeID(static_cast<uint32_t>(CollisionTypeId::BOSS));
    bodyCollider_->SetOwner(this);

    // CollisionManagerに登録
    CollisionManager::GetInstance()->AddCollider(bodyCollider_.get());

    // ビヘイビアツリーを使用する場合
    if (useBehaviorTree_) {
        // ビヘイビアツリーの初期化
        behaviorTree_ = std::make_unique<BossBehaviorTree>(this, player_);

#ifdef _DEBUG
        // ノードエディタの初期化
        nodeEditor_ = std::make_unique<BossNodeEditor>();
        nodeEditor_->Initialize();
        // 現在のビヘイビアツリーをエディタにインポート（後で実装）
        // nodeEditor_->ImportFromBehaviorTree(behaviorTree_.get());
#endif
    } else {
        // ステートマシンの初期化（互換性のため残す）
        stateMachine_ = std::make_unique<BossStateMachine>();
        stateMachine_->Initialize(this);

        // 各状態を追加
        stateMachine_->AddState("Idle", std::make_unique<BossIdleState>());
        stateMachine_->AddState("Dash", std::make_unique<BossDashState>());
        stateMachine_->AddState("Shoot", std::make_unique<BossShootState>());

        // 初期状態をIdleに設定
        stateMachine_->ChangeState("Idle");
    }
}

void Boss::Finalize()
{
    // Colliderを削除
    if (bodyCollider_) {
        CollisionManager::GetInstance()->RemoveCollider(bodyCollider_.get());
    }
}

void Boss::Update(float deltaTime)
{
    // HPバーの更新
    if (phase_ == 1) {
        hpBarSprite1_->SetSize(Vector2(hpBarSize1_.x * (hp_ - 100.0f) / 100.0f, hpBarSize1_.y));
        hpBarSprite2_->SetSize(Vector2(hpBarSize2_.x, hpBarSize2_.y));
    }
    else if (phase_ == 2) {
        hpBarSprite1_->SetSize(Vector2(0.0f, hpBarSize1_.y));
        hpBarSprite2_->SetSize(Vector2(hpBarSize2_.x * (hp_ / 100.0f), hpBarSize2_.y));
    }
    hpBarSprite1_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));
    hpBarSprite2_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));
    hpBarBGSprite_->SetPos(Vector2(
        WinApp::clientWidth * 0.65f,
        WinApp::clientHeight * 0.05f));

    hpBarSprite1_->Update();
    hpBarSprite2_->Update();
    hpBarBGSprite_->Update();

    // フェーズとライフの更新
    UpdatePhaseAndLive();

    // AIシステムの更新
    if (!isDead_ && !isPause_) {
        if (useBehaviorTree_ && behaviorTree_) {
            // ビヘイビアツリーの更新
            behaviorTree_->Update(deltaTime);
        } else if (stateMachine_) {
            // ステートマシンの更新（互換性のため残す）
            stateMachine_->Update(deltaTime);
        }
    }

    // ヒットエフェクトの更新
    UpdateHitEffect(Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f);

    // モデルの更新
    model_->SetTransform(transform_);
    model_->Update();
}

void Boss::Draw()
{
    model_->Draw();
}

void Boss::DrawSprite()
{
    hpBarBGSprite_->Draw();
    hpBarSprite2_->Draw();
    hpBarSprite1_->Draw();
}

void Boss::OnHit(float damage)
{
    if (isReadyToChangePhase_) {
        phase_ = 2;
        isReadyToChangePhase_ = false;
    }

    hp_ -= damage;
    hp_ = std::max<float>(hp_, 0.0f);

    hitEffectTimer_ = 0.f;
    isPlayHitEffect_ = true;
}

void Boss::UpdateHitEffect(Vector4 color, float duration)
{
    if (!isPlayHitEffect_)  return;

    hitEffectTimer_ += FrameTimer::GetInstance()->GetDeltaTime();

    if (hitEffectTimer_ <= duration) {
        model_->SetMaterialColor(color);
    }
    else {
        isPlayHitEffect_ = false;
        model_->SetMaterialColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

void Boss::UpdatePhaseAndLive()
{
    if (hp_ <= kPhase2HP) {
        isReadyToChangePhase_ = true;
    }

    if (hp_ <= 0.0f && life_ > 0) {
        life_--;

        if (life_ == 0) {
            isDead_ = true;
            return;
        }

        isReadyToChangePhase_ = false;
        hp_ = kMaxHp_;
        phase_ = 1;
    }
}

void Boss::DrawImGui()
{
#ifdef _DEBUG

    // ===== セクション1: 基本ステータス =====
    ImGui::SeparatorText("Basic Status");

    // HP表示（数値 + プログレスバー）
    ImGui::Text("HP: %.1f / %.1f (%.1f%%)", hp_, kMaxHp_, (hp_ / kMaxHp_) * 100.0f);
    ImGui::ProgressBar(hp_ / kMaxHp_, ImVec2(-1.0f, 0.0f), "");

    // ライフ、フェーズ
    ImGui::Text("Life: %d", life_);
    ImGui::Text("Phase: %d", phase_);

    // 状態フラグ（警告は赤色でハイライト）
    if (isDead_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Dead: YES");
    } else {
        ImGui::Text("Dead: NO");
    }
    ImGui::SameLine();
    if (isPause_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Paused: YES");
    } else {
        ImGui::Text("Paused: NO");
    }

    ImGui::Text("Ready to Change Phase: %s", isReadyToChangePhase_ ? "YES" : "NO");

    // ===== セクション2: 状態マシン =====
    ImGui::SeparatorText("State Machine");
    if (stateMachine_) {
        const std::string& stateName = stateMachine_->GetCurrentStateName();

        // 状態名を色分け（Idle=青、Dash=黄色、Shoot=オレンジ）
        ImVec4 stateColor;
        if (stateName == "Idle") {
            stateColor = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);  // 青
        } else if (stateName == "Dash") {
            stateColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色
        } else if (stateName == "Shoot") {
            stateColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);  // オレンジ
        } else {
            stateColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // 白（デフォルト）
        }

        ImGui::Text("Current State: ");
        ImGui::SameLine();
        ImGui::TextColored(stateColor, "%s", stateName.c_str());

        BossStateBase* currentState = stateMachine_->GetCurrentState();
        if (currentState) {
            float stateTimer = currentState->GetStateTimer();
            ImGui::Text("State Timer: %.2f sec", stateTimer);

            // 状態タイマーのプログレスバー（想定最大時間5秒）
            float maxDuration = 5.0f;
            float progress = std::min<float>(stateTimer / maxDuration, 1.0f);
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
        }
    }

    // ===== セクション5: 座標情報（折りたたみ可能） =====
    if (ImGui::CollapsingHeader("Transform")) {
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            transform_.translate.x, transform_.translate.y, transform_.translate.z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
            transform_.rotate.x, transform_.rotate.y, transform_.rotate.z);
        ImGui::Text("Scale: (%.2f, %.2f, %.2f)",
            transform_.scale.x, transform_.scale.y, transform_.scale.z);
    }

    // ===== セクション6: コライダー（折りたたみ可能） =====
    if (ImGui::CollapsingHeader("Collider")) {
        if (bodyCollider_) {
            ImGui::Text("Active: %s", bodyCollider_->IsActive() ? "Yes" : "No");
            ImGui::Text("Type ID: %d (Enemy)", bodyCollider_->GetTypeID());

            Vector3 offset = bodyCollider_->GetOffset();
            ImGui::Text("Offset: (%.2f, %.2f, %.2f)", offset.x, offset.y, offset.z);

            Vector3 size = bodyCollider_->GetSize();
            ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);

            Vector3 center = bodyCollider_->GetCenter();
            ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
        }
    }

    // ===== セクション7: デバッグコントロール =====
    ImGui::SeparatorText("Debug Controls");

    // AIシステム選択
    ImGui::Text("AI System:");
    if (ImGui::RadioButton("Behavior Tree", useBehaviorTree_)) {
        useBehaviorTree_ = true;
        if (!behaviorTree_ && player_) {
            behaviorTree_ = std::make_unique<BossBehaviorTree>(this, player_);
        }
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("State Machine", !useBehaviorTree_)) {
        useBehaviorTree_ = false;
        if (!stateMachine_) {
            stateMachine_ = std::make_unique<BossStateMachine>();
            stateMachine_->Initialize(this);
            stateMachine_->AddState("Idle", std::make_unique<BossIdleState>());
            stateMachine_->AddState("Dash", std::make_unique<BossDashState>());
            stateMachine_->AddState("Shoot", std::make_unique<BossShootState>());
            stateMachine_->ChangeState("Idle");
        }
    }

    // ビヘイビアツリーの制御
    if (useBehaviorTree_ && behaviorTree_) {
        // JSONから直接ビヘイビアツリーに読み込み（デバッグ・リリース共通）
        ImGui::SameLine();
        if (ImGui::Button("Load Tree from JSON")) {
            if (behaviorTree_->LoadFromJSON("resources/Json/BossTree.json")) {
                ImGui::Text("Tree loaded successfully!");
                // デバッグビルドの場合、エディタにも反映
                //if (nodeEditor_) {
                //    nodeEditor_->ImportFromBehaviorTree(behaviorTree_.get());
                //}
            }
        }

        // デバッグビルド専用：ノードエディタ機能
        if (nodeEditor_) {
            ImGui::SameLine();
            if (ImGui::Button("Node Editor")) {
                showNodeEditor_ = !showNodeEditor_;
                nodeEditor_->SetVisible(showNodeEditor_);
            }

            // デフォルトツリー生成
            ImGui::SameLine();
            if (ImGui::Button("Create Default Tree")) {
                nodeEditor_->CreateDefaultTree();
                ImGui::Text("Default tree created!");
            }
        }
    }

    // HP操作
    float tempHp = hp_;
    if (ImGui::SliderFloat("Set HP", &tempHp, 0.0f, kMaxHp_)) {
        hp_ = std::clamp(tempHp, 0.0f, kMaxHp_);
    }

    // フェーズ切り替え
    ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.48f, 0);
    if (ImGui::Button("Set Phase 1", buttonSize)) {
        SetPhase(1);
        hp_ = kMaxHp_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Phase 2", buttonSize)) {
        SetPhase(2);
        hp_ = 100.0f;
    }

    // 状態強制遷移
    ImGui::Spacing();
    ImGui::Text("Force State Transition:");

    if (useBehaviorTree_) {
        // ビヘイビアツリー使用時
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Using Behavior Tree");
        ImGui::Text("(State transitions are automatic)");
    } else if (stateMachine_) {
        // ステートマシン使用時
        static int selectedStateIndex = 0;
        const char* stateNames[] = { "Idle", "Dash", "Shoot" };

        ImGui::Combo("Select State", &selectedStateIndex, stateNames, IM_ARRAYSIZE(stateNames));

        // 選択された状態に遷移するボタン
        ImVec2 fullButtonSize(ImGui::GetContentRegionAvail().x, 0);
        if (ImGui::Button("Force Transition", fullButtonSize)) {
            stateMachine_->ChangeState(stateNames[selectedStateIndex]);
        }
    }

    // 一時停止トグル
    ImGui::Spacing();
    ImGui::Checkbox("Pause Boss", &isPause_);

    // 死亡/復活コントロール
    ImGui::Spacing();
    ImVec2 fullButtonSize(ImGui::GetContentRegionAvail().x * 0.48f, 0);

    // Killボタン（赤色）
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Kill Boss", fullButtonSize)) {
        isDead_ = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Reviveボタン（緑色）
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("Revive Boss", fullButtonSize)) {
        isDead_ = false;
        hp_ = kMaxHp_;
        life_ = 1;
        SetPhase(1);
    }
    ImGui::PopStyleColor(3);

    // ノードエディタの更新
    if (nodeEditor_ && showNodeEditor_) {
        nodeEditor_->Update();
    }

#endif
}

void Boss::RequestBulletSpawn(const Vector3& position, const Vector3& velocity) {
    pendingBullets_.push_back({ position, velocity });
}

std::vector<Boss::BulletSpawnRequest> Boss::ConsumePendingBullets() {
    auto result = std::move(pendingBullets_);
    pendingBullets_.clear();
    return result;
}

void Boss::SetPlayer(Player* player) {
    player_ = player;
    if (behaviorTree_) {
        behaviorTree_->SetPlayer(player);
    }
}