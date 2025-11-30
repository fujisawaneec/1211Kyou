#include "BTBossIdle.h"
#include "../../Boss.h"
#include "../../../Player/Player.h"
#include "Vector3.h"
#include <cmath>

BTBossIdle::BTBossIdle() {
    name_ = "BossIdle";
}

BTNodeStatus BTBossIdle::Execute(BTBlackboard* blackboard) {
    Boss* boss = blackboard->GetBoss();
    if (!boss) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    float deltaTime = blackboard->GetDeltaTime();

    // 初回実行時の初期化
    if (isFirstExecute_) {
        elapsedTime_ = 0.0f;
        isFirstExecute_ = false;

        // 次のアクションカウンターをインクリメント
        int actionCounter = blackboard->GetInt("ActionCounter", 0);
        blackboard->SetInt("ActionCounter", actionCounter + 1);
    }

    // プレイヤーの方向を向く
    LookAtPlayer(boss, deltaTime);

    // 経過時間を更新
    elapsedTime_ += deltaTime;

    // 待機時間が経過したら成功を返す
    if (elapsedTime_ >= idleDuration_) {
        isFirstExecute_ = true;  // 次回実行時のためにリセット
        status_ = BTNodeStatus::Success;
        return BTNodeStatus::Success;
    }

    // まだ待機中
    status_ = BTNodeStatus::Running;
    return BTNodeStatus::Running;
}

void BTBossIdle::Reset() {
    BTNode::Reset();
    elapsedTime_ = 0.0f;
    isFirstExecute_ = true;
}

void BTBossIdle::LookAtPlayer(Boss* boss, float deltaTime) {
    Player* player = boss->GetPlayer();
    if (!player) {
        return;
    }

    Vector3 playerPos = player->GetTransform().translate;
    Vector3 bossPos = boss->GetTransform().translate;
    Vector3 toPlayer = playerPos - bossPos;
    toPlayer.y = 0.0f;  // Y軸は無視

    if (toPlayer.Length() > 0.01f) {
        toPlayer.Normalize();

        // 向きたい角度を計算
        float targetAngle = atan2f(toPlayer.x, toPlayer.z);

        // 現在の角度
        float currentAngle = boss->GetTransform().rotate.y;

        // 角度差を計算（-π～πの範囲に正規化）
        float angleDiff = targetAngle - currentAngle;
        while (angleDiff > 3.14159265f) angleDiff -= 2.0f * 3.14159265f;
        while (angleDiff < -3.14159265f) angleDiff += 2.0f * 3.14159265f;

        // スムーズに回転（回転速度を調整）
        const float rotationSpeed = 5.0f;  // ラジアン/秒
        float rotationAmount = angleDiff * rotationSpeed * deltaTime;

        // 回転量を制限（急激な回転を防ぐ）
        const float maxRotationPerFrame = rotationSpeed * deltaTime;
        if (fabs(rotationAmount) > maxRotationPerFrame) {
            rotationAmount = (rotationAmount > 0) ? maxRotationPerFrame : -maxRotationPerFrame;
        }

        // 回転を適用
        Transform transform = boss->GetTransform();
        transform.rotate.y += rotationAmount;
        boss->SetTransform(transform);
    }
}

