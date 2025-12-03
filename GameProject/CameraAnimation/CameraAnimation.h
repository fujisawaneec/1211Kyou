#pragma once
#include "CameraKeyframe.h"
#include "Camera.h"
#include "Quaternion.h"
#include "Transform.h"
#include "../Common/GameConst.h"
#include <vector>
#include <string>
#include <memory>

/// <summary>
/// カメラアニメーションクラス
/// キーフレーム間の補間によって滑らかなカメラ動作を実現
/// </summary>
class CameraAnimation {
    // 定数
private:
    static constexpr float kDefaultFov = 45.0f;    ///< デフォルトFOV値

public:
    /// <summary>
    /// 再生状態
    /// </summary>
    enum class PlayState {
        STOPPED,    ///< 停止中
        PLAYING,    ///< 再生中
        PAUSED      ///< 一時停止中
    };

    /// <summary>
    /// アニメーション開始モード
    /// </summary>
    enum class StartMode {
        JUMP_CUT,        ///< 即座に最初のキーフレームにジャンプ
        SMOOTH_BLEND     ///< 現在位置から最初のキーフレームまで補間
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraAnimation();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraAnimation();

    /// <summary>
    /// カメラのセット
    /// </summary>
    /// <param name="camera">アニメーションを適用するカメラ</param>
    void SetCamera(Camera* camera) { camera_ = camera; }

    /// <summary>
    /// ターゲットトランスフォームのセット
    /// </summary>
    /// <param name="target">相対座標の基準となるターゲット（nullptrで解除）</param>
    void SetTarget(const Transform* target) { targetTransform_ = target; }

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime);

    /// <summary>
    /// キーフレームの追加
    /// </summary>
    /// <param name="keyframe">追加するキーフレーム</param>
    void AddKeyframe(const CameraKeyframe& keyframe);

    /// <summary>
    /// 現在のカメラ状態からキーフレームを追加
    /// </summary>
    /// <param name="time">キーフレームの時刻</param>
    /// <param name="interpolation">補間タイプ</param>
    void AddKeyframeFromCurrentCamera(float time,
        CameraKeyframe::InterpolationType interpolation = CameraKeyframe::InterpolationType::LINEAR);

    /// <summary>
    /// キーフレームの削除
    /// </summary>
    /// <param name="index">削除するキーフレームのインデックス</param>
    void RemoveKeyframe(size_t index);

    /// <summary>
    /// キーフレームの編集
    /// </summary>
    /// <param name="index">編集するキーフレームのインデックス</param>
    /// <param name="keyframe">新しいキーフレームデータ</param>
    void EditKeyframe(size_t index, const CameraKeyframe& keyframe);

    /// <summary>
    /// すべてのキーフレームをクリア
    /// </summary>
    void ClearKeyframes();

    /// <summary>
    /// 再生開始
    /// </summary>
    void Play();

    /// <summary>
    /// 一時停止
    /// </summary>
    void Pause();

    /// <summary>
    /// 停止（時間を0にリセット）
    /// </summary>
    void Stop();

    /// <summary>
    /// FOV復元なしで停止（アニメーション切り替え時用）
    /// </summary>
    void StopWithoutRestore();

    /// <summary>
    /// 現在時刻をリセット
    /// </summary>
    void Reset();

    /// <summary>
    /// JSONファイルから読み込み
    /// </summary>
    /// <param name="filepath">JSONファイルパス</param>
    bool LoadFromJson(const std::string& filepath);

    /// <summary>
    /// JSONファイルに保存
    /// </summary>
    /// <param name="filepath">保存先ファイルパス</param>
    bool SaveToJson(const std::string& filepath) const;

#ifdef _DEBUG
    /// <summary>
    /// ImGuiでのデバッグ表示
    /// </summary>
    void DrawImGui();
#endif

    //-----------------------------------------Getter-----------------------------------------//

    /// <summary>
    /// キーフレーム数を取得
    /// </summary>
    [[nodiscard]] size_t GetKeyframeCount() const { return keyframes_.size(); }

    /// <summary>
    /// 指定インデックスのキーフレームを取得
    /// </summary>
    [[nodiscard]] const CameraKeyframe& GetKeyframe(size_t index) const { return keyframes_[index]; }

    /// <summary>
    /// アニメーションの総時間を取得
    /// </summary>
    [[nodiscard]] float GetDuration() const { return duration_; }

    /// <summary>
    /// 現在の再生時間を取得
    /// </summary>
    [[nodiscard]] float GetCurrentTime() const { return currentTime_; }

    /// <summary>
    /// 再生状態を取得
    /// </summary>
    [[nodiscard]] PlayState GetPlayState() const { return playState_; }

    /// <summary>
    /// ループ設定を取得
    /// </summary>
    [[nodiscard]] bool IsLooping() const { return isLooping_; }

    /// <summary>
    /// アニメーション名を取得
    /// </summary>
    [[nodiscard]] const std::string& GetAnimationName() const { return animationName_; }

    /// <summary>
    /// 現在キーフレームを編集中か判定
    /// </summary>
    [[nodiscard]] bool IsEditingKeyframe() const;

    /// <summary>
    /// 選択中のキーフレームインデックスを取得
    /// </summary>
    [[nodiscard]] int GetSelectedKeyframeIndex() const;

    /// <summary>
    /// ターゲットトランスフォームを取得
    /// </summary>
    [[nodiscard]] const Transform* GetTarget() const { return targetTransform_; }

    /// <summary>
    /// 開始モードを取得
    /// </summary>
    [[nodiscard]] StartMode GetStartMode() const { return startMode_; }

    /// <summary>
    /// ブレンド時間を取得
    /// </summary>
    [[nodiscard]] float GetBlendDuration() const { return blendDuration_; }

