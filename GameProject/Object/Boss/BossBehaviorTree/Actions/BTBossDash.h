#pragma once
#include "../../../../BehaviorTree/Core/BTNode.h"
#include "../../../../BehaviorTree/Core/BTBlackboard.h"
#include "Vector3.h"

class Boss;

/// <summary>
/// ボスのダッシュアクションノード
/// </summary>
class BTBossDash : public BTNode {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTBossDash();

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BTBossDash() = default;

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
    float GetDashSpeed() const { return dashSpeed_; }
    void SetDashSpeed(float speed) { dashSpeed_ = speed; }
    float GetDashDuration() const { return dashDuration_; }
    void SetDashDuration(float duration) { dashDuration_ = duration; }

    /// <summary>
    /// JSONからパラメータを適用
    /// </summary>
    /// <param name="params">パラメータJSON</param>
    void ApplyParameters(const nlohmann::json& params) override {
        if (params.contains("dashSpeed")) {
            dashSpeed_ = params["dashSpeed"];
        }
        if (params.contains("dashDuration")) {
            dashDuration_ = params["dashDuration"];
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
    /// ダッシュパラメータの初期化
    /// </summary>
    /// <param name="boss">ボス</param>
    void InitializeDash(Boss* boss);

    /// <summary>
    /// ダッシュ移動の更新
    /// </summary>
    /// <param name="boss">ボス</param>
    /// <param name="deltaTime">経過時間</param>
    void UpdateDashMovement(Boss* boss, float deltaTime);

    /// <summary>
    /// エリア内に収まる位置を計算
    /// </summary>
    /// <param name="position">調整前の位置</param>
    /// <returns>エリア内に収まる位置</returns>
    Vector3 ClampToArea(const Vector3& position);

    // ダッシュ方向
    Vector3 dashDirection_;

    // ダッシュ速度
    float dashSpeed_ = 60.0f;

    // ダッシュ時間
    float dashDuration_ = 0.5f;

    // ダッシュ開始位置
    Vector3 startPosition_;

    // ダッシュ目標位置
    Vector3 targetPosition_;

    // 経過時間
    float elapsedTime_ = 0.0f;

    // 初回実行フラグ
    bool isFirstExecute_ = true;
};