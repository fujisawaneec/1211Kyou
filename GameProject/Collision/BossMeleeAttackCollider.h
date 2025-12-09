#pragma once
#include "OBBCollider.h"

class Boss;
class Player;

/// <summary>
/// ボス近接攻撃用コライダークラス
/// プレイヤーへのダメージ処理を管理
/// </summary>
class BossMeleeAttackCollider : public OBBCollider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="boss">このコライダーを所有するボス</param>
	BossMeleeAttackCollider(Boss* boss);

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~BossMeleeAttackCollider() = default;

	/// <summary>
	/// 衝突開始時のコールバック
	/// プレイヤーに接触した場合ダメージを与える
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollisionEnter(Collider* other) override;

	/// <summary>
	/// 衝突継続中のコールバック
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollisionStay(Collider* other) override;

	/// <summary>
	/// コライダーの状態をリセット
	/// 次の攻撃に備えてヒットフラグをクリア
	/// </summary>
	void Reset();

	/// <summary>
	/// ダメージ量を設定
	/// </summary>
	/// <param name="damage">設定するダメージ量</param>
	void SetDamage(float damage) { damage_ = damage; }

	/// <summary>
	/// ダメージ量を取得
	/// </summary>
	/// <returns>現在のダメージ量</returns>
	float GetDamage() const { return damage_; }

	/// <summary>
	/// プレイヤーにヒットしたかどうかを取得
	/// </summary>
	/// <returns>ヒット済みならtrue</returns>
	bool HasHitPlayer() const { return hasHitPlayer_; }

private:
	Boss* boss_ = nullptr;           ///< このコライダーを所有するボス
	float damage_ = 10.0f;           ///< 攻撃ダメージ量
	bool hasHitPlayer_ = false;      ///< 多重ヒット防止フラグ
};
