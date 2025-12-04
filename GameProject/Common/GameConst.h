#pragma once

/// <summary>
/// ゲーム全体で使用する定数定義
/// 変更頻度が極めて低い固定値のみを定義
/// 調整が必要なパラメータはGlobalVariablesを使用すること
/// </summary>
namespace GameConst {

    /// <summary>
    /// ステージのの範囲
    /// </summary>
    inline constexpr float kStageXMin = -100.0f;
    inline constexpr float kStageXMax = 100.0f;
    inline constexpr float kStageZMin = -140.0f;
    inline constexpr float kStageZMax = 60.0f;

    /// <summary>
    /// 方向ベクトルの有効判定閾値
    /// これより小さい長さのベクトルは無効とみなす
    /// </summary>
    constexpr float kDirectionEpsilon = 0.01f;

}
