#pragma once
#include <DirectXMath.h>

/// <summary>
/// カメラシステムの設定値を管理
/// マジックナンバーを撲滅し、意味を明確化
/// </summary>
namespace CameraConfig {

    //==================== 共通設定 ====================

    /// <summary>
    /// カメラ追従の滑らかさ（0.0-1.0、値が大きいほど即座に追従）
    /// </summary>
    constexpr float FOLLOW_SMOOTHNESS = 0.18f;

    /// <summary>
    /// オフセット補間速度（0.0-1.0）
    /// </summary>
    constexpr float OFFSET_LERP_SPEED = 0.08f;

    /// <summary>
    /// 回転補間速度（0.0-1.0）
    /// </summary>
    constexpr float ROTATION_LERP_SPEED = 0.15f;

    /// <summary>
    /// 標準FOV（ラジアン）
    /// </summary>
    constexpr float STANDARD_FOV = 0.44999998807907104;

    /// <summary>
    /// ゲームシーンまだ消えないパーティクルが別のシーンでも表示されるのを防ぐため、
    /// カメラを画面外に移動させる際のY座標
    /// シーン遷移時などで使用
    /// </summary>
    constexpr float HIDDEN_Y = -1000.0f;

    //==================== FirstPerson設定 ====================

    namespace FirstPerson {
        /// <summary>
        /// デフォルトカメラオフセット
        /// </summary>
        constexpr float DEFAULT_OFFSET_X = 0.0f;
        constexpr float DEFAULT_OFFSET_Y = 2.0f;
        constexpr float DEFAULT_OFFSET_Z = -40.0f;

        /// <summary>
        /// デフォルトX軸角度（ラジアン）
        /// </summary>
        constexpr float DEFAULT_ANGLE_X = DirectX::XMConvertToRadians(8.0f);

        /// <summary>
        /// 回転感度
        /// </summary>
        constexpr float DEFAULT_ROTATE_SPEED = 0.05f;

        /// <summary>
        /// ゲームパッド回転感度倍率
        /// </summary>
        constexpr float GAMEPAD_ROTATE_MULTIPLIER = 1.0f;
    }

    //==================== TopDown設定 ====================

    namespace TopDown {
        /// <summary>
        /// 基準カメラ高さ
        /// </summary>
        constexpr float BASE_HEIGHT = 10.0f;

        /// <summary>
        /// ターゲット間距離に対する高さ倍率
        /// </summary>
        constexpr float HEIGHT_MULTIPLIER = 0.6f;

        /// <summary>
        /// カメラ高さの制限
        /// </summary>
        constexpr float MIN_HEIGHT = 26.0f;
        constexpr float MAX_HEIGHT = 500.0f;

        /// <summary>
        /// デフォルトカメラ角度（ラジアン）
        /// </summary>
        constexpr float DEFAULT_ANGLE_X = DirectX::XMConvertToRadians(25.0f);

        /// <summary>
        /// 後方オフセット設定
        /// </summary>
        constexpr float BASE_BACK_OFFSET = -10.0f;
        constexpr float BACK_OFFSET_MULTIPLIER = 1.5f;
        constexpr float MIN_BACK_OFFSET = -500.0f;
        constexpr float MAX_BACK_OFFSET = -52.0f;
    }

    //==================== アニメーション設定 ====================

    namespace Animation {
        /// <summary>
        /// デフォルト再生速度
        /// </summary>
        constexpr float DEFAULT_PLAY_SPEED = 1.0f;

        /// <summary>
        /// キーフレーム予約数
        /// </summary>
        constexpr size_t KEYFRAME_RESERVE_COUNT = 32;

        /// <summary>
        /// デフォルトFOV（ラジアン）
        /// </summary>
        constexpr float DEFAULT_FOV = 0.45f;
    }
}