#pragma once
#include "PlayerState.h"

/// <summary>
/// 攻撃状態クラス
/// ターゲット検索、移動、攻撃実行、コンボ管理を制御
/// </summary>
class AttackState : public PlayerState
{
public:
	AttackState() : PlayerState("Attack") {}

	void Enter(Player* player) override;
	void Update(Player* player, float deltaTime) override;
	void Exit(Player* player) override;
	void HandleInput(Player* player) override;

private:
	/// <summary>
	/// 攻撃フェーズ
	/// </summary>
	enum AttackPhase {
		SearchTarget,    ///< ターゲット検索フェーズ
		MoveToTarget,    ///< ターゲットへの移動フェーズ
		ExecuteAttack,   ///< 攻撃実行フェーズ
		Recovery         ///< 硬直フェーズ
	};

	AttackPhase phase_ = SearchTarget;                ///< 現在の攻撃フェーズ
	class Boss* targetEnemy_ = nullptr;               ///< 攻撃対象のボス
	float searchTimer_ = 0.0f;                        ///< SearchTarget待機時間
	float maxSearchTime_ = 0.1f;                      ///< 最大検索時間（0.1秒 = 6フレーム@60fps）
	float moveTimer_ = 0.0f;                          ///< 移動タイマー
	float maxMoveTime_ = 0.3f;                        ///< 最大移動時間

	float attackTimer_ = 0.0f;                        ///< 攻撃タイマー
	float attackDuration_ = 0.3f;                     ///< 攻撃持続時間
	int comboCount_ = 0;                              ///< 現在のコンボ数
	int maxCombo_ = 2;                                ///< 最大コンボ数
	float comboWindow_ = 0.8f;                        ///< コンボ受付時間
	bool canCombo_ = false;                           ///< コンボ可能フラグ

	/// <summary>
	/// ターゲット検索処理
	/// 攻撃範囲内の最も近い敵を検索してターゲットに設定する
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void SearchForTarget(Player* player);

	/// <summary>
	/// ターゲットへの移動処理
	/// ターゲットに向かって移動し、攻撃範囲内に到達したら攻撃実行フェーズに遷移
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	void ProcessMoveToTarget(Player* player, float deltaTime);

	/// <summary>
	/// 攻撃実行処理
	/// アニメーション再生、ダメージ判定、コンボ管理を行う
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	void ProcessExecuteAttack(Player* player, float deltaTime);

public:
	/// <summary>
	/// 現在の攻撃フェーズを取得（デバッグ用）
	/// </summary>
	/// <returns>現在の攻撃フェーズ</returns>
	AttackPhase GetPhase() const { return phase_; }

	/// <summary>
	/// 現在のターゲット敵を取得（デバッグ用）
	/// </summary>
	/// <returns>ターゲット敵のポインタ（nullptrの場合もある）</returns>
	Boss* GetTargetEnemy() const { return targetEnemy_; }

	/// <summary>
	/// 攻撃タイマーを取得（デバッグ用）
	/// </summary>
	/// <returns>現在の攻撃タイマー値（秒）</returns>
	float GetAttackTimer() const { return attackTimer_; }

	/// <summary>
	/// 検索タイマーを取得（デバッグ用）
	/// </summary>
	/// <returns>現在の検索タイマー値（秒）</returns>
	float GetSearchTimer() const { return searchTimer_; }

	/// <summary>
	/// ImGuiデバッグ情報の描画
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	void DrawImGui(Player* player) override;

	// DrawImGui用のゲッター追加
	int GetComboCount() const { return comboCount_; }
	int GetMaxCombo() const { return maxCombo_; }
	float GetMoveTimer() const { return moveTimer_; }
	float GetMaxMoveTime() const { return maxMoveTime_; }
	float GetAttackDuration() const { return attackDuration_; }
	float GetComboWindow() const { return comboWindow_; }
	bool CanCombo() const { return canCombo_; }

	// DrawImGui用のセッター追加（デバッグ調整用）
	void SetMaxCombo(int combo) { maxCombo_ = combo; }
	void SetAttackDuration(float duration) { attackDuration_ = duration; }
	void SetMaxMoveTime(float time) { maxMoveTime_ = time; }
	void SetComboWindow(float window) { comboWindow_ = window; }
};