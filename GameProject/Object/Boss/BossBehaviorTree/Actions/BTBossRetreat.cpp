#include "BTBossRetreat.h"
#include "../../Boss.h"
#include "../../../Player/Player.h"
#include "../../../../Common/GameConst.h"

#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

BTBossRetreat::BTBossRetreat() {
    name_ = "BossRetreat";
}

BTNodeStatus BTBossRetreat::Execute(BTBlackboard* blackboard) {
    Boss* boss = blackboard->GetBoss();
    if (!boss) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    Player* player = blackboard->GetPlayer();
    if (!player) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    float deltaTime = blackboard->GetDeltaTime();

    // 初回実行時の初期化
    if (isFirstExecute_) {
        InitializeRetreat(boss, player);
        isFirstExecute_ = false;

        // 既に目標距離以上離れている場合は即座に成功
        if (retreatDuration_ <= 0.0f) {
            isFirstExecute_ = true;
            status_ = BTNodeStatus::Success;
            return BTNodeStatus::Success;
        }
    }

    // 離脱移動の更新
    UpdateRetreatMovement(boss, deltaTime);

    // 経過時間を更新
    elapsedTime_ += deltaTime;

    // 終了判定（位置ベース）
    Vector3 currentPos = boss->GetTransform().translate;
    Vector3 diff = currentPos - targetPosition_;
    diff.y = 0.0f;  // 水平距離のみ
    float distanceToTarget = diff.Length();

    if (distanceToTarget < kArrivalThreshold) {
        // 目標位置に到達
        boss->SetTranslate(targetPosition_);

        // リセットして成功を返す
        isFirstExecute_ = true;
        elapsedTime_ = 0.0f;
        status_ = BTNodeStatus::Success;
        return BTNodeStatus::Success;
    }

    // まだ離脱中
    status_ = BTNodeStatus::Running;
    return BTNodeStatus::Running;
}

void BTBossRetreat::Reset() {
    BTNode::Reset();
    elapsedTime_ = 0.0f;
    isFirstExecute_ = true;
    retreatDuration_ = 0.0f;
}

void BTBossRetreat::InitializeRetreat(Boss* boss, Player* player) {
    // タイマーリセット
    elapsedTime_ = 0.0f;

    // 開始位置を記録
    startPosition_ = boss->GetTransform().translate;

    // プレイヤー位置を取得
    Vector3 playerPos = player->GetTransform().translate;

    // プレイヤーからボスへの方向ベクトル（離れる方向）
    Vector3 toRetreat = startPosition_ - playerPos;
    toRetreat.y = 0.0f;  // 水平面のみ
    float currentDistance = toRetreat.Length();

    if (currentDistance > kDirectionEpsilon) {
        Vector3 retreatDirection = toRetreat.Normalize();

        // プレイヤーを向いたまま（retreatDirectionの逆方向を向く）
        float angle = atan2f(-retreatDirection.x, -retreatDirection.z);
        boss->SetRotate(Vector3(0.0f, angle, 0.0f));

        // 目標位置 = 現在位置から targetDistance_ まで離れた位置
        float retreatDistance = targetDistance_ - currentDistance;
        if (retreatDistance > 0.0f) {
            targetPosition_ = startPosition_ + retreatDirection * retreatDistance;
            targetPosition_ = ClampToArea(targetPosition_);

            // 実際の移動距離から所要時間を計算
            Vector3 actualMove = targetPosition_ - startPosition_;
            actualMove.y = 0.0f;
            float actualDistance = actualMove.Length();
            retreatDuration_ = actualDistance / retreatSpeed_;
        }
        else {
            // 既に目標距離以上離れている
            targetPosition_ = startPosition_;
            retreatDuration_ = 0.0f;
        }
    }
    else {
        // プレイヤーとほぼ同じ位置
        targetPosition_ = startPosition_;
        retreatDuration_ = 0.0f;
    }
}

void BTBossRetreat::UpdateRetreatMovement(Boss* boss, float deltaTime) {
    deltaTime; // 未使用警告抑制

    if (retreatDuration_ > 0.0f) {
        // 離脱中の移動
        float t = elapsedTime_ / retreatDuration_;

        // clamp to [0, 1]
        t = std::clamp(t, 0.0f, 1.0f);

        // イージング（加速→減速）: smoothstep
        t = t * t * (kEasingCoeffA - kEasingCoeffB * t);

        Vector3 newPosition = Vector3::Lerp(startPosition_, targetPosition_, t);
        boss->SetTranslate(newPosition);
    }
}

Vector3 BTBossRetreat::ClampToArea(const Vector3& position) {
    Vector3 clampedPos = position;

    // GameConstantsのステージ境界を使用
    // X座標の制限
    clampedPos.x = std::clamp(clampedPos.x, GameConst::kStageXMin + GameConst::kAreaMargin, GameConst::kStageXMax - GameConst::kAreaMargin);

    // Z座標の制限
    clampedPos.z = std::clamp(clampedPos.z, GameConst::kStageZMin + GameConst::kAreaMargin, GameConst::kStageZMax - GameConst::kAreaMargin);

    // Y座標は元の値を保持
    clampedPos.y = position.y;

    return clampedPos;
}

nlohmann::json BTBossRetreat::ExtractParameters() const {
    return {
        {"retreatSpeed", retreatSpeed_},
        {"targetDistance", targetDistance_}
    };
}

#ifdef _DEBUG
bool BTBossRetreat::DrawImGui() {
    bool changed = false;

    if (ImGui::DragFloat("Retreat Speed##retreat", &retreatSpeed_, 1.0f, 10.0f, 200.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Target Distance##retreat", &targetDistance_, 0.5f, 1.0f, 100.0f)) {
        changed = true;
    }

    ImGui::Separator();
    ImGui::Text("Runtime Info:");
    ImGui::Text("Duration: %.2f sec", retreatDuration_);
    ImGui::Text("Elapsed: %.2f sec", elapsedTime_);

    return changed;
}
#endif
