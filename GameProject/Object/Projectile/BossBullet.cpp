#include "BossBullet.h"
#include "BossBulletCollider.h"
#include "../../Object/Player/Player.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "CollisionManager.h"
#include "EmitterManager.h"
#include "RandomEngine.h"

uint32_t BossBullet::id = 0;

BossBullet::BossBullet(EmitterManager* emittermanager) {
    // 弾のパラメータ設定
    damage_ = 10.0f;
    lifeTime_ = 5.0f;

    // ランダムな回転速度を設定
    RandomEngine* rng = RandomEngine::GetInstance();

    rotationSpeed_ = Vector3(
        rng->GetFloat(-10.0f, 10.0f),
        rng->GetFloat(-10.0f, 10.0f),
        rng->GetFloat(-10.0f, 10.0f)
    );

    // エミッターマネージャーの設定
    emitterManager_ = emittermanager;

    // エフェクトプリセットをロード
    if (emitterManager_) {
        bulletEmitterName_ = "boss_bullet" + std::to_string(id);
        explodeEmitterName_ = "boss_bullet_explode" + std::to_string(id);
        emitterManager_->LoadPreset("boss_bullet", bulletEmitterName_);
        emitterManager_->SetEmitterActive(bulletEmitterName_, false);
        emitterManager_->LoadPreset("boss_bullet_explode", explodeEmitterName_);
        emitterManager_->SetEmitterActive(explodeEmitterName_, false);
    }

    id++;

    if (id > 10000) {
        id = 0; // IDのリセット
    }
}

BossBullet::~BossBullet() = default;

void BossBullet::Initialize(const Vector3& position, const Vector3& velocity) {
    // 親クラスの初期化
    Projectile::Initialize(position, velocity);

    // モデルをロード
    SetModel();

    model_->Update();

    // スケールを設定（球体モデルのサイズ調整）
    transform_.scale = Vector3(0.0f, 0.0f, 0.0f);

    // 弾の色を設定（赤っぽい色）
    if (model_) {
        model_->SetMaterialColor(Vector4(1.0f, 0.3f, 0.3f, 1.0f));
        model_->SetTransform(transform_);
    }

    if (emitterManager_) {
        emitterManager_->SetEmitterActive(bulletEmitterName_, true);
        emitterManager_->SetEmitterPosition(bulletEmitterName_, position);
    }

    // コライダーの設定
    if (!collider_) {
        collider_ = std::make_unique<BossBulletCollider>(this);
    }
    collider_->SetTransform(&transform_);
    collider_->SetRadius(1.0f);  // 衝突判定の半径
    collider_->SetOffset(Vector3(0.0f, 0.0f, 0.0f));
    collider_->SetTypeID(static_cast<uint32_t>(CollisionTypeId::BOSS_ATTACK));
    collider_->SetOwner(this);
    collider_->SetActive(true);
    collider_->Reset();  // 状態をリセット

    // CollisionManagerに登録
    CollisionManager::GetInstance()->AddCollider(collider_.get());
}

void BossBullet::Finalize() {
    // CollisionManagerから削除
    if (collider_) {
        CollisionManager::GetInstance()->RemoveCollider(collider_.get());
    }

    if (emitterManager_) {
        emitterManager_->CreateTemporaryEmitterFrom(
            explodeEmitterName_,
            explodeEmitterName_ + "temp",
            0.5f);
        emitterManager_->RemoveEmitter(bulletEmitterName_);
        emitterManager_->RemoveEmitter(explodeEmitterName_);
    }
}

void BossBullet::Update(float deltaTime) {
    if (!isActive_) {
        return;
    }

    // 親クラスの更新処理
    Projectile::Update(deltaTime);

    // 回転アニメーション
    transform_.rotate += rotationSpeed_ * deltaTime;

    // モデルの更新
    if (model_) {
        model_->SetTransform(transform_);
    }

    // 軌跡エフェクト
    if (emitterManager_) {
        emitterManager_->SetEmitterPosition(bulletEmitterName_, transform_.translate);
        emitterManager_->SetEmitterPosition(explodeEmitterName_, transform_.translate);
    }

    // エリア外に出たら非アクティブ化
    Vector3 pos = transform_.translate;
    if (pos.x < Player::X_MIN || pos.x > Player::X_MAX ||
        pos.z < Player::Z_MIN || pos.z > Player::Z_MAX ||
        pos.y < -10.0f || pos.y > 50.0f) {
        isActive_ = false;
    }
}

void BossBullet::SetModel() {
    if (model_) {
        // モデルをロード
        model_->SetModel("sphere.gltf");

        if (!model_->GetModel()) {
            // sphereモデルがない場合は、代替モデルを使用
            model_->SetModel("white_cube.gltf");
        }
    }
}