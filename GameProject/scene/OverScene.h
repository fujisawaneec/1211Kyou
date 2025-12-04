#pragma once
#include <memory>

#include "BaseScene.h"
#include "vector2.h"
#include "CameraSystem/CameraConfig.h"

class Sprite;

/// <summary>
/// クリアシーンクラス
/// ゲームクリア時の演出と結果表示を管理
/// </summary>
class OverScene : public BaseScene
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

    // sprite
    std::unique_ptr<Sprite> backGround_ = nullptr;
    std::unique_ptr<Sprite> titleText_ = nullptr;
    std::unique_ptr<Sprite> pressButtonText_ = nullptr;

    // カメラ非表示Y座標
    float cameraHiddenY_ = CameraConfig::HIDDEN_Y;

    // UI位置・サイズ用変数
    float titleTextHalfWidth_ = 250.0f;  ///< タイトルテキスト半幅（センタリング用）
    float titleTextY_ = 300.0f;  ///< タイトルテキストY座標
    float buttonBottomOffset_ = 300.0f;  ///< ボタン下端からのオフセット
};

