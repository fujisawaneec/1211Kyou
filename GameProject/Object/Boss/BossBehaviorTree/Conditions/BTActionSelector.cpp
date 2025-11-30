#include "BTActionSelector.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

BTActionSelector::BTActionSelector(ActionType type)
    : expectedType_(type) {
    name_ = (type == ActionType::Dash) ? "ActionSelector(Dash)" : "ActionSelector(Shoot)";
}

BTNodeStatus BTActionSelector::Execute(BTBlackboard* blackboard) {
    // アクションカウンターを取得
    int actionCounter = blackboard->GetInt("ActionCounter", 0);

    // カウンターの偶奇で判定
    int currentType = actionCounter % 2;

    // 期待するタイプと一致すれば成功
    if (currentType == static_cast<int>(expectedType_)) {
        status_ = BTNodeStatus::Success;
        return BTNodeStatus::Success;
    }

    status_ = BTNodeStatus::Failure;
    return BTNodeStatus::Failure;
}

void BTActionSelector::Reset() {
    BTNode::Reset();
}

nlohmann::json BTActionSelector::ExtractParameters() const {
    return {{"actionType", static_cast<int>(expectedType_)}};
}

#ifdef _DEBUG
bool BTActionSelector::DrawImGui() {
    bool changed = false;

    int actionType = static_cast<int>(expectedType_);
    const char* items[] = { "Dash", "Shoot" };
    if (ImGui::Combo("Action Type##selector", &actionType, items, IM_ARRAYSIZE(items))) {
        expectedType_ = static_cast<ActionType>(actionType);
        changed = true;
    }

    return changed;
}
#endif