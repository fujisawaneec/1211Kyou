#ifdef _DEBUG

#include "BossNodeInspectors.h"
#include <imgui.h>
#include "../BossBehaviorTree/Actions/BTBossDash.h"
#include "../BossBehaviorTree/Actions/BTBossShoot.h"
#include "../BossBehaviorTree/Actions/BTBossIdle.h"
#include "../BossBehaviorTree/Conditions/BTActionSelector.h"

// ========== BTBossDash ==========
bool BTBossDashInspector::DrawUI() {
    bool changed = false;

    float speed = node_->GetDashSpeed();
    if (ImGui::DragFloat("Speed##dash", &speed, 1.0f, 10.0f, 200.0f)) {
        node_->SetDashSpeed(speed);
        changed = true;
    }

    float duration = node_->GetDashDuration();
    if (ImGui::DragFloat("Duration##dash", &duration, 0.05f, 0.1f, 3.0f)) {
        node_->SetDashDuration(duration);
        changed = true;
    }

    return changed;
}

nlohmann::json BTBossDashInspector::ExtractParams() const {
    return {
        {"dashSpeed", node_->GetDashSpeed()},
        {"dashDuration", node_->GetDashDuration()}
    };
}

void BTBossDashInspector::ApplyParams(const nlohmann::json& params) {
    if (params.contains("dashSpeed")) node_->SetDashSpeed(params["dashSpeed"]);
    if (params.contains("dashDuration")) node_->SetDashDuration(params["dashDuration"]);
}

// ========== BTBossShoot ==========
bool BTBossShootInspector::DrawUI() {
    bool changed = false;

    float chargeTime = node_->GetChargeTime();
    if (ImGui::DragFloat("Charge Time##shoot", &chargeTime, 0.05f, 0.0f, 3.0f)) {
        node_->SetChargeTime(chargeTime);
        changed = true;
    }

    float recoveryTime = node_->GetRecoveryTime();
    if (ImGui::DragFloat("Recovery Time##shoot", &recoveryTime, 0.05f, 0.0f, 3.0f)) {
        node_->SetRecoveryTime(recoveryTime);
        changed = true;
    }

    float bulletSpeed = node_->GetBulletSpeed();
    if (ImGui::DragFloat("Bullet Speed##shoot", &bulletSpeed, 1.0f, 5.0f, 100.0f)) {
        node_->SetBulletSpeed(bulletSpeed);
        changed = true;
    }

    float spreadAngle = node_->GetSpreadAngle();
    if (ImGui::SliderAngle("Spread Angle##shoot", &spreadAngle, 0.0f, 45.0f)) {
        node_->SetSpreadAngle(spreadAngle);
        changed = true;
    }

    return changed;
}

nlohmann::json BTBossShootInspector::ExtractParams() const {
    return {
        {"chargeTime", node_->GetChargeTime()},
        {"recoveryTime", node_->GetRecoveryTime()},
        {"bulletSpeed", node_->GetBulletSpeed()},
        {"spreadAngle", node_->GetSpreadAngle()}
    };
}

void BTBossShootInspector::ApplyParams(const nlohmann::json& params) {
    if (params.contains("chargeTime")) node_->SetChargeTime(params["chargeTime"]);
    if (params.contains("recoveryTime")) node_->SetRecoveryTime(params["recoveryTime"]);
    if (params.contains("bulletSpeed")) node_->SetBulletSpeed(params["bulletSpeed"]);
    if (params.contains("spreadAngle")) node_->SetSpreadAngle(params["spreadAngle"]);
}

// ========== BTBossIdle ==========
bool BTBossIdleInspector::DrawUI() {
    bool changed = false;

    float duration = node_->GetIdleDuration();
    if (ImGui::DragFloat("Idle Duration##idle", &duration, 0.1f, 0.0f, 10.0f)) {
        node_->SetIdleDuration(duration);
        changed = true;
    }

    return changed;
}

nlohmann::json BTBossIdleInspector::ExtractParams() const {
    return {{"idleDuration", node_->GetIdleDuration()}};
}

void BTBossIdleInspector::ApplyParams(const nlohmann::json& params) {
    if (params.contains("idleDuration")) node_->SetIdleDuration(params["idleDuration"]);
}

// ========== BTActionSelector ==========
bool BTActionSelectorInspector::DrawUI() {
    bool changed = false;

    int actionType = static_cast<int>(node_->GetActionType());
    const char* items[] = { "Dash", "Shoot" };
    if (ImGui::Combo("Action Type##selector", &actionType, items, IM_ARRAYSIZE(items))) {
        node_->SetActionType(static_cast<BTActionSelector::ActionType>(actionType));
        changed = true;
    }

    return changed;
}

nlohmann::json BTActionSelectorInspector::ExtractParams() const {
    return {{"actionType", static_cast<int>(node_->GetActionType())}};
}

void BTActionSelectorInspector::ApplyParams(const nlohmann::json& params) {
    if (params.contains("actionType")) {
        node_->SetActionType(static_cast<BTActionSelector::ActionType>(params["actionType"].get<int>()));
    }
}

// ========== ファクトリ関数 ==========
std::unique_ptr<IBTNodeInspector> CreateNodeInspector(const BTNodePtr& node) {
    if (!node) return nullptr;

    // dynamic_castでノードタイプを判定し、対応するインスペクターを生成
    if (auto* p = dynamic_cast<BTBossDash*>(node.get()))
        return std::make_unique<BTBossDashInspector>(p);

    if (auto* p = dynamic_cast<BTBossShoot*>(node.get()))
        return std::make_unique<BTBossShootInspector>(p);

    if (auto* p = dynamic_cast<BTBossIdle*>(node.get()))
        return std::make_unique<BTBossIdleInspector>(p);

    if (auto* p = dynamic_cast<BTActionSelector*>(node.get()))
        return std::make_unique<BTActionSelectorInspector>(p);

    return nullptr;
}

#endif // _DEBUG
