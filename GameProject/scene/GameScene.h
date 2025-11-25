#pragma once
#include "BaseScene.h"
#include "Transform.h"
#include  "SkyBox.h"
#include "Object/Boss/Boss.h"
#include "Object/Player/Player.h"
#include "Input/InputHandler.h"
#include "../Object/Projectile/BossBullet.h"

#include <memory>
#include <vector>

// クラス前方宣言
class Object3d;
class EmitterManager;
class Sprite;
class BoneTracker;
class CameraManager;
class ThirdPersonController;
class TopDownController;
class CameraAnimationController;
class BossBullet;

/// <summary>
/// ゲームメインシーンクラス
/// プレイヤーとボスの戦闘、ゲームプレイの中核を管理
/// </summary>
class GameScene : public BaseScene
{
public: // メンバ関数
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;
    void DrawWithoutEffect() override;

    /// <summary>
    /// ImGuiの描画
    /// </summary>
    void DrawImGui() override;

    /// <summary>
    /// ゲームオーバー演出開始
    /// </summary>
    void StartOverAnim();

    /// <summary>
    /// ゲームオーバー演出更新
    /// </summary>
    void UpdateOverAnim();

    /// <summary>
    /// ゲームクリア演出開始
    /// </summary>
    void StartClearAnim();

    /// <summary>
    /// ゲームクリア演出更新
    /// </summary>
    void UpdateClearAnim();

    /// <summary>
    /// カメラモードの更新処理
    /// </summary>
    void UpdateCameraMode();

    /// <summary>
    /// 入力処理の更新
    /// </summary>
    void UpdateInput();

    /// <summary>
    /// プロジェクタイル（弾）の更新処理
    /// </summary>
    void UpdateProjectiles(float deltaTime);

    /// <summary>
    /// ボスフェーズ2の境界線パーティクル制御
    /// </summary>
    void UpdateBossBorder();

    /// <summary>
    /// ボスの弾の生成処理
    /// </summary>
    void CreateBossBullet();

private: // メンバ変数

    std::unique_ptr<SkyBox> skyBox_;                            // スカイボックス（環境マップ）

    std::unique_ptr<Object3d> ground_;                          // 地面オブジェクト

    std::unique_ptr<Player> player_;                            // プレイヤーキャラクター

    std::unique_ptr<Boss> boss_;                                // ボスキャラクター

    std::vector<std::unique_ptr<BossBullet>> bossBullets_;      // ボスの弾のコンテナ

    std::unique_ptr<InputHandler> inputHandler_;                // 入力ハンドラー

    // Camera system components
    CameraManager* cameraManager_ = nullptr;                    // カメラシステム管理
    ThirdPersonController* firstPersonController_ = nullptr;    // 一人称視点コントローラー
    TopDownController* topDownController_ = nullptr;            // トップダウン視点コントローラー
    CameraAnimationController* animationController_ = nullptr;  // カメラアニメーションコントローラー
    bool cameraMode_ = false;                                   // カメラモード (true: FirstPerson, false: TopDown)

    Transform groundUvTransform_{};                             // 地面のUVトランスフォーム（テクスチャスクロール等に使用）

    std::unique_ptr<EmitterManager> emitterManager_;            // パーティクルエミッター管理

    std::unique_ptr<Sprite> toTitleSprite_;                     // タイトルに戻るボタンテキスト

    bool isStart_ = false;                                      // ゲーム開始フラグ
    
    // ボスフェーズ2境界線パーティクル管理
    bool borderEmittersActive_ = false;                         // 境界線エミッターアクティブ状態

    float battleAreaSize_ = 20.0f;                              // 戦闘エリアのサイズ

    // ゲームオーバー演出関連
    float overAnimTimer_ = 0.0f;                                // ゲームオーバー演出タイマー
    bool isOver_ = false;                                       // ゲームオーバーフラグ
    bool isOver1Emit_ = false;                                  // ゲームオーバー演出用エミッター発生フラグ
    bool isOver2Emit_ = false;                                  // ゲームオーバー演出用エミッター発生フラグ

    // ゲームクリア演出関連
    float clearAnimTimer_ = 0.0f;                               // ゲームクリア演出タイマー
    bool isClear_ = false;                                      // ゲームクリアフラグ
    bool isClear1Emit_ = false;                                 // ゲームクリア演出用エミッター発生フラグ
    bool isClear2Emit_ = false;                                 // ゲームクリア演出用エミッター発生フラグ
    const uint32_t kSlashEmitterMaxCount_ = 100;                // 斬撃エミッター最大数
    const float kSlashEmitterMaxRadius_ = 10.0f;                // 斬撃エミッター最大発生半径
    uint32_t currentSlashCount_ = 1;                            // 現在の斬撃発生数
    float currentSlashRadius_ = 2.f;                            // 現在の斬撃発生半径

    bool isDebug_ = false;                                      // デバッグモードフラグ

    // ダッシュエフェクト補間用
    Vector3 dashEmitterPosition_{};                              // エミッターの補間位置
    bool previousIsDashing_ = false;                             // 前フレームのダッシュ状態
    bool dashEmitterActive_ = false;                             // エミッターのアクティブ状態

    /// <summary>
    /// ダッシュエフェクトエミッターの更新（Lerp補間）
    /// </summary>
    /// <param name="deltaTime">フレーム間の経過時間</param>
    void UpdateDashEmitter(float deltaTime);
};