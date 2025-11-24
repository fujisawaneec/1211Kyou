#include "BTBossShoot.h"
#include "../../Boss.h"
#include "../../../Player/Player.h"
#include <cmath>

BTBossShoot::BTBossShoot() {
    name_ = "BossShoot";
}

BTNodeStatus BTBossShoot::Execute(BTBlackboard* blackboard) {
    Boss* boss = blackboard->GetBoss();
    if (!boss) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    float deltaTime = blackboard->GetDeltaTime();

    // 初回実行時の初期化
    if (isFirstExecute_) {
        InitializeShoot(boss);
        isFirstExecute_ = false;
        hasFired_ = false;
    }

    // プレイヤーの方向を向く（射撃準備中）
    if (elapsedTime_ < chargeTime_) {
        AimAtPlayer(boss, deltaTime);
    }

    // 弾を発射
    if (elapsedTime_ >= chargeTime_ && !hasFired_) {
        FireBullets(boss);
        hasFired_ = true;
    }

    // 経過時間を更新
    elapsedTime_ += deltaTime;

    // 状態終了チェック
    if (elapsedTime_ >= totalDuration_) {
        // リセットして成功を返す
        isFirstExecute_ = true;
        elapsedTime_ = 0.0f;
        hasFired_ = false;
        status_ = BTNodeStatus::Success;
        return BTNodeStatus::Success;
    }

    // まだ射撃処理中
    status_ = BTNodeStatus::Running;
    return BTNodeStatus::Running;
}

void BTBossShoot::Reset() {
    BTNode::Reset();
    elapsedTime_ = 0.0f;
    isFirstExecute_ = true;
    hasFired_ = false;
}

void BTBossShoot::InitializeShoot(Boss* boss) {
    // タイマーリセット
    elapsedTime_ = 0.0f;

    // フェーズに応じてパラメータ調整
    if (boss->GetPhase() == 2) {
        // フェーズ2: より速い弾、短い硬直
        bulletSpeed_ = 20.0f;
        chargeTime_ = 0.5f;
        recoveryTime_ = 0.3f;
        totalDuration_ = chargeTime_ + recoveryTime_;
    } else {
        // フェーズ1: 通常パラメータ
        bulletSpeed_ = 100.0f;
        chargeTime_ = 0.5f;
        recoveryTime_ = 0.5f;
        totalDuration_ = chargeTime_ + recoveryTime_;
    }
}

void BTBossShoot::AimAtPlayer(Boss* boss, float deltaTime) {
    Player* player = boss->GetPlayer();
    if (!player) {
        return;
    }

    Vector3 playerPos = player->GetTransform().translate;
    Vector3 bossPos = boss->GetTransform().translate;
    Vector3 toPlayer = playerPos - bossPos;
    toPlayer.y = 0.0f; // Y軸は無視

    if (toPlayer.Length() > 0.01f) {
        toPlayer = toPlayer.Normalize();
        float angle = atan2f(toPlayer.x, toPlayer.z);
        boss->SetRotate(Vector3(0.0f, angle, 0.0f));
    }
}

void BTBossShoot::FireBullets(Boss* boss) {
    Player* player = boss->GetPlayer();
    if (!player) {
        return;
    }

    // 発射位置（ボスの前方少し上）
    Vector3 firePosition = boss->GetTransform().translate;

    // プレイヤーへの方向を計算
    Vector3 playerPos = player->GetTransform().translate;
    Vector3 toPlayer = playerPos - firePosition;

    // 水平方向のみで計算（Y成分は小さく保つ）
    float distance = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
    if (distance > 0.01f) {
        toPlayer = toPlayer.Normalize();
    }

    // 3発の弾を生成リクエスト
    for (int i = 0; i < 3; i++) {
        // 発射角度を計算（-15度、0度、+15度）
        float angleOffset = 0.0f;
        if (i == 0) {
            angleOffset = -spreadAngle_;  // 左側
        } else if (i == 2) {
            angleOffset = spreadAngle_;   // 右側
        }

        // 発射方向を計算
        Vector3 bulletDirection = CalculateBulletDirection(toPlayer, angleOffset);

        // 弾の速度ベクトルを計算
        Vector3 bulletVelocity = bulletDirection * bulletSpeed_;

        // ボスに弾生成をリクエスト
        boss->RequestBulletSpawn(firePosition, bulletVelocity);
    }
}

Vector3 BTBossShoot::CalculateBulletDirection(const Vector3& baseDirection, float angleOffset) {
    if (std::abs(angleOffset) < 0.001f) {
        // オフセットがない場合はそのまま返す
        return baseDirection;
    }

    // Y軸回転行列を作成
    float cos_angle = cosf(angleOffset);
    float sin_angle = sinf(angleOffset);

    // 回転を適用（Y軸回転）
    Vector3 rotatedDirection;
    rotatedDirection.x = baseDirection.x * cos_angle - baseDirection.z * sin_angle;
    rotatedDirection.y = baseDirection.y;
    rotatedDirection.z = baseDirection.x * sin_angle + baseDirection.z * cos_angle;

    rotatedDirection = rotatedDirection.Normalize();
    return rotatedDirection;
}