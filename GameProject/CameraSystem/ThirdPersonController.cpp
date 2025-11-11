#include "ThirdPersonController.h"
#include "Vec3Func.h"
#include "Mat4x4Func.h"
#include <cmath>
#include <DirectXMath.h>

ThirdPersonController::ThirdPersonController() {
  input_ = Input::GetInstance();
}

void ThirdPersonController::Update(float deltaTime) {
  if (!isActive_ || !camera_ || !primaryTarget_) {
    return;
  }

  // 標準FOVを設定
  if (camera_) {
    camera_->SetFovY(standardFov_);
  }

  ProcessInput(deltaTime);
  UpdateRotation();
  UpdatePosition();
}

void ThirdPersonController::Activate() {
  isActive_ = true;

  // 標準FOVを設定
  if (camera_) {
    camera_->SetFovY(standardFov_);
  }

  if (primaryTarget_) {
    Reset();
  }
}

void ThirdPersonController::Reset() {
  if (!primaryTarget_ || !camera_) {
    return;
  }

  // ターゲット位置に即座に移動
  interpolatedTargetPos_ = primaryTarget_->translate;

  // カメラをターゲットの向きに合わせる（回転はラジアン単位）
  camera_->SetRotate(Vector3(0.0f, primaryTarget_->rotate.y, 0.0f));
  destinationAngleY_ = primaryTarget_->rotate.y;
  // CameraConfig::FirstPerson::DEFAULT_ANGLE_Xはすでにラジアン単位
  destinationAngleX_ = CameraConfig::FirstPerson::DEFAULT_ANGLE_X;
  destinationAngleZ_ = 0.0f;

  // オフセットをリセット
  offset_ = offsetOrigin_;

  // カメラ位置を更新
  Vector3 offset = CalculateOffset();
  camera_->SetTranslate(interpolatedTargetPos_ + offset);
}

void ThirdPersonController::ProcessInput(float deltaTime) {
  // ターゲット注視モードの場合は手動回転を無効化
  if (enableLookAtTarget_ && secondaryTarget_) {
    isRotating_ = false;
    return;
  }

  // ゲームパッド入力
  if (!input_->RStickInDeadZone()) {
    isRotating_ = true;
    float rotateX = input_->GetRightStick().x;
    // rotateSpeed_はラジアン/フレーム単位（約0.00087ラジアン = 約0.05度/フレーム）
    destinationAngleY_ += rotateX * rotateSpeed_ *
      CameraConfig::FirstPerson::GAMEPAD_ROTATE_MULTIPLIER;
  } else {
    isRotating_ = false;
  }

  // 右スティック押し込みでターゲットの後ろにリセット
  if (input_->TriggerButton(XButtons.R_Thumbstick)) {
    destinationAngleY_ = primaryTarget_->rotate.y;
  }

  // キーボード入力（ラジアン単位でカメラ回転）
  if (input_->PushKey(DIK_LEFT)) {
    destinationAngleY_ -= rotateSpeed_;
  }
  if (input_->PushKey(DIK_RIGHT)) {
    destinationAngleY_ += rotateSpeed_;
  }
}

void ThirdPersonController::UpdateRotation() {
  // 現在の回転角度を取得
  Vector3 currentRotation = camera_->GetRotate();

  // ターゲット注視モードの場合
  if (enableLookAtTarget_ && secondaryTarget_) {
    // ボスへの注視回転を計算
    Vector3 lookAtRotation = CalculateLookAtRotation();

    // 注視回転を目標角度として設定
    destinationAngleX_ = lookAtRotation.x;
    destinationAngleY_ = lookAtRotation.y;
    destinationAngleZ_ = lookAtRotation.z;
  }

  // 目標角度に向けて補間
  float angleY = Vec3::LerpShortAngle(currentRotation.y, destinationAngleY_, rotationLerpSpeed_);
  float angleX = Vec3::LerpShortAngle(currentRotation.x, destinationAngleX_, rotationLerpSpeed_);
  float angleZ = Vec3::LerpShortAngle(currentRotation.z, destinationAngleZ_, rotationLerpSpeed_);

  // カメラに適用
  camera_->SetRotate(Vector3(angleX, angleY, angleZ));
}

void ThirdPersonController::UpdatePosition() {
  // オフセットを全軸で補間（より滑らかなカメラ動作のため）
  offset_.x = Vec3::Lerp(offset_.x, offsetOrigin_.x, offsetLerpSpeed_);
  offset_.y = Vec3::Lerp(offset_.y, offsetOrigin_.y, offsetLerpSpeed_);
  offset_.z = Vec3::Lerp(offset_.z, offsetOrigin_.z, offsetLerpSpeed_);

  // ターゲット位置に補間して追従
  interpolatedTargetPos_ = Vec3::Lerp(interpolatedTargetPos_,
    primaryTarget_->translate,
    followSmoothness_);

  // カメラ位置を更新
  Vector3 offset = CalculateOffset();
  camera_->SetTranslate(interpolatedTargetPos_ + offset);
}

Vector3 ThirdPersonController::CalculateOffset() const {
  Vector3 offset = offset_;

  // カメラの回転行列を生成
  Matrix4x4 rotationMatrix = Mat4x4::MakeRotateXYZ(camera_->GetRotate());

  // オフセットを回転変換
  offset = Mat4x4::TransformNormal(rotationMatrix, offset);

  return offset;
}

Vector3 ThirdPersonController::CalculateLookAtRotation() const {
  // セカンダリターゲットが無効な場合は現在の回転を返す
  if (!secondaryTarget_ || !primaryTarget_) {
    return camera_->GetRotate();
  }

  // プレイヤーからボスへの方向ベクトルを計算
  Vector3 direction = secondaryTarget_->translate - primaryTarget_->translate;

  // 水平面での角度を計算（Y軸回転、ラジアン）
  float angleY = std::atan2(direction.x, direction.z);

  // 垂直方向の角度を計算（X軸回転、ラジアン）
  float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
  float angleX = -std::atan2(direction.y, horizontalDistance);

  // 三人称視点用の見下ろし角度を追加（15度をラジアンで）
  const float THIRD_PERSON_LOOK_DOWN_ANGLE = DirectX::XMConvertToRadians(15.0f);
  angleX += THIRD_PERSON_LOOK_DOWN_ANGLE;

  // ラジアン単位のまま返す（UpdateRotationもラジアンで処理）
  return Vector3(angleX, angleY, 0.0f);
}