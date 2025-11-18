#pragma once
#include <memory>
#include <vector>

#include "Transform.h"
#include "vector2.h"
#include "Vector4.h"
#include "Vector3.h"

class Sprite;
class OBBCollider;
class Object3d;
class BossStateMachine;
class Player;
class BossShootState;

/// <summary>
/// ボスエネミークラス
/// HPとフェーズ管理、ダメージ処理を制御
/// </summary>
class Boss
{
public:
    /// <summary>
    /// 弾生成リクエスト構造体
    /// </summary>
    struct BulletSpawnRequest {
        Vector3 position;  // 発射位置
        Vector3 velocity;  // 弾の速度ベクトル
    };

public:
    Boss();
    ~Boss();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// 更新（deltaTime版）
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// スプライト描画
    /// </summary>
    void DrawSprite();

    /// <summary>
    /// ImGuiの描画
    /// </summary>
    void DrawImGui();

    /// <summary>
    /// ダメージ処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void OnHit(float damage);

    /// <summary>
    /// ダメージされるとき色変わる演出の更新処理
    /// </summary>
    /// <param name="color">変化後の色</param>
    /// <param name="duration">変化時間</param>
    void UpdateHitEffect(Vector4 color, float duration);

    /// <summary>
    /// フェーズとlifeの更新処理
    /// </summary>
    void UpdatePhaseAndLive();

    /// <summary>
    /// 弾生成リクエストを追加
    /// </summary>
    /// <param name="position">弾の発射位置</param>
    /// <param name="velocity">弾の速度ベクトル</param>
    void RequestBulletSpawn(const Vector3& position, const Vector3& velocity);

    /// <summary>
    /// 保留中の弾生成リクエストを取得して消費
    /// </summary>
    /// <returns>弾生成リクエストのリスト（moveで返される）</returns>
    std::vector<BulletSpawnRequest> ConsumePendingBullets();

    //-----------------------------Getters/Setters------------------------------//
    /// <summary>
    /// 座標変換情報を設定
    /// </summary>
    /// <param name="transform">新しい座標変換情報</param>
    void SetTransform(const Transform& transform) { transform_ = transform; }

    /// <summary>
    /// Translateを設定
    /// </summary>
    /// <param name="translate">新しいTranslate値</param>
    void SetTranslate(Vector3 translate) { transform_.translate = translate; }

    /// <summary>
    /// 位置を設定
    /// </summary>
    /// <param name="position">新しい位置</param>
    void SetPosition(const Vector3& position) { transform_.translate = position; }

    /// <summary>
    /// 回転を設定
    /// </summary>
    /// <param name="rotation">新しい回転（オイラー角）</param>
    void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }

    /// <summary>
    /// HPを設定
    /// </summary>
    /// <param name="hp">新しいHP値</param>
    void SetHp(float hp) { hp_ = hp; }

    /// <summary>
    /// フェーズを設定
    /// </summary>
    /// <param name="phase">新しいフェーズ番号</param>
    void SetPhase(uint32_t phase) { if (phase <= 2 && phase > 0) phase_ = phase; }

    /// <summary>
    /// プレイヤーの参照を設定
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    void SetPlayer(Player* player) { player_ = player; }

    /// <summary>
    /// 一時行動停止フラグを設定
    /// </summary>
    /// <param name="isPause">一時行動停止フラグの値</param>
    void SetIsPause(bool isPause) { isPause_ = isPause; }

    /// <summary>
    /// 座標変換情報を取得
    /// </summary>
    /// <returns>現在の座標変換情報の参照</returns>
    const Transform& GetTransform() const { return transform_; }

    /// <summary>
    /// 座標変換情報を取得（非const版）
    /// </summary>
    /// <returns>現在の座標変換情報の参照</returns>
    Transform& GetWorldTransform() { return transform_; }

    /// <summary>
    /// 座標変換情報のポインタを取得
    /// </summary>
    /// <returns>座標変換情報への非constポインタ</returns>
    Transform* GetTransformPtr() { return &transform_; }

    /// <summary>
    /// ステートマシンを取得
    /// </summary>
    /// <returns>ステートマシンのポインタ</returns>
    BossStateMachine* GetStateMachine() const { return stateMachine_.get(); }

    /// <summary>
    /// プレイヤーを取得
    /// </summary>
    /// <returns>プレイヤーのポインタ</returns>
    Player* GetPlayer() const { return player_; }

    /// <summary>
    /// HPを取得
    /// </summary>
    /// <returns>現在のHP値</returns>
    float GetHp() const { return hp_; }

    /// <summary>
    /// 現在のフェーズを取得
    /// </summary>
    /// <returns>フェーズ番号</returns>
    uint32_t GetPhase() const { return phase_; }

    /// <summary>
    /// 死亡状態を取得
    /// </summary>
    /// <returns> 死亡しているかどうかの真偽値 </returns>
    bool IsDead() const { return isDead_; }

    /// <summary>
    /// コライダーを取得
    /// </summary>
    /// <returns>ボスのAABBコライダーのポインタ</returns>
    OBBCollider* GetCollider() const { return bodyCollider_.get(); }

private:
    // ボスの3Dモデルオブジェクト（描画とアニメーション管理）
    std::unique_ptr<Object3d> model_;

    // ボスの座標変換情報（位置、回転、スケール）
    Transform transform_{};

    // ステートマシン
    std::unique_ptr<BossStateMachine> stateMachine_;

    // プレイヤーへの参照
    Player* player_ = nullptr;

    // 最大HP
    const float kMaxHp_ = 200.0f;

    // フェーズ2開始HP閾値
    const float kPhase2HP = 110.0f;

    // ボスの現在HP（0になると撃破、初期値200）
    float hp_ = kMaxHp_;

    // ボスのライフ（HPが0になるたびに減少、0でゲームクリア）
    uint8_t life_ = 1;

    // 現在の戦闘フェーズ（HP200~100:フェーズ1、HP100~0:フェーズ2）
    uint32_t phase_ = 1;

    // フェーズ変更準備完了フラグ
    bool isReadyToChangePhase_ = false;

    // 死亡フラグ
    bool isDead_ = false;

    // 一時行動停止フラグ
    bool isPause_ = false;

    // ボス本体の衝突判定用AABBコライダー（矩形境界ボックス）
    std::unique_ptr<OBBCollider> bodyCollider_;

    // ヒットエフェクトの再生状態を示すフラグ。
    bool isPlayHitEffect_ = false;

    // ヒットエフェクトのタイマー
    float hitEffectTimer_ = 0.0f;

    // 弾生成リクエストのキュー（GameSceneが処理）
    std::vector<BulletSpawnRequest> pendingBullets_;

    // HPバースプライト
    std::unique_ptr<Sprite> hpBarSprite1_;
    Vector2 hpBarSize1_{};
    std::unique_ptr<Sprite> hpBarSprite2_;
    Vector2 hpBarSize2_{};
    std::unique_ptr<Sprite> hpBarBGSprite_;
};

