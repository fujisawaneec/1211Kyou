#include "TitleScene.h"

#include "SceneManager.h"
#include "TextureManager.h"
#include "Object3dBasic.h"
#include "SpriteBasic.h"
#include "ModelManager.h"
#include "Input.h"
#include "Draw2D.h"
#include "GlobalVariables.h"
#include "GPUParticle.h"

#ifdef _DEBUG
#include"ImGui.h"
#include "DebugCamera.h"
#include "DebugUIManager.h"
#endif

void TitleScene::Initialize()
{
  // エミッタマネージャ生成
  emitterManager_ = std::make_unique<EmitterManager>(GPUParticle::GetInstance());

#ifdef _DEBUG
	DebugCamera::GetInstance()->Initialize();
  Object3dBasic::GetInstance()->SetDebug(false);
  Draw2D::GetInstance()->SetDebug(false);
  GPUParticle::GetInstance()->SetIsDebug(false);

  DebugUIManager::GetInstance()->RegisterGameObject("BackGround",
    [this]() { if (titleBG_) titleBG_->DrawImGui(); });

  DebugUIManager::GetInstance()->RegisterGameObject("TitleText",
    [this]() { if (titleText_) titleText_->DrawImGui(); });

  DebugUIManager::GetInstance()->RegisterGameObject("StartButtonText",
    [this]() { if (startButtonText_) startButtonText_->DrawImGui(); });

  DebugUIManager::GetInstance()->SetEmitterManager(emitterManager_.get());
#endif

	/// ================================== ///
	///              初期化処理              ///
	/// ================================== ///

  // 背景画像の初期化
  titleBG_ = make_unique<Sprite>();
  titleBG_->Initialize("black.png");
  titleBG_->SetPos(Vector2(0.f, 0.f));
  titleBG_->SetSize(Vector2(static_cast<float>(WinApp::clientWidth), static_cast<float>(WinApp::clientHeight)));

  // タイトルテキストの初期化
  titleText_ = make_unique<Sprite>();
  titleText_->Initialize("title_text/title_text_1.png");
  titleText_->SetSize(Vector2(500.f, 200.f));
  titleText_->SetPos(Vector2(WinApp::clientWidth / 2.f - titleText_->GetSize().x / 2.f, 100.f));

  // スタートボタンテキストの初期化
  startButtonText_ = make_unique<Sprite>();
  startButtonText_->Initialize("title_button.png");
  startButtonText_->SetPos(Vector2(WinApp::clientWidth / 2.f - startButtonText_->GetSize().x / 2.f, WinApp::clientHeight - 250.f));

  emitterManager_->LoadEmittersFromJSON("title_preset");

}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
	/// ================================== ///
	///              更新処理               ///
	/// ================================== ///

  titleBG_->SetSize(Vector2(static_cast<float>(WinApp::clientWidth), static_cast<float>(WinApp::clientHeight)));
  titleText_->SetPos(Vector2(WinApp::clientWidth / 2.f - titleText_->GetSize().x / 2.f, 100.f));
  startButtonText_->SetPos(Vector2(WinApp::clientWidth / 2.f - startButtonText_->GetSize().x / 2.f, WinApp::clientHeight - 250.f));

  titleBG_->Update();
  titleText_->Update();
  startButtonText_->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
    Input::GetInstance()->TriggerButton(XButtons.A))
	{
		SceneManager::GetInstance()->ChangeScene("game");
	}
}

void TitleScene::Draw()
{
	/// ================================== ///
	///              描画処理               ///
	/// ================================== ///

	//------------------背景Spriteの描画------------------//
	// スプライト共通描画設定
	SpriteBasic::GetInstance()->SetCommonRenderSetting();

  titleBG_->Draw();


	//-------------------Modelの描画-------------------//
	// 3Dモデル共通描画設定
	Object3dBasic::GetInstance()->SetCommonRenderSetting();




	//------------------前景Spriteの描画------------------//
	// スプライト共通描画設定
	SpriteBasic::GetInstance()->SetCommonRenderSetting();

  titleText_->Draw();
  startButtonText_->Draw();

}

void TitleScene::DrawWithoutEffect()
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

void TitleScene::DrawImGui()
{
#ifdef _DEBUG

	/// ================================== ///
	///             ImGuiの描画              ///
	/// ================================== ///


#endif // _DEBUG
}