#include "Player.h"
#include "Object3d.h"
#include "Input.h"
#include "Camera.h"
#include "State/PlayerStateMachine.h"
#include "State/IdleState.h"
#include "State/MoveState.h"
#include "State/DashState.h"
#include "State/AttackState.h"
#include "State/ShootState.h"
#include "State/ParryState.h"
#include "Input/InputHandler.h"
#include "OBBCollider.h"
#include "../../Collision/MeleeAttackCollider.h"
#include "CollisionManager.h"
#include "../../Collision/CollisionTypeIdDef.h"
#include "../Boss/Boss.h"
#include "GlobalVariables.h"
#include "../../Common/GameConst.h"
#include "FrameTimer.h"
#include "Sprite.h"

#include <cmath>
#include <algorithm>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif


Player::Player()
    : camera_(nullptr)
    , targetAngle_(0.0f)
    , mode_(false)
{
    // 動的制限を初期化（無効化）
    ClearDynamicBounds();
}

Player::~Player()
{
}

void Player::Initialize()
{
    GlobalVariables* gv = GlobalVariables::GetInstance();

    model_ = std::make_unique<Object3d>();
    model_->Initialize();
    model_->SetModel("white_cube.gltf");

    transform_.translate = Vector3(0.0f, initialY_, initialZ_);
    transform_.rotate = Vector3(0.0f, 0.0f, 0.0f);
    transform_.scale = Vector3(1.0f, 1.0f, 1.0f);

    model_->SetTransform(transform_);

    // HPバースプライトの初期化
    hpBarSprite_ = std::make_unique<Sprite>();
    hpBarSprite_->Initialize("white.png");
    hpBarSize_ = Vector2(500.0f, 30.0f);
    hpBarSprite_->SetSize(hpBarSize_);
    hpBarSprite_->SetAnchorPoint(Vector2(1.0f, 0.0f));
    hpBarSprite_->SetColor({ 0.3f, 1.0f, 0.3f, 1.0f });
    hpBarSprite_->SetPos(Vector2(
        WinApp::clientWidth * hpBarScreenXRatio_,
        WinApp::clientHeight * hpBarScreenYRatio_));

    hpBarBGSprite_ = std::make_unique<Sprite>();
    hpBarBGSprite_->Initialize("white.png");
    hpBarBGSprite_->SetSize(hpBarSize_);
    hpBarBGSprite_->SetAnchorPoint(Vector2(1.0f, 0.0f));
    hpBarBGSprite_->SetPos(Vector2(
        WinApp::clientWidth * hpBarScreenXRatio_,
        WinApp::clientHeight * hpBarScreenYRatio_));

    // State Machineの初期化
    stateMachine_ = std::make_unique<PlayerStateMachine>(this);
    stateMachine_->RegisterState("Idle", std::make_unique<IdleState>());
    stateMachine_->RegisterState("Move", std::make_unique<MoveState>());
    stateMachine_->RegisterState("Dash", std::make_unique<DashState>());
    stateMachine_->RegisterState("Attack", std::make_unique<AttackState>());
    stateMachine_->RegisterState("Shoot", std::make_unique<ShootState>());
    stateMachine_->RegisterState("Parry", std::make_unique<ParryState>());
    stateMachine_->ChangeState("Idle");
    stateMachine_->Initialize();

    // Colliderの初期化
    SetupColliders();

    // 攻撃ブロックの初期化
    attackBlock_ = std::make_unique<Object3d>();
    attackBlock_->Initialize();
    attackBlock_->SetModel("white_cube.gltf");
}

void Player::Finalize()
{
    // Colliderを削除
    if (bodyCollider_) {
        CollisionManager::GetInstance()->RemoveCollider(bodyCollider_.get());
    }
    if (meleeAttackCollider_) {
        CollisionManager::GetInstance()->RemoveCollider(meleeAttackCollider_.get());
    }
}

