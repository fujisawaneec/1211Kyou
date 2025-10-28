#pragma once
#include "CameraAnimation/CameraAnimation.h"
#include "CameraAnimation/CameraKeyframe.h"
#include "Camera.h"
#include <memory>
#include <vector>

#ifdef _DEBUG

// Forward declarations
class CameraAnimationTimeline;
class CameraAnimationCurveEditor;
class CameraAnimationHistory;

/// <summary>
/// 高度なカメラアニメーションエディター
/// プロフェッショナルなアニメーション編集機能を提供
/// </summary>
class CameraAnimationEditor {
public:
    /// <summary>
    /// エディターのレイアウトモード
    /// </summary>
    enum class LayoutMode {
        COMPACT,        ///< コンパクトモード（基本機能のみ）
        STANDARD,       ///< 標準モード（タイムライン＋インスペクター）
        ADVANCED,       ///< 詳細モード（全機能表示）
        CUSTOM          ///< カスタムレイアウト
    };

    /// <summary>
    /// プレビューモード
    /// </summary>
    enum class PreviewMode {
        NONE,           ///< プレビューなし
        CURRENT_FRAME,  ///< 現在フレームのみ
        MOTION_PATH,    ///< モーションパス表示
        GHOST_FRAMES    ///< ゴースト（オニオンスキン）表示
    };

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraAnimationEditor();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraAnimationEditor();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="animation">編集対象のアニメーション</param>
    /// <param name="camera">カメラ</param>
    void Initialize(CameraAnimation* animation, Camera* camera);

    /// <summary>
    /// 初期化（CameraAnimationController使用）
    /// </summary>
    /// <param name="controller">アニメーションコントローラー</param>
    /// <param name="camera">カメラ</param>
    void Initialize(class CameraAnimationController* controller, Camera* camera);

    /// <summary>
    /// エディターUIの描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// キーボードショートカット処理
    /// </summary>
    void ProcessShortcuts();

    /// <summary>
    /// エディターを開く
    /// </summary>
    void Open() { isOpen_ = true; }

    /// <summary>
    /// エディターを閉じる
    /// </summary>
    void Close() { isOpen_ = false; }

    /// <summary>
    /// エディターが開いているか
    /// </summary>
    bool IsOpen() const { return isOpen_; }

    /// <summary>
    /// レイアウトモードの設定
    /// </summary>
    void SetLayoutMode(LayoutMode mode) { layoutMode_ = mode; }


private:
    /// <summary>
    /// メインメニューバーの描画
    /// </summary>
    void DrawMenuBar();

    /// <summary>
    /// アニメーション選択UIの描画
    /// </summary>
    void DrawAnimationSelector();

    /// <summary>
    /// タイムラインパネルの描画
    /// </summary>
    void DrawTimelinePanel();

    /// <summary>
    /// インスペクターパネルの描画
    /// </summary>
    void DrawInspectorPanel();

    /// <summary>
    /// カーブエディターパネルの描画
    /// </summary>
    void DrawCurveEditorPanel();

    /// <summary>
    /// プレビューパネルの描画
    /// </summary>
    void DrawPreviewPanel();

    /// <summary>
    /// ステータスバーの描画
    /// </summary>
    void DrawStatusBar();

    /// <summary>
    /// 再生コントロールの描画
    /// </summary>
    void DrawPlaybackControls();

    /// <summary>
    /// キーフレームリストの描画
    /// </summary>
    void DrawKeyframeList();

    /// <summary>
    /// モーションパスの描画（3D空間）
    /// </summary>
    void DrawMotionPath();

    /// <summary>
    /// グリッドスナップの処理
    /// </summary>
    /// <param name="time">入力時間</param>
    /// <returns>スナップ後の時間</returns>
    float SnapToGrid(float time) const;

    /// <summary>
    /// キーフレームの選択処理
    /// </summary>
    void HandleKeyframeSelection();

    /// <summary>
    /// ドラッグ＆ドロップ処理
    /// </summary>
    void HandleDragAndDrop();

    /// <summary>
    /// コピー処理
    /// </summary>
    void CopySelectedKeyframes();

    /// <summary>
    /// ペースト処理
    /// </summary>
    void PasteKeyframes();

    /// <summary>
    /// 削除処理
    /// </summary>
    void DeleteSelectedKeyframes();

    /// <summary>
    /// アンドゥ処理
    /// </summary>
    void Undo();

    /// <summary>
    /// リドゥ処理
    /// </summary>
    void Redo();

private:
    // エディター状態
    bool isOpen_ = false;                        ///< エディターが開いているか
    LayoutMode layoutMode_ = LayoutMode::STANDARD; ///< レイアウトモード
    PreviewMode previewMode_ = PreviewMode::MOTION_PATH; ///< プレビューモード

    // 編集対象
    CameraAnimation* animation_ = nullptr;       ///< 編集中のアニメーション
    Camera* camera_ = nullptr;                   ///< カメラ
    class CameraAnimationController* controller_ = nullptr; ///< アニメーションコントローラー（複数管理用）

    // UIコンポーネント
    std::unique_ptr<CameraAnimationTimeline> timeline_;     ///< タイムラインコンポーネント
    std::unique_ptr<CameraAnimationCurveEditor> curveEditor_; ///< カーブエディター
    std::unique_ptr<CameraAnimationHistory> history_;        ///< 編集履歴

    // 選択状態
    std::vector<int> selectedKeyframes_;         ///< 選択中のキーフレームインデックス
    int hoveredKeyframe_ = -1;                   ///< ホバー中のキーフレーム
    bool isMultiSelecting_ = false;              ///< 複数選択中か

    // ドラッグ状態
    bool isDragging_ = false;                    ///< ドラッグ中か
    float dragStartTime_ = 0.0f;                 ///< ドラッグ開始時の時間
    std::vector<float> dragStartTimes_;          ///< ドラッグ開始時の各キーフレーム時間

    // タイムライン設定
    float timelineZoom_ = 1.0f;                  ///< タイムラインのズーム率
    float timelineOffset_ = 0.0f;                ///< タイムラインのオフセット
    float gridSnapInterval_ = 0.1f;              ///< グリッドスナップ間隔
    bool enableGridSnap_ = true;                 ///< グリッドスナップ有効化

    // プレビュー設定
    bool showMotionPath_ = true;                 ///< モーションパス表示
    bool showGhostFrames_ = false;               ///< ゴーストフレーム表示
    int ghostFrameCount_ = 3;                    ///< ゴーストフレーム数
    float ghostFrameOpacity_ = 0.3f;             ///< ゴーストフレーム透明度

    // コピーバッファ
    std::vector<CameraKeyframe> clipboard_;      ///< コピーしたキーフレーム

    // 再生状態
    bool isScrubbing_ = false;                   ///< スクラブ再生中か
    float scrubTime_ = 0.0f;                     ///< スクラブ時間

    // UIレイアウト設定
    float timelinePanelHeight_ = 200.0f;         ///< タイムラインパネル高さ
    float inspectorPanelWidth_ = 300.0f;         ///< インスペクターパネル幅
    float curveEditorHeight_ = 200.0f;           ///< カーブエディター高さ
    float previewPanelSize_ = 200.0f;            ///< プレビューパネルサイズ

    // 設定フラグ
    bool autoKeyframe_ = false;                  ///< 自動キーフレーム挿入
    bool showTimeRuler_ = true;                  ///< 時間ルーラー表示
    bool showPropertyTracks_ = true;             ///< プロパティトラック表示
    bool lockTimelineScroll_ = false;            ///< タイムラインスクロールロック
};

#endif // _DEBUG