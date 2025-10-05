#pragma once
#include "BaseScene.h"
#include"Sprite.h"
#include"Object3d.h"
#include <vector>
#include <memory>
#include "AABB.h"
#include "EmitterManager.h"

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

private: // メンバ変数

  std::unique_ptr<EmitterManager> emitterManager_;

  //背景画像
  std::unique_ptr<Sprite> titleBG_;

  // タイトル画像
  std::unique_ptr<Sprite> titleText_;

  // スタートボタン画像
  std::unique_ptr<Sprite> startButtonText_;
};
