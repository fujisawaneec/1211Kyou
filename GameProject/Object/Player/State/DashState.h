#pragma once
#include "PlayerState.h"

/// <summary>
/// ダッシュ状態クラス
/// 短時間の高速移動を制御
/// </summary>
class DashState : public PlayerState
{
public:
	DashState() : PlayerState("Dash") {}

	void Enter(Player* player) override;
	void Update(Player* player, float deltaTime) override;
	void Exit(Player* player) override;
	void HandleInput(Player* player) override;

	/// <summary>
	/// ImGuiデバッグ情報の描画
	/// </summary>
	void DrawImGui(Player* player) override;

	// DrawImGui用のゲッター
	float GetTimer() const { return timer_; }
	float GetDuration() const { return duration_; }
	float GetSpeed() const { return speed_; }

	// DrawImGui用のセッター（デバッグ調整用）
	void SetDuration(float duration) { duration_ = duration; }
	void SetSpeed(float speed) { speed_ = speed; }

private:
	float timer_ = 0.0f;        ///< ダッシュ経過時間
	float duration_ = 0.05f;    ///< ダッシュ持続時間
	float speed_ = 10.0f;       ///< ダッシュ速度
};