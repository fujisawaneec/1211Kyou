#pragma once
#include <memory>

#include "Transform.h"
#include "Vector4.h"

class OBBCollider;
class Object3d;

/// <summary>
/// ボスエネミークラス
/// HPとフェーズ管理、ダメージ処理を制御
/// </summary>
class Boss
{
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
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

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

    //-----------------------------Getters/Setters------------------------------//
    /// <summary>
    /// 座標変換情報を設定
    /// </summary>
    /// <param name="transform">新しい座標変換情報</param>
    void SetTransform(const Transform& transform) { transform_ = transform; }

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
    /// 座標変換情報を取得
    /// </summary>
    /// <returns>現在の座標変換情報の参照</returns>
    const Transform& GetTransform() const { return transform_; }

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
    /// ボスの3Dモデルオブジェクト（描画とアニメーション管理）
    std::unique_ptr<Object3d> model_;

    /// ボスの座標変換情報（位置、回転、スケール）
    Transform transform_{};

    /// 最大HP
    const float maxHp_ = 200.0f;

    /// ボスの現在HP（0になると撃破、初期値200）
    float hp_ = maxHp_;

    /// ボスのライフ（HPが0になるたびに減少、0でゲームクリア）
    uint8_t life_ = 2;

    /// 現在の戦闘フェーズ（HP200~100:フェーズ1、HP100~0:フェーズ2）
    uint32_t phase_ = 1;

    /// フェーズ変更準備完了フラグ
    bool isReadyToChangePhase_ = false;

    /// 死亡フラグ
    bool isDead_ = false;

    /// ボス本体の衝突判定用AABBコライダー（矩形境界ボックス）
    std::unique_ptr<OBBCollider> bodyCollider_;

    /// ヒットエフェクトの再生状態を示すフラグ。
    bool isPlayHitEffect_ = false;

    /// ヒットエフェクトのタイマー
    float hitEffectTimer_ = 0.0f;
};

