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

#ifdef _DEBUG
#include "ImGui.h"
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

    // ステートマシンの初期化
    stateMachine_ = std::make_unique<BossStateMachine>();
    stateMachine_->Initialize(this);

    // 各状態を追加
    stateMachine_->AddState("Idle", std::make_unique<BossIdleState>());
    stateMachine_->AddState("Dash", std::make_unique<BossDashState>());
    stateMachine_->AddState("Shoot", std::make_unique<BossShootState>());

    // 初期状態をIdleに設定
    stateMachine_->ChangeState("Idle");
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

    // ステートマシンの更新
    if (stateMachine_ && !isDead_ && !isPause_) {
        stateMachine_->Update(deltaTime);
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

    // ボスの状態
    ImGui::Text("=== Boss Status ===");
    ImGui::Text("HP: %.1f", hp_);
    ImGui::Text("Phase: %d", phase_);
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform_.translate.x, transform_.translate.y, transform_.translate.z);
    ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform_.rotate.x, transform_.rotate.y, transform_.rotate.z);
    ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform_.scale.x, transform_.scale.y, transform_.scale.z);

    // フェーズ切り替えボタン（デバッグ用）
    ImGui::Separator();
    ImGui::Text("=== Debug Controls ===");
    if (ImGui::Button("Set Phase 1")) {
        SetPhase(1);
        hp_ = kMaxHp_;  // HPも回復
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Phase 2")) {
        SetPhase(2);
        hp_ = 100.0f;  // フェーズ2のHP
    }

    // Collider情報
    ImGui::Separator();
    ImGui::Text("=== Collider Info ===");
    if (bodyCollider_) {
        ImGui::Text("Collider Active: %s", bodyCollider_->IsActive() ? "Yes" : "No");
        ImGui::Text("Type ID: %d (Enemy)", bodyCollider_->GetTypeID());

        Vector3 offset = bodyCollider_->GetOffset();
        ImGui::Text("Offset: (%.2f, %.2f, %.2f)", offset.x, offset.y, offset.z);

        Vector3 size = bodyCollider_->GetSize();
        ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);

        Vector3 center = bodyCollider_->GetCenter();
        ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
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