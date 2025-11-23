# BossNodeEditor 実装計画書

## 1. プロジェクト概要

### 1.1 目的
BossNodeEditorは、ボスキャラクターのAI行動を制御するビヘイビアツリーを視覚的に編集するためのツールです。imgui-node-editorライブラリを使用して、ノードベースのビジュアルエディタを実装しています。

### 1.2 技術スタック
- **GUI Framework**: Dear ImGui
- **Node Editor**: imgui-node-editor (ax::NodeEditor)
- **言語**: C++ (DirectX 12プロジェクト内)
- **デバッグビルド専用**: #ifdef _DEBUG で囲まれた実装

## 2. 現在の実装状況

### 2.1 基本アーキテクチャ

#### データ構造
```cpp
// エディタノード（ID: 10000番台）
struct EditorNode {
    int id;                    // エディタ固有ID
    ImVec2 position;          // エディタ上の位置
    std::string nodeType;     // ノードタイプ名
    std::string displayName;  // 表示名
    BTNodePtr runtimeNode;    // 実際の実行ノード
    std::vector<int> inputPinIds;   // 入力ピンID
    std::vector<int> outputPinIds;  // 出力ピンID
    ImVec4 color;            // ノードカラー
};

// ピン（ID: 20000番台）
struct EditorPin {
    int id;
    int nodeId;
    bool isInput;
    std::string name;
};

// リンク（ID: 30000番台）
struct EditorLink {
    int id;
    int startPinId;
    int endPinId;
    int startNodeId;
    int endNodeId;
};
```

### 2.2 実装済み機能

#### ✅ 基本機能
- **エディタの初期化・終了処理**
  - imgui-node-editorコンテキストの管理
  - 設定ファイル（resources/Json/BossNodeEditor.json）の利用

#### ✅ UI機能
- **ツールバー**
  - Save/Loadボタン（仮実装）
  - Clearボタン
  - Build Runtime Treeボタン
  - ノードタイプ選択ドロップダウン
  - Add Nodeボタン

- **ノード描画**
  - 統一されたノード幅（200px）
  - カテゴリ別のカラーリング
  - 入力/出力ピンバーの描画
  - ノードタイプとディスプレイ名の表示

- **インタラクション**
  - ノードの配置と移動
  - リンクの作成（BeginCreate/EndCreate）
  - ノード・リンクの削除処理
  - ハイライト表示機能（実行中ノードの可視化）

#### ✅ ノードファクトリ（BossNodeFactory）
- **ノードカテゴリ分類**
  - Composite（BTSelector, BTSequence）
  - Action（Idle, Dash, Shoot等）
  - Condition（BTActionSelector）
  - Decorator（将来の拡張用）

- **ノード生成・管理**
  - カテゴリ別のノード取得
  - 依存関係（Boss/Player）を持つノード生成
  - ノードタイプ情報の管理（色、表示名、カテゴリ）

## 3. imgui-node-editor直接実装の経緯

### 3.1 採用理由
- ビヘイビアツリーの視覚的編集に最適なノードベースインターフェース
- ImGuiとの親和性が高い
- 豊富な機能（ズーム、パン、ノード配置、リンク描画）

### 3.2 実装上の工夫

#### ID範囲の分離
ID競合を防ぐため、各要素に独立したID範囲を割り当て：
```cpp
nextNodeId_ = 10000;    // ノード: 10000番台
nextPinId_ = 20000;     // ピン: 20000番台
nextLinkId_ = 30000;    // リンク: 30000番台
```

#### マウスボタンの設定
```cpp
editorConfig_->NavigateButtonIndex = 1;     // 中ボタンでナビゲート
editorConfig_->ContextMenuButtonIndex = 2;  // 右ボタンでコンテキストメニュー
```

## 4. 解決した問題と対処法

### 4.1 右クリックコンテキストメニューの問題
**問題**: imgui-node-editorのコンテキストメニューが表示されない
**対処**:
- ノード作成機能をツールバーのドロップダウンとボタンに移行
- DrawContextMenu()を一時的に無効化

### 4.2 GetViewRect() API非対応
**問題**: `ed::GetViewRect()`が存在しない（C3861エラー）
**対処**: `ed::GetScreenSize()`と`ed::ScreenToCanvas()`を使用した代替実装

### 4.3 座標系の問題
**問題**: BeginChild()使用時にノード移動でテキスト位置がずれる
**対処**:
- BeginChild()を削除
- Dummy + SameLine方式でテキストセンタリング実装

### 4.4 Unicode文字の表示問題
**問題**: Unicode文字（⚡、●など）が正しく表示されない
**対処**: すべてのUnicode文字を削除し、テキストベースの表示に変更

### 4.5 アクセス違反エラー
**問題**: Add Nodeボタンクリック時にthis = 0x384でアクセス違反
**対処**: DrawToolbar()内でのed::GetScreenSize()呼び出しを削除

## 5. 未実装機能と今後の実装予定

### 5.1 高優先度タスク

#### 🔴 JSON保存/読み込み機能
現在は仮実装のため、完全な実装が必要：

**実装方針**:
- ノード、リンク、ピンの情報をJSON形式で保存
- ノード位置、タイプ、接続関係の保持
- ランタイムツリーの再構築可能な形式
- nlohmann/jsonライブラリの活用

