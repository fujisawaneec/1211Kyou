#pragma once
#include "ICameraController.h"
#include "CameraAnimation/CameraAnimation.h"
#include <memory>
#include <string>

/// <summary>
/// カメラアニメーションコントローラー
/// アニメーション再生を優先度システムで管理
/// </summary>
class CameraAnimationController : public ICameraController {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraAnimationController();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraAnimationController() override = default;

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// アクティブ状態を判定
    /// アニメーション再生中のみアクティブ
    /// </summary>
    /// <returns>アクティブな場合true</returns>
    bool IsActive() const override;

    /// <summary>
    /// 優先度を取得
    /// アニメーションは最高優先度
    /// </summary>
    /// <returns>コントローラーの優先度</returns>
    CameraControlPriority GetPriority() const override {
        return CameraControlPriority::ANIMATION;
    }

    /// <summary>
    /// アクティブ化（アニメーション再生開始）
    /// </summary>
    void Activate() override;

    /// <summary>
    /// 非アクティブ化（アニメーション停止）
    /// </summary>
    void Deactivate() override;

    /// <summary>
    /// カメラを設定
    /// </summary>
    /// <param name="camera">制御対象のカメラ</param>
    void SetCamera(Camera* camera) override;

    /// <summary>
    /// アニメーションターゲットを設定
    /// </summary>
    /// <param name="target">相対座標の基準となるターゲット（nullptrで解除）</param>
    /// <param name="applyToAll">全アニメーションに適用する場合true（デフォルトはfalse）</param>
    void SetAnimationTarget(const Transform* target, bool applyToAll = false);

    /// <summary>
    /// 特定のアニメーションにターゲットを設定
    /// </summary>
    /// <param name="animationName">アニメーション名</param>
    /// <param name="target">相対座標の基準となるターゲット（nullptrで解除）</param>
    void SetAnimationTargetByName(const std::string& animationName, const Transform* target);

    /// <summary>
    /// 現在のアニメーションのみにターゲットを設定
    /// </summary>
    /// <param name="target">相対座標の基準となるターゲット（nullptrで解除）</param>
    void SetCurrentAnimationTarget(const Transform* target);

    //==================== アニメーション制御 ====================

    /// <summary>
    /// アニメーションを読み込み
    /// </summary>
    /// <param name="name">JSONファイルパス</param>
    /// <returns>読み込み成功した場合true</returns>
    bool LoadAnimation(const std::string& name);

    /// <summary>
    /// アニメーション再生
    /// </summary>
    void Play();

    /// <summary>
    /// アニメーション一時停止
    /// </summary>
    void Pause();

    /// <summary>
    /// アニメーション停止
    /// </summary>
    void Stop();

    /// <summary>
    /// アニメーションリセット
    /// </summary>
    void Reset();

    /// <summary>
    /// アニメーション開始モードを設定
    /// </summary>
    /// <param name="mode">開始モード</param>
    /// <param name="blendDuration">ブレンド時間（秒）</param>
    void SetAnimationStartMode(CameraAnimation::StartMode mode, float blendDuration = 0.5f);

    /// <summary>
    /// 特定のアニメーションの開始モードを設定
    /// </summary>
    /// <param name="animationName">アニメーション名</param>
    /// <param name="mode">開始モード</param>
    /// <param name="blendDuration">ブレンド時間（秒）</param>
    void SetAnimationStartModeByName(const std::string& animationName,
                                      CameraAnimation::StartMode mode, float blendDuration = 0.5f);

    //==================== キーフレーム管理 ====================

    /// <summary>
    /// キーフレームを追加
    /// </summary>
    /// <param name="keyframe">追加するキーフレーム</param>
    void AddKeyframe(const CameraKeyframe& keyframe);

    /// <summary>
    /// 現在のカメラ状態からキーフレームを追加
    /// </summary>
    /// <param name="time">キーフレームの時刻</param>
    /// <param name="interpolation">補間タイプ</param>
    void AddKeyframeFromCurrentCamera(float time,
        CameraKeyframe::InterpolationType interpolation =
            CameraKeyframe::InterpolationType::LINEAR);

    /// <summary>
    /// キーフレームを削除
    /// </summary>
    /// <param name="index">削除するキーフレームのインデックス</param>
    void RemoveKeyframe(size_t index);

    /// <summary>
    /// すべてのキーフレームをクリア
    /// </summary>
    void ClearKeyframes();

    //==================== Setter ====================

    /// <summary>
    /// ループ設定
    /// </summary>
    /// <param name="loop">ループする場合true</param>
    void SetLooping(bool loop);

