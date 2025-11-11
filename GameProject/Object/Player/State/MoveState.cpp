#include "MoveState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input/InputHandler.h"
#ifdef _DEBUG
#include <imgui.h>
#endif

void MoveState::Enter(Player* player)
{
	// モデルの歩行アニメーションを再生
  // TODO: アニメーション作成後に実装
	// player->GetModel()->PlayAnimation("Walk");
	moveTime_ = 0.0f;
}

void MoveState::Update(Player* player, float deltaTime)
{
	// 通常速度で移動
	player->Move(1.0f);
	moveTime_ += deltaTime;
}

void MoveState::Exit(Player* player)
{
	// 歩行状態終了時の処理
	moveTime_ = 0.0f;
}

void MoveState::HandleInput(Player* player)
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
	
	// 移動入力がなければIdleへ
	if (!input->IsMoving())
	{
		stateMachine->ChangeState("Idle");
		return;
	}
}

void MoveState::DrawImGui(Player* player)
{
#ifdef _DEBUG
	ImGui::Text("=== Move State Details ===");
	ImGui::Separator();

	// 移動時間
	ImGui::Text("Move Time: %.2f seconds", moveTime_);

	// 移動速度情報
	if (player) {
		ImGui::Text("Current Speed: %.2f", player->GetSpeed());
		const Vector3& velocity = player->GetVelocity();
		ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
		ImGui::Text("Velocity Magnitude: %.2f", velocity.Length());
	}

	// 移動方向
	if (ImGui::TreeNode("Movement Direction")) {
		InputHandler* input = player->GetInputHandler();
		if (input && input->IsMoving()) {
			ImGui::Text("Input Active: YES");
			// TODO: Display actual input direction vector
		} else {
			ImGui::Text("Input Active: NO");
		}
		ImGui::TreePop();
	}

	// アニメーション情報
	if (ImGui::TreeNode("Animation Info")) {
		ImGui::Text("Animation: Walk");
		ImGui::Text("TODO: Add walk/run animation blending");
		ImGui::TreePop();
	}

	// 利用可能なアクション
	if (ImGui::TreeNode("Available Actions")) {
		ImGui::BulletText("Press Space to Dash");
		ImGui::BulletText("Press Z to Attack");
		ImGui::BulletText("Press Left Ctrl to Shoot");
		ImGui::BulletText("Press F to Parry");
		ImGui::BulletText("Release movement keys to stop");
		ImGui::TreePop();
	}
#endif
}