void Player::Update()
{
    // GlobalVariablesから値を同期
    GlobalVariables* gv = GlobalVariables::GetInstance();
    speed_ = gv->GetValueFloat("Player", "Speed");
    attackMinDist_ = gv->GetValueFloat("Player", "AttackStartDistance");
    attackMoveRotationLerp_ = gv->GetValueFloat("Player", "AttackMoveRotationLerp");
    bossLookatLerp_ = gv->GetValueFloat("Player", "BossLookatLerp");
    attackMoveSpeed_ = gv->GetValueFloat("Player", "AttackMoveSpeed");

    // 死亡判定
    if (hp_ <= 0.0f) isDead_ = true;

    // HPバーの更新
    hpBarSprite_->SetSize(Vector2(hpBarSize_.x * (hp_ / kMaxHp), hpBarSize_.y));
    hpBarSprite_->SetPos(Vector2(
        WinApp::clientWidth * hpBarScreenXRatio_,
        WinApp::clientHeight * hpBarScreenYRatio_));
    hpBarBGSprite_->SetPos(Vector2(
        WinApp::clientWidth * hpBarScreenXRatio_,
        WinApp::clientHeight * hpBarScreenYRatio_));
    hpBarSprite_->Update();
    hpBarBGSprite_->Update();

    // State Machineの更新
    if (stateMachine_) {
        stateMachine_->HandleInput();
        stateMachine_->Update(FrameTimer::GetInstance()->GetDeltaTime());
    }

    // フェーズ2時はボス方向を向く
    LookAtBoss();

    // 実効的な制限を計算（静的制限と動的制限の交差）
    float effectiveXMin = std::max<float>(GameConst::kStageXMin, dynamicXMin_);
    float effectiveXMax = std::min<float>(GameConst::kStageXMax, dynamicXMax_);
    float effectiveZMin = std::max<float>(GameConst::kStageZMin, dynamicZMin_);
    float effectiveZMax = std::min<float>(GameConst::kStageZMax, dynamicZMax_);

    // 位置制限適用
    transform_.translate.x = std::min<float>(transform_.translate.x, effectiveXMax);
    transform_.translate.x = std::max<float>(transform_.translate.x, effectiveXMin);
    transform_.translate.z = std::min<float>(transform_.translate.z, effectiveZMax);
    transform_.translate.z = std::max<float>(transform_.translate.z, effectiveZMin);

    // モデルの更新
    model_->SetTransform(transform_);
    model_->Update();

    // 攻撃ブロックの更新（表示中のみ）
    if (attackBlockVisible_ && attackBlock_) {
        attackBlock_->Update();
    }

    // 攻撃範囲Colliderの更新
    UpdateAttackCollider();
}

void Player::Draw()
{
    model_->Draw();

    // 攻撃ブロックの描画（表示中のみ）
    if (attackBlockVisible_ && attackBlock_) {
        attackBlock_->Draw();
    }
}

void Player::DrawSprite()
{
    hpBarBGSprite_->Draw();
    hpBarSprite_->Draw();
}

void Player::Move(float speedMultiplier, bool isApplyDirCalulate)
{
    if (!inputHandlerPtr_) return;

    GlobalVariables* gv = GlobalVariables::GetInstance();
    float deadzone = gv->GetValueFloat("Player", "MoveInputDeadzone");
    float rotationLerpSpeed = gv->GetValueFloat("Player", "RotationLerpSpeed");

    Vector2 moveDir = inputHandlerPtr_->GetMoveDirection();
    if (moveDir.Length() < deadzone) return;

    // 3Dベクトルに変換
    velocity_ = { moveDir.x, 0.0f, moveDir.y };
    velocity_ = velocity_.Normalize() * speed_ * speedMultiplier;

    // カメラモードに応じて移動方向を調整
    if (mode_ && camera_) {
        Matrix4x4 rotationMatrix = Mat4x4::MakeRotateY(camera_->GetRotateY());
        velocity_ = Mat4x4::TransformNormal(rotationMatrix, velocity_);
    }

    // 位置を更新
    transform_.translate += velocity_;

    // 移動方向を向く
    if (velocity_.Length() > kVelocityEpsilon && isApplyDirCalulate) {
        targetAngle_ = std::atan2(velocity_.x, velocity_.z);
        transform_.rotate.y = Vec3::LerpShortAngle(transform_.rotate.y, targetAngle_, rotationLerpSpeed);
    }
}

