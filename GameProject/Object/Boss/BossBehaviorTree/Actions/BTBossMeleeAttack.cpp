#include "BTBossMeleeAttack.h"
#include "../../Boss.h"
#include "../../../Player/Player.h"
#include "../../../../Collision/BossMeleeAttackCollider.h"
#include "../../../../Common/GameConst.h"
#include "Object3d.h"
#include "Mat4x4Func.h"
#include <cmath>
#include <algorithm>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

BTBossMeleeAttack::BTBossMeleeAttack() {
    name_ = "BossMeleeAttack";
}

BTNodeStatus BTBossMeleeAttack::Execute(BTBlackboard* blackboard) {
    Boss* boss = blackboard->GetBoss();
    if (!boss) {
        status_ = BTNodeStatus::Failure;
        return BTNodeStatus::Failure;
    }

    float deltaTime = blackboard->GetDeltaTime();

    // 初回実行時の初期化
    if (isFirstExecute_) {
        InitializeMeleeAttack(boss);
        isFirstExecute_ = false;
    }

    // 経過時間を更新
    elapsedTime_ += deltaTime;
    phaseTimer_ += deltaTime;

    // フェーズに応じた処理
    switch (currentPhase_) {
    case MeleePhase::Prepare:
        ProcessPreparePhase(boss, deltaTime);
        // 準備時間終了でExecuteフェーズへ
        if (phaseTimer_ >= prepareTime_) {
            currentPhase_ = MeleePhase::Execute;
            phaseTimer_ = 0.0f;
            // 予兆エフェクトをOFF
            boss->SetAttackSignEmitterActive(false);
            // コライダーを有効化
            if (boss->GetMeleeAttackCollider()) {
                boss->GetMeleeAttackCollider()->SetActive(true);
                boss->GetMeleeAttackCollider()->Reset();
                colliderActivated_ = true;
            }
            // ★突進の初期化（Execute開始時にプレイヤー位置を確定）
            InitializeRush(boss);
        }
        break;

    case MeleePhase::Execute:
        ProcessExecutePhase(boss, deltaTime);
        // 攻撃時間終了でRecoveryフェーズへ
        if (phaseTimer_ >= attackDuration_) {
            currentPhase_ = MeleePhase::Recovery;
            phaseTimer_ = 0.0f;
            // コライダーを無効化
            if (boss->GetMeleeAttackCollider()) {
                boss->GetMeleeAttackCollider()->SetActive(false);
            }
            // ブロックを非表示
            boss->SetMeleeAttackBlockVisible(false);
        }
        break;

    case MeleePhase::Recovery:
        ProcessRecoveryPhase(boss);
        // 硬直時間終了で完了
        if (phaseTimer_ >= recoveryTime_) {
            // リセットして成功を返す
            isFirstExecute_ = true;
            elapsedTime_ = 0.0f;
            phaseTimer_ = 0.0f;
            currentPhase_ = MeleePhase::Prepare;
            colliderActivated_ = false;
            status_ = BTNodeStatus::Success;
            return BTNodeStatus::Success;
        }
        break;
    }

    // まだ処理中
    status_ = BTNodeStatus::Running;
    return BTNodeStatus::Running;
}

void BTBossMeleeAttack::Reset() {
    BTNode::Reset();
    elapsedTime_ = 0.0f;
    phaseTimer_ = 0.0f;
    blockAngle_ = 0.0f;
    isFirstExecute_ = true;
    currentPhase_ = MeleePhase::Prepare;
    colliderActivated_ = false;
    rushInitialized_ = false;
}

void BTBossMeleeAttack::InitializeMeleeAttack(Boss* boss) {
    // タイマーリセット
    elapsedTime_ = 0.0f;
    phaseTimer_ = 0.0f;
    blockAngle_ = kBlockStartAngle;
    currentPhase_ = MeleePhase::Prepare;
    colliderActivated_ = false;

    // totalDurationを計算
    totalDuration_ = prepareTime_ + attackDuration_ + recoveryTime_;

    // ブロックを表示
    boss->SetMeleeAttackBlockVisible(true);

    // 予兆エフェクトをON
    boss->SetAttackSignEmitterActive(true);

    // 初期位置を設定
    UpdateBlockPosition(boss);

    // 突進フラグをリセット（Execute開始時に初期化する）
    rushInitialized_ = false;
}

