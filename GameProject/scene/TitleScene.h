#pragma once
#include "BaseScene.h"
#include"Sprite.h"
#include"Object3d.h"
#include <vector>
#include <memory>
#include "AABB.h"
#include "EmitterManager.h"
#include "PostEffectManager.h"

class TitleScene : public BaseScene
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
	/// タイトルテキストアニメーションを再生
	/// </summary>
	void PlayTitleAnimation();

	/// <summary>
	/// タイトルテキストアニメーションを停止
	/// </summary>
	void StopTitleAnimation();

	/// <summary>
	/// タイトルテキストアニメーションをリセット
	/// </summary>
	void ResetTitleAnimation();

private: // メンバ変数

  std::unique_ptr<EmitterManager> emitterManager_;

  //背景画像
  std::unique_ptr<Sprite> titleBG_;

  // タイトル画像（アニメーション用に10枚）
  std::vector<std::unique_ptr<Sprite>> titleTextSprites_;

  // スタートボタン画像
  std::unique_ptr<Sprite> startButtonText_;

  // タイトルテキストエフェクト用スプライト（拡大フェードアウト用）
  std::unique_ptr<Sprite> titleTextEffect_;

  //RGBSplit Parameter
  RGBSplitParam rgbSplitParam_{};
  //Vignette Parameter
  VignetteParam vignetteParam_{};

  float offsetY = -1000.0f;

  // === タイトルテキストアニメーション制御用変数 === //
  // 現在表示中のフレーム番号（0〜9）
  int currentFrame_ = 0;

  // フレームカウンター（アニメーション速度制御用）
  int frameCounter_ = 0;

  // アニメーション速度（何フレームごとに切り替えるか）
  int animationSpeed_ = 1;

  // アニメーション再生中フラグ
  bool isPlaying_ = false;

  // ループ再生フラグ
  bool isLoop_ = false;

  // アニメーション完了フラグ
  bool animationComplete_ = false;

  // === スタートボタン点滅アニメーション用変数 === //
  // 点滅用タイマー
  float blinkTimer_ = 0.0f;

  // 点滅速度（周期の速さ）
  float blinkSpeed_ = 1.2f;

  // 点滅の最小アルファ値
  float blinkMinAlpha_ = 0.0f;

  // 点滅の最大アルファ値
  float blinkMaxAlpha_ = 1.0f;

  // 点滅アニメーション有効フラグ
  bool isButtonBlinking_ = true;

  // === タイトルテキスト拡大エフェクト用変数 === //
  // エフェクト再生中フラグ
  bool isEffectPlaying_ = false;

  // エフェクトタイマー
  float effectTimer_ = 0.0f;

  // エフェクトの持続時間（秒）
  float effectDuration_ = 1.5f;

  // 現在のスケール値
  float effectScale_ = 1.0f;

  // スケール拡大速度（最終スケール）
  float effectMaxScale_ = 1.5f;

  // 現在のアルファ値
  float effectAlpha_ = 0.5f;

  // 初期アルファ値
  float effectInitialAlpha_ = 0.5f;

  // エフェクト開始トリガー（一度だけ実行）
  bool effectTriggered_ = false;

  // === slashパーティクルエミッターアニメーション用変数 === //
  // アニメーション中フラグ
  bool isSlashEmitterAnimating_ = false;

  // アニメーションタイマー
  float slashEmitterAnimTimer_ = 0.0f;

  // アニメーション持続時間（タイトルテキストと同期）
  float slashEmitterAnimDuration_ = 3.5f;  // 10フレーム * animationSpeed / 60fps

  // Count値のアニメーション
  uint32_t slashEmitterStartCount_ = 1;    // 初期値
  uint32_t slashEmitterEndCount_ = 300;      // 目標値

  // Frequency値のアニメーション
  float slashEmitterStartFreq_ = 1.0f;       // 初期値
  float slashEmitterEndFreq_ = 0.001f;       // 目標値
};
