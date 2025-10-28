#pragma once
#include "ICameraController.h"
#include "Camera.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

/// <summary>
/// カメラシステム統合管理クラス
/// 優先度ベースの権限管理とコントローラー調停を担当
/// </summary>
class CameraManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得
    /// </summary>
    /// <returns>CameraManagerのインスタンス</returns>
    static CameraManager* GetInstance();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="camera">管理対象のカメラ</param>
    void Initialize(Camera* camera);

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// 更新処理
    /// 最高優先度のアクティブなコントローラーのみを実行
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime);

    //==================== コントローラー管理 ====================

    /// <summary>
    /// コントローラーを登録
    /// </summary>
    /// <param name="name">コントローラー識別名</param>
    /// <param name="controller">コントローラーインスタンス</param>
    void RegisterController(const std::string& name,
                           std::unique_ptr<ICameraController> controller);

    /// <summary>
    /// コントローラーを取得
    /// </summary>
    /// <param name="name">コントローラー識別名</param>
    /// <returns>コントローラーのポインタ（存在しない場合nullptr）</returns>
    ICameraController* GetController(const std::string& name);

    /// <summary>
    /// コントローラーを削除
    /// </summary>
    /// <param name="name">コントローラー識別名</param>
    /// <returns>削除成功した場合true</returns>
    bool RemoveController(const std::string& name);

    /// <summary>
    /// コントローラーをアクティブ化
    /// </summary>
    /// <param name="name">コントローラー識別名</param>
    /// <returns>アクティブ化成功した場合true</returns>
    bool ActivateController(const std::string& name);

    /// <summary>
    /// コントローラーを非アクティブ化
    /// </summary>
    /// <param name="name">コントローラー識別名</param>
    /// <returns>非アクティブ化成功した場合true</returns>
    bool DeactivateController(const std::string& name);

    /// <summary>
    /// 全コントローラーを非アクティブ化
    /// </summary>
    void DeactivateAllControllers();

    //==================== 状態取得 ====================

    /// <summary>
    /// 現在アクティブな最高優先度コントローラーを取得
    /// </summary>
    /// <returns>アクティブなコントローラー（存在しない場合nullptr）</returns>
    ICameraController* GetActiveController() const;

    /// <summary>
    /// 現在アクティブな最高優先度コントローラーの名前を取得
    /// </summary>
    /// <returns>コントローラー名（存在しない場合空文字列）</returns>
    std::string GetActiveControllerName() const;

    /// <summary>
    /// 登録されているコントローラー数を取得
    /// </summary>
    /// <returns>コントローラー数</returns>
    size_t GetControllerCount() const { return controllers_.size(); }

    /// <summary>
    /// カメラを取得
    /// </summary>
    /// <returns>管理対象のカメラ</returns>
    Camera* GetCamera() const { return camera_; }

    //==================== デバッグ ====================

    /// <summary>
    /// デバッグ情報を取得
    /// </summary>
    /// <returns>デバッグ情報文字列</returns>
    std::string GetDebugInfo() const;

private:
    /// <summary>
    /// コンストラクタ（シングルトン）
    /// </summary>
    CameraManager() = default;

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraManager() = default;

    // コピー・ムーブ禁止
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    CameraManager& operator=(CameraManager&&) = delete;

    /// <summary>
    /// コントローラーを優先度順にソート
    /// </summary>
    void SortControllersByPriority();

    /// <summary>
    /// 最高優先度のアクティブなコントローラーを検索
    /// </summary>
    /// <returns>見つかったコントローラーのインデックス（-1 = 見つからない）</returns>
    int FindHighestPriorityActiveController() const;

private:
    /// <summary>
    /// コントローラー管理エントリ
    /// </summary>
    struct ControllerEntry {
        std::string name;                               ///< 識別名
        std::unique_ptr<ICameraController> controller;  ///< コントローラー実体

        /// <summary>
        /// 優先度による比較演算子
        /// </summary>
        bool operator<(const ControllerEntry& other) const {
            return static_cast<int>(controller->GetPriority()) >
                   static_cast<int>(other.controller->GetPriority());
        }
    };

    // シングルトンインスタンス
    static CameraManager* instance_;

    // 管理対象カメラ
    Camera* camera_ = nullptr;

    // コントローラーリスト（優先度順）
    std::vector<ControllerEntry> controllers_;

    // 名前からインデックスへのマップ（高速検索用）
    std::unordered_map<std::string, size_t> nameToIndex_;

    // ソートが必要かのフラグ
    bool needsSort_ = false;
};