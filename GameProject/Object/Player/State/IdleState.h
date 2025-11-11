#pragma once
#include "PlayerState.h"

/// <summary>
/// 待機状態クラス
/// プレイヤーが何も操作していない時の状態
/// </summary>
class IdleState : public PlayerState
{
public:
	IdleState() : PlayerState("Idle") {}

	/// <summary>
	/// 待機状態開始時の処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void Enter(Player* player) override;

	/// <summary>
	/// 待機状態の更新処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	void Update(Player* player, float deltaTime) override;

	/// <summary>
	/// 待機状態終了時の処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void Exit(Player* player) override;

	/// <summary>
	/// 待機状態での入力処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void HandleInput(Player* player) override;

	/// <summary>
	/// ImGuiデバッグ情報の描画
	/// </summary>
	void DrawImGui(Player* player) override;

private:
	float idleTime_ = 0.0f;  ///< 待機状態の継続時間
};