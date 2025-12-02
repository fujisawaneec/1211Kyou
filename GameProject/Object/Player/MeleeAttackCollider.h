#pragma once
#include "OBBCollider.h"
#include <vector>
#include <unordered_set>

class Player;
class Boss;

/// <summary>
/// 近接攻撃用コライダークラス
/// プレイヤーの近接攻撃判定と敵へのダメージ処理を管理
/// </summary>
class MeleeAttackCollider : public OBBCollider {
	//=========================================================================================
	// 定数
	//=========================================================================================
private:
    static constexpr float kAttackDamage = 10.0f;  ///< 攻撃ダメージ量

private:
	Player* player_ = nullptr;  ///< このコライダーを所有するプレイヤーへのポインタ
	std::unordered_set<uint32_t> hitEnemies_;  ///< この攻撃で既にヒットした敵のIDセット（多重ヒット防止用）
	Boss* detectedEnemy_ = nullptr;  ///< 現在検出されている敵への参照
	bool canDamage = false;  ///< ダメージを与えられる状態かどうか

#ifdef _DEBUG
	int collisionCount_ = 0;  ///< デバッグ用：衝突検出回数
#endif
	
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="player">このコライダーを所有するプレイヤー</param>
	MeleeAttackCollider(Player* player);

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~MeleeAttackCollider() = default;

	/// <summary>
	/// 衝突開始時のコールバック（敵を検出してリストに追加）
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollisionEnter(Collider* other) override;

	/// <summary>
	/// 衝突継続中のコールバック
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollisionStay(Collider* other) override;

	/// <summary>
	/// コライダーの状態をリセット（新しい攻撃の開始時に呼び出す）
	/// hitEnemiesリストとdetectedEnemyをクリア
	/// </summary>
	void Reset();

	/// <summary>
	/// 検出された敵全員にダメージを与える
	/// canDamageフラグがtrueの時のみ実行可能
	/// </summary>
	void Damage();

	/// <summary>
	/// 現在検出されている敵を取得
	/// </summary>
	/// <returns>検出された敵へのポインタ（いない場合はnullptr）</returns>
	Boss* GetDetectedEnemy() const { return detectedEnemy_; }

	/// <summary>
	/// 指定した敵IDが既にヒット済みかチェック
	/// </summary>
	/// <param name="enemyId">チェックする敵のID</param>
	/// <returns>ヒット済みならtrue</returns>
	bool HasHitEnemy(uint32_t enemyId) const;

#ifdef _DEBUG
	/// <summary>
	/// デバッグ用：衝突検出回数を取得
	/// </summary>
	/// <returns>衝突検出回数</returns>
	int GetCollisionCount() const { return collisionCount_; }
#endif
};