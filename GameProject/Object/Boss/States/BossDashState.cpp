#include "BossDashState.h"
#include "../Boss.h"
#include "BossStateMachine.h"
#include "../../Player/Player.h"
#include "RandomEngine.h"
#include <algorithm>

BossDashState::BossDashState() {
}

void BossDashState::Enter(Boss* boss) {
    // タイマーリセット
    stateTimer_ = 0.0f;

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
        boss->SetRotation(Vector3(0.0f, angle, 0.0f));
    }
}

void BossDashState::Update(Boss* boss, float deltaTime) {

    // フェーズ2では速度を下げる
    if (boss->GetPhase() == 2) {
        dashSpeed_ = 30.0f;
    }else
    {
        dashSpeed_ = 60.0f;
    }

    stateTimer_ += deltaTime;

    if (stateTimer_ < dashDuration_) {
        // ダッシュ中の移動
        float t = stateTimer_ / dashDuration_;

        // イージング（加速→減速）
        t = t * t * (3.0f - 2.0f * t);

        Vector3 newPosition = Vector3::Lerp(startPosition_, targetPosition_, t);
        boss->SetPosition(newPosition);

        // ダッシュエフェクト的な表現（少し振動させる）
        float vibration = sinf(stateTimer_ * 50.0f) * 0.05f;
        Vector3 currentPos = boss->GetTransform().translate;
        currentPos.y += vibration;
        boss->SetPosition(currentPos);
    } else {
        // ダッシュ終了、最終位置に設定
        boss->SetPosition(targetPosition_);

        // 待機状態へ遷移
        BossStateMachine* stateMachine = boss->GetStateMachine();
        if (stateMachine) {
            stateMachine->ChangeState("Idle");
        }
    }
}

void BossDashState::Exit(Boss* boss) {

}

Vector3 BossDashState::ClampToArea(const Vector3& position) {
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