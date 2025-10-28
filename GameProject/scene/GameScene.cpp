#include "GameScene.h"
#include "ModelManager.h"
#include "Object3dBasic.h"
#include "SpriteBasic.h"
#include "Input.h"
#include "DebugCamera.h"
#include "Draw2D.h"
#include "FrameTimer.h"
#include "GPUParticle.h"
#include "SceneManager.h"
#include "EmitterManager.h"
#include "Object3d.h"
#include "Model.h"
#include "ShadowRenderer.h"
#include "CollisionManager.h"
#include "../Collision/CollisionTypeIdDef.h"

// Camera System includes
#include "CameraSystem/CameraManager.h"
#include "CameraSystem/FirstPersonController.h"
#include "CameraSystem/TopDownController.h"
#include "CameraSystem/CameraAnimationController.h"

#include <numbers>

#ifdef _DEBUG
#include"ImGui.h"
#include "DebugCamera.h"
#include "DebugUIManager.h"
#include "CameraSystem/CameraDebugUI.h"
#endif

void GameScene::Initialize()
{
  // CollisionManagerの初期化
  CollisionManager* collisionManager = CollisionManager::GetInstance();

  // EmitterManagerの生成
  emitterManager_ = std::make_unique<EmitterManager>(GPUParticle::GetInstance());

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

  // CameraSystemとCameraAnimationEditorを別々に登録
  DebugUIManager::GetInstance()->RegisterGameObject("CameraSystem",
    []() { CameraDebugUI::Draw(); });

  DebugUIManager::GetInstance()->RegisterGameObject("CameraAnimationEditor",
    []() {
      CameraDebugUI::DrawAnimationEditorOnly();
      // 更新処理も呼び出す（deltaTimeは取得可能）
      CameraDebugUI::UpdateAnimationEditor(
        FrameTimer::GetInstance()->GetDeltaTime());
    });

  DebugUIManager::GetInstance()->SetEmitterManager(emitterManager_.get());
#endif
  /// ================================== ///
  ///              初期化処理             ///
  /// ================================== ///

  // シャドウマッピンの最大描画距離の設定
  ShadowRenderer::GetInstance()->SetMaxShadowDistance(100.f);
  // 平行光源の方向の設定
  Object3dBasic::GetInstance()->SetDirectionalLightDirection(Vector3(0.0f, -1.0f, -0.05f));

  // エミッターマネージャーの初期化
  emitterManager_->LoadPreset("over1", "over1");
  emitterManager_->LoadPreset("over2", "over2");

  // タイトルボタンテキストの初期化
  toTitleText_ = std::make_unique<Sprite>();
  toTitleText_->Initialize("game_button_text.png");
  toTitleText_->SetPos(Vector2(WinApp::clientWidth / 2.f - toTitleText_->GetSize().x / 2.f, 200.f));

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

  // カメラシステムの初期化
  cameraManager_ = CameraManager::GetInstance();
  cameraManager_->Initialize((*Object3dBasic::GetInstance()->GetCamera()));

  // FirstPersonControllerを登録
  auto fpController = std::make_unique<FirstPersonController>();
  firstPersonController_ = fpController.get();
  firstPersonController_->SetTarget(&player_->GetTransform());
  cameraManager_->RegisterController("FirstPerson", std::move(fpController));

  // TopDownControllerを登録
  auto tdController = std::make_unique<TopDownController>();
  topDownController_ = tdController.get();
  topDownController_->SetTarget(&player_->GetTransform());
  std::vector<const Transform*> additionalTargets = { &boss_->GetTransform() };
  topDownController_->SetAdditionalTargets(additionalTargets);
  cameraManager_->RegisterController("TopDown", std::move(tdController));

  // CameraAnimationControllerを登録
  auto animController = std::make_unique<CameraAnimationController>();
  animationController_ = animController.get();
  cameraManager_->RegisterController("Animation", std::move(animController));

  // ゲーム開始アニメーションを再生
  animationController_->LoadAnimation("game_start");
  animationController_->Play();

  // game_overアニメーションの設定
  animationController_->LoadAnimationFromFile("over_anim", "over_anim");
  animationController_->SetAnimationTargetByName("over_anim", player_->GetTransformPtr());


  // デフォルトモードを設定（TopDown）
  cameraMode_ = false;
  cameraManager_->ActivateController("TopDown");


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
  // AnimationEditorのクリーンアップ
  CameraDebugUI::CleanupAnimationEditor();
#endif

  // オブジェクトの終了処理
  if (player_) {
    player_->Finalize();
  }
  if (boss_) {
    boss_->Finalize();
  }

  // CameraManagerのクリーンアップ
  if (cameraManager_) {
    cameraManager_->Finalize();
    cameraManager_ = nullptr;
  }

  // CollisionManagerのリセット
  CollisionManager::GetInstance()->Reset();
}

