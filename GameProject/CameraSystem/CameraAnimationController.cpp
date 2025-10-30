#include "CameraAnimationController.h"

CameraAnimationController::CameraAnimationController() {
    // デフォルトアニメーションを作成
    animations_["Default"] = std::make_unique<CameraAnimation>();
    animations_["Default"]->SetAnimationName("Default");
    currentAnimationName_ = "Default";
}

void CameraAnimationController::Update(float deltaTime) {
    auto* animation = GetCurrentAnimation();
    if (!animation || !camera_) {
        return;
    }

    // アニメーション更新
    animation->Update(deltaTime);

    // 再生完了時の自動非アクティブ化
    if (autoDeactivateOnComplete_) {
        auto state = animation->GetPlayState();
        if (state == CameraAnimation::PlayState::STOPPED &&
            !animation->IsLooping()) {
            // ワンショット再生が完了したら自動的に非アクティブ化
            isActive_ = false;
        }
    }
}

bool CameraAnimationController::IsActive() const {
    // isActive_がtrueなら、アニメーションの有無に関わらずアクティブとする
    // これによりプレビューモード時に確実にアクティブになる
    if (isActive_) {
        return true;
    }

    auto* animation = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (!animation) {
        return false;
    }

    // アニメーション再生中または編集中の場合もアクティブ
    return animation->GetPlayState() == CameraAnimation::PlayState::PLAYING ||
           animation->IsEditingKeyframe();
}

void CameraAnimationController::Activate() {
    isActive_ = true;
    //auto* animation = GetCurrentAnimation();
    //if (animation) {
    //    animation->Play();
    //}
}

void CameraAnimationController::Deactivate() {
    isActive_ = false;
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->Stop();
    }
}

void CameraAnimationController::SetCamera(Camera* camera) {
    ICameraController::SetCamera(camera);

    // 全てのアニメーションにカメラを設定
    for (auto& pair : animations_) {
        pair.second->SetCamera(camera);
    }
}

void CameraAnimationController::SetAnimationTarget(const Transform* target, bool applyToAll) {
    if (applyToAll) {
        // 全てのアニメーションにターゲットを設定
        for (auto& pair : animations_) {
            pair.second->SetTarget(target);
        }
    } else {
        // 現在のアニメーションのみにターゲットを設定
        auto* animation = GetCurrentAnimation();
        if (animation) {
            animation->SetTarget(target);
        }
    }
}

void CameraAnimationController::SetAnimationTargetByName(const std::string& animationName, const Transform* target) {
    auto it = animations_.find(animationName);
    if (it != animations_.end()) {
        it->second->SetTarget(target);
    }
}

void CameraAnimationController::SetCurrentAnimationTarget(const Transform* target) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->SetTarget(target);
    }
}

bool CameraAnimationController::LoadAnimation(const std::string& name) {
    // デフォルトアニメーションに読み込む（後方互換性のため）
    auto* animation = GetCurrentAnimation();
    if (!animation) {
        return false;
    }
    return animation->LoadFromJson(name);
}

void CameraAnimationController::Play() {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->Play();
        isActive_ = true;
    }
}

void CameraAnimationController::Pause() {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->Pause();
    }
}

void CameraAnimationController::Stop() {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->Stop();
        isActive_ = false;
    }
}

void CameraAnimationController::Reset() {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->Reset();
    }
}

void CameraAnimationController::SetAnimationStartMode(CameraAnimation::StartMode mode, float blendDuration) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->SetStartMode(mode);
        animation->SetBlendDuration(blendDuration);
    }
}

void CameraAnimationController::SetAnimationStartModeByName(const std::string& animationName,
                                                           CameraAnimation::StartMode mode, float blendDuration) {
    auto it = animations_.find(animationName);
    if (it != animations_.end()) {
        it->second->SetStartMode(mode);
        it->second->SetBlendDuration(blendDuration);
    }
}

void CameraAnimationController::AddKeyframe(const CameraKeyframe& keyframe) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->AddKeyframe(keyframe);
    }
}

void CameraAnimationController::AddKeyframeFromCurrentCamera(float time,
    CameraKeyframe::InterpolationType interpolation) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->AddKeyframeFromCurrentCamera(time, interpolation);
    }
}

void CameraAnimationController::RemoveKeyframe(size_t index) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->RemoveKeyframe(index);
    }
}

void CameraAnimationController::ClearKeyframes() {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->ClearKeyframes();
    }
}

void CameraAnimationController::SetLooping(bool loop) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->SetLooping(loop);
    }
}

void CameraAnimationController::SetPlaySpeed(float speed) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->SetPlaySpeed(speed);
    }
}

void CameraAnimationController::SetAnimationName(const std::string& name) {
    auto* animation = GetCurrentAnimation();
    if (animation) {
        animation->SetAnimationName(name);
    }
}

CameraAnimation::PlayState CameraAnimationController::GetPlayState() const {
    auto* animation = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (animation) {
        return animation->GetPlayState();
    }
    return CameraAnimation::PlayState::STOPPED;
}

float CameraAnimationController::GetDuration() const {
    auto* animation = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (animation) {
        return animation->GetDuration();
    }
    return 0.0f;
}

