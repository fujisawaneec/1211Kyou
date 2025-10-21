#pragma once

#include "Input.h"
#include "Mat4x4Func.h"
#include "Vec3Func.h"
#include "Transform.h"
#include "Camera.h"

#include <math.h>
#include <memory>
#include <DirectXMath.h>

class CameraAnimation;

struct TopDownSettings {
  float baseHeight = 10.0f;
  float heightMultiplier = 0.6f;
  float minHeight = 26.0f;
  float maxHeight = 500.0f;
  float angleX = DirectX::XMConvertToRadians(25.0f);
  float baseBackOffset = -10.0f;
  float backOffsetMultiplier = 1.5f;
  float minBackOffset = -500.0f;
  float maxBackOffset = -52.0f;
};

class FollowCamera {

public: // メンバ関数
  FollowCamera();
  ~FollowCamera();

  void Initialize(Camera* camera);

  void Finalize();

  void Update();

  void FirstPersonMode();
  void TopDownMode();

  void Reset();

  void ResetOffset();

  void DrawImGui();

  // offsetの計算関数
  Vector3 CalculateOffset() const;

  // 画面揺れ
  void ShakeScreen(float power);

  // ゲーム開始時のCameraアニメーション再生
  void PlayStartCameraAnimation();

  //-----------------------------Setters------------------------------//
  void SetTarget(const Transform* target);
  void SetTarget2(const Transform* target2);
  void SetOffset(const Vector3& offset)
  {
    offset_.x = offset.x;
    offset_.y = offset.y;
    offsetOrigin_.z = offset.z;
  }
  void SetRotateSpeed(const float speed) { rotateSpeed_ = speed; }

  /// <summary>
  /// Set the camera mode.
  /// </summary>
  /// <param name="mode">true: FirstPersonMode, false: TopDownMode</param>
  void SetMode(bool mode) { mode_ = mode; } // true: FirstPersonMode, false: TopDownMode

  //-----------------------------Getters------------------------------//
  Camera* GetCamera() { return camera_; }
  const Matrix4x4& GetViewProjection() const { return camera_->GetViewProjectionMatrix(); }
  const Vector3& GetOffset() const { return offset_; }
  const Transform* GetTarget2() const { return target2_; }
  bool GetMode() const { return mode_; } // true: FirstPersonMode, false: TopDownMode

private: // メンバ変数
  Camera* camera_;

  std::unique_ptr<CameraAnimation> cameraAnimation_;

  const Transform* target_ = nullptr;
  const Transform* target2_ = nullptr;

  Input* input_ = nullptr;

  bool mode_ = false; // true: FirstPersonMode, false: TopDownMode

  Vector3 interTargetPos_;

  Vector3 offset_ = { 0.0f, 2.0f, -40.0f };

  Vector3 offsetOrigin_ = { 0.0f, 2.0f, -40.0f };

  float t_ = 0.18f;

  float destinationAngleY_ = 0.0f;
  float destinationAngleX_ = 8.0f;
  float destinationAngleZ_ = 0.0f;

  bool isRotating_ = false;

  float rotateSpeed_ = 0.05f;

  // TopDownMode専用パラメータ
  TopDownSettings topDownSettings_;

};