**JSON構造案**:
```json
{
  "nodes": [
    {
      "id": 10001,
      "type": "BTSequence",
      "position": [100, 50],
      "properties": {}
    }
  ],
  "links": [
    {
      "id": 30001,
      "start": 10001,
      "end": 10002
    }
  ]
}
```

#### 🔴 ノードインスペクター
**実装方針**:
- 選択ノードのプロパティ表示
- ノードパラメータの編集機能
- BTActionSelectorのActionType編集
- リアルタイム更新

### 5.2 中優先度タスク

#### 🟡 ランタイムデバッグ機能
- 実行中ノードのリアルタイムハイライト
- ノード実行回数の表示
- 実行パスの可視化
- ブレークポイント機能

#### 🟡 コンテキストメニューの復活
- Suspend/Resume方式の改善
- ImGuiポップアップとの適切な統合
- ノード作成、削除、編集機能

#### 🟡 ビヘイビアツリーとの双方向同期
- ImportFromBehaviorTree()の完全実装
- ランタイム変更の反映
- ホットリロード対応

### 5.3 低優先度タスク

#### 🟢 UI/UXの改善
- ノードの自動整列機能
- グリッドスナップ
- ミニマップ表示
- キーボードショートカット（Ctrl+S: 保存、Delete: 削除等）

#### 🟢 高度な編集機能
- コピー＆ペースト（Ctrl+C/V）
- 複数選択と一括操作
- アンドゥ/リドゥ機能（Ctrl+Z/Y）
- ノードグループ化

#### 🟢 拡張機能
- カスタムノードテンプレート
- ノード検索機能
- エクスポート機能（画像、ドキュメント）
- バージョン管理対応

## 6. 技術的課題と対策

### 6.1 パフォーマンス
- **課題**: 大規模ツリー（100ノード以上）での描画パフォーマンス
- **対策**:
  - ビューポートカリングの実装
  - ノードのレベルオブディテール（LOD）
  - 描画バッチング

### 6.2 データ整合性
- **課題**: ランタイムノードとエディタノードの同期
- **対策**:
  - 明確な責任分離
  - イベントベースの同期機構
  - トランザクション的な更新

### 6.3 エラーハンドリング
- **課題**: ユーザビリティを損なわないエラー処理
- **対策**:
  - 適切なエラーメッセージ表示
  - 自動回復機能
  - ログシステムの統合

## 7. テスト計画

### 7.1 単体テスト
- ノード作成/削除のテスト
- リンク作成/削除のテスト
- JSON入出力のテスト
- 循環参照検出のテスト

### 7.2 統合テスト
- ビヘイビアツリーとの連携テスト
- 実行時デバッグ機能のテスト
- パフォーマンステスト（大規模ツリー）

### 7.3 ユーザビリティテスト
- エディタ操作の直感性
- エラーメッセージの適切性
- ドキュメントの充実度

## 8. 開発スケジュール（案）

### Phase 1: 基盤機能完成（2週間）
- JSON保存/読み込み機能
- ノードインスペクター
- 基本的なエラーハンドリング

### Phase 2: デバッグ機能強化（1週間）
- ランタイムデバッグ機能
- 実行パスの可視化
- ログシステム統合

### Phase 3: UX改善（1週間）
- コンテキストメニュー復活
- キーボードショートカット
- UI全体の洗練

### Phase 4: 高度な機能（2週間）
- コピー＆ペースト
- アンドゥ/リドゥ
- ノード検索
- 自動整列

## 9. リファレンス

### 関連ファイル
- `GameProject/Object/Boss/BossNodeEditor/BossNodeEditor.cpp` - メイン実装
- `GameProject/Object/Boss/BossNodeEditor/BossNodeEditor.h` - ヘッダーファイル
- `GameProject/Object/Boss/BossNodeEditor/BossNodeFactory.cpp` - ノードファクトリ
- `GameProject/Object/Boss/BossNodeEditor/BossNodeFactory.h` - ファクトリヘッダー

### 外部ライブラリ
- [imgui-node-editor](https://github.com/thedmd/imgui-node-editor) - ノードエディタライブラリ
- [Dear ImGui](https://github.com/ocornut/imgui) - GUIフレームワーク

### 参考資料
- Unity Shader Graph - UIデザインの参考
- Unreal Engine Blueprints - ノードベースプログラミングの参考
- Behavior Designer - ビヘイビアツリーエディタの参考

## 10. まとめ

BossNodeEditorは、imgui-node-editorを活用した強力なビヘイビアツリーエディタとして基本機能が実装されています。現在までに多くの技術的課題を解決し、安定した基盤が構築できました。

### 次のステップ
1. **JSON保存/読み込み機能の実装** - データの永続化
2. **ノードインスペクターの実装** - 詳細な編集機能
3. **ランタイムデバッグ機能の強化** - 開発効率の向上
4. **コンテキストメニューの復活と改善** - 操作性の向上
5. **UI/UXの継続的改善** - ユーザー体験の向上

このドキュメントは、開発の進捗に応じて定期的に更新される予定です。

---
*Last Updated: 2024-11-24*
*Version: 1.0.0*