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
#include "../Object/Projectile/BossBullet.h"
#include "GlobalVariables.h"
#include "Vec3Func.h"
#include <algorithm>
#include <cmath>

#include "Object/Player/State/PlayerState.h"
#include "Object/Player/State/PlayerStateMachine.h"

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
    ShadowRenderer::GetInstance()->SetMaxShadowDistance(kShadowMaxDistance);
    // 平行光源の方向の設定
    Object3dBasic::GetInstance()->SetDirectionalLightDirection(Vector3(0.0f, -1.0f, kDirectionalLightZ));

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
    ground_->SetEnableHighlight(false);

    // Input Handlerの初期化
    inputHandler_ = std::make_unique<InputHandler>();
    inputHandler_->Initialize();

    //-----------Playerの初期化----------------//
    player_ = std::make_unique<Player>();
    player_->Initialize();
    player_->SetCamera((*Object3dBasic::GetInstance()->GetCamera()));
    player_->SetInputHandler(inputHandler_.get());

    //-----------Bossの初期化--------------------//
    boss_ = std::make_unique<Boss>();
    boss_->Initialize();
    // ボスにプレイヤーの参照を設定
    boss_->SetPlayer(player_.get());
    // ゲーム開始時の演出が終わるまで一時停止状態に設定
    boss_->SetIsPause(true);

    // プレイヤーにボスの参照を設定
    player_->SetBoss(boss_.get());

    // カメラマネージャーの初期化
    cameraManager_ = CameraManager::GetInstance();
    cameraManager_->Initialize((*Object3dBasic::GetInstance()->GetCamera()));

    // ThirdPersonControllerを登録
    auto tpController = std::make_unique<ThirdPersonController>();
    firstPersonController_ = tpController.get();
    firstPersonController_->SetTarget(&player_->GetTransform());
    // ボスをセカンダリターゲットとして設定し、注視機能を有効化
    firstPersonController_->SetSecondaryTarget(&boss_->GetTransform());
    firstPersonController_->EnableLookAtTarget(true);
    cameraManager_->RegisterController("ThirdPerson", std::move(tpController));

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
    animationController_->LoadAnimationFromFile("game_start");
    cameraManager_->ActivateController("Animation");
    animationController_->SwitchAnimation("game_start");
    animationController_->Play();

    // オーバー演出アニメーションの読み込みと設定
    animationController_->LoadAnimationFromFile("over_anim");
    animationController_->SetAnimationTargetByName("over_anim", player_->GetTransformPtr());

    // クリア演出アニメーションの読み込みと設定
    animationController_->LoadAnimationFromFile("clear_anim");
    animationController_->SetAnimationTargetByName("clear_anim", boss_->GetTransformPtr());

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

    // シーンのエミッターをまとめて読み込む
    emitterManager_->LoadScenePreset("gamescene_preset");

    // ボスフェーズ2用の境界線初期状態は無効化（ボスフェーズ2まで非表示）
    emitterManager_->SetEmitterActive("boss_border_left", false);
    emitterManager_->SetEmitterActive("boss_border_right", false);
    emitterManager_->SetEmitterActive("boss_border_front", false);
    emitterManager_->SetEmitterActive("boss_border_back", false);

    // ダッシュエフェクトパラメータの登録
    GlobalVariables* gv = GlobalVariables::GetInstance();
    gv->CreateGroup("DashEffect");
    gv->AddItem("DashEffect", "LerpSpeed", 35.0f);

    // ダッシュエミッター位置を初期化
    dashEmitterPosition_ = player_->GetTranslate();
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

    //if (Input::GetInstance()->TriggerKey(DIK_O)) {
    //    StartOverAnim();
    //}

    //if (Input::GetInstance()->TriggerKey(DIK_P)) {
    //    StartClearAnim();
    //}
