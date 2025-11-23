#include "BossBehaviorTree.h"
#include "../../../BehaviorTree/Composites/BTSelector.h"
#include "../../../BehaviorTree/Composites/BTSequence.h"
#include "Actions/BTBossIdle.h"
#include "Actions/BTBossDash.h"
#include "Actions/BTBossShoot.h"
#include "Conditions/BTActionSelector.h"
#include "../Boss.h"
#include "../../Player/Player.h"

BossBehaviorTree::BossBehaviorTree(Boss* boss, Player* player) {
    // ブラックボードの初期化
    blackboard_ = std::make_unique<BTBlackboard>();
    blackboard_->SetBoss(boss);
    blackboard_->SetPlayer(player);
    blackboard_->SetInt("ActionCounter", 0);

    // ツリーの構築
    BuildTree();
}

BossBehaviorTree::~BossBehaviorTree() = default;

void BossBehaviorTree::Update(float deltaTime) {
    if (!rootNode_) {
        return;
    }

    // ブラックボードに経過時間を設定
    blackboard_->SetDeltaTime(deltaTime);

    // ルートノードを実行
    BTNodeStatus status = rootNode_->Execute(blackboard_.get());

    // 完了したらリセット
    if (status != BTNodeStatus::Running) {
        rootNode_->Reset();
    }
}

void BossBehaviorTree::Reset() {
    if (rootNode_) {
        rootNode_->Reset();
    }
    blackboard_->SetInt("ActionCounter", 0);
}

void BossBehaviorTree::SetPlayer(Player* player) {
    blackboard_->SetPlayer(player);
}

const std::string& BossBehaviorTree::GetCurrentNodeName() const {
    return currentNodeName_;
}

void BossBehaviorTree::BuildTree() {
    // ルートノードは行動ツリー
    rootNode_ = BuildActionTree();
}

BTNodePtr BossBehaviorTree::BuildActionTree() {
    // ルートシーケンス（Idle → Action を繰り返す）
    auto rootSequence = std::make_shared<BTSequence>();
    rootSequence->SetName("MainLoop");

    // 1. Idle（待機）
    auto idleNode = std::make_shared<BTBossIdle>();
    rootSequence->AddChild(idleNode);

    // 2. Action選択（DashかShootか）
    auto actionSelector = std::make_shared<BTSelector>();
    actionSelector->SetName("ActionSelector");

    // 2-1. Dashブランチ（偶数カウンター）
    auto dashSequence = std::make_shared<BTSequence>();
    dashSequence->SetName("DashSequence");

    auto dashCondition = std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Dash);
    auto dashAction = std::make_shared<BTBossDash>();

    dashSequence->AddChild(dashCondition);
    dashSequence->AddChild(dashAction);

    // 2-2. Shootブランチ（奇数カウンター）
    auto shootSequence = std::make_shared<BTSequence>();
    shootSequence->SetName("ShootSequence");

    auto shootCondition = std::make_shared<BTActionSelector>(BTActionSelector::ActionType::Shoot);
    auto shootAction = std::make_shared<BTBossShoot>();

    shootSequence->AddChild(shootCondition);
    shootSequence->AddChild(shootAction);

    // アクション選択にブランチを追加
    actionSelector->AddChild(dashSequence);
    actionSelector->AddChild(shootSequence);

    // ルートシーケンスにアクション選択を追加
    rootSequence->AddChild(actionSelector);

    return rootSequence;
}

/// <summary>
/// ルートノードを外部から設定
/// </summary>
void BossBehaviorTree::SetRootNode(BTNodePtr rootNode) {
    if (rootNode) {
        rootNode_ = rootNode;
        // 既存のツリーをリセット
        Reset();
        currentNodeName_ = "External Tree";
    }
}