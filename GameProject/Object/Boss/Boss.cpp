#include "Boss.h"

#include <algorithm>
#include "Object3d.h"
#include "OBBCollider.h"
#include "CollisionManager.h"
#include "../../Collision/CollisionTypeIdDef.h"
#include "FrameTimer.h"

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

    // Colliderの初期化
    bodyCollider_ = std::make_unique<OBBCollider>();
    bodyCollider_->SetTransform(&transform_);
    bodyCollider_->SetSize(Vector3(3.2f, 3.2f, 3.2f));
    bodyCollider_->SetOffset(Vector3(0.0f, 0.0f, 0.0f));
    bodyCollider_->SetTypeID(static_cast<uint32_t>(CollisionTypeId::BOSS));
    bodyCollider_->SetOwner(this);

    // CollisionManagerに登録
    CollisionManager::GetInstance()->AddCollider(bodyCollider_.get());
}

void Boss::Finalize()
{
    // Colliderを削除
    if (bodyCollider_) {
        CollisionManager::GetInstance()->RemoveCollider(bodyCollider_.get());
    }
}

void Boss::Update()
{
    UpdatePhaseAndLive();
    UpdateHitEffect(Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f);
    model_->SetTransform(transform_);
    model_->Update();
}

void Boss::Draw()
{
    model_->Draw();
}

void Boss::OnHit(float damage)
{
    if (isReadyToChangePhase_)
    {
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
    } else
    {
        isPlayHitEffect_ = false;
        model_->SetMaterialColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

void Boss::UpdatePhaseAndLive()
{
    if (hp_ <= 100.f) {
        isReadyToChangePhase_ = true;
    }

    if (hp_ <= 0.0f && life_ > 0) {
        life_--;

        if (life_ == 0) {
            isDead_ = true;
            return;
        }

        hp_ = maxHp_;
        phase_ = 1;
    }
}

void Boss::DrawImGui()
{
#ifdef _DEBUG

    // ボスの状態
    ImGui::Text("=== Boss Status ===");
    ImGui::Text("HP: %.1f", hp_);
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform_.translate.x, transform_.translate.y, transform_.translate.z);
    ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform_.rotate.x, transform_.rotate.y, transform_.rotate.z);
    ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform_.scale.x, transform_.scale.y, transform_.scale.z);

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