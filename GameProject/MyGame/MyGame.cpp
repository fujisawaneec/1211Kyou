#include "MyGame.h"
#include"Audio.h"
#include"Input.h"
#include "scene/SceneFactory.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "Draw2D.h"
#include "Object3dBasic.h"
#include "PostEffectManager.h"
#include "FrameTimer.h"
#include "GlobalVariables.h"
#include "ModelManager.h"
#include "GPUParticle.h"
#include "SpriteBasic.h"
#include "TransitionManager.h"

void MyGame::Initialize()
{
  winApp_->SetWindowSize(1920, 1080);

  winApp_->SetWindowTitle(L"LE3B_12_キョウ_ゲンソ_Slash");

  TakoFramework::Initialize();

#pragma region 汎用機能初期化-------------------------------------------------------------------------------------------------------------------
  // 入力クラスの初期化
  Input::GetInstance()->Initialize(winApp_);

  // オーディオの初期化
  Audio::GetInstance()->Initialize("resources/Sound/");

#pragma endregion

  // シーンの初期化
  sceneFactory_ = new SceneFactory();
  SceneManager::GetInstance()->SetSceneFactory(sceneFactory_);
  SceneManager::GetInstance()->ChangeScene("title", 0.0f);

  TextureManager::GetInstance()->LoadTexture("white.png");
  TextureManager::GetInstance()->LoadTexture("black.png");
  TextureManager::GetInstance()->LoadTexture("circle.png");
  TextureManager::GetInstance()->LoadTexture("my_skybox.dds");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_1.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_2.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_3.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_4.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_5.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_6.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_7.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_8.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_9.png");
  TextureManager::GetInstance()->LoadTexture("title_text/title_text_10.png");
  TextureManager::GetInstance()->LoadTexture("title_button.png");
  TextureManager::GetInstance()->LoadTexture("game_button_text.png");
  TextureManager::GetInstance()->LoadTexture("gameClear_Text.png");
  TextureManager::GetInstance()->LoadTexture("gameOver_Text.png");

  spriteBasicOnresizeId_ = winApp_->RegisterOnResizeFunc(std::bind(&SpriteBasic::OnResize, SpriteBasic::GetInstance(), std::placeholders::_1));

  // GPUパーティクルの初期化
  GPUParticle::GetInstance()->Initialize(dx12_, defaultCamera_);
}

void MyGame::Finalize()
{
  winApp_->UnregisterOnResizeFunc(spriteBasicOnresizeId_);

  // GPUパーティクルの解放
  GPUParticle::GetInstance()->Finalize();

  // Audioの解放
  Audio::GetInstance()->Finalize();

  // 入力クラスの解放
  Input::GetInstance()->Finalize();

  TakoFramework::Finalize();
}

void MyGame::Update()
{
  // カメラの更新
  defaultCamera_->Update();

  // 入力情報の更新
  Input::GetInstance()->Update();

  // F11キーでフルスクリーン切り替え
  if (Input::GetInstance()->TriggerKey(DIK_F11))
  {
    ToggleFullScreen();
  }

  // GPUパーティクルの更新
  GPUParticle::GetInstance()->Update();

  TakoFramework::Update();

  //　サウンドの更新
  Audio::GetInstance()->Update();

  // ゲームパッドの状態をリフレッシュ
  Input::GetInstance()->RefreshGamePadState();
}

void MyGame::Draw()
{
  /// ============================================= ///
  /// ------------------シーン描画-------------------///
  /// ============================================= ///

  //ポストエフェクト適用対象のレンダーテクスチャを描画先に設定
  dx12_->SetEffectRenderTexture();

  // テクスチャ用のsrvヒープの設定
  SrvManager::GetInstance()->BeginDraw();

  SceneManager::GetInstance()->Draw();

  GPUParticle::GetInstance()->Draw();

  Draw2D::GetInstance()->Draw();

  /// ===================================================== ///
  /// ------------------ポストエフェクト描画-------------------///
  /// ===================================================== ///

    // ポストエフェクトの描画
  PostEffectManager::GetInstance()->Draw();

  /// ===================================================== ///
  /// ------------ポストエフェクト非適用対象の描画---------------///
  /// ===================================================== ///
  // ポストエフェクト非適用対象のレンダーテクスチャを描画先に設定
  dx12_->SetNonEffectRenderTexture();

  // シーンの描画
  SceneManager::GetInstance()->DrawWithoutEffect();

  TransitionManager::GetInstance()->Draw();

  Draw2D::GetInstance()->Reset();

  /// ============================================= ///
  /// ---------最終結果をスワップチェーンに描画---------///
  /// ============================================= ///
  bool isDrawToSwapChain = true;

#ifdef _DEBUG
  isDrawToSwapChain = !DebugUIManager::GetInstance()->IsWindowVisible("GameViewport");
#endif

  PostEffectManager::GetInstance()->DrawFinalResult(isDrawToSwapChain);


  /// ========================================= ///
  ///-------------------ImGui-------------------///
  /// ========================================= ///
#ifdef _DEBUG

  imguiManager_->Begin();

  TakoFramework::Draw();

  //SceneManager::GetInstance()->DrawImGui();

  Draw2D::GetInstance()->ImGui();

  imguiManager_->End();

  //imguiの描画
  imguiManager_->Draw();
#endif


  // 描画後の処理
  dx12_->EndDraw();
}
