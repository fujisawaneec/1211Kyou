#pragma once
#include "CameraAnimation/CameraAnimation.h"
#include "CameraAnimation/CameraKeyframe.h"
#include <vector>
#include <imgui.h>

#ifdef _DEBUG

/// <summary>
/// カメラアニメーションのカーブエディター
/// 補間カーブの視覚的編集機能を提供
/// </summary>
class CameraAnimationCurveEditor {
public:
    /// <summary>
    /// カーブタイプ
    /// </summary>
    enum class CurveType {
        POSITION_X,     ///< X位置カーブ
        POSITION_Y,     ///< Y位置カーブ
        POSITION_Z,     ///< Z位置カーブ
        ROTATION_X,     ///< X回転カーブ
        ROTATION_Y,     ///< Y回転カーブ
        ROTATION_Z,     ///< Z回転カーブ
        FOV,            ///< FOVカーブ
        COUNT
    };

    /// <summary>
    /// ハンドルタイプ
    /// </summary>
    enum class HandleType {
        NONE,           ///< なし
        LEFT,           ///< 左タンジェント
        RIGHT,          ///< 右タンジェント
        BOTH            ///< 両方
    };

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraAnimationCurveEditor();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraAnimationCurveEditor();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="animation">対象のアニメーション</param>
    void Initialize(CameraAnimation* animation);

    /// <summary>
    /// カーブエディターの描画
    /// </summary>
    /// <param name="selectedKeyframes">選択中のキーフレーム</param>
    void Draw(const std::vector<int>& selectedKeyframes);

    /// <summary>
    /// アクティブカーブの設定
    /// </summary>
    void SetActiveCurve(CurveType type) { activeCurve_ = type; }

    /// <summary>
    /// カーブの可視性設定
    /// </summary>
    void SetCurveVisible(CurveType type, bool visible);

    /// <summary>
    /// グリッドスナップの有効化
    /// </summary>
    void SetGridSnapEnabled(bool enable) { enableGridSnap_ = enable; }

private:
    /// <summary>
    /// グラフエリアの描画
    /// </summary>
    void DrawGraphArea();

    /// <summary>
    /// グリッドの描画
    /// </summary>
    void DrawGrid();

    /// <summary>
    /// 軸の描画
    /// </summary>
    void DrawAxes();

    /// <summary>
    /// カーブの描画
    /// </summary>
    /// <param name="curveType">カーブタイプ</param>
    void DrawCurve(CurveType curveType);

    /// <summary>
    /// キーポイントの描画
    /// </summary>
    /// <param name="index">キーフレームインデックス</param>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="isSelected">選択状態</param>
    void DrawKeyPoint(int index, float x, float y, bool isSelected);

    /// <summary>
    /// タンジェントハンドルの描画
    /// </summary>
    /// <param name="centerX">中心X座標</param>
    /// <param name="centerY">中心Y座標</param>
    /// <param name="handleX">ハンドルX座標</param>
    /// <param name="handleY">ハンドルY座標</param>
    /// <param name="isLeft">左ハンドルか</param>
    void DrawTangentHandle(float centerX, float centerY, float handleX, float handleY, bool isLeft);

    /// <summary>
    /// マウス入力の処理
    /// </summary>
    void HandleMouseInput();

    /// <summary>
    /// カーブプロパティの表示
    /// </summary>
    void DrawCurveProperties();

    /// <summary>
    /// プリセットイージングボタン
    /// </summary>
    void DrawEasingPresets();

    /// <summary>
    /// 値をグラフ座標に変換
    /// </summary>
    ImVec2 ValueToGraph(float time, float value) const;

    /// <summary>
    /// グラフ座標を値に変換
    /// </summary>
    void GraphToValue(const ImVec2& pos, float& time, float& value) const;

    /// <summary>
    /// カーブ値の取得
    /// </summary>
    float GetCurveValue(const CameraKeyframe& kf, CurveType type) const;

    /// <summary>
    /// カーブ値の設定
    /// </summary>
    void SetCurveValue(CameraKeyframe& kf, CurveType type, float value);

    /// <summary>
    /// ベジェ補間の計算
    /// </summary>
    float CalculateBezier(float t, float p0, float p1, float p2, float p3) const;

    /// <summary>
    /// イージング関数の適用
    /// </summary>
    float ApplyEasing(float t, CameraKeyframe::InterpolationType type) const;

    /// <summary>
    /// グリッドにスナップ
    /// </summary>
    void SnapToGrid(float& time, float& value) const;

private:
    // 参照
    CameraAnimation* animation_ = nullptr;       ///< 対象アニメーション

    // グラフ設定
    ImVec2 graphPos_;                           ///< グラフ位置
    ImVec2 graphSize_ = ImVec2(600, 300);       ///< グラフサイズ
    float timeRange_ = 10.0f;                   ///< 時間範囲
    float valueRangeMin_ = -10.0f;              ///< 値の最小範囲
    float valueRangeMax_ = 10.0f;               ///< 値の最大範囲
    float zoomX_ = 1.0f;                         ///< X軸ズーム
    float zoomY_ = 1.0f;                         ///< Y軸ズーム
    float panX_ = 0.0f;                          ///< X軸パン
    float panY_ = 0.0f;                          ///< Y軸パン

    // カーブ設定
    CurveType activeCurve_ = CurveType::POSITION_X;  ///< アクティブカーブ
    bool curveVisible_[static_cast<int>(CurveType::COUNT)]; ///< カーブ可視性
    ImU32 curveColors_[static_cast<int>(CurveType::COUNT)]; ///< カーブ色

    // 選択状態
    int selectedKeyPoint_ = -1;                  ///< 選択中のキーポイント
    int selectedEasingIndex_ = 0;                ///< 選択中のイージングプリセットインデックス
    HandleType selectedHandle_ = HandleType::NONE; ///< 選択中のハンドル
    bool isDragging_ = false;                    ///< ドラッグ中か
    ImVec2 dragStartPos_;                         ///< ドラッグ開始位置
    float dragStartTime_;                         ///< ドラッグ開始時の時間
    float dragStartValue_;                        ///< ドラッグ開始時の値

    // グリッドスナップ
    bool enableGridSnap_ = false;                ///< グリッドスナップ有効
    float gridSnapIntervalX_ = 0.1f;             ///< X軸スナップ間隔
    float gridSnapIntervalY_ = 1.0f;             ///< Y軸スナップ間隔

    // 表示設定
    bool showGrid_ = true;                       ///< グリッド表示
    bool showAxes_ = true;                       ///< 軸表示
    bool showTangents_ = true;                   ///< タンジェント表示
    bool showValues_ = true;                     ///< 値表示
    int curveResolution_ = 50;                   ///< カーブ解像度

    // タンジェント設定（将来的にベジェカーブ実装用）
    struct TangentData {
        float leftLength = 0.3f;                 ///< 左タンジェント長さ
        float leftAngle = 0.0f;                  ///< 左タンジェント角度
        float rightLength = 0.3f;                ///< 右タンジェント長さ
        float rightAngle = 0.0f;                 ///< 右タンジェント角度
        bool broken = false;                     ///< 分離タンジェント
    };
    std::vector<TangentData> tangents_;          ///< タンジェントデータ
};

#endif // _DEBUG