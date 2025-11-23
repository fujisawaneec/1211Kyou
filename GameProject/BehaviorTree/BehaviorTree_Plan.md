# ビヘイビアツリーシステム実装ドキュメント

## 📌 概要

本ドキュメントは、3DActionProjectのボスAIシステムをステートマシンからビヘイビアツリーに移行した実装内容をまとめたものです。

### ビヘイビアツリーとは

ビヘイビアツリー（Behavior Tree）は、ゲームAIの意思決定を階層的なツリー構造で表現する手法です。各ノードが特定の動作や条件を表し、ツリーを上から下へ評価することでAIの行動が決定されます。

### 移行の背景

**ステートマシンの課題：**
- 状態数の増加に伴う複雑性の増大
- 状態遷移の硬直性
- 新しい行動パターンの追加が困難

**ビヘイビアツリーの利点：**
- モジュール化された行動の組み合わせ
- 条件分岐の柔軟な表現
- デバッグとテストの容易性
- 再利用可能なノードによる拡張性

## 🏗️ アーキテクチャ

### システム構成図

```
Boss (ゲームオブジェクト)
    ├── BossBehaviorTree (ツリー管理)
    │   ├── BTBlackboard (データ共有)
    │   └── ルートノード
    │       └── ツリー構造（下記参照）
    └── BossStateMachine (互換性のため保持)
```

### クラス階層

```
BTNode (基底クラス)
├── BTComposite (複合ノード基底)
│   ├── BTSelector (OR条件)
│   └── BTSequence (AND条件)
├── BTDecorator (デコレータ基底) ※未実装
└── BTLeaf (リーフノード基底) ※暗黙的
    ├── BTBossIdle
    ├── BTBossDash
    ├── BTBossShoot
    └── BTActionSelector
```

### ビヘイビアツリー構造

```
MainLoop (Sequence)
├── Idle (待機アクション)
└── ActionSelector (Selector)
    ├── DashSequence (Sequence)
    │   ├── DashCondition (偶数チェック)
    │   └── DashAction
    └── ShootSequence (Sequence)
        ├── ShootCondition (奇数チェック)
        └── ShootAction
```

## 📦 実装クラス詳細

### コアシステム

#### BTNode
**ファイル:** `Core/BTNode.h, BTNode.cpp`
**役割:** 全ノードの基底クラス

```cpp
class BTNode {
public:
    virtual BTNodeStatus Execute(BTBlackboard* blackboard) = 0;
    virtual void Reset();

    // Getter/Setter
    const std::string& GetName() const;
    void SetName(const std::string& name);
    BTNodeStatus GetStatus() const;
    bool IsRunning() const;

protected:
    BTNodeStatus status_ = BTNodeStatus::Failure;
    bool isRunning_ = false;
    std::string name_ = "BTNode";
};
```

**BTNodeStatus列挙型:**
- `Success`: 成功
- `Failure`: 失敗
- `Running`: 実行中

#### BTBlackboard
**ファイル:** `Core/BTBlackboard.h`
**役割:** ノード間でのデータ共有

主要メソッド：
- `SetBoss(Boss*)` / `GetBoss()`: ボス参照の管理
- `SetPlayer(Player*)` / `GetPlayer()`: プレイヤー参照の管理
- `SetDeltaTime(float)` / `GetDeltaTime()`: フレーム時間の管理
- `SetInt/GetInt`, `SetFloat/GetFloat`, `SetVector3/GetVector3`: 汎用データ管理

```cpp
// 使用例
blackboard->SetInt("ActionCounter", 0);
int counter = blackboard->GetInt("ActionCounter", 0);
```

#### BTComposite
**ファイル:** `Core/BTComposite.h, BTComposite.cpp`
**役割:** 複数の子ノードを持つノードの基底クラス

主要メソッド：
- `AddChild(BTNodePtr)`: 子ノード追加
- `RemoveChild(BTNodePtr)`: 子ノード削除
- `GetChildren()`: 子ノードリスト取得
- `Reset()`: 全子ノードのリセット

