#include "BTBossDash.h"
#include "../../Boss.h"
#include "../../../Player/Player.h"
#include "RandomEngine.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

BTBossDash::BTBossDash() {
    name_ = "BossDash";
}

BTNodeStatus BTBossDash::Execute(BTBlackboard* blackboard) {
    Boss* boss = blackboard->GetBoss();
    if (!boss) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    float deltaTime = blackboard->GetDeltaTime();

    // 初回実行時の初期化
    if (isFirstExecute_) {
        InitializeDash(boss);
        isFirstExecute_ = false;
    }

    // ダッシュ移動の更新
    UpdateDashMovement(boss, deltaTime);

    // 経過時間を更新
    elapsedTime_ += deltaTime;

    // ダッシュが完了したか確認
    if (elapsedTime_ >= dashDuration_) {
        // 最終位置に設定
        boss->SetTranslate(targetPosition_);

        // リセットして成功を返す
        isFirstExecute_ = true;
        elapsedTime_ = 0.0f;
        status_ = BTNodeStatus::Success;
        return BTNodeStatus::Success;
    }

    // まだダッシュ中
    status_ = BTNodeStatus::Running;
    return BTNodeStatus::Running;
}

void BTBossDash::Reset() {
    BTNode::Reset();
    elapsedTime_ = 0.0f;
    isFirstExecute_ = true;
}

void BTBossDash::InitializeDash(Boss* boss) {
    // タイマーリセット
    elapsedTime_ = 0.0f;

    // 開始位置を記録
    startPosition_ = boss->GetTransform().translate;

    // ランダムな方向を生成
    RandomEngine* rng = RandomEngine::GetInstance();

    // XZ平面上のランダムな方向を取得（Y=0で正規化済み）
    dashDirection_ = rng->GetRandomDirectionXZ();

    // ランダムなダッシュ距離を取得
    float dashDistance = rng->GetFloat(10.0f, 50.0f);

    // 目標位置を計算
    targetPosition_ = startPosition_ + dashDirection_ * dashDistance;

    // エリア内に収まるよう調整
    targetPosition_ = ClampToArea(targetPosition_);

    // ダッシュ方向を再計算（エリア制限後）
    dashDirection_ = targetPosition_ - startPosition_;
    float actualDistance = dashDirection_.Length();
    if (actualDistance > 0.01f) {
        dashDirection_ = dashDirection_.Normalize();
        // ダッシュ時間を調整（距離に応じて）
        dashDuration_ = actualDistance / dashSpeed_;
    }

    // ダッシュ方向を向く
    if (dashDirection_.Length() > 0.01f) {
        float angle = atan2f(dashDirection_.x, dashDirection_.z);
        boss->SetRotate(Vector3(0.0f, angle, 0.0f));
    }
}

void BTBossDash::UpdateDashMovement(Boss* boss, float deltaTime) {
    if (elapsedTime_ < dashDuration_) {
        // ダッシュ中の移動
        float t = elapsedTime_ / dashDuration_;

        // イージング（加速→減速）
        t = t * t * (3.0f - 2.0f * t);

        Vector3 newPosition = Vector3::Lerp(startPosition_, targetPosition_, t);
        boss->SetTranslate(newPosition);

        // ダッシュエフェクト的な表現（少し振動させる）
        float vibration = sinf(elapsedTime_ * 50.0f) * 0.05f;
        Vector3 currentPos = boss->GetTransform().translate;
        currentPos.y += vibration;
        boss->SetTranslate(currentPos);
    }
}

Vector3 BTBossDash::ClampToArea(const Vector3& position) {
    Vector3 clampedPos = position;

    // Playerクラスと同じ静的境界を使用
    // X座標の制限（-100.0f ～ 100.0f）
    clampedPos.x = std::clamp(clampedPos.x, Player::X_MIN + 5.0f, Player::X_MAX - 5.0f);

    // Z座標の制限（-140.0f ～ 60.0f）
    clampedPos.z = std::clamp(clampedPos.z, Player::Z_MIN + 5.0f, Player::Z_MAX - 5.0f);

    // Y座標は元の値を保持
    clampedPos.y = position.y;

    return clampedPos;
}

nlohmann::json BTBossDash::ExtractParameters() const {
    return {
        {"dashSpeed", dashSpeed_},
        {"dashDuration", dashDuration_}
    };
}

#ifdef _DEBUG
bool BTBossDash::DrawImGui() {
    bool changed = false;

    if (ImGui::DragFloat("Speed##dash", &dashSpeed_, 1.0f, 10.0f, 200.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Duration##dash", &dashDuration_, 0.05f, 0.1f, 3.0f)) {
        changed = true;
    }

    return changed;
}
#endif