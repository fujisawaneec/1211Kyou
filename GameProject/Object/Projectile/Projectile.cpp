#include "Projectile.h"
#include "Object3d.h"
#include "Model.h"

Projectile::Projectile() {
}

Projectile::~Projectile() {
}

void Projectile::Initialize(const Vector3& position, const Vector3& velocity) {
    // 位置と速度を設定
    transform_.translate = position;
    velocity_ = velocity;

    // モデルの初期化
    if (!model_) {
        model_ = std::make_unique<Object3d>();
        model_->Initialize();
    }

    // モデルにTransformを設定
    model_->SetTransform(transform_);

    // タイマーリセット
    elapsedTime_ = 0.0f;

    // アクティブ化
    isActive_ = true;
}

void Projectile::Update(float deltaTime) {
    if (!isActive_) {
        return;
    }

    // 生存時間を更新
    UpdateLifetime(deltaTime);

    // 移動処理
    Move(deltaTime);

    // モデルの更新
    if (model_) {
        model_->SetTransform(transform_);
        model_->Update();
    }
}

void Projectile::Draw() {
    if (!isActive_ || !model_) {
        return;
    }

    // モデルを描画
    model_->Draw();
}

void Projectile::UpdateLifetime(float deltaTime) {
    elapsedTime_ += deltaTime;

    // 生存時間を超えたら非アクティブ化
    if (elapsedTime_ >= lifeTime_) {
        isActive_ = false;
    }
}

void Projectile::Move(float deltaTime) {
    // 速度に基づいて移動
    transform_.translate += velocity_ * deltaTime;
}