#endif

    // ゲーム開始演出終了後、ボスの一時停止を解除
    if (animationController_->GetPlayState() != CameraAnimation::PlayState::PLAYING && !isStart_) {
        isStart_ = true;
        boss_->SetIsPause(false);
    }

    // ゲームクリア判定
    if (boss_->IsDead()) {
        StartClearAnim();
        //SceneManager::GetInstance()->ChangeScene("clear", "Fade", 0.3f);
    }

    // ゲームオーバー判定
    if (player_->IsDead()) {
        StartOverAnim();
    }

    // カメラモードの更新
    UpdateCameraMode();

    // 入力の更新
    UpdateInput();

    // オブジェクトの更新処理
    skyBox_->Update();
    ground_->Update();
    player_->Update();
    boss_->Update(FrameTimer::GetInstance()->GetDeltaTime());
    toTitleSprite_->Update();
    cameraManager_->Update(FrameTimer::GetInstance()->GetDeltaTime());

    // ボスからの弾生成リクエストを処理
    CreateBossBullet();

    // プロジェクタイルの更新
    float deltaTime = FrameTimer::GetInstance()->GetDeltaTime();
    UpdateProjectiles(deltaTime);

    // プレイヤーの位置にオーバー演出エミッターをセット
    emitterManager_->SetEmitterPosition("over1", player_->GetTranslate());
    emitterManager_->SetEmitterPosition("over2", player_->GetTranslate());

    // ダッシュエミッターのLerp補間処理
    UpdateDashEmitter(deltaTime);

    // ボスの位置にクリア演出エミッターをセット
    emitterManager_->SetEmitterPosition("clear_slash", boss_->GetTranslate());

    UpdateBossBorder();

    // エミッターマネージャーの更新
    emitterManager_->Update();

    // ゲームオーバーアニメーションの更新
    UpdateOverAnim();

    // ゲームクリアアニメーションの更新
    UpdateClearAnim();

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
    if (ShadowRenderer::GetInstance()->IsEnabled()) {
        ShadowRenderer::GetInstance()->BeginShadowPass();
        ground_->Draw();
        player_->Draw();
        boss_->Draw();

        // ボスの弾のシャドウ
        //for (const auto& bullet : bossBullets_) {
        //    if (bullet && bullet->IsActive()) {
        //        bullet->Draw();
        //    }
        //}

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

    // ボスの弾を描画
    for (const auto& bullet : bossBullets_) {
        if (bullet && bullet->IsActive()) {
            bullet->Draw();
        }
    }

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

    player_->DrawSprite();
    boss_->DrawSprite();
}

void GameScene::DrawImGui()
{
#ifdef _DEBUG

#endif // DEBUG
}

void GameScene::StartOverAnim()
{
    if (isOver_) return;

    cameraManager_->DeactivateAllControllers();
    cameraManager_->ActivateController("Animation");
    animationController_->SwitchAnimation("over_anim");
    animationController_->Play();
    isOver_ = true;
}

void GameScene::UpdateOverAnim()
{
    // オーバーアニメーションタイマーの更新
    if (isOver_) overAnimTimer_ += FrameTimer::GetInstance()->GetDeltaTime();

    // オーバーアニメーション中のエミッター制御
    if (overAnimTimer_ > kOverEmit1Time && !isOver1Emit_) {
        emitterManager_->CreateTemporaryEmitterFrom("over1", "over1_temp", 0.5f);
        isOver1Emit_ = true;
    }

    if (overAnimTimer_ > kOverEmit2Time && !isOver2Emit_) {
        emitterManager_->CreateTemporaryEmitterFrom("over2", "over2_temp", 0.1f);
        isOver2Emit_ = true;
    }

    // プレイヤースケールの減少
    if (isOver2Emit_) {
        Vector3 newScale = player_->GetScale() - Vector3(kScaleDecreaseRate, kScaleDecreaseRate, kScaleDecreaseRate) * FrameTimer::GetInstance()->GetDeltaTime();
        newScale.x = std::max<float>(newScale.x, 0.0f);
        newScale.y = std::max<float>(newScale.y, 0.0f);
        newScale.z = std::max<float>(newScale.z, 0.0f);
        player_->SetScale(newScale);
    }

    // シーン遷移
    if (overAnimTimer_ > kOverTotalTime) {
        SceneManager::GetInstance()->ChangeScene("over", "Fade", 0.3f);
    }
}