void BTBossMeleeAttack::AimAtPlayer(Boss* boss, float deltaTime) {
    Player* player = boss->GetPlayer();
    if (!player) {
        return;
    }

    Vector3 playerPos = player->GetTransform().translate;
    Vector3 bossPos = boss->GetTransform().translate;
    Vector3 toPlayer = playerPos - bossPos;
    toPlayer.y = 0.0f; // Y軸は無視

    if (toPlayer.Length() > kDirectionEpsilon) {
        toPlayer = toPlayer.Normalize();
        float targetAngle = atan2f(toPlayer.x, toPlayer.z);

        // 現在の角度を取得
        float currentAngle = boss->GetRotate().y;

        // 角度差を計算
        float angleDiff = targetAngle - currentAngle;

        // -πからπの範囲に正規化
        while (angleDiff > 3.14159f) angleDiff -= 2.0f * 3.14159f;
        while (angleDiff < -3.14159f) angleDiff += 2.0f * 3.14159f;

        // スムーズに回転（補間率を調整）
        float rotationSpeed = 5.0f;
        float newAngle = currentAngle + angleDiff * rotationSpeed * deltaTime;

        boss->SetRotate(Vector3(0.0f, newAngle, 0.0f));
    }
}

void BTBossMeleeAttack::ProcessPreparePhase(Boss* boss, float deltaTime) {
    // プレイヤーの方向を向く
    AimAtPlayer(boss, deltaTime);

    // ブロック位置を更新（振らない、開始位置に固定）
    UpdateBlockPosition(boss);

    // 予兆エフェクトの位置を更新
    Object3d* block = boss->GetMeleeAttackBlock();
    if (block) {
        boss->SetAttackSignEmitterPosition(block->GetTransform().translate);
    }
}

void BTBossMeleeAttack::ProcessExecutePhase(Boss* boss, float deltaTime) {
    // ヒット判定をチェック
    BossMeleeAttackCollider* collider = boss->GetMeleeAttackCollider();
    bool hasHit = collider && collider->HasHitPlayer();

    if (hasHit) {
        // ヒット時: プレイヤーと一定距離を保って停止
        Player* player = boss->GetPlayer();
        if (player) {
            Vector3 playerPos = player->GetTransform().translate;
            Vector3 bossPos = boss->GetTransform().translate;
            Vector3 toPlayer = playerPos - bossPos;
            toPlayer.y = 0.0f;
            float currentDistance = toPlayer.Length();

            // 現在の距離がstopDistance_より近い場合は、stopDistance_まで後退
            if (currentDistance < stopDistance_ && currentDistance > kDirectionEpsilon) {
                Vector3 direction = toPlayer.Normalize();
                Vector3 stopPos = playerPos - direction * stopDistance_;
                stopPos = ClampToArea(stopPos);
                boss->SetTranslate(stopPos);
            }
            // そうでなければ現在位置を維持（移動しない）
        }
    } else {
        // ミス時: 通常通り突進
        float t = phaseTimer_ / attackDuration_;
        t = std::clamp(t, 0.0f, 1.0f);
        t = t * t * (3.0f - 2.0f * t);  // Smoothstep

        Vector3 newPosition = Vector3::Lerp(startPosition_, targetPosition_, t);
        boss->SetTranslate(newPosition);
    }

    // ブロックを回転させる（右から左へ）
    float rotationSpeed = swingAngle_ / attackDuration_;
    blockAngle_ += rotationSpeed * deltaTime;

    // ブロック位置を更新
    UpdateBlockPosition(boss);
}

void BTBossMeleeAttack::ProcessRecoveryPhase(Boss* boss) {
    // 硬直中は特に処理なし
    (void)boss;
}

Vector3 BTBossMeleeAttack::ClampToArea(const Vector3& position) {
    Vector3 clampedPos = position;
    clampedPos.x = std::clamp(clampedPos.x,
        GameConst::kStageXMin + GameConst::kAreaMargin,
        GameConst::kStageXMax - GameConst::kAreaMargin);
    clampedPos.z = std::clamp(clampedPos.z,
        GameConst::kStageZMin + GameConst::kAreaMargin,
        GameConst::kStageZMax - GameConst::kAreaMargin);
    clampedPos.y = position.y;
    return clampedPos;
}

