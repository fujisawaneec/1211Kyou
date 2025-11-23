#pragma once
#include <unordered_map>
#include <any>
#include <string>
#include <optional>
#include "Vector3.h"

class Boss;
class Player;

/// <summary>
/// ビヘイビアツリーのブラックボード
/// ノード間でデータを共有するための仕組み
/// </summary>
class BTBlackboard {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    BTBlackboard() = default;

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BTBlackboard() = default;

    /// <summary>
    /// ボスの設定
    /// </summary>
    /// <param name="boss">ボスのポインタ</param>
    void SetBoss(Boss* boss) { boss_ = boss; }

    /// <summary>
    /// ボスの取得
    /// </summary>
    /// <returns>ボスのポインタ</returns>
    Boss* GetBoss() const { return boss_; }

    /// <summary>
    /// プレイヤーの設定
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    void SetPlayer(Player* player) { player_ = player; }

    /// <summary>
    /// プレイヤーの取得
    /// </summary>
    /// <returns>プレイヤーのポインタ</returns>
    Player* GetPlayer() const { return player_; }

    /// <summary>
    /// フレームの経過時間を設定
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    void SetDeltaTime(float deltaTime) { deltaTime_ = deltaTime; }

    /// <summary>
    /// フレームの経過時間を取得
    /// </summary>
    /// <returns>経過時間</returns>
    float GetDeltaTime() const { return deltaTime_; }

    /// <summary>
    /// 汎用データの設定
    /// </summary>
    /// <template name="T">データ型</template>
    /// <param name="key">キー</param>
    /// <param name="value">値</param>
    template<typename T>
    void SetValue(const std::string& key, const T& value) {
        data_[key] = value;
    }

    /// <summary>
    /// 汎用データの取得
    /// </summary>
    /// <template name="T">データ型</template>
    /// <param name="key">キー</param>
    /// <returns>値（存在しない場合はnullopt）</returns>
    template<typename T>
    std::optional<T> GetValue(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            try {
                return std::any_cast<T>(it->second);
            }
            catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    /// <summary>
    /// 整数値の設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="value">値</param>
    void SetInt(const std::string& key, int value) {
        data_[key] = value;
    }

    /// <summary>
    /// 整数値の取得
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="defaultValue">デフォルト値</param>
    /// <returns>値</returns>
    int GetInt(const std::string& key, int defaultValue = 0) const {
        auto value = GetValue<int>(key);
        return value.value_or(defaultValue);
    }

    /// <summary>
    /// 浮動小数点値の設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="value">値</param>
    void SetFloat(const std::string& key, float value) {
        data_[key] = value;
    }

    /// <summary>
    /// 浮動小数点値の取得
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="defaultValue">デフォルト値</param>
    /// <returns>値</returns>
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
        auto value = GetValue<float>(key);
        return value.value_or(defaultValue);
    }

    /// <summary>
    /// ベクトル値の設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="value">値</param>
    void SetVector3(const std::string& key, const Vector3& value) {
        data_[key] = value;
    }

    /// <summary>
    /// ベクトル値の取得
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="defaultValue">デフォルト値</param>
    /// <returns>値</returns>
    Vector3 GetVector3(const std::string& key, const Vector3& defaultValue = Vector3()) const {
        auto value = GetValue<Vector3>(key);
        return value.value_or(defaultValue);
    }

    /// <summary>
    /// キーが存在するかチェック
    /// </summary>
    /// <param name="key">キー</param>
    /// <returns>存在する場合true</returns>
    bool HasKey(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    /// <summary>
    /// キーの削除
    /// </summary>
    /// <param name="key">キー</param>
    void RemoveKey(const std::string& key) {
        data_.erase(key);
    }

    /// <summary>
    /// 全データのクリア
    /// </summary>
    void Clear() {
        data_.clear();
    }

private:
    // ボスのポインタ
    Boss* boss_ = nullptr;

    // プレイヤーのポインタ
    Player* player_ = nullptr;

    // フレームの経過時間
    float deltaTime_ = 0.0f;

    // 汎用データストレージ
    std::unordered_map<std::string, std::any> data_;
};