float CameraAnimationController::GetCurrentTime() const {
    auto* animation = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (animation) {
        return animation->GetCurrentTime();
    }
    return 0.0f;
}

bool CameraAnimationController::IsEditingKeyframe() const {
    auto* anim = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (anim) {
        return anim->IsEditingKeyframe();
    }
    return false;
}

const Transform* CameraAnimationController::GetAnimationTarget() const {
    auto* anim = const_cast<CameraAnimationController*>(this)->GetCurrentAnimation();
    if (anim) {
        return anim->GetTarget();
    }
    return nullptr;
}

//==================== アニメーション管理の実装 ====================

CameraAnimation* CameraAnimationController::GetCurrentAnimation() {
    auto it = animations_.find(currentAnimationName_);
    if (it != animations_.end()) {
        return it->second.get();
    }
    return nullptr;
}

CameraAnimation* CameraAnimationController::GetAnimation(const std::string& name) {
    auto it = animations_.find(name);
    if (it != animations_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool CameraAnimationController::CreateAnimation(const std::string& name) {
    // 既に存在する場合は失敗
    if (animations_.find(name) != animations_.end()) {
        return false;
    }

    // 新規作成
    animations_[name] = std::make_unique<CameraAnimation>();
    animations_[name]->SetAnimationName(name);

    // カメラを設定
    if (camera_) {
        animations_[name]->SetCamera(camera_);
    }

    return true;
}

bool CameraAnimationController::SwitchAnimation(const std::string& name) {
    // 存在チェック
    if (animations_.find(name) == animations_.end()) {
        return false;
    }

    // 現在のアニメーションを停止（FOV復元なし）
    auto* current = GetCurrentAnimation();
    if (current) {
        current->StopWithoutRestore();
    }

    // 切り替え
    currentAnimationName_ = name;

    // カメラを再設定
    auto* newAnim = GetCurrentAnimation();
    if (newAnim && camera_) {
        newAnim->SetCamera(camera_);
    }

    return true;
}

bool CameraAnimationController::DeleteAnimation(const std::string& name) {
    // デフォルトアニメーションは削除不可
    if (name == "Default") {
        return false;
    }

    // 存在チェック
    auto it = animations_.find(name);
    if (it == animations_.end()) {
        return false;
    }

    // 現在アクティブなアニメーションを削除しようとしている場合
    if (name == currentAnimationName_) {
        // Defaultに切り替え
        currentAnimationName_ = "Default";
    }

    // 削除
    animations_.erase(it);
    return true;
}

bool CameraAnimationController::RenameAnimation(const std::string& oldName, const std::string& newName) {
    // Defaultはリネーム不可
    if (oldName == "Default") {
        return false;
    }

    // 存在チェック
    auto it = animations_.find(oldName);
    if (it == animations_.end()) {
        return false;
    }

    // 新しい名前が既に存在する場合は失敗
    if (animations_.find(newName) != animations_.end()) {
        return false;
    }

    // 移動
    auto animation = std::move(it->second);
    animation->SetAnimationName(newName);
    animations_.erase(it);
    animations_[newName] = std::move(animation);

    // 現在アクティブなアニメーションの場合は名前を更新
    if (oldName == currentAnimationName_) {
        currentAnimationName_ = newName;
    }

    return true;
}

bool CameraAnimationController::DuplicateAnimation(const std::string& sourceName, const std::string& newName) {
    // ソース存在チェック
    auto* source = GetAnimation(sourceName);
    if (!source) {
        return false;
    }

    // 新しい名前が既に存在する場合は失敗
    if (animations_.find(newName) != animations_.end()) {
        return false;
    }

    // 新規作成
    animations_[newName] = std::make_unique<CameraAnimation>();
    animations_[newName]->SetAnimationName(newName);

    // カメラを設定
    if (camera_) {
        animations_[newName]->SetCamera(camera_);
    }

    // キーフレームをコピー
    for (size_t i = 0; i < source->GetKeyframeCount(); ++i) {
        animations_[newName]->AddKeyframe(source->GetKeyframe(i));
    }

    // 設定をコピー
    animations_[newName]->SetLooping(source->IsLooping());

    return true;
}

bool CameraAnimationController::LoadAnimationFromFile(const std::string& name) {
    // 新規アニメーション作成
    if (!CreateAnimation(name)) {
        // 既に存在する場合は上書き確認が必要だが、ここでは単純に失敗とする
        return false;
    }

    // JSONから読み込み（filepathを正しく渡す）
    auto* anim = GetAnimation(name);
    if (!anim || !anim->LoadFromJson(name)) {
        // 失敗した場合は削除
        DeleteAnimation(name);
        return false;
    }

    return true;
}

bool CameraAnimationController::SaveAnimationToFile(const std::string& name) {
    auto* anim = GetAnimation(name);
    if (!anim) {
        return false;
    }

    return anim->SaveToJson(name);
}

std::vector<std::string> CameraAnimationController::GetAnimationList() const {
    std::vector<std::string> names;
    names.reserve(animations_.size());

    for (const auto& pair : animations_) {
        names.push_back(pair.first);
    }

    return names;
}