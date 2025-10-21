# CameraAnimation システム

キーフレームベースのカメラアニメーションシステムです。
時間軸上にキーフレームを配置し、その間を補間することで滑らかなカメラ動作を実現します。

## 機能

- **キーフレーム補間**：位置、回転、FOVの補間
- **イージング関数**：LINEAR、EASE_IN、EASE_OUT、EASE_IN_OUT
- **JSON形式での保存/読み込み**
- **ImGuiによるリアルタイム編集**（デバッグビルド）
- **ループ/ワンショット再生**
- **再生速度調整**

## 使い方

### 基本的な使用例

```cpp
#include "CameraAnimation/CameraAnimation.h"

class GameScene : public BaseScene {
private:
    std::unique_ptr<CameraAnimation> cameraAnimation_;

public:
    void Initialize() override {
        // カメラアニメーションの作成
        cameraAnimation_ = std::make_unique<CameraAnimation>();

        // JSONファイルから読み込み
        cameraAnimation_->LoadFromJson("intro_camera.json");

        // カメラを設定
        cameraAnimation_->SetCamera(camera_);

        // 再生開始
        cameraAnimation_->Play();
    }

    void Update() override {
        // アニメーションの更新
        float deltaTime = 1.0f / 60.0f; // 60FPS想定
        cameraAnimation_->Update(deltaTime);

        // デバッグUI表示
        #ifdef _DEBUG
        cameraAnimation_->DrawImGui();
        #endif
    }
};
```

### コードでキーフレームを作成

```cpp
// アニメーション作成
auto cameraAnim = std::make_unique<CameraAnimation>();
cameraAnim->SetCamera(camera_);
cameraAnim->SetAnimationName("My Animation");
cameraAnim->SetLooping(true);

// キーフレーム追加
cameraAnim->AddKeyframe(CameraKeyframe(
    0.0f,                                       // 時間
    Vector3(0.0f, 10.0f, -20.0f),             // 位置
    Vector3(0.2f, 0.0f, 0.0f),                 // 回転
    0.45f,                                      // FOV
    CameraKeyframe::InterpolationType::LINEAR  // 補間タイプ
));

cameraAnim->AddKeyframe(CameraKeyframe(
    5.0f,
    Vector3(10.0f, 15.0f, -10.0f),
    Vector3(0.3f, 1.57f, 0.0f),
    0.6f,
    CameraKeyframe::InterpolationType::EASE_IN_OUT
));

// 再生
cameraAnim->Play();
```

### 現在のカメラ位置からキーフレーム作成

```cpp
// 現在のカメラ状態を時刻2.5秒のキーフレームとして追加
cameraAnimation_->AddKeyframeFromCurrentCamera(
    2.5f,
    CameraKeyframe::InterpolationType::EASE_IN_OUT
);
```

### 再生制御

```cpp
// 再生
cameraAnimation_->Play();

// 一時停止
cameraAnimation_->Pause();

// 停止（時間を0にリセット）
cameraAnimation_->Stop();

// 現在時刻をリセット
cameraAnimation_->Reset();

// 特定時刻にシーク
cameraAnimation_->SetCurrentTime(3.5f);

// ループ設定
cameraAnimation_->SetLooping(true);

// 再生速度変更（2倍速）
cameraAnimation_->SetPlaySpeed(2.0f);
```

## JSON形式

```json
{
    "animation_name": "Camera Animation Name",
    "duration": 10.0,
    "loop": true,
    "play_speed": 1.0,
    "keyframes": [
        {
            "time": 0.0,
            "position": [x, y, z],
            "rotation": [pitch, yaw, roll],  // ラジアン
            "fov": 0.45,                     // ラジアン
            "interpolation": "LINEAR"         // または "EASE_IN", "EASE_OUT", "EASE_IN_OUT"
        }
    ]
}
```

## サンプルファイル

`resources/Json/CameraAnimations/` ディレクトリに以下のサンプルが含まれています：

- **sample_orbit.json**: オブジェクト周回カメラ（ループ）
- **intro_camera.json**: イントロシーケンス（ワンショット）

## ImGui デバッグUI

デバッグビルドでは、以下の機能が使用できます：

- 再生コントロール（Play/Pause/Stop/Reset）
- タイムラインスライダー
- キーフレームの追加/編集/削除
- 現在のカメラ位置からキーフレーム作成
- JSON保存/読み込み
- アニメーション名の編集

## 注意事項

- キーフレームは最低2つ必要です
- 回転値はラジアン単位です
- キーフレームは時間順に自動ソートされます
- JSONファイルは `resources/Json/CameraAnimations/` に保存されます