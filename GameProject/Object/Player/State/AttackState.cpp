#include "AttackState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input/InputHandler.h"
#include "../MeleeAttackCollider.h"
#include "../../Boss/Boss.h"
#include "CollisionManager.h"
#include "Object3d.h"
#include <cmath>
#ifdef _DEBUG
#include <imgui.h>
#endif

void AttackState::Enter(Player* player)
{
    // 攻撃アニメーションを再生
    // TODO: アニメーション作成後に実装
    // player->GetModel()->PlayAnimation("Attack" + std::to_string(comboCount_));

    attackTimer_ = 0.0f;
    searchTimer_ = 0.0f;
    moveTimer_ = 0.0f;
    canCombo_ = false;
    phase_ = SearchTarget;
    targetEnemy_ = nullptr;

    // 攻撃範囲Colliderを有効化
    if (player->GetMeleeAttackCollider()) {
        player->GetMeleeAttackCollider()->SetActive(true);
        player->GetMeleeAttackCollider()->Reset();
    }

    // 攻撃ブロックを表示して初期位置設定
    if (player->GetAttackBlock()) {
        player->SetAttackBlockVisible(true);
        blockAngle_ = GetStartAngle();
        UpdateBlockPosition(player);
    }

    // SearchForTargetはUpdateのSearchTargetフェーズで呼ばれる
}

void AttackState::Update(Player* player, float deltaTime)
{
    switch (phase_) {
    case SearchTarget:
        // 待機時間をカウント
        searchTimer_ += deltaTime;

        // 毎フレーム敵を検索
        SearchForTarget(player);

        // 一定時間待機後、次のフェーズへ
        if (searchTimer_ >= maxSearchTime_) {
            if (targetEnemy_) {
                phase_ = MoveToTarget;
            }
            else {
                phase_ = ExecuteAttack;
            }
        }
        break;

    case MoveToTarget:
        ProcessMoveToTarget(player, deltaTime);
        break;

    case ExecuteAttack:
        ProcessExecuteAttack(player, deltaTime);
        break;

    case Recovery:
        attackTimer_ += deltaTime;
        if (attackTimer_ >= attackDuration_) {
            PlayerStateMachine* stateMachine = player->GetStateMachine();
            if (stateMachine) {
                stateMachine->ChangeState("Idle");
            }
        }
        break;
    }
}

void AttackState::Exit(Player* player)
{
    // 攻撃範囲Colliderを無効化
    if (player->GetMeleeAttackCollider()) {
        player->GetMeleeAttackCollider()->SetActive(false);
    }

    // 攻撃ブロックを非表示
    player->SetAttackBlockVisible(false);

    // コンボカウントをリセット
    if (attackTimer_ >= attackDuration_) {
        comboCount_ = 0;
    }

    canCombo_ = false;

    targetEnemy_ = nullptr;
}

void AttackState::HandleInput(Player* player)
{
    InputHandler* input = player->GetInputHandler();
    if (!input) return;

    // 攻撃実行中のみコンボ受付
    if (phase_ == ExecuteAttack && canCombo_ && input->IsAttacking() && comboCount_ < maxCombo_) {
        comboCount_ += 1;
        Enter(player); // 次の攻撃を開始
    }
}

void AttackState::SearchForTarget(Player* player)
{
    if (!player->GetMeleeAttackCollider()) return;

    targetEnemy_ = player->GetMeleeAttackCollider()->GetDetectedEnemy();
}

void AttackState::ProcessMoveToTarget(Player* player, float deltaTime)
{
    if (!targetEnemy_) {
        phase_ = ExecuteAttack;
        return;
    }

    moveTimer_ += deltaTime;

    // 一定時間敵に近づく
    if (moveTimer_ < maxMoveTime_) {
        player->MoveToTarget(targetEnemy_, deltaTime);
    }
    else {
        phase_ = ExecuteAttack;
    }
}

