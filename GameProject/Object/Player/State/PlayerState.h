#pragma once
#include <string>

class Player;
class PlayerStateMachine;

/// <summary>
/// プレイヤーステート基底クラス
/// 各状態の基本インターフェースを定義
/// </summary>
class PlayerState
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="name">状態名</param>
	PlayerState(const std::string& name) : stateName_(name) {}

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~PlayerState() = default;

	/// <summary>
	/// 状態開始時の処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	virtual void Enter(Player* player) = 0;

	/// <summary>
	/// 状態の更新処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	virtual void Update(Player* player, float deltaTime) = 0;

	/// <summary>
	/// 状態終了時の処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	virtual void Exit(Player* player) = 0;

	/// <summary>
	/// 入力処理
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	virtual void HandleInput(Player* player) {}

	/// <summary>
	/// 指定状態への遷移可否を判定
	/// </summary>
	/// <param name="stateName">遷移先の状態名</param>
	/// <returns>遷移可能な場合true</returns>
	virtual bool CanTransitionTo(const std::string& stateName) const { return true; }

	/// <summary>
	/// 状態名を取得
	/// </summary>
	/// <returns>状態名の参照</returns>
	const std::string& GetName() const { return stateName_; }

	/// <summary>
	/// ImGuiを使用したデバッグ情報の描画
	/// 各ステート固有のデバッグ情報を表示するために使用
	/// </summary>
	/// <param name="player">プレイヤーインスタンス</param>
	virtual void DrawImGui(Player* player) {}

protected:
	/// <summary>
	/// 状態を変更
	/// 現在の状態からExit()を呼び出し、新しい状態のEnter()を呼び出す
	/// </summary>
	/// <param name="stateMachine">状態管理を行うステートマシンインスタンス</param>
	/// <param name="newState">遷移先の新しい状態インスタンス（PlayerStateの派生クラス）</param>
	void ChangeState(PlayerStateMachine* stateMachine, PlayerState* newState);

private:
	std::string stateName_;
};