void GameScene::Update()
{
  /// ================================== ///
  ///              更新処理               ///
  /// ================================== ///

  // カメラモード切り替え
  if (Input::GetInstance()->TriggerKey(DIK_P)) {
    cameraMode_ = !cameraMode_;
    cameraManager_->DeactivateAllControllers();

    if (cameraMode_) {
      cameraManager_->ActivateController("FirstPerson");
    } else {
      cameraManager_->ActivateController("TopDown");
    }
  }

  player_->SetMode(cameraMode_);

  skyBox_->Update();
  ground_->Update();
  player_->Update();
  boss_->Update();
  toTitleText_->Update();

  cameraManager_->Update(FrameTimer::GetInstance()->GetDeltaTime());

  Vector3 playerPos = { .x = player_->GetTransform().translate.x,
                      .y = player_->GetTransform().translate.y,
                      .z = player_->GetTransform().translate.z };

  emitterManager_->SetEmitterPosition("over1", playerPos);
  emitterManager_->SetEmitterPosition("over2", playerPos);

  emitterManager_->Update();

  UpdateOverAnim();

  // 衝突判定の実行
  CollisionManager::GetInstance()->CheckAllCollisions();

  if (Input::GetInstance()->TriggerKey(DIK_O))
  {
    StartOverAnim();
  }

  if (Input::GetInstance()->TriggerKey(DIK_2))
  {
    emitterManager_->CreateTemporaryEmitterFrom("over2", "over2_temp", 0.1f);
  }

  if (Input::GetInstance()->TriggerKey(DIK_1))
  {
    emitterManager_->CreateTemporaryEmitterFrom("over1", "over1_temp", 0.1f);
  }

  // シーン遷移
  if (Input::GetInstance()->TriggerKey(DIK_RETURN))
  {
    SceneManager::GetInstance()->ChangeScene("title", "Fade", 0.3f);
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

  toTitleText_->Draw();
}

void GameScene::DrawImGui()
{
#ifdef _DEBUG

#endif // DEBUG
}

void GameScene::StartOverAnim()
{
  isOver_ = true;

  animationController_->SwitchAnimation("over_anim");
  animationController_->Play();

}

void GameScene::UpdateOverAnim()
{
  if (isOver_) overAnimTimer_ += FrameTimer::GetInstance()->GetDeltaTime();

  if (overAnimTimer_ > 1.5f && !isOver1Emit)
  {
    emitterManager_->CreateTemporaryEmitterFrom("over1", "over1_temp", 0.5f);
    isOver1Emit = true;
  }

  if (overAnimTimer_ > 2.0f && !isOver2Emit)
  {
    emitterManager_->CreateTemporaryEmitterFrom("over2", "over2_temp", 0.1f);
    isOver2Emit = true;
  }

  if (overAnimTimer_ > 2.8f)
  {
    SceneManager::GetInstance()->ChangeScene("title", "Fade", 0.3f);
  }
}
