#pragma once
#include "PlayerState.h"
#include "Vector3.h"

/// <summary>
/// 射撃状態クラス
/// 遠距離攻撃の照準と発射を制御
/// </summary>
class ShootState : public PlayerState
{
public:
	ShootState() : PlayerState("Shoot") {}

	void Enter(Player* player) override;
	void Update(Player* player, float deltaTime) override;
	void Exit(Player* player) override;
	void HandleInput(Player* player) override;

	/// <summary>
	/// ImGuiデバッグ情報の描画
	/// </summary>
	void DrawImGui(Player* player) override;

	// DrawImGui用のゲッター
	float GetFireRate() const { return fireRate_; }
	float GetFireRateTimer() const { return fireRateTimer_; }
	const Vector3& GetAimDirection() const { return aimDirection_; }

	// DrawImGui用のセッター（デバッグ調整用）
	void SetFireRate(float rate) { fireRate_ = rate; }

private:
	float fireRate_ = 0.2f;         ///< 発射レート（秒間隔）
	float fireRateTimer_ = 0.0f;    ///< 発射間隔タイマー
	Vector3 aimDirection_;           ///< 照準方向ベクトル

	/// <summary>
	/// 照準方向を計算
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void CalculateAimDirection(Player* player);

	/// <summary>
	/// 弾を発射
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void Fire(Player* player);
};