void BTBossMeleeAttack::InitializeRush(Boss* boss) {
    rushInitialized_ = true;
    startPosition_ = boss->GetTransform().translate;

    Player* player = boss->GetPlayer();
    if (player) {
        Vector3 playerPos = player->GetTransform().translate;
        Vector3 toPlayer = playerPos - startPosition_;
        toPlayer.y = 0.0f;

        if (toPlayer.Length() > kDirectionEpsilon) {
            rushDirection_ = toPlayer.Normalize();

            // 突進方向に向く
            float angle = atan2f(rushDirection_.x, rushDirection_.z);
            boss->SetRotate(Vector3(0.0f, angle, 0.0f));

            // 目標位置 = 開始位置 + 方向 * 突進距離
            targetPosition_ = startPosition_ + rushDirection_ * rushDistance_;
            targetPosition_ = ClampToArea(targetPosition_);
        } else {
            rushDirection_ = Vector3(0.0f, 0.0f, 1.0f);
            targetPosition_ = startPosition_;
        }
    } else {
        rushDirection_ = Vector3(0.0f, 0.0f, 1.0f);
        targetPosition_ = startPosition_;
    }
}

void BTBossMeleeAttack::UpdateBlockPosition(Boss* boss) {
    Object3d* block = boss->GetMeleeAttackBlock();
    if (!block) return;

    // ボスの位置と回転を取得
    Vector3 bossPos = boss->GetTranslate();
    float bossRotY = boss->GetRotate().y;

    // ワールド空間での角度を計算
    float worldAngle = bossRotY + blockAngle_;

    // Mat4x4::MakeRotateYで回転行列を作成
    Matrix4x4 rotationMatrix = Mat4x4::MakeRotateY(worldAngle);

    // 回転行列を使ってローカルオフセットをワールド座標に変換
    Vector3 localOffset = { 0.0f, 0.0f, blockRadius_ };  // 前方向オフセット
    Vector3 worldOffset = Mat4x4::TransformNormal(rotationMatrix, localOffset);

    // ブロック位置を計算
    Vector3 blockPos = bossPos + worldOffset;

    // ブロックのTransform設定
    Transform blockTransform;
    blockTransform.translate = blockPos;
    blockTransform.rotate = { 0.0f, worldAngle, 0.0f };
    blockTransform.scale = { blockScale_, blockScale_, blockScale_ };
    block->SetTransform(blockTransform);
    block->Update();

    // コライダーの向き設定（ボスの向きに追従）
    BossMeleeAttackCollider* collider = boss->GetMeleeAttackCollider();
    if (collider && colliderActivated_) {
        collider->SetOrientation(Mat4x4::MakeRotateY(bossRotY));
    }
}

nlohmann::json BTBossMeleeAttack::ExtractParameters() const {
    return {
        {"prepareTime", prepareTime_},
        {"attackDuration", attackDuration_},
        {"recoveryTime", recoveryTime_},
        {"blockRadius", blockRadius_},
        {"blockScale", blockScale_},
        {"swingAngle", swingAngle_},
        {"rushDistance", rushDistance_},
        {"stopDistance", stopDistance_}
    };
}

#ifdef _DEBUG
bool BTBossMeleeAttack::DrawImGui() {
    bool changed = false;

    if (ImGui::DragFloat("Prepare Time##melee", &prepareTime_, 0.05f, 0.0f, 3.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Attack Duration##melee", &attackDuration_, 0.05f, 0.0f, 3.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Recovery Time##melee", &recoveryTime_, 0.05f, 0.0f, 3.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Block Radius##melee", &blockRadius_, 0.1f, 0.5f, 10.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Block Scale##melee", &blockScale_, 0.1f, 0.1f, 5.0f)) {
        changed = true;
    }
    if (ImGui::SliderAngle("Swing Angle##melee", &swingAngle_, 0.0f, 360.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Rush Distance##melee", &rushDistance_, 1.0f, 0.0f, 50.0f)) {
        changed = true;
    }
    if (ImGui::DragFloat("Stop Distance##melee", &stopDistance_, 0.5f, 1.0f, 20.0f)) {
        changed = true;
    }

    return changed;
}
#endif
