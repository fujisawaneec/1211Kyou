// Engine includes
#include "GameScene.h"
#include "ModelManager.h"
#include "Object3dBasic.h"
#include "SpriteBasic.h"
#include "Sprite.h"
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

// Game includes
#include "../Collision/CollisionTypeIdDef.h"
#include "CameraSystem/CameraManager.h"
#include "CameraSystem/ThirdPersonController.h"
#include "CameraSystem/TopDownController.h"
#include "CameraSystem/CameraAnimationController.h"

// Debug includes
#ifdef _DEBUG
#include"ImGui.h"
#include "DebugCamera.h"
#include "DebugUIManager.h"
#include "CameraSystem/CameraDebugUI.h"
#endif

void GameScene::Initialize()
{
    // CollisionManagerを取得
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

    // ゲームオブジェクトをDebugUIManagerに登録
    DebugUIManager::GetInstance()->RegisterGameObject("Player",
        [this]() { if (player_) player_->DrawImGui(); });
    DebugUIManager::GetInstance()->RegisterGameObject("Boss",
        [this]() { if (boss_) boss_->DrawImGui(); });

    // CameraSystemデバッグUI登録
    DebugUIManager::GetInstance()->RegisterGameObject("CameraSystem",
        []() { CameraDebugUI::Draw(); });

    // CameraAnimationEditorデバッグUI登録
    DebugUIManager::GetInstance()->RegisterGameObject("CameraAnimationEditor",
        []() {
            CameraDebugUI::DrawAnimationEditorOnly();
            // 更新処理も呼び出す
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
    emitterManager_->LoadScenePreset("gamescene_preset");

    // タイトルボタンテキストの初期化
    toTitleSprite_ = std::make_unique<Sprite>();
    toTitleSprite_->Initialize("game_button_text.png");
    toTitleSprite_->SetPos(Vector2(WinApp::clientWidth / 2.f - toTitleSprite_->GetSize().x / 2.f, 200.f));

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

    // Input Handlerの初期化
    inputHandler_ = std::make_unique<InputHandler>();
    inputHandler_->Initialize();

    // 敵モデルの初期化
    boss_ = std::make_unique<Boss>();
    boss_->Initialize();

    // Playerの初期化
    player_ = std::make_unique<Player>();
    player_->Initialize();
    player_->SetCamera((*Object3dBasic::GetInstance()->GetCamera()));
    player_->SetInputHandler(inputHandler_.get());

    // カメラシステムの初期化
    cameraManager_ = CameraManager::GetInstance();
    cameraManager_->Initialize((*Object3dBasic::GetInstance()->GetCamera()));

    // FirstPersonControllerを登録
    auto fpController = std::make_unique<ThirdPersonController>();
    firstPersonController_ = fpController.get();
    firstPersonController_->SetTarget(&player_->GetTransform());
    // ボスをセカンダリターゲットとして設定し、注視機能を有効化
    firstPersonController_->SetSecondaryTarget(&boss_->GetTransform());
    firstPersonController_->EnableLookAtTarget(true);
    cameraManager_->RegisterController("ThirdPerson", std::move(fpController));

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
    animationController_->LoadAnimationFromFile("over_anim");
    animationController_->SetAnimationTargetByName("over_anim", player_->GetTransformPtr());

    // デフォルトモードを設定（TopDown）
    cameraMode_ = false;
    cameraManager_->ActivateController("TopDown");


    // 衝突マスクの設定（どのタイプ同士が衝突判定を行うか）
    collisionManager->SetCollisionMask(
        static_cast<uint32_t>(CollisionTypeId::PLAYER_MELEE_ATTACK),
        static_cast<uint32_t>(CollisionTypeId::BOSS),
        true
    );

    collisionManager->SetCollisionMask(
        static_cast<uint32_t>(CollisionTypeId::PLAYER),
        static_cast<uint32_t>(CollisionTypeId::BOSS_ATTACK),
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

#ifdef _DEBUG
    // Pキーでカメラモード切り替え
    if (Input::GetInstance()->TriggerKey(DIK_P)) {
        cameraMode_ = !cameraMode_;
    }

    // ゲームオーバーアニメーションEnterキーで再生
    if (Input::GetInstance()->TriggerKey(DIK_RETURN))
    {
        StartOverAnim();
    }
#endif

    // ゲームクリア判定
    if (boss_->IsDead())
    {
        /// TODO: ゲームクリアシーンを作成したら"title"をそちらに変更
        SceneManager::GetInstance()->ChangeScene("title", "Fade", 0.3f);
    }

    // カメラモードの更新
    UpdateCameraMode();

    // 入力の更新
    UpdateInput();

    // オブジェクトの更新処理
    skyBox_->Update();
    ground_->Update();
    player_->Update();
    boss_->Update();
    toTitleSprite_->Update();
    cameraManager_->Update(FrameTimer::GetInstance()->GetDeltaTime());

    // プレイヤーの位置にエミッターをセット
    Vector3 playerPos = { .x = player_->GetTransform().translate.x,
                        .y = player_->GetTransform().translate.y,
                        .z = player_->GetTransform().translate.z };

    emitterManager_->SetEmitterPosition("over1", playerPos);
    emitterManager_->SetEmitterPosition("over2", playerPos);

    // エミッターマネージャーの更新
    emitterManager_->Update();

    // ゲームオーバーアニメーションの更新
    UpdateOverAnim();

    // 衝突判定の実行
    CollisionManager::GetInstance()->CheckAllCollisions();
}

void GameScene::Draw()
{
    /// ================================== ///
    ///              描画処理               ///
    /// ================================== ///

    //-------------------SkyBoxの描画-------------------//
    skyBox_->Draw();

    //------------------シャドウマップの描画------------------//
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

    //toTitleSprite_->Draw();
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

    if (overAnimTimer_ > 2.0f && !isOver1Emit)
    {
        emitterManager_->CreateTemporaryEmitterFrom("over1", "over1_temp", 0.5f);
        isOver1Emit = true;
    }

    if (overAnimTimer_ > 2.8f && !isOver2Emit)
    {
        emitterManager_->CreateTemporaryEmitterFrom("over2", "over2_temp", 0.1f);
        isOver2Emit = true;
    }

    if (overAnimTimer_ > 3.8f)
    {
        SceneManager::GetInstance()->ChangeScene("title", "Fade", 0.3f);
    }
}

void GameScene::UpdateCameraMode()
{
    if (boss_->GetPhase() == 1)
    {
        cameraMode_ = false;
        // フェーズ1: 動的制限を解除（ステージ全体を移動可能）
        player_->ClearDynamicBounds();
    }
    else if (boss_->GetPhase() == 2)
    {
        cameraMode_ = true;
        // フェーズ2: ボス中心の戦闘エリアに移動制限
        Vector3 bossPos = boss_->GetTransform().translate;
        float battleAreaSize = 20.0f;  // 戦闘エリアのサイズ（片側）
        player_->SetDynamicBoundsFromCenter(bossPos, battleAreaSize, battleAreaSize);
    }

    if (cameraMode_) {
        cameraManager_->ActivateController("ThirdPerson");
    } else {
        cameraManager_->ActivateController("TopDown");
    }

    // カメラモードをPlayerに設定
    player_->SetMode(cameraMode_);
}

void GameScene::UpdateInput()
{
    // 入力ハンドラーの更新。カメラアニメーション再生中、デバッグカメラ操作中は入力をリセットし、操作を受け付けない
    if (animationController_->GetPlayState() != CameraAnimation::PlayState::PLAYING
#ifdef  _DEBUG
        && !Object3dBasic::GetInstance()->GetDebug()
#endif
        )
    {
        inputHandler_->Update();

    } else
    {
        inputHandler_->ResetInputs();
    }
}
