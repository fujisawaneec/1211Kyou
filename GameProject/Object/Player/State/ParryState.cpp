#include "ParryState.h"
#include "PlayerStateMachine.h"
#include "../Player.h"
#include "Input/InputHandler.h"
#include <algorithm>  // for std::min
#ifdef _DEBUG
#include <imgui.h>
#endif

void ParryState::Enter(Player* player)
{
	// パリィアニメーションを再生
  // TODO: アニメーション作成後に実装
	// player->GetModel()->PlayAnimation("Parry");
	
	parryTimer_ = 0.0f;
	perfectParryActive_ = false;
}

void ParryState::Update(Player* player, float deltaTime)
{
	parryTimer_ += deltaTime;
	
	// パリィ時間が終了したら元の状態に戻る
	if (parryTimer_ >= parryDuration_)
	{
		PlayerStateMachine* stateMachine = player->GetStateMachine();
		if (stateMachine)
		{
			stateMachine->ChangeState("Idle");
		}
	}
}

void ParryState::Exit(Player* player)
{
	parryTimer_ = 0.0f;
	perfectParryActive_ = false;
}

void ParryState::HandleInput(Player* player)
{
	// パリィ中は入力を受け付けない
}

void ParryState::OnParrySuccess(Player* player)
{
	if (IsInParryWindow())
	{
		// パーフェクトパリィ成功
		perfectParryActive_ = true;

		// カウンター攻撃への遷移
		PlayerStateMachine* stateMachine = player->GetStateMachine();
		if (stateMachine)
		{
			stateMachine->ChangeState("Attack");
		}
	}
}

void ParryState::DrawImGui(Player* player)
{
#ifdef _DEBUG
	ImGui::Text("=== Parry State Details ===");
	ImGui::Separator();

	// パリィタイミング表示
	bool inWindow = IsInParryWindow();
	if (inWindow) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "PERFECT PARRY WINDOW!");
	} else {
		ImGui::Text("Normal Parry");
	}

	// タイマー情報
	ImGui::Text("Parry Timer: %.3f / %.3f", parryTimer_, parryDuration_);

	// パリィウィンドウの視覚化
	float windowProgress = (parryWindow_ > 0.0f) ? std::min<float>(1.0f, parryTimer_ / parryWindow_) : 0.0f;
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Perfect Window:");
	ImGui::ProgressBar(windowProgress, ImVec2(-1, 0), "");

	// 全体の進行状況
	float totalProgress = (parryDuration_ > 0.0f) ? (parryTimer_ / parryDuration_) : 0.0f;
	ImGui::Text("Total Progress:");
	ImGui::ProgressBar(totalProgress);

	// パーフェクトパリィステータス
	if (perfectParryActive_) {
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PERFECT PARRY ACTIVE!");
	}

	// パラメータ調整
	if (ImGui::TreeNode("Parry Parameters")) {
		ImGui::SliderFloat("Parry Window", &parryWindow_, 0.05f, 0.5f, "%.3f sec");
		ImGui::SliderFloat("Parry Duration", &parryDuration_, 0.2f, 2.0f, "%.2f sec");

		// プリセット
		if (ImGui::Button("Easy")) {
			parryWindow_ = 0.3f;
			parryDuration_ = 0.8f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Normal")) {
			parryWindow_ = 0.2f;
			parryDuration_ = 0.5f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Hard")) {
			parryWindow_ = 0.1f;
			parryDuration_ = 0.3f;
		}

		ImGui::TreePop();
	}

	// フレーム情報
	if (ImGui::TreeNode("Frame Data")) {
		ImGui::Text("Perfect Window Frames: %d", (int)(parryWindow_ * 60.0f));
		ImGui::Text("Total Frames: %d", (int)(parryDuration_ * 60.0f));
		ImGui::Text("Current Frame: %d", (int)(parryTimer_ * 60.0f));
		ImGui::TreePop();
	}
#endif
}