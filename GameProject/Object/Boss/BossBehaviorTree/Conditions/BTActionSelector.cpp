#include "BTActionSelector.h"

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
        return BTNodeStatus::Success;
    }

    return BTNodeStatus::Failure;
}

void BTActionSelector::Reset() {
    BTNode::Reset();
}