    /// <summary>
    /// 再生速度設定
    /// </summary>
    /// <param name="speed">再生速度（1.0が標準）</param>
    void SetPlaySpeed(float speed);

    /// <summary>
    /// アニメーション名設定
    /// </summary>
    /// <param name="name">アニメーション名</param>
    void SetAnimationName(const std::string& name);

    //==================== Getter ====================

    //==================== アニメーション管理 ====================

    /// <summary>
    /// 新規アニメーションを作成
    /// </summary>
    /// <param name="name">アニメーション名</param>
    /// <returns>作成成功した場合true</returns>
    bool CreateAnimation(const std::string& name);

    /// <summary>
    /// アニメーションを切り替え
    /// </summary>
    /// <param name="name">切り替え先のアニメーション名</param>
    /// <returns>切り替え成功した場合true</returns>
    bool SwitchAnimation(const std::string& name);

    /// <summary>
    /// アニメーションを削除
    /// </summary>
    /// <param name="name">削除するアニメーション名</param>
    /// <returns>削除成功した場合true</returns>
    bool DeleteAnimation(const std::string& name);

    /// <summary>
    /// アニメーションをリネーム
    /// </summary>
    /// <param name="oldName">現在の名前</param>
    /// <param name="newName">新しい名前</param>
    /// <returns>リネーム成功した場合true</returns>
    bool RenameAnimation(const std::string& oldName, const std::string& newName);

    /// <summary>
    /// アニメーションを複製
    /// </summary>
    /// <param name="sourceName">複製元のアニメーション名</param>
    /// <param name="newName">複製先の名前</param>
    /// <returns>複製成功した場合true</returns>
    bool DuplicateAnimation(const std::string& sourceName, const std::string& newName);

    /// <summary>
    /// アニメーションをファイルから読み込み
    /// </summary>
    /// <param name="filepath">JSONファイルパス</param>
    /// <param name="name">アニメーション名</param>
    /// <returns>読み込み成功した場合true</returns>
    bool LoadAnimationFromFile(const std::string& name);

    /// <summary>
    /// アニメーションをファイルに保存
    /// </summary>
    /// <param name="name">保存するアニメーション名</param>
    /// <param name="filepath">保存先ファイルパス</param>
    /// <returns>保存成功した場合true</returns>
    bool SaveAnimationToFile(const std::string& name);

    /// <summary>
    /// 現在のアニメーションオブジェクトを取得
    /// </summary>
    /// <returns>現在のアニメーションオブジェクト</returns>
    CameraAnimation* GetCurrentAnimation();

    /// <summary>
    /// 指定した名前のアニメーションを取得
    /// </summary>
    /// <param name="name">アニメーション名</param>
    /// <returns>アニメーションオブジェクト（存在しない場合nullptr）</returns>
    CameraAnimation* GetAnimation(const std::string& name);

    /// <summary>
    /// アニメーション名のリストを取得
    /// </summary>
    /// <returns>全アニメーション名のリスト</returns>
    std::vector<std::string> GetAnimationList() const;

    /// <summary>
    /// アニメーション数を取得
    /// </summary>
    /// <returns>登録されているアニメーション数</returns>
    size_t GetAnimationCount() const { return animations_.size(); }

    /// <summary>
    /// 現在のアニメーション名を取得
    /// </summary>
    /// <returns>現在のアニメーション名</returns>
    const std::string& GetCurrentAnimationName() const { return currentAnimationName_; }

    /// <summary>
    /// 再生状態を取得
    /// </summary>
    /// <returns>現在の再生状態</returns>
    CameraAnimation::PlayState GetPlayState() const;

    /// <summary>
    /// アニメーションの総時間を取得
    /// </summary>
    /// <returns>総時間（秒）</returns>
    float GetDuration() const;

    /// <summary>
    /// 現在の再生時間を取得
    /// </summary>
    /// <returns>再生時間（秒）</returns>
    float GetCurrentTime() const;

    /// <summary>
    /// キーフレーム編集中かを判定
    /// </summary>
    /// <returns>編集中の場合true</returns>
    bool IsEditingKeyframe() const;

    /// <summary>
    /// アニメーションターゲットを取得
    /// </summary>
    /// <returns>現在のターゲット（設定されていない場合nullptr）</returns>
    const Transform* GetAnimationTarget() const;

private:
    // カメラアニメーションオブジェクト（複数管理）
    std::map<std::string, std::unique_ptr<CameraAnimation>> animations_;

    // 現在アクティブなアニメーション名
    std::string currentAnimationName_ = "Default";

    // 再生完了時に自動で非アクティブ化するかのフラグ
    bool autoDeactivateOnComplete_ = true;
};