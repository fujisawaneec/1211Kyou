#include "ShootState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input/InputHandler.h"
#include "Camera.h"
#include "Matrix4x4.h"
#include <algorithm>  // for std::max
#ifdef _DEBUG
#include <imgui.h>
#endif

void ShootState::Enter(Player* player)
{
	// 射撃アニメーションを再生
  // TODO: アニメーション作成後に実装
	// player->GetModel()->PlayAnimation("Shoot");
	
	fireRateTimer_ = 0.0f;
}

void ShootState::Update(Player* player, float deltaTime)
{
	// 発射レートタイマーの更新
	if (fireRateTimer_ > 0.0f)
	{
		fireRateTimer_ -= deltaTime;
	}
	
	// エイム方向の計算
	CalculateAimDirection(player);
	
	// 射撃可能なら発射
	if (fireRateTimer_ <= 0.0f)
	{
		Fire(player);
		fireRateTimer_ = fireRate_;
	}
	
	// 移動処理
	player->Move(0.5f);
}

void ShootState::Exit(Player* player)
{
	fireRateTimer_ = 0.0f;
}

void ShootState::HandleInput(Player* player)
{
	InputHandler* input = player->GetInputHandler();
	if (!input) return;
	
	// 射撃ボタンが離されたら元の状態に戻る
	if (!input->IsShooting())
	{
		PlayerStateMachine* stateMachine = player->GetStateMachine();
		if (stateMachine)
		{
			if (input->IsMoving())
			{
				stateMachine->ChangeState("Move");
			}
			else
			{
				stateMachine->ChangeState("Idle");
			}
		}
	}
}

void ShootState::CalculateAimDirection(Player* player)
{
	Camera* camera = player->GetCamera();
	if (camera)
	{
		// カメラのビュー行列から前方ベクトルを取得
		Matrix4x4 viewMatrix = camera->GetViewMatrix();
		Matrix4x4 invViewMatrix = Mat4x4::Inverse(viewMatrix);
		aimDirection_ = Vector3(-invViewMatrix.m[2][0], -invViewMatrix.m[2][1], -invViewMatrix.m[2][2]).Normalize();
	}
}

void ShootState::Fire(Player* player)
{
	// 弾を発射する処理
	// TODO: 弾オブジェクトの生成と発射処理を実装
}

void ShootState::DrawImGui(Player* player)
{
#ifdef _DEBUG
	ImGui::Text("=== Shoot State Details ===");
	ImGui::Separator();

	// 発射レート情報
	ImGui::Text("Fire Rate: %.2f (%.1f shots/sec)", fireRate_, 1.0f / fireRate_);
	ImGui::Text("Next Shot In: %.2f", std::max<float>(0.0f, fireRate_ - fireRateTimer_));

	// プログレスバー
	float progress = (fireRate_ > 0.0f) ? (fireRateTimer_ / fireRate_) : 0.0f;
	ImGui::ProgressBar(progress, ImVec2(-1, 0), "Reload");

	// 照準方向
	if (ImGui::TreeNode("Aim Direction")) {
		ImGui::Text("X: %.3f", aimDirection_.x);
		ImGui::Text("Y: %.3f", aimDirection_.y);
		ImGui::Text("Z: %.3f", aimDirection_.z);
		ImGui::Text("Length: %.3f", aimDirection_.Length());
		ImGui::TreePop();
	}

	// パラメータ調整
	if (ImGui::TreeNode("Shooting Parameters")) {
		ImGui::SliderFloat("Fire Rate", &fireRate_, 0.05f, 2.0f, "%.2f sec");

		// プリセット
		if (ImGui::Button("Rapid Fire")) {
			fireRate_ = 0.05f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Normal")) {
			fireRate_ = 0.2f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Slow")) {
			fireRate_ = 0.5f;
		}

		ImGui::TreePop();
	}
#endif
}