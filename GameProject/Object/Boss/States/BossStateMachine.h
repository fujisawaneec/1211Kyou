#pragma once

#include <memory>
#include <unordered_map>
#include <string>

class Boss;
class BossStateBase;

/// <summary>
/// ボスのステートマシン
/// </summary>
class BossStateMachine {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BossStateMachine();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossStateMachine();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    void Initialize(Boss* boss);

    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// 状態の追加
    /// </summary>
    /// <param name="name">状態名</param>
    /// <param name="state">状態オブジェクト</param>
    void AddState(const std::string& name, std::unique_ptr<BossStateBase> state);

    /// <summary>
    /// 状態の変更
    /// </summary>
    /// <param name="stateName">遷移先の状態名</param>
    void ChangeState(const std::string& stateName);

    /// <summary>
    /// 現在の状態名を取得
    /// </summary>
    /// <returns>現在の状態名</returns>
    const std::string& GetCurrentStateName() const { return currentStateName_; }

    /// <summary>
    /// 現在の状態を取得
    /// </summary>
    /// <returns>現在の状態</returns>
    BossStateBase* GetCurrentState() const { return currentState_; }

private:
    /// <summary>
    /// ボスのポインタ
    /// </summary>
    Boss* boss_ = nullptr;

    /// <summary>
    /// 状態のマップ
    /// </summary>
    std::unordered_map<std::string, std::unique_ptr<BossStateBase>> states_;

    /// <summary>
    /// 現在の状態
    /// </summary>
    BossStateBase* currentState_ = nullptr;

    /// <summary>
    /// 現在の状態名
    /// </summary>
    std::string currentStateName_;
};