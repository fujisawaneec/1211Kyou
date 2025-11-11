#include "InputHandler.h"
#include "Object/Player/Player.h"
#include "Input.h"
#include "Vector2.h"

InputHandler::InputHandler()
{
}

InputHandler::~InputHandler()
{
}

void InputHandler::Initialize()
{
  isMoving_ = false;
  isDashing_ = false;
  isAttacking_ = false;
  isShooting_ = false;
  isParrying_ = false;
  isPaused_ = false;
  moveDirection_ = Vector2(0.0f, 0.0f);
}

void InputHandler::Update()
{
  Input* input = Input::GetInstance();

  moveDirection_ = Vector2(0.0f, 0.0f); // 初期化

  // 移動入力
  if (input->IsConnect()) moveDirection_ += input->GetLeftStick();

  moveDirection_ += {static_cast<float>(input->PushKey(DIK_D) - input->PushKey(DIK_A)), static_cast<float>(input->PushKey(DIK_W) - input->PushKey(DIK_S)) };

  // 各アクションの入力状態を更新
  isMoving_ = !input->LStickInDeadZone() || moveDirection_.Length() > 0.0f;
  isDashing_ = input->TriggerKey(DIK_SPACE) || input->TriggerButton(XButtons.A);
  isAttacking_ = input->TriggerKey(DIK_Z) || input->TriggerButton(XButtons.X);
  isShooting_ = input->PushKey(DIK_LCONTROL) || input->GetRightTrigger() > 0.5f;
  isParrying_ = input->TriggerKey(DIK_F) || input->TriggerButton(XButtons.B);
  isPaused_ = input->TriggerKey(DIK_ESCAPE) || input->TriggerButton(XButtons.Start);
}

void InputHandler::ResetInputs()
{
    moveDirection_ = Vector2(0.0f, 0.0f);
    isMoving_ = false;
    isDashing_ = false;
    isAttacking_ = false;
    isShooting_ = false;
    isParrying_ = false;
    isPaused_ = false;
}

bool InputHandler::IsMoving() const
{
  return isMoving_;
}

bool InputHandler::IsDashing() const
{
  return isDashing_;
}

bool InputHandler::IsAttacking() const
{
  return isAttacking_;
}

bool InputHandler::IsShooting() const
{
  return isShooting_;
}

bool InputHandler::IsParrying() const
{
  return isParrying_;
}

bool InputHandler::IsPaused() const
{
  return isPaused_;
}

Vector2 InputHandler::GetMoveDirection() const
{
  return moveDirection_;
}