void Player::MoveToTarget(Boss* target, float deltaTime)
{
    if (!target) return;

    // 初回呼び出し時の初期化
    if (!isMoveInitialized_) {
        moveStartPosition_ = transform_.translate;

        Vector3 targetPos = target->GetTransform().translate;
        Vector3 toTarget = targetPos - moveStartPosition_;
        toTarget.y = 0.0f;
        float distance = toTarget.Length();

        if (distance > attackMinDist_ && distance > kDirectionEpsilon) {
            Vector3 direction = toTarget.Normalize();

            // 目標位置 = ターゲット位置から attackMinDist_ 手前
            float moveDistance = distance - attackMinDist_;
            moveTargetPosition_ = moveStartPosition_ + direction * moveDistance;
            moveTargetPosition_.y = moveStartPosition_.y;

            // 所要時間を計算
            moveDuration_ = moveDistance / attackMoveSpeed_;

            // ターゲット方向を向く
            targetAngle_ = std::atan2(direction.x, direction.z);
        }
        else {
            // 既に攻撃範囲内
            moveTargetPosition_ = moveStartPosition_;
            moveDuration_ = 0.0f;
        }

        moveElapsedTime_ = 0.0f;
        isMoveInitialized_ = true;
    }

    // イージング移動
    if (moveDuration_ > 0.0f) {
        float t = moveElapsedTime_ / moveDuration_;
        t = std::clamp(t, 0.0f, 1.0f);

        // smoothstep イージング
        t = t * t * (kMoveEasingCoeffA - kMoveEasingCoeffB * t);

        Vector3 newPos = Vector3::Lerp(moveStartPosition_, moveTargetPosition_, t);
        transform_.translate = newPos;
    }

    // 回転の補間
    transform_.rotate.y = Vec3::LerpShortAngle(transform_.rotate.y, targetAngle_, attackMoveRotationLerp_);

    moveElapsedTime_ += deltaTime;
}

void Player::ResetMoveToTarget()
{
    isMoveInitialized_ = false;
    moveElapsedTime_ = 0.0f;
    moveDuration_ = 0.0f;
}

bool Player::HasReachedTarget() const
{
    static constexpr float kMoveArrivalThreshold = 0.5f;

    if (moveDuration_ <= 0.0f) return true;  // 移動不要

    Vector3 diff = transform_.translate - moveTargetPosition_;
    diff.y = 0.0f;
    return diff.Length() < kMoveArrivalThreshold;
}

void Player::SetupColliders()
{
    GlobalVariables* gv = GlobalVariables::GetInstance();
    float bodySize = gv->GetValueFloat("Player", "BodyColliderSize");
    float meleeX = gv->GetValueFloat("Player", "MeleeColliderX");
    float meleeY = gv->GetValueFloat("Player", "MeleeColliderY");
    float meleeZ = gv->GetValueFloat("Player", "MeleeColliderZ");
    float meleeOffsetZ = gv->GetValueFloat("Player", "MeleeColliderOffsetZ");

    // 本体のCollider
    bodyCollider_ = std::make_unique<OBBCollider>();
    bodyCollider_->SetTransform(&transform_);
    bodyCollider_->SetSize(Vector3(bodySize, bodySize, bodySize));
    bodyCollider_->SetOffset(Vector3(0.0f, 0.0f, 0.0f));
    bodyCollider_->SetTypeID(static_cast<uint32_t>(CollisionTypeId::PLAYER));
    bodyCollider_->SetOwner(this);

    // 攻撃範囲のCollider
    meleeAttackCollider_ = std::make_unique<MeleeAttackCollider>(this);
    meleeAttackCollider_->SetTransform(&transform_);
    meleeAttackCollider_->SetSize(Vector3(meleeX, meleeY, meleeZ));
    meleeAttackCollider_->SetOffset(Vector3(0.0f, 0.0f, meleeOffsetZ));
    meleeAttackCollider_->SetActive(false);

    // CollisionManagerに登録
    CollisionManager* collisionManager = CollisionManager::GetInstance();
    collisionManager->AddCollider(bodyCollider_.get());
    collisionManager->AddCollider(meleeAttackCollider_.get());
}

void Player::UpdateAttackCollider()
{
    if (!meleeAttackCollider_) return;

    // 攻撃状態の時のみ前方に配置
    if (meleeAttackCollider_->IsActive()) {
        // プレイヤーの回転行列を作成（Y軸回転のみ）
        Matrix4x4 rotationMatrix = Mat4x4::MakeRotateY(transform_.rotate.y);
        meleeAttackCollider_->SetOrientation(rotationMatrix);
    }
}

