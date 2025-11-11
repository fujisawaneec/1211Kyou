#include "BossStateMachine.h"
#include "BossStateBase.h"
#include "../Boss.h"
#include <cassert>

BossStateMachine::BossStateMachine() {
}

BossStateMachine::~BossStateMachine() {
    // 現在の状態を終了
    if (currentState_ && boss_) {
        currentState_->Exit(boss_);
    }
}

void BossStateMachine::Initialize(Boss* boss) {
    assert(boss != nullptr);
    boss_ = boss;
}

void BossStateMachine::Update(float deltaTime) {
    if (!boss_ || !currentState_) {
        return;
    }

    // 現在の状態を更新
    currentState_->Update(boss_, deltaTime);
}

void BossStateMachine::AddState(const std::string& name, std::unique_ptr<BossStateBase> state) {
    assert(state != nullptr);
    states_[name] = std::move(state);
}

void BossStateMachine::ChangeState(const std::string& stateName) {
    // 状態が存在するか確認
    auto it = states_.find(stateName);
    if (it == states_.end()) {
        assert(false && "指定された状態が存在しません");
        return;
    }

    // 現在と同じ状態への遷移は無視
    if (currentStateName_ == stateName) {
        return;
    }

    // 現在の状態を終了
    if (currentState_ && boss_) {
        currentState_->Exit(boss_);
    }

    // 新しい状態に変更
    currentStateName_ = stateName;
    currentState_ = it->second.get();

    // 新しい状態を開始
    if (currentState_ && boss_) {
        currentState_->Enter(boss_);
    }
}