    /// <summary>
    /// ブレンド中かを判定
    /// </summary>
    [[nodiscard]] bool IsBlending() const { return isBlending_; }

    /// <summary>
    /// ブレンド進行度を取得（0.0～1.0）
    /// </summary>
    [[nodiscard]] float GetBlendProgress() const { return blendProgress_; }

    //-----------------------------------------Setter-----------------------------------------//

    /// <summary>
    /// ループ設定
    /// </summary>
    void SetLooping(bool loop) { isLooping_ = loop; }

    /// <summary>
    /// 再生速度の設定
    /// </summary>
    void SetPlaySpeed(float speed) { playSpeed_ = speed; }

    /// <summary>
    /// アニメーション名の設定
    /// </summary>
    void SetAnimationName(const std::string& name) { animationName_ = name; }

    /// <summary>
    /// 現在時刻の設定（シーク）
    /// </summary>
    void SetCurrentTime(float time);

    /// <summary>
    /// 指定したキーフレームをカメラに適用
    /// </summary>
    /// <param name="index">適用するキーフレームのインデックス（省略時は選択中のキーフレーム）</param>
    void ApplyKeyframeToCamera(int index = -1);

    /// <summary>
    /// 開始モードの設定
    /// </summary>
    /// <param name="mode">開始モード</param>
    void SetStartMode(StartMode mode) { startMode_ = mode; }

    /// <summary>
    /// ブレンド時間の設定
    /// </summary>
    /// <param name="duration">ブレンド時間（秒）</param>
    void SetBlendDuration(float duration) { blendDuration_ = duration; }

private:
    /// <summary>
    /// キーフレームを時間でソート
    /// </summary>
    void SortKeyframes();

    /// <summary>
    /// アニメーションの総時間を更新
    /// </summary>
    void UpdateDuration();

    /// <summary>
    /// 現在時刻に対応する2つのキーフレームを検索
    /// </summary>
    /// <param name="time">検索時刻</param>
    /// <param name="prevIndex">前のキーフレームインデックス（出力）</param>
    /// <param name="nextIndex">次のキーフレームインデックス（出力）</param>
    /// <returns>キーフレームが見つかったか</returns>
    bool FindKeyframeIndices(float time, size_t& prevIndex, size_t& nextIndex) const;

    /// <summary>
    /// キーフレーム間の補間
    /// </summary>
    /// <param name="prev">前のキーフレーム</param>
    /// <param name="next">次のキーフレーム</param>
    /// <param name="t">補間係数（0.0～1.0）</param>
    void InterpolateKeyframes(const CameraKeyframe& prev, const CameraKeyframe& next, float t);

    /// <summary>
    /// イージング関数の適用
    /// </summary>
    /// <param name="t">元の補間係数</param>
    /// <param name="type">補間タイプ</param>
    /// <returns>イージング適用後の補間係数</returns>
    float ApplyEasing(float t, CameraKeyframe::InterpolationType type) const;

    /// <summary>
    /// オイラー角をクォータニオンに変換
    /// </summary>
    /// <param name="euler">オイラー角（ラジアン）</param>
    /// <returns>クォータニオン</returns>
    Quaternion EulerToQuaternion(const Vector3& euler) const;

    /// <summary>
    /// クォータニオンをオイラー角に変換
    /// </summary>
    /// <param name="q">クォータニオン</param>
    /// <returns>オイラー角（ラジアン）</returns>
    Vector3 QuaternionToEuler(const Quaternion& q) const;

    /// <summary>
    /// 選択解除時の処理（カメラを元の値に戻す）
    /// </summary>
    void ClearDeselectState();

    /// <summary>
    /// キーフレームをカメラに直接適用（内部用）
    /// </summary>
    void ApplyKeyframeDirectly(const CameraKeyframe& kf);

private:
    std::string animationName_ = "Untitled";  ///< アニメーション名

    std::vector<CameraKeyframe> keyframes_;  ///< キーフレーム配列

    Camera* camera_ = nullptr;  ///< アニメーション対象のカメラ

    const Transform* targetTransform_ = nullptr;  ///< ターゲットトランスフォーム（相対座標の基準）

    float currentTime_ = 0.0f;  ///< 現在の再生時間（秒）

    float duration_ = 0.0f;  ///< アニメーションの総時間（秒）

    float playSpeed_ = 1.0f;  ///< 再生速度（1.0が標準）

    PlayState playState_ = PlayState::STOPPED;  ///< 再生状態

    bool isLooping_ = false;  ///< ループ再生フラグ

    StartMode startMode_ = StartMode::JUMP_CUT;  ///< 開始モード
    float blendDuration_ = 0.5f;                 ///< ブレンド時間（秒）
    float blendProgress_ = 0.0f;                 ///< ブレンド進行度（0.0～1.0）
    bool isBlending_ = false;                    ///< ブレンド中フラグ

    // ブレンド開始時のカメラ状態
    Vector3 blendStartPosition_;  ///< ブレンド開始時の位置
    Vector3 blendStartRotation_;  ///< ブレンド開始時の回転
    float blendStartFov_;         ///< ブレンド開始時のFOV

    // FOV復元用
    float originalFov_;           ///< アニメーション開始前の元のFOV値
    bool hasOriginalFov_;         ///< 元のFOVが保存されているかのフラグ

#ifdef _DEBUG
    int selectedKeyframeIndex_ = -1;  ///< ImGui用：選択中のキーフレームインデックス
    bool showTimeline_ = true;  ///< ImGui用：タイムライン表示フラグ
    bool autoSortKeyframes_ = true;  ///< ImGui用：キーフレーム自動ソートフラグ
    CameraKeyframe tempKeyframe_;  ///< 編集用の一時キーフレーム
#endif
};