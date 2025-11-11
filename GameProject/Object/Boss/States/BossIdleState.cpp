#include "BossIdleState.h"
#include "../Boss.h"
#include "BossStateMachine.h"
#include "../../Player/Player.h"
#include "Vector3.h"
#include <random>
#include <cmath>

// 静的メンバー変数の初期化
int BossIdleState::actionCounter_ = 0;

BossIdleState::BossIdleState() {
}

void BossIdleState::Enter(Boss* boss) {
    // タイマーリセット
    stateTimer_ = 0.0f;

    // フェーズに応じて待機時間を調整
    if (boss->GetPhase() == 1) {
        // フェーズ1: 少し長めの待機
        idleDuration_ = 2.0f + static_cast<float>(rand() % 1000) / 1000.0f; // 2.0～3.0秒
    } else {
        // フェーズ2: より短い待機（より激しい戦闘）
        idleDuration_ = 1.0f + static_cast<float>(rand() % 500) / 1000.0f; // 1.0～1.5秒
    }

    // 次のアクションを決定（交互に実行）
    nextActionType_ = actionCounter_ % 2;
}

void BossIdleState::Update(Boss* boss, float deltaTime) {
    stateTimer_ += deltaTime;

    // プレイヤーの方向を向く
    if (boss->GetPlayer()) {
        Vector3 playerPos = boss->GetPlayer()->GetTransform().translate;
        Vector3 bossPos = boss->GetTransform().translate;
        Vector3 toPlayer = playerPos - bossPos;
        toPlayer.y = 0.0f; // Y軸は無視

        if (toPlayer.Length() > 0.01f) {
            toPlayer.Normalize();

            // 向きたい角度を計算
            float targetAngle = atan2f(toPlayer.x, toPlayer.z);

            // 現在の角度
            float currentAngle = boss->GetTransform().rotate.y;

            // 角度差を計算（-πからπの範囲に正規化）
            float angleDiff = targetAngle - currentAngle;
            while (angleDiff > 3.14159f) angleDiff -= 2.0f * 3.14159f;
            while (angleDiff < -3.14159f) angleDiff += 2.0f * 3.14159f;

            // スムーズに回転
            float rotateSpeed = 5.0f * deltaTime;
            if (std::abs(angleDiff) < rotateSpeed) {
                currentAngle = targetAngle;
            } else {
                currentAngle += (angleDiff > 0 ? rotateSpeed : -rotateSpeed);
            }

            boss->SetRotation(Vector3(0.0f, currentAngle, 0.0f));
        }
    }

    // 待機時間が経過したら次の行動へ
    if (stateTimer_ >= idleDuration_) {
        BossStateMachine* stateMachine = boss->GetStateMachine();
        if (stateMachine) {
            if (nextActionType_ == 0) {
                // ダッシュ移動へ遷移
                stateMachine->ChangeState("Dash");
            } else {
                // 射撃攻撃へ遷移
                stateMachine->ChangeState("Shoot");
            }
            actionCounter_++;
        }
    }
}

void BossIdleState::Exit(Boss* boss) {
    // 特に処理なし
}