void AttackState::ProcessExecuteAttack(Player* player, float deltaTime)
{
    attackTimer_ += deltaTime;

    // ブロックを回転させる（偶数コンボ: +方向、奇数コンボ: -方向）
    float direction = (comboCount_ % 2 == 0) ? 1.0f : -1.0f;
    blockAngle_ += blockSwingAngle_ / attackDuration_ * deltaTime * direction;
    UpdateBlockPosition(player);

    // コンボ受付時間の判定
    if (attackTimer_ >= attackDuration_ - comboWindow_ && attackTimer_ < attackDuration_) {
        player->GetMeleeAttackCollider()->Damage();
        canCombo_ = true;
    }

    // 攻撃時間が終了したら硬直へ
    if (attackTimer_ >= attackDuration_) {
        phase_ = Recovery;
        attackTimer_ = 0.0f;
    }
}

float AttackState::GetStartAngle() const
{
    // 偶数コンボ: 右側開始（-π/2）、奇数コンボ: 左側開始（π/2）
    return (comboCount_ % 2 == 0) ? -kBlockStartAngle : kBlockStartAngle;
}

void AttackState::UpdateBlockPosition(Player* player)
{
    Object3d* block = player->GetAttackBlock();
    if (!block) return;

    // プレイヤーの向きを考慮した回転角度
    float playerRotY = player->GetRotate().y;
    float worldAngle = playerRotY + blockAngle_;

    // ブロック位置を計算（プレイヤー中心から半径blockRadius_の円周上）
    Vector3 playerPos = player->GetTranslate();
    Vector3 blockPos = {
        playerPos.x + sinf(worldAngle) * blockRadius_,
        playerPos.y,
        playerPos.z + cosf(worldAngle) * blockRadius_
    };

    // ブロックのトランスフォーム設定
    Transform blockTransform;
    blockTransform.translate = blockPos;
    blockTransform.rotate = { 0.0f, worldAngle, 0.0f };
    blockTransform.scale = { kBlockScale, kBlockScale, kBlockScale };

    block->SetTransform(blockTransform);
}

void AttackState::DrawImGui(Player* player)
{
#ifdef _DEBUG
    ImGui::Text("=== Attack State Details ===");
    ImGui::Separator();

    // フェーズ情報
    const char* phaseNames[] = { "SearchTarget", "MoveToTarget", "ExecuteAttack", "Recovery" };
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Phase: %s", phaseNames[phase_]);

    // タイマー情報
    if (ImGui::TreeNode("Timers")) {
        ImGui::Text("Search Timer: %.2f / %.2f", searchTimer_, maxSearchTime_);
        ImGui::ProgressBar(searchTimer_ / maxSearchTime_);

        ImGui::Text("Move Timer: %.2f / %.2f", moveTimer_, maxMoveTime_);
        ImGui::ProgressBar(moveTimer_ / maxMoveTime_);

        ImGui::Text("Attack Timer: %.2f / %.2f", attackTimer_, attackDuration_);
        ImGui::ProgressBar(attackTimer_ / attackDuration_);

        ImGui::TreePop();
    }

    // コンボ情報
    if (ImGui::TreeNode("Combo System")) {
        ImGui::Text("Combo Count: %d / %d", comboCount_, maxCombo_);
        ImGui::SliderInt("Max Combo", &maxCombo_, 1, 5);

        ImGui::Text("Combo Window: %.2f", comboWindow_);
        ImGui::SliderFloat("Combo Window", &comboWindow_, 0.1f, 2.0f);

        if (canCombo_) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Can Combo: YES");
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Can Combo: NO");
        }

        ImGui::TreePop();
    }

    // ターゲット情報
    if (ImGui::TreeNode("Target Info")) {
        if (targetEnemy_) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Target: DETECTED");
            // TODO: ターゲットの詳細情報（位置、距離など）を表示
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Target: NONE");
        }

        ImGui::TreePop();
    }

    // 調整可能なパラメータ
    if (ImGui::TreeNode("Attack Parameters")) {
        ImGui::SliderFloat("Attack Duration", &attackDuration_, 0.1f, 2.0f);
        ImGui::SliderFloat("Max Search Time", &maxSearchTime_, 0.05f, 1.0f);
        ImGui::SliderFloat("Max Move Time", &maxMoveTime_, 0.1f, 2.0f);

        ImGui::TreePop();
    }
#endif
}