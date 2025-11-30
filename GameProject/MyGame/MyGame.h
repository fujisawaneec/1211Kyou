#pragma once
#include"TakoFramework.h"
#include"Vector2.h"

/// <summary>
/// 3Dアクションゲームのメインクラス
/// ゲーム全体の初期化、更新、描画処理を管理する
/// </summary>
class MyGame : public TakoFramework
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

private: // メンバ変数

  uint32_t spriteBasicOnresizeId_ = 0;
};