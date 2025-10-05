#include "GameScene.h"
#include "ModelManager.h"
#include "Object3dBasic.h"
#include "SpriteBasic.h"
#include "Input.h"
#include "DebugCamera.h"
#include "Draw2D.h"
#include "GlobalVariables.h"
#include "FrameTimer.h"
#include "GPUParticle.h"
#include "SceneManager.h"
#include "EmitterManager.h"
#include "Object3d.h"
#include "Model.h"
#include "ShadowRenderer.h"
#include "CollisionManager.h"
#include "../Collision/CollisionTypeIdDef.h"

#include <numbers>

#ifdef _DEBUG
#include"ImGui.h"
#include "DebugCamera.h"
#include "DebugUIManager.h"
#endif

void GameScene::Initialize()
{
  // CollisionManagerの初期化
  CollisionManager* collisionManager = CollisionManager::GetInstance();

#ifdef _DEBUG
  DebugCamera::GetInstance()->Initialize();
  Object3dBasic::GetInstance()->SetDebug(false);
  Draw2D::GetInstance()->SetDebug(false);
  GPUParticle::GetInstance()->SetIsDebug(false);

  // デバッグビルドではコライダー表示をデフォルトでON
  collisionManager->SetDebugDrawEnabled(true);
  
  // DebugUIManagerにシーン名を設定
  DebugUIManager::GetInstance()->SetSceneName("GameScene");

  // ゲームオブジェクトをDebugUIManagerに登録（SceneHierarchyで選択可能）
  DebugUIManager::GetInstance()->RegisterGameObject("Player",
    [this]() { if (player_) player_->DrawImGui(); });
  DebugUIManager::GetInstance()->RegisterGameObject("Boss",
    [this]() { if (boss_) boss_->DrawImGui(); });
  DebugUIManager::GetInstance()->RegisterGameObject("FollowCamera",
    [this]() { if (followCamera_) followCamera_->DrawImGui(); });

  emitterManager_ = std::make_unique<EmitterManager>(GPUParticle::GetInstance());
  DebugUIManager::GetInstance()->SetEmitterManager(emitterManager_.get());
#endif
  /// ================================== ///
  ///              初期化処理             ///
  /// ================================== ///

  // SkyBoxの初期化
  skyBox_ = std::make_unique<SkyBox>();
  skyBox_->Initialize("my_skybox.dds");

  collisionManager->Initialize();

  // 床モデルのUV変換設定
  groundUvTransform_.translate = Vector3(0.0f, 0.0f, 0.0f);
  groundUvTransform_.rotate = Vector3(0.0f, 0.0f, 0.0f);
  groundUvTransform_.scale = Vector3(100.0f, 100.0f, 100.0f);
  // 床モデルの初期化
  ground_ = std::make_unique<Object3d>();
  ground_->Initialize();
  ground_->SetModel("ground_black.gltf");
  ground_->SetUvTransform(groundUvTransform_);

  // 敵モデルの初期化
  boss_ = std::make_unique<Boss>();
  boss_->Initialize();

  // Playerの初期化
  player_ = std::make_unique<Player>();
  player_->Initialize();
  player_->SetCamera((*Object3dBasic::GetInstance()->GetCamera()));

  followCamera_ = std::make_unique<FollowCamera>();
  followCamera_->Initialize((*Object3dBasic::GetInstance()->GetCamera()));
  followCamera_->SetTarget(&player_->GetTransform());
  followCamera_->SetTarget2(&boss_->GetTransform());
  
  // 衝突マスクの設定（どのタイプ同士が衝突判定を行うか）
  collisionManager->SetCollisionMask(
    static_cast<uint32_t>(CollisionTypeId::kPlayerMeleeAttack),
    static_cast<uint32_t>(CollisionTypeId::kEnemy),
    true
  );
  
  collisionManager->SetCollisionMask(
    static_cast<uint32_t>(CollisionTypeId::kPlayer),
    static_cast<uint32_t>(CollisionTypeId::kEnemyAttack),
    true
  );
}

void GameScene::Finalize()
{
#ifdef _DEBUG
  // デバッグ情報の登録解除
  DebugUIManager::GetInstance()->ClearDebugInfo();
  // ゲームオブジェクトの登録解除
  DebugUIManager::GetInstance()->ClearGameObjects();
#endif

  // オブジェクトの終了処理
  if (player_) {
    player_->Finalize();
  }
  if (boss_) {
    boss_->Finalize();
  }
  
  // CollisionManagerのリセット
  CollisionManager::GetInstance()->Reset();
}

void GameScene::Update()
{
  /// ================================== ///
  ///              更新処理               ///
  /// ================================== ///

  player_->SetMode(followCamera_->GetMode());

  skyBox_->Update();
  ground_->Update();
  player_->Update();
  boss_->Update();
  followCamera_->Update();
  emitterManager_->Update();
  
  // 衝突判定の実行
  CollisionManager::GetInstance()->CheckAllCollisions();

  // シーン遷移
  if (Input::GetInstance()->TriggerKey(DIK_RETURN))
  {
    SceneManager::GetInstance()->ChangeScene("title");
  }
}

void GameScene::Draw()
{
  /// ================================== ///
  ///              描画処理               ///
  /// ================================== ///

  skyBox_->Draw();

  if (ShadowRenderer::GetInstance()->IsEnabled())
  {
    ShadowRenderer::GetInstance()->BeginShadowPass();
    ground_->Draw();
    player_->Draw();
    boss_->Draw();
    ShadowRenderer::GetInstance()->EndShadowPass();
  }

  //------------------背景Spriteの描画------------------//
  // スプライト共通描画設定
  SpriteBasic::GetInstance()->SetCommonRenderSetting();



  //-------------------Modelの描画-------------------//
  // 3Dモデル共通描画設定
  Object3dBasic::GetInstance()->SetCommonRenderSetting();

  ground_->Draw();
  player_->Draw();
  boss_->Draw();

  //------------------前景Spriteの描画------------------//
  // スプライト共通描画設定
  SpriteBasic::GetInstance()->SetCommonRenderSetting();


#ifdef _DEBUG
  // コライダーのデバッグ描画
  CollisionManager::GetInstance()->DrawColliders();
#endif

}

void GameScene::DrawWithoutEffect()
{
  /// ================================== ///
  ///              描画処理               ///
  /// ================================== ///

  //------------------背景Spriteの描画------------------//
  // スプライト共通描画設定
  SpriteBasic::GetInstance()->SetCommonRenderSetting();



  //-------------------Modelの描画-------------------//
   // 3Dモデル共通描画設定
  Object3dBasic::GetInstance()->SetCommonRenderSetting();



  //------------------前景Spriteの描画------------------//
  // スプライト共通描画設定
  SpriteBasic::GetInstance()->SetCommonRenderSetting();

}

void GameScene::DrawImGui()
{
#ifdef _DEBUG

#endif // DEBUG
}
