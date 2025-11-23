#include "BTSequence.h"

BTSequence::BTSequence() {
    name_ = "Sequence";
}

BTNodeStatus BTSequence::Execute(BTBlackboard* blackboard) {
    if (children_.empty()) {
        return BTNodeStatus::Success;
    }

    // 前回Runningだった場合、その子ノードから続行
    for (size_t i = currentChildIndex_; i < children_.size(); ++i) {
        BTNodeStatus childStatus = children_[i]->Execute(blackboard);

        if (childStatus == BTNodeStatus::Failure) {
            // 失敗したら即座に失敗を返す
            currentChildIndex_ = 0;
            status_ = BTNodeStatus::Failure;
            isRunning_ = false;
            return status_;
        }
        else if (childStatus == BTNodeStatus::Running) {
            // 実行中なら現在のインデックスを記憶
            currentChildIndex_ = i;
            status_ = BTNodeStatus::Running;
            isRunning_ = true;
            return status_;
        }
        // Successの場合は次の子ノードへ
    }

    // 全ての子ノードが成功
    currentChildIndex_ = 0;
    status_ = BTNodeStatus::Success;
    isRunning_ = false;
    return status_;
}