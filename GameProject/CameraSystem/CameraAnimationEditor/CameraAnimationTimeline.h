#pragma once
#include "CameraAnimation/CameraAnimation.h"
#include "CameraAnimation/CameraKeyframe.h"
#include <vector>
#include <imgui.h>

#ifdef _DEBUG

/// <summary>
/// カメラアニメーションのタイムラインUI
/// ビジュアルなタイムライン表示とインタラクション
/// </summary>
class CameraAnimationTimeline {
public:
    /// <summary>
    /// トラックタイプ
    /// </summary>
    enum class TrackType {
        SUMMARY,        ///< サマリートラック（全キーフレーム表示）
        POSITION_X,     ///< X位置トラック
        POSITION_Y,     ///< Y位置トラック
        POSITION_Z,     ///< Z位置トラック
        ROTATION_X,     ///< X回転トラック
        ROTATION_Y,     ///< Y回転トラック
        ROTATION_Z,     ///< Z回転トラック
        FOV,            ///< FOVトラック
        COUNT
    };

    /// <summary>
    /// キーフレームの表示スタイル
    /// </summary>
    enum class KeyframeStyle {
        DIAMOND,        ///< ダイヤモンド型
        CIRCLE,         ///< 円形
        SQUARE,         ///< 四角形
        TRIANGLE        ///< 三角形
    };

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraAnimationTimeline();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraAnimationTimeline();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="animation">対象のアニメーション</param>
    void Initialize(CameraAnimation* animation);

    /// <summary>
    /// タイムラインの描画
    /// </summary>
    /// <param name="zoom">ズーム率</param>
    /// <param name="offset">タイムオフセット</param>
    void Draw(float zoom = 1.0f, float offset = 0.0f);

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// 選択中のキーフレームを取得
    /// </summary>
    const std::vector<int>& GetSelectedKeyframes() const { return selectedKeyframes_; }

    /// <summary>
    /// ホバー中のキーフレームを取得
    /// </summary>
    int GetHoveredKeyframe() const { return hoveredKeyframe_; }

    /// <summary>
    /// ドラッグ中かどうか
    /// </summary>
    bool IsDragging() const { return isDragging_; }

    /// <summary>
    /// スクラブ中かどうか
    /// </summary>
    bool IsScrubbing() const { return isScrubbing_; }

    /// <summary>
    /// キーフレームスタイルの設定
    /// </summary>
    void SetKeyframeStyle(KeyframeStyle style) { keyframeStyle_ = style; }

    /// <summary>
    /// トラックの可視性設定
    /// </summary>
    void SetTrackVisible(TrackType track, bool visible);

    /// <summary>
    /// グリッドスナップの有効化
    /// </summary>
    void SetGridSnapEnabled(bool enable) { enableGridSnap_ = enable; }

    /// <summary>
    /// グリッドスナップ間隔の設定
    /// </summary>
    void SetGridSnapInterval(float interval) { gridSnapInterval_ = interval; }

    /// <summary>
    /// プレビューモードの設定
    /// </summary>
    /// <param name="enabled">プレビューモードが有効か</param>
    void SetPreviewMode(bool enabled) {
        isPreviewModeEnabled_ = enabled;
        if (!enabled) {
            isKeyframePreviewActive_ = false;
            previewKeyframeIndex_ = -1;
        }
    }

    /// <summary>
    /// キーフレームプレビュー中かどうか
    /// </summary>
    bool IsKeyframePreviewActive() const { return isKeyframePreviewActive_; }

    /// <summary>
    /// プレビュー中のキーフレームインデックス取得
    /// </summary>
    int GetPreviewKeyframeIndex() const { return previewKeyframeIndex_; }


private:
    /// <summary>
    /// タイムルーラーの描画
    /// </summary>
    void DrawTimeRuler(float zoom, float offset);

    /// <summary>
    /// グリッドの描画
    /// </summary>
    void DrawGrid(float zoom, float offset);

    /// <summary>
    /// 再生ヘッドの描画
    /// </summary>
    void DrawPlayhead(float zoom, float offset);

    /// <summary>
    /// トラックの描画
    /// </summary>
    /// <param name="trackType">トラックタイプ</param>
    /// <param name="yPos">Y座標</param>
    /// <param name="zoom">ズーム率</param>
    /// <param name="offset">オフセット</param>
    void DrawTrack(TrackType trackType, float yPos, float zoom, float offset);

    /// <summary>
    /// キーフレームの描画
    /// </summary>
    /// <param name="index">キーフレームインデックス</param>
    /// <param name="xPos">X座標</param>
    /// <param name="yPos">Y座標</param>
    /// <param name="isSelected">選択状態</param>
    /// <param name="isHovered">ホバー状態</param>
    void DrawKeyframe(int index, float xPos, float yPos, bool isSelected, bool isHovered);

