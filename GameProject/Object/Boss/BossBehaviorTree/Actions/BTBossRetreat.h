#pragma once
#include "../../../../BehaviorTree/Core/BTNode.h"
#include "../../../../BehaviorTree/Core/BTBlackboard.h"
#include "Vector3.h"

class Boss;
class Player;

/// <summary>
/// ボスの離脱アクションノード
/// プレイヤーを向いたまま後方にイージング移動で離れる
/// </summary>
class BTBossRetreat : public BTNode {
    //=========================================================================================
    // 定数
    //=========================================================================================
private:
    static constexpr float kDirectionEpsilon = 0.01f;  ///< 方向判定の閾値
    static constexpr float kArrivalThreshold = 0.5f;   ///< 到達判定の閾値
    static constexpr float kEasingCoeffA = 3.0f;       ///< イージング係数A
    static constexpr float kEasingCoeffB = 2.0f;       ///< イージング係数B

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTBossRetreat();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTBossRetreat() = default;

    /// <summary>
    /// ノードの実行
    /// </summary>
    /// <param name="blackboard">ブラックボード</param>
    /// <returns>実行結果</returns>
    BTNodeStatus Execute(BTBlackboard* blackboard) override;

    /// <summary>
    /// ノードのリセット
    /// </summary>
    void Reset() override;

    // パラメータ取得・設定
    float GetRetreatSpeed() const { return retreatSpeed_; }
    void SetRetreatSpeed(float speed) { retreatSpeed_ = speed; }
    float GetTargetDistance() const { return targetDistance_; }
    void SetTargetDistance(float distance) { targetDistance_ = distance; }

    /// <summary>
    /// JSONからパラメータを適用
    /// </summary>
    /// <param name="params">パラメータJSON</param>
    void ApplyParameters(const nlohmann::json& params) override {
        if (params.contains("retreatSpeed")) {
            retreatSpeed_ = params["retreatSpeed"];
        }
        if (params.contains("targetDistance")) {
            targetDistance_ = params["targetDistance"];
        }
    }

    /// <summary>
    /// パラメータをJSONとして抽出
    /// </summary>
    nlohmann::json ExtractParameters() const override;

#ifdef _DEBUG
    /// <summary>
    /// ImGuiでパラメータ編集UIを描画
    /// </summary>
    bool DrawImGui() override;
#endif

private:
    /// <summary>
    /// 離脱パラメータの初期化
    /// </summary>
    /// <param name="boss">ボス</param>
    /// <param name="player">プレイヤー</param>
    void InitializeRetreat(Boss* boss, Player* player);

    /// <summary>
    /// 離脱移動の更新
    /// </summary>
    /// <param name="boss">ボス</param>
    /// <param name="deltaTime">経過時間</param>
    void UpdateRetreatMovement(Boss* boss, float deltaTime);

    /// <summary>
    /// エリア内に収まる位置を計算
    /// </summary>
    /// <param name="position">調整前の位置</param>
    /// <returns>エリア内に収まる位置</returns>
    Vector3 ClampToArea(const Vector3& position);

    //=========================================================================================
    // メンバ変数
    //=========================================================================================
private:
    // パラメータ
    float retreatSpeed_ = 60.0f;       ///< 離脱速度
    float targetDistance_ = 55.0f;     ///< 目標距離（プレイヤーからの距離）

    // 状態管理
    Vector3 startPosition_;            ///< 開始位置
    Vector3 targetPosition_;           ///< 目標位置（計算済み）
    float elapsedTime_ = 0.0f;         ///< 経過時間
    float retreatDuration_ = 0.0f;     ///< 離脱所要時間（距離から動的計算）
    bool isFirstExecute_ = true;       ///< 初回実行フラグ
};
