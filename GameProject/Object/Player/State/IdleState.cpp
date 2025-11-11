#include "IdleState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input.h"
#include "Input/InputHandler.h"
#ifdef _DEBUG
#include <imgui.h>
#endif

void IdleState::Enter(Player* player)
{
	// モデルのアイドルアニメーションを再生
  // TODO: アニメーション作成後に実装
	// player->GetModel()->PlayAnimation("Idle");
	idleTime_ = 0.0f;
}

void IdleState::Update(Player* player, float deltaTime)
{
	// アイドル状態での処理
	idleTime_ += deltaTime;
}

void IdleState::Exit(Player* player)
{
	// アイドル状態終了時の処理
	idleTime_ = 0.0f;
}

void IdleState::HandleInput(Player* player)
{
	InputHandler* input = player->GetInputHandler();
	if (!input) return;
	
	PlayerStateMachine* stateMachine = player->GetStateMachine();
	if (!stateMachine) return;
	
	// 優先度順に状態遷移をチェック
	
	// パリィ
	if (input->IsParrying())
	{
		stateMachine->ChangeState("Parry");
		return;
	}
	
	// 攻撃
	if (input->IsAttacking())
	{
		stateMachine->ChangeState("Attack");
		return;
	}
	
	// 射撃
	if (input->IsShooting())
	{
		stateMachine->ChangeState("Shoot");
		return;
	}
	
	// ダッシュ
	if (input->IsDashing())
	{
		stateMachine->ChangeState("Dash");
		return;
	}
	
	// 移動
	if (input->IsMoving())
	{
		stateMachine->ChangeState("Move");
		return;
	}
}

void IdleState::DrawImGui(Player* player)
{
#ifdef _DEBUG
	ImGui::Text("=== Idle State Details ===");
	ImGui::Separator();

	// 待機時間
	ImGui::Text("Idle Time: %.2f seconds", idleTime_);

	// 待機アニメーション情報
	if (ImGui::TreeNode("Animation Info")) {
		ImGui::Text("Animation: Idle");
		ImGui::Text("TODO: Add idle animation variations");
		ImGui::TreePop();
	}

	// 利用可能なアクション
	if (ImGui::TreeNode("Available Actions")) {
		ImGui::BulletText("Press W/A/S/D to Move");
		ImGui::BulletText("Press Z to Attack");
		ImGui::BulletText("Press Left Ctrl to Shoot");
		ImGui::BulletText("Press Space to Dash");
		ImGui::BulletText("Press F to Parry");
		ImGui::TreePop();
	}
#endif
}