    /// <summary>
    /// 選択矩形の描画
    /// </summary>
    void DrawSelectionRect();

    /// <summary>
    /// マウス入力の処理
    /// </summary>
    void HandleMouseInput(float zoom, float offset);

    /// <summary>
    /// キーボード入力の処理
    /// </summary>
    void HandleKeyboardInput();

    /// <summary>
    /// 時間をX座標に変換
    /// </summary>
    float TimeToScreenX(float time, float zoom, float offset) const;

    /// <summary>
    /// X座標を時間に変換
    /// </summary>
    float ScreenXToTime(float x, float zoom, float offset) const;

    /// <summary>
    /// キーフレームのヒットテスト
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="trackType">トラックタイプ</param>
    /// <returns>ヒットしたキーフレームインデックス（-1でヒットなし）</returns>
    int HitTestKeyframe(float x, float y, TrackType trackType) const;

    /// <summary>
    /// 矩形選択の処理
    /// </summary>
    void ProcessRectSelection();

    /// <summary>
    /// キーフレームの移動処理
    /// </summary>
    void ProcessKeyframeDrag(float zoom, float offset);

    /// <summary>
    /// グリッドにスナップ
    /// </summary>
    float SnapToGrid(float time) const;

    /// <summary>
    /// トラック名の取得
    /// </summary>
    const char* GetTrackName(TrackType track) const;

    /// <summary>
    /// トラックの色を取得
    /// </summary>
    ImU32 GetTrackColor(TrackType track) const;

private:
    // 参照
    CameraAnimation* animation_ = nullptr;       ///< 対象アニメーション

    // UI設定
    float timelineHeight_ = 300.0f;              ///< タイムライン高さ
    float trackHeight_ = 30.0f;                  ///< トラック高さ
    float rulerHeight_ = 25.0f;                  ///< ルーラー高さ
    float trackLabelWidth_ = 100.0f;             ///< トラックラベル幅
    float keyframeSize_ = 10.0f;                 ///< キーフレームサイズ

    // スタイル設定
    KeyframeStyle keyframeStyle_ = KeyframeStyle::DIAMOND;  ///< キーフレームスタイル
    ImU32 gridColor_ = IM_COL32(60, 60, 60, 255);          ///< グリッド色
    ImU32 playheadColor_ = IM_COL32(255, 100, 100, 255);   ///< 再生ヘッド色
    ImU32 selectedColor_ = IM_COL32(255, 200, 100, 255);   ///< 選択色
    ImU32 hoveredColor_ = IM_COL32(200, 200, 255, 255);    ///< ホバー色

    // 選択状態
    std::vector<int> selectedKeyframes_;         ///< 選択中のキーフレーム
    int hoveredKeyframe_ = -1;                   ///< ホバー中のキーフレーム
    TrackType hoveredTrack_ = TrackType::SUMMARY; ///< ホバー中のトラック

    // ドラッグ状態
    bool isDragging_ = false;                    ///< ドラッグ中か
    bool isRectSelecting_ = false;               ///< 矩形選択中か
    ImVec2 dragStartPos_;                         ///< ドラッグ開始位置
    ImVec2 dragCurrentPos_;                       ///< 現在のドラッグ位置
    std::vector<float> dragStartTimes_;          ///< ドラッグ開始時の時間

    // スクラブ状態
    bool isScrubbing_ = false;                   ///< スクラブ中か
    float scrubTime_ = 0.0f;                     ///< スクラブ時間

    // プレビューモード制御
    bool isPreviewModeEnabled_ = false;          ///< プレビューモードが有効か
    bool isKeyframePreviewActive_ = false;       ///< キーフレームプレビュー中か
    float previewTime_ = 0.0f;                   ///< プレビュー時刻
    int previewKeyframeIndex_ = -1;              ///< プレビュー中のキーフレームインデックス

    // スクロール状態
    float scrollX_ = 0.0f;                       ///< 水平スクロール
    float scrollY_ = 0.0f;                       ///< 垂直スクロール

    // グリッドスナップ
    bool enableGridSnap_ = true;                 ///< グリッドスナップ有効
    float gridSnapInterval_ = 0.1f;              ///< スナップ間隔

    // トラック可視性
    bool trackVisible_[static_cast<int>(TrackType::COUNT)]; ///< トラック可視性配列

    // ズーム設定
    float minZoom_ = 0.1f;                       ///< 最小ズーム
    float maxZoom_ = 10.0f;                      ///< 最大ズーム

    // アニメーション
    float hoverAnimTime_ = 0.0f;                 ///< ホバーアニメーション時間
    float selectionAnimTime_ = 0.0f;             ///< 選択アニメーション時間
};

#endif // _DEBUG