void Player::LookAtBoss()
{
    // ボス参照がない、またはフェーズ2でなければスキップ
    if (!targetEnemy_ || targetEnemy_->GetPhase() != 2) return;

    // ボスへの方向ベクトルを計算
    Vector3 toTarget = targetEnemy_->GetTransform().translate - transform_.translate;
    toTarget.y = 0.0f;  // Y軸は無視

    if (toTarget.Length() < 0.01f) return;  // 距離が近すぎる場合はスキップ

    // 目標角度を計算
    float targetAngle = std::atan2(toTarget.x, toTarget.z);

    // スムーズに補間して回転
    transform_.rotate.y = Vec3::LerpShortAngle(transform_.rotate.y, targetAngle, bossLookatLerp_);
}

void Player::OnHit(float damage)
{
    if (IsInvincible()) return;

    hp_ -= damage;
    hp_ = std::max<float>(hp_, 0.0f);
}

void Player::DrawImGui()
{
#ifdef _DEBUG
    static int selectedTab = 0;  // タブの選択状態を保持

    // タブバー開始
    if (ImGui::BeginTabBar("PlayerDebugTabs", ImGuiTabBarFlags_None)) {

        // ========== General タブ ==========
        if (ImGui::BeginTabItem("General")) {
            selectedTab = 0;

            // HP
            ImGui::Text("Health");
            ImGui::SliderFloat("HP", &hp_, 0.0f, 200.0f, "%.1f");
            ImGui::ProgressBar(hp_ / 100.0f, ImVec2(-1, 0), "");
            ImGui::Checkbox("Invincible", &isInvincible_);

            ImGui::Separator();

            // Transform
            if (ImGui::TreeNode("Transform")) {
                ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.01f);
                ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
                ImGui::TreePop();
            }

            // Speed
            ImGui::Separator();
            ImGui::SliderFloat("Move Speed", &speed_, 0.0f, 2.0f);

            ImGui::EndTabItem();
        }

        // ========== States タブ ==========
        if (ImGui::BeginTabItem("States")) {
            selectedTab = 1;

            // 現在のステート情報
            if (stateMachine_) {
                PlayerState* currentState = stateMachine_->GetCurrentState();
                if (currentState) {
                    // 現在のアクティブステート名を強調表示
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active State: %s", currentState->GetName().c_str());
                }

                ImGui::Separator();

                // 全ステート詳細表示（新機能）
                if (ImGui::TreeNode("All States Details")) {
                    static std::string selectedStateName = "Idle";  // 選択中のステート名を保持

                    // ステート選択コンボボックス
                    auto stateNames = stateMachine_->GetAllStateNames();
                    if (ImGui::BeginCombo("Select State", selectedStateName.c_str())) {
                        for (const auto& name : stateNames) {
                            bool isSelected = (selectedStateName == name);

                            // 現在アクティブなステートには★マークを付ける
                            std::string displayName = name;
                            if (currentState && currentState->GetName() == name) {
                                displayName = name + " [ACTIVE]";
                            }

                            if (ImGui::Selectable(displayName.c_str(), isSelected)) {
                                selectedStateName = name;
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::Separator();

                    // 選択されたステートの詳細表示
                    PlayerState* selectedState = stateMachine_->GetState(selectedStateName);
                    if (selectedState) {
                        // 現在のステートなら緑色、そうでなければ青色でヘッダー表示
                        if (currentState && currentState->GetName() == selectedStateName) {
                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.3f, 0.4f, 1.0f));
                        }

                        if (ImGui::CollapsingHeader((selectedStateName + " State Details").c_str(),
                            ImGuiTreeNodeFlags_DefaultOpen)) {
                            // 選択されたステートのDrawImGuiを呼び出し
                            selectedState->DrawImGui(this);
                        }
                        ImGui::PopStyleColor();
                    }

                    ImGui::TreePop();
                }

                ImGui::Separator();

                // ステート手動切り替え（デバッグ用）
                if (ImGui::TreeNode("Manual State Change")) {
                    if (ImGui::Button("Idle")) stateMachine_->ChangeState("Idle");
                    ImGui::SameLine();
                    if (ImGui::Button("Move")) stateMachine_->ChangeState("Move");
                    ImGui::SameLine();
                    if (ImGui::Button("Dash")) stateMachine_->ChangeState("Dash");

                    if (ImGui::Button("Attack")) stateMachine_->ChangeState("Attack");
                    ImGui::SameLine();
                    if (ImGui::Button("Shoot")) stateMachine_->ChangeState("Shoot");
                    ImGui::SameLine();
                    if (ImGui::Button("Parry")) stateMachine_->ChangeState("Parry");

                    ImGui::TreePop();
                }
            }

            ImGui::EndTabItem();
        }

        // ========== Combat タブ ==========
        if (ImGui::BeginTabItem("Combat")) {
            selectedTab = 2;

            // Body Collider
            if (ImGui::TreeNode("Body Collider")) {
                if (bodyCollider_) {
                    bool isActive = bodyCollider_->IsActive();
                    if (ImGui::Checkbox("Active", &isActive)) {
                        bodyCollider_->SetActive(isActive);
                    }

                    // 実際の中心座標（読み取り専用）
                    Vector3 actualCenter = bodyCollider_->GetCenter();
                    ImGui::Text("Actual Center: (%.2f, %.2f, %.2f)",
                        actualCenter.x, actualCenter.y, actualCenter.z);

                    // オフセット（調整可能）
                    Vector3 offset = bodyCollider_->GetOffset();
                    if (ImGui::DragFloat3("Offset", &offset.x, 0.1f)) {
                        bodyCollider_->SetOffset(offset);
                    }

                    Vector3 size = bodyCollider_->GetSize();
                    if (ImGui::DragFloat3("Size", &size.x, 0.1f, 0.1f, 10.0f)) {
                        bodyCollider_->SetSize(size);
                    }
                }
                ImGui::TreePop();
            }

            // Attack Collider
            if (ImGui::TreeNode("Attack Collider")) {
                if (meleeAttackCollider_) {
                    bool isActive = meleeAttackCollider_->IsActive();
                    ImGui::Text("Active: %s", isActive ? "YES" : "NO");

                    // 実際の中心座標（読み取り専用）
                    Vector3 actualCenter = meleeAttackCollider_->GetCenter();
                    ImGui::Text("Actual Center: (%.2f, %.2f, %.2f)",
                        actualCenter.x, actualCenter.y, actualCenter.z);

                    // オフセット（調整可能）
                    Vector3 offset = meleeAttackCollider_->GetOffset();
                    if (ImGui::DragFloat3("Offset", &offset.x, 0.1f)) {
                        meleeAttackCollider_->SetOffset(offset);
                    }

                    // サイズ（調整可能）
                    Vector3 size = meleeAttackCollider_->GetSize();
                    if (ImGui::DragFloat3("Size", &size.x, 0.1f, 0.1f, 50.0f)) {
                        meleeAttackCollider_->SetSize(size);
                    }

                    ImGui::Separator();
                    Boss* detectedEnemy = meleeAttackCollider_->GetDetectedEnemy();
                    if (detectedEnemy) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Enemy Detected!");
                    }
                    else {
                        ImGui::Text("No Enemy Detected");
                    }
                    ImGui::Text("Collision Count: %d", meleeAttackCollider_->GetCollisionCount());
                }
                ImGui::TreePop();
            }

            // Combat Parameters
            if (ImGui::TreeNode("Combat Parameters")) {
                ImGui::SliderFloat("Attack Move Speed", &attackMoveSpeed_, 0.5f, 10.0f);
                ImGui::SliderFloat("Attack Start Distance", &attackMinDist_, 1.0f, 10.0f);
                ImGui::SliderFloat("Attack Move Rotation Lerp", &attackMoveRotationLerp_, 0.01f, 1.0f);
                ImGui::SliderFloat("Boss Lookat Lerp", &bossLookatLerp_, 0.01f, 2.0f);
                ImGui::TreePop();
            }

            // Initial Position (調整用)
            if (ImGui::TreeNode("Initial Position")) {
                ImGui::DragFloat("Initial Y", &initialY_, 0.1f, 0.0f, 10.0f);
                ImGui::DragFloat("Initial Z", &initialZ_, 1.0f, -200.0f, 0.0f);
                ImGui::TreePop();
            }

            ImGui::EndTabItem();
        }

        // ========== Physics タブ ==========
        if (ImGui::BeginTabItem("Physics")) {
            selectedTab = 3;

            // Velocity
            ImGui::Text("Velocity");
            ImGui::Text("X: %.3f, Y: %.3f, Z: %.3f", velocity_.x, velocity_.y, velocity_.z);
            ImGui::Text("Magnitude: %.3f", velocity_.Length());

            // Velocity Graph
            static float velocityHistory[100] = { 0 };
            static int historyOffset = 0;
            velocityHistory[historyOffset] = velocity_.Length();
            historyOffset = (historyOffset + 1) % 100;
            ImGui::PlotLines("Velocity History", velocityHistory, 100, historyOffset, nullptr, 0.0f, 20.0f, ImVec2(0, 80));

            ImGui::Separator();

            // Target Angle
            ImGui::Text("Target Angle: %.2f degrees", targetAngle_ * 180.0f / 3.14159f);
            ImGui::Text("Current Y Rotation: %.2f degrees", transform_.rotate.y * 180.0f / 3.14159f);

            ImGui::Separator();

            // Movement Direction Visualization
            if (ImGui::TreeNode("Movement Visualization")) {
                float angle = transform_.rotate.y;
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 center = ImGui::GetCursorScreenPos();
                center.x += 50;
                center.y += 50;

                // Draw circle
                draw_list->AddCircle(center, 40, IM_COL32(100, 100, 100, 255));

                // Draw direction arrow
                float arrowX = cosf(angle) * 35;
                float arrowY = sinf(angle) * 35;
                draw_list->AddLine(center,
                    ImVec2(center.x + arrowX, center.y - arrowY),
                    IM_COL32(0, 255, 0, 255), 3.0f);

                ImGui::Dummy(ImVec2(100, 100));
                ImGui::TreePop();
            }

            ImGui::EndTabItem();
        }

        // ========== Debug タブ ==========
        if (ImGui::BeginTabItem("Debug")) {
            selectedTab = 4;

            // Input Status
            if (ImGui::TreeNode("Input Status")) {
                if (inputHandlerPtr_) {
                    bool moving = inputHandlerPtr_->IsMoving();
                    bool attacking = inputHandlerPtr_->IsAttacking();
                    bool shooting = inputHandlerPtr_->IsShooting();
                    bool dashing = inputHandlerPtr_->IsDashing();
                    bool parrying = inputHandlerPtr_->IsParrying();

                    ImGui::Text("Moving: %s", moving ? "✓" : "✗");
                    ImGui::Text("Attacking: %s", attacking ? "✓" : "✗");
                    ImGui::Text("Shooting: %s", shooting ? "✓" : "✗");
                    ImGui::Text("Dashing: %s", dashing ? "✓" : "✗");
                    ImGui::Text("Parrying: %s", parrying ? "✓" : "✗");
                }
                ImGui::TreePop();
            }

            // Model Debug
            ImGui::Separator();
            if (ImGui::Button("Show Model Debug Info")) isDisModelDebugInfo_ = !isDisModelDebugInfo_;

            // Animation Control (TODO)
            if (ImGui::TreeNode("Animation Control")) {
                ImGui::Text("TODO: Animation system integration");
                // 将来的にアニメーション制御UIを追加
                ImGui::TreePop();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (isDisModelDebugInfo_) {
        model_->DrawImGui();
    }

#endif
}

bool Player::CanShoot() const
{
    if (isDead_) return false;
    if (targetEnemy_->GetPhase() == 2) return false; // フェーズ2では射撃不可

    return true;
}

void Player::SetDynamicBounds(float xMin, float xMax, float zMin, float zMax)
{
    dynamicXMin_ = xMin;
    dynamicXMax_ = xMax;
    dynamicZMin_ = zMin;
    dynamicZMax_ = zMax;
}

void Player::SetDynamicBoundsFromCenter(const Vector3& center, float xRange, float zRange)
{
    dynamicXMin_ = center.x - xRange;
    dynamicXMax_ = center.x + xRange;
    dynamicZMin_ = center.z - zRange;
    dynamicZMax_ = center.z + zRange;
}

void Player::ClearDynamicBounds()
{
    // 非常に大きな値に設定して実質的に無効化
    dynamicXMin_ = -kBoundaryDisabled;
    dynamicXMax_ = kBoundaryDisabled;
    dynamicZMin_ = -kBoundaryDisabled;
    dynamicZMax_ = kBoundaryDisabled;
}

void Player::RequestBulletSpawn(const Vector3& position, const Vector3& velocity)
{
    pendingBullets_.push_back({ position, velocity });
}

std::vector<Player::BulletSpawnRequest> Player::ConsumePendingBullets()
{
    return std::move(pendingBullets_);
}