void GameScene::StartClearAnim()
{
    if (isClear_) return;

    cameraManager_->DeactivateAllControllers();
    cameraManager_->ActivateController("Animation");
    animationController_->SwitchAnimation("clear_anim");
    animationController_->Play();
    boss_->SetIsPause(true);
    player_->SetScale(Vector3(0.f, 0.f, 0.f)); // プレイヤーを非表示にするためスケールを0に設定
    isClear_ = true;
}

void GameScene::UpdateClearAnim()
{
    // オーバーアニメーションタイマーの更新
    if (isClear_) clearAnimTimer_ += FrameTimer::GetInstance()->GetDeltaTime();

    // オーバーアニメーション中のエミッター制御
    if (clearAnimTimer_ > 0.5f && !isClear1Emit_) {
        emitterManager_->SetEmitterActive("clear_slash", true);
        emitterManager_->SetEmitterCount("clear_slash", currentSlashCount_);
        emitterManager_->SetEmitterRadius("clear_slash", currentSlashRadius_);

        if (currentSlashCount_ < kSlashEmitterMaxCount_ || currentSlashRadius_ < kSlashEmitterMaxRadius_) {
            currentSlashCount_ += 1;
            currentSlashRadius_ += 0.05f;
        }
        else {
            emitterManager_->SetEmitterActive("clear_slash", false);
            isClear1Emit_ = true;
        }
    }

    if (isClear1Emit_ && !isClear2Emit_) {
        emitterManager_->SetEmitterPosition("over2", boss_->GetTranslate());
        emitterManager_->CreateTemporaryEmitterFrom("over2", "over2_temp", 0.1f);
        isClear2Emit_ = true;
    }

    // ボススケールの減少
    if (isClear2Emit_) {
        Vector3 newScale = boss_->GetScale() - Vector3(kScaleDecreaseRate, kScaleDecreaseRate, kScaleDecreaseRate) * FrameTimer::GetInstance()->GetDeltaTime();
        newScale.x = std::max<float>(newScale.x, 0.0f);
        newScale.y = std::max<float>(newScale.y, 0.0f);
        newScale.z = std::max<float>(newScale.z, 0.0f);
        boss_->SetScale(newScale);
    }

    // シーン遷移
    if (boss_->GetScale().x <= 0.f) {
        SceneManager::GetInstance()->ChangeScene("clear", "Fade", 0.3f);
    }
}

void GameScene::UpdateCameraMode()
{
    if (player_->IsDead() || boss_->IsDead() || !isStart_) {
        return;
    }

    if (boss_->GetPhase() == 1) {
        cameraMode_ = false;
        // フェーズ1: 動的制限を解除（ステージ全体を移動可能）
        player_->ClearDynamicBounds();
    }
    else if (boss_->GetPhase() == 2) {
        cameraMode_ = true;
        // フェーズ2: ボス中心の戦闘エリアに移動制限
        Vector3 bossPos = boss_->GetTransform().translate;
        player_->SetDynamicBoundsFromCenter(bossPos, battleAreaSize_, battleAreaSize_);
    }

    if (cameraMode_) {
        cameraManager_->ActivateController("ThirdPerson");
    }
    else {
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
        ) {
        inputHandler_->Update();

    }
    else {
        inputHandler_->ResetInputs();
    }
}

void GameScene::UpdateProjectiles(float deltaTime)
{
    // ボスの弾の更新
    for (auto& bullet : bossBullets_) {
        if (bullet && bullet->IsActive()) {
            bullet->Update(deltaTime);
        }
    }

    // 非アクティブなボスの弾を削除
    bossBullets_.erase(
        std::remove_if(bossBullets_.begin(), bossBullets_.end(),
            [](const std::unique_ptr<BossBullet>& bullet) {
                if (bullet && !bullet->IsActive()) {
                    // Finalize()で自動的にコライダーが削除される
                    bullet->Finalize();
                    return true;
                }
                return false;
            }),
        bossBullets_.end()
    );
}

void GameScene::UpdateBossBorder()
{
    // ボスフェーズ2の境界線パーティクル制御
    if (boss_) {
        bool shouldShowBorder = (boss_->GetPhase() == 2);

        if (shouldShowBorder && !borderEmittersActive_) {
            Vector3 bossPos = boss_->GetTransform().translate;

            // フェーズ2突入時：境界線を有効化
            emitterManager_->SetEmitterActive("boss_border_left", true);
            emitterManager_->SetEmitterActive("boss_border_right", true);
            emitterManager_->SetEmitterActive("boss_border_front", true);
            emitterManager_->SetEmitterActive("boss_border_back", true);

            borderEmittersActive_ = true;
        }
        else if (!shouldShowBorder && borderEmittersActive_) {
            // フェーズ1に戻った時：境界線を無効化
            emitterManager_->SetEmitterActive("boss_border_left", false);
            emitterManager_->SetEmitterActive("boss_border_right", false);
            emitterManager_->SetEmitterActive("boss_border_front", false);
            emitterManager_->SetEmitterActive("boss_border_back", false);

            borderEmittersActive_ = false;
        }

        if (borderEmittersActive_) {
            // フェーズ2継続中：ボスの移動に追従
            Vector3 bossPos = Vector3(boss_->GetTransform().translate.x, 0.f, boss_->GetTransform().translate.z);

            emitterManager_->SetEmitterPosition("boss_border_left",
                bossPos + Vector3(0.0f, 0.0f, -battleAreaSize_));
            emitterManager_->SetEmitterPosition("boss_border_right",
                bossPos + Vector3(0.0f, 0.0f, battleAreaSize_));
            emitterManager_->SetEmitterPosition("boss_border_front",
                bossPos + Vector3(-battleAreaSize_, 0.0f, 0.0f));
            emitterManager_->SetEmitterPosition("boss_border_back",
                bossPos + Vector3(battleAreaSize_, 0.0f, 0.0f));
        }
    }
}

void GameScene::CreateBossBullet()
{
    for (const auto& request : boss_->ConsumePendingBullets()) {
        auto bullet = std::make_unique<BossBullet>(emitterManager_.get());
        bullet->Initialize(request.position, request.velocity);
        bossBullets_.push_back(std::move(bullet));
    }
}

void GameScene::UpdateDashEmitter(float deltaTime)
{
    // ダッシュ状態の判定
    bool isDashing = false;
    if (player_ && player_->GetStateMachine() && player_->GetStateMachine()->GetCurrentState()) {
        isDashing = (player_->GetStateMachine()->GetCurrentState()->GetName() == "Dash");
    }

    // ダッシュ開始時: エミッター有効化 & 位置リセット
    if (isDashing && !previousIsDashing_) {
        emitterManager_->SetEmitterActive("player_dash", true);
        dashEmitterActive_ = true;
        dashEmitterPosition_ = player_->GetTranslate();
    }

    // エミッターがアクティブな間は補間を継続（ダッシュ終了後も追いつくまで続ける）
    if (dashEmitterActive_) {
        // GlobalVariablesから補間速度を取得
        float lerpSpeed = GlobalVariables::GetInstance()->GetValueFloat("DashEffect", "LerpSpeed");

        // フレームレート非依存の指数減衰補間
        // t = 1 - e^(-speed * dt) で、どのFPSでも同じ視覚的結果
        float t = 1.0f - std::exp(-lerpSpeed * deltaTime);

        // エミッター位置をプレイヤー位置に向かって補間
        dashEmitterPosition_ = Vec3::Lerp(dashEmitterPosition_, player_->GetTranslate(), t);

        // エミッター位置を更新
        emitterManager_->SetEmitterPosition("player_dash", dashEmitterPosition_);

        // ダッシュ終了後、エミッターがプレイヤー位置に十分近づいたら無効化
        if (!isDashing) {
            Vector3 diff = player_->GetTranslate() - dashEmitterPosition_;
            float distanceSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

            if (distanceSquared < kDashEmitterThreshold * kDashEmitterThreshold) {
                emitterManager_->SetEmitterActive("player_dash", false);
                dashEmitterActive_ = false;
            }
        }
    }

    // 状態を保存
    previousIsDashing_ = isDashing;
}