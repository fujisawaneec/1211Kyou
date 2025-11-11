#include "AttackState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input/InputHandler.h"
#include "../MeleeAttackCollider.h"
#include "../../Boss/Boss.h"
#include "CollisionManager.h"
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
				} else {
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
			if (attackTimer_ >= attackDuration_ + 0.2f) {
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
	} else {
		phase_ = ExecuteAttack;
	}
}

void AttackState::ProcessExecuteAttack(Player* player, float deltaTime)
{
	attackTimer_ += deltaTime;

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
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Can Combo: NO");
		}

		ImGui::TreePop();
	}

	// ターゲット情報
	if (ImGui::TreeNode("Target Info")) {
		if (targetEnemy_) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Target: DETECTED");
			// TODO: ターゲットの詳細情報（位置、距離など）を表示
		} else {
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