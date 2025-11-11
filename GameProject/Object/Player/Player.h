#pragma once
#include <memory>

#include "Collider.h"
#include "Transform.h"

class OBBCollider;
class Object3d;
class PlayerStateMachine;
class InputHandler;
class Camera;
class MeleeAttackCollider;
class Boss;

/// <summary>
/// プレイヤーキャラクタークラス
/// 移動、攻撃、ステート管理などプレイヤーの動作を制御
/// </summary>
class Player
{
public: // メンバ関数
    Player();
    ~Player();

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
    /// 移動
    /// </summary>
    /// <param name="speedMultiplier">速度倍率（デフォルト1.0）</param>
    void Move(float speedMultiplier = 1.0f);

    /// <summary>
    /// ターゲットへ移動
    /// </summary>
    /// <param name="target">移動先のボスエネミー</param>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void MoveToTarget(Boss* target, float deltaTime);

    /// <summary>
    /// ImGuiの描画
    /// </summary>
    void DrawImGui();

    /// <summary>
    /// コライダーの初期設定
    /// </summary>
    void SetupColliders();

    /// <summary>
    /// 攻撃コライダーの更新
    /// </summary>
    void UpdateAttackCollider();

    /// <summary>
    /// 近接攻撃ヒット時の処理
    /// </summary>
    /// <param name="other">衝突相手のコライダー</param>
    void OnMeleeAttackHit(Collider* other);

    //-----------------------------Getters/Setters------------------------------//
    /// <summary>
    /// 移動速度を設定
    /// </summary>
    /// <param name="speed">新しい移動速度</param>
    void SetSpeed(float speed) { speed_ = speed; }

    /// <summary>
    /// カメラを設定
    /// </summary>
    /// <param name="camera">使用するカメラのポインタ</param>
    void SetCamera(Camera* camera) { camera_ = camera; }

    /// <summary>
    /// カメラモードを設定
    /// </summary>
    /// <param name="mode">true: 一人称視点, false: トップダウン視点</param>
    void SetMode(bool mode) { mode_ = mode; }

    /// <summary>
    /// 座標変換情報を設定
    /// </summary>
    /// <param name="transform">新しい座標変換情報</param>
    void SetTransform(const Transform& transform) { transform_ = transform; }

    /// <summary>
    /// HPを設定
    /// </summary>
    /// <param name="hp">新しいHP値（0未満は0に補正）</param>
    void SetHp(float hp) { hp_ = hp; if (hp_ < 0.f) hp_ = 0.f; }

    /// <summary>
    /// 入力ハンドラを設定
    /// </summary>
    void SetInputHandler(InputHandler* inputHandler) { inputHandlerPtr_ = inputHandler; }

    /// <summary>
    /// 移動速度を取得
    /// </summary>
    /// <returns>現在の移動速度</returns>
    float GetSpeed() const { return speed_; }

    /// <summary>
    /// カメラを取得
    /// </summary>
    /// <returns>現在のカメラのポインタ</returns>
    Camera* GetCamera() const { return camera_; }

    /// <summary>
    /// カメラモードを取得
    /// </summary>
    /// <returns>true: 一人称視点, false: トップダウン視点</returns>
    bool GetMode() const { return mode_; }

    /// <summary>
    /// HPを取得
    /// </summary>
    /// <returns>現在のHP値</returns>
    float GetHp() const { return hp_; }

    /// <summary>
    /// 座標変換情報を取得
    /// </summary>
    /// <returns>現在の座標変換情報の参照</returns>
    const Transform& GetTransform() const { return transform_; }

    /// <summary>
    /// 3Dモデルを取得
    /// </summary>
    /// <returns>プレイヤーモデルのポインタ</returns>
    Object3d* GetModel() const { return model_.get(); }

    /// <summary>
    /// ステートマシンを取得
    /// </summary>
    /// <returns>プレイヤーステートマシンのポインタ</returns>
    PlayerStateMachine* GetStateMachine() const { return stateMachine_.get(); }

    /// <summary>
    /// 近接攻撃コライダーを取得
    /// </summary>
    /// <returns>近接攻撃コライダーのポインタ</returns>
    MeleeAttackCollider* GetMeleeAttackCollider() const { return meleeAttackCollider_.get(); }

    /// <summary>
    /// 座標変換情報のポインタを取得
    /// </summary>
    /// <returns>座標変換情報への非constポインタ</returns>
    Transform* GetTransformPtr() { return &transform_; }

    /// <summary>
    /// Velocityを取得
    /// </summary>
    /// <returns>現在のVelocity値の参照</returns>
    Vector3& GetVelocity() { return velocity_; }

    /// <summary>
    /// InputHandlerを取得
    /// </summary>
    /// <returns>InputHandlerのポインタ</returns>
    InputHandler* GetInputHandler() { return inputHandlerPtr_; };

    /// <summary>
    /// 動的移動範囲を設定
    /// </summary>
    /// <param name="xMin">X座標の最小値</param>
    /// <param name="xMax">X座標の最大値</param>
    /// <param name="zMin">Z座標の最小値</param>
    /// <param name="zMax">Z座標の最大値</param>
    void SetDynamicBounds(float xMin, float xMax, float zMin, float zMax);

    /// <summary>
    /// 中心点と範囲から動的移動範囲を設定
    /// </summary>
    /// <param name="center">中心座標</param>
    /// <param name="xRange">X方向の範囲（片側）</param>
    /// <param name="zRange">Z方向の範囲（片側）</param>
    void SetDynamicBoundsFromCenter(const Vector3& center, float xRange, float zRange);

    /// <summary>
    /// 動的移動範囲をクリア（無効化）
    /// </summary>
    void ClearDynamicBounds();

private: // メンバ変数
    // ステージ全体の移動制限（静的）
    static float X_MIN;
    static float X_MAX;
    static float Z_MIN;
    static float Z_MAX;

    // 動的移動制限（ボス戦闘エリアなど）
    float dynamicXMin_;
    float dynamicXMax_;
    float dynamicZMin_;
    float dynamicZMax_;

    std::unique_ptr<Object3d> model_; ///< モデル
    Camera* camera_ = nullptr;        ///< カメラ
    Transform transform_{};           ///< 変形情報
    Vector3 velocity_{};              ///< 速度
    float speed_ = 0.5f;              ///< 移動速度
    float targetAngle_ = 0.f;         ///< 目標角度
    float hp_ = 100.f;                ///< 体力

    bool mode_ = false;               ///< true: ThirdPersonMode, false: TopDownMode
    bool isDisModelDebugInfo_ = false;///< モデルデバッグ情報の表示

    // システム
    std::unique_ptr<PlayerStateMachine> stateMachine_;
    InputHandler* inputHandlerPtr_;

    // Colliders
    std::unique_ptr<OBBCollider> bodyCollider_;
    std::unique_ptr<MeleeAttackCollider> meleeAttackCollider_;

    // 攻撃関連
    Boss* targetEnemy_ = nullptr;
    bool isAttackHit_ = false;
    float attackMoveSpeed_ = 2.0f;
};

