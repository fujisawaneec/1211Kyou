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

    // ドラッグ状態
    bool isDragging_ = false;                    ///< ドラッグ中か
    std::vector<float> dragStartTimes_;          ///< ドラッグ開始時の各キーフレーム時間

    // タイムライン設定
    float gridSnapInterval_ = 0.1f;              ///< グリッドスナップ間隔
    bool enableGridSnap_ = true;                 ///< グリッドスナップ有効化

    // コピーバッファ
    std::vector<CameraKeyframe> clipboard_;      ///< コピーしたキーフレーム

    // プレビュー機能
    bool enablePreview_ = false;                  ///< プレビューモード有効化
    std::string previousControllerName_;         ///< プレビュー前のコントローラー名
};

#endif // _DEBUG