### コンポジットノード

#### BTSelector
**ファイル:** `Composites/BTSelector.h, BTSelector.cpp`
**役割:** OR条件（いずれかが成功すれば成功）

動作：
1. 子ノードを順番に実行
2. 最初に`Success`を返した時点で`Success`を返す
3. 全て`Failure`なら`Failure`を返す
4. `Running`の場合は次回その子から再開

#### BTSequence
**ファイル:** `Composites/BTSequence.h, BTSequence.cpp`
**役割:** AND条件（全てが成功で成功）

動作：
1. 子ノードを順番に実行
2. `Failure`が返されたら即座に`Failure`を返す
3. 全て`Success`なら`Success`を返す
4. `Running`の場合は次回その子から再開

### ボス専用アクションノード

#### BTBossIdle
**ファイル:** `BossAI/Actions/BTBossIdle.h, BTBossIdle.cpp`
**役割:** 待機状態の処理

機能：
- プレイヤーの方向を向く（スムーズ回転）
- フェーズに応じた待機時間の調整
  - フェーズ1: 1.3〜2.0秒
  - フェーズ2: 0.8〜1.5秒
- ActionCounterのインクリメント

#### BTBossDash
**ファイル:** `BossAI/Actions/BTBossDash.h, BTBossDash.cpp`
**役割:** ダッシュ移動の処理

機能：
- XZ平面上のランダムな方向へダッシュ
- 距離: 10.0f〜50.0f（ランダム）
- 速度: フェーズ1=60.0f、フェーズ2=30.0f
- Hermite補間によるイージング（加速→減速）
- Y軸振動エフェクト

#### BTBossShoot
**ファイル:** `BossAI/Actions/BTBossShoot.h, BTBossShoot.cpp`
**役割:** 射撃攻撃の処理

機能：
- 3方向同時発射（-15度、0度、+15度）
- 弾速: フェーズ1=100.0f、フェーズ2=20.0f
- 準備時間: 0.5秒
- 硬直時間: フェーズ1=0.5秒、フェーズ2=0.3秒

### 条件ノード

#### BTActionSelector
**ファイル:** `BossAI/Conditions/BTActionSelector.h, BTActionSelector.cpp`
**役割:** Dash/Shoot の選択条件

```cpp
enum class ActionType {
    Dash = 0,   // ActionCounter が偶数
    Shoot = 1   // ActionCounter が奇数
};
```

### 統合クラス

#### BossBehaviorTree
**ファイル:** `BossAI/BossBehaviorTree.h, BossBehaviorTree.cpp`
**役割:** ビヘイビアツリー全体の管理

主要メソッド：
- `Update(float deltaTime)`: ツリーの実行
- `Reset()`: ツリーのリセット
- `SetPlayer(Player*)`: プレイヤー参照の更新
- `BuildTree()`: ツリー構造の構築

## 🎮 Bossクラスとの統合

### Boss.h の変更点

```cpp
class Boss {
    // ... 既存のメンバー ...

    // 追加されたメンバー
    std::unique_ptr<BossBehaviorTree> behaviorTree_;
    bool useBehaviorTree_ = true;  // デフォルトでビヘイビアツリーを使用

    // 追加されたメソッド
    BossStateMachine* GetStateMachine() const;
    Player* GetPlayer() const;
    void SetPlayer(Player* player);
};
```

### Boss.cpp の変更点

**初期化処理：**
```cpp
void Boss::Initialize() {
    // ... 既存の初期化 ...

    if (useBehaviorTree_) {
        behaviorTree_ = std::make_unique<BossBehaviorTree>(this, player_);
    } else {
        // ステートマシンの初期化（互換性のため保持）
    }
}
```

**更新処理：**
```cpp
void Boss::Update(float deltaTime) {
    // ... HP更新など ...

    if (!isDead_ && !isPause_) {
        if (useBehaviorTree_ && behaviorTree_) {
            behaviorTree_->Update(deltaTime);
        } else if (stateMachine_) {
            stateMachine_->Update(deltaTime);
        }
    }
}
```

## 🛠️ 使用方法

### 基本的な使用例

```cpp
// ボスの初期化時
Boss* boss = new Boss();
boss->Initialize();
boss->SetPlayer(player);  // プレイヤー参照を設定

// 更新処理
void GameScene::Update(float deltaTime) {
    boss->Update(deltaTime);  // ビヘイビアツリーが自動実行
}
```

### デバッグUIでの切り替え

ImGuiのデバッグウィンドウで、ビヘイビアツリーとステートマシンを切り替え可能：

1. F1キーでImGuiを開く
2. Boss Debug ウィンドウを開く
3. "AI System" セクションで選択
   - "Behavior Tree": ビヘイビアツリーを使用
   - "State Machine": ステートマシンを使用（互換性確認用）

## 🔧 拡張ガイド

### 新規アクションノードの追加

1. `BossAI/Actions/` に新しいクラスを作成
2. `BTNode` を継承
3. `Execute()` メソッドを実装

```cpp
// 例: BTBossSpecialAttack.h
class BTBossSpecialAttack : public BTNode {
public:
    BTBossSpecialAttack();
    BTNodeStatus Execute(BTBlackboard* blackboard) override;
    void Reset() override;

private:
    float chargeTime_ = 2.0f;
    float elapsedTime_ = 0.0f;
    bool isFirstExecute_ = true;
};
```

### 新規条件ノードの追加

```cpp
// 例: HP条件チェック
class BTHPCondition : public BTNode {
public:
    BTHPCondition(float threshold, bool checkBelow = true);
    BTNodeStatus Execute(BTBlackboard* blackboard) override;

private:
    float hpThreshold_;
    bool checkBelow_;
};
```

### カスタムツリーの構築

```cpp
BTNodePtr BuildCustomTree() {
    auto root = std::make_shared<BTSelector>();

    // 低HPブランチ
    auto lowHPSequence = std::make_shared<BTSequence>();
    lowHPSequence->AddChild(std::make_shared<BTHPCondition>(50.0f));
    lowHPSequence->AddChild(std::make_shared<BTBossSpecialAttack>());

    // 通常ブランチ
    auto normalSequence = std::make_shared<BTSequence>();
    normalSequence->AddChild(std::make_shared<BTBossIdle>());
    normalSequence->AddChild(std::make_shared<BTBossShoot>());

    root->AddChild(lowHPSequence);
    root->AddChild(normalSequence);

    return root;
}
```

## 📈 今後の拡張案

### デコレーターノード
- **BTRepeat**: アクションの繰り返し実行
- **BTCooldown**: クールダウン時間の管理
- **BTInverter**: 結果の反転
- **BTTimeout**: タイムアウト処理

### 並列実行
- **BTParallel**: 複数ノードの同時実行
- **BTMonitor**: 条件監視ノード

### デバッグ機能
- ツリー構造の視覚化（ImGui）
- ノード実行履歴の記録
- パフォーマンスプロファイリング

### AI拡張
- 学習機能の追加
- 動的なツリー構造の変更
- プレイヤーの行動パターン認識

## 📝 注意事項

1. **メモリ管理**: `std::shared_ptr` を使用してノードを管理
2. **実行順序**: 子ノードは左から右（配列順）に実行
3. **状態の永続性**: `Running` 状態のノードは次フレームで再開
4. **デバッグ**: ImGuiでAIシステムの切り替えが可能

## 🎯 パフォーマンス

- ツリー評価: O(n) ※nは評価されるノード数
- メモリ使用量: 最小限（ノード数に比例）
- 更新頻度: 毎フレーム（60FPS）

## 📚 参考資料

- [Behavior Trees in Robotics and AI](https://arxiv.org/abs/1709.00084)
- [Game AI Pro: Behavior Trees](http://www.gameaipro.com/)
- [Unreal Engine Behavior Tree Documentation](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ArtificialIntelligence/BehaviorTrees/)

---

最終更新日: 2024年11月24日
作成者: Claude Code Assistant
バージョン: 1.0.0