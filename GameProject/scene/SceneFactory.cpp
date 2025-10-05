#include "SceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"

#ifdef _DEBUG
#include "DebugUIManager.h"
#endif

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
  BaseScene* newScene = nullptr;

  if (sceneName == "title") {
    newScene = new TitleScene();
  } else if (sceneName == "game") {
    newScene = new GameScene();
  } else {
#ifdef _DEBUG
    DebugUIManager::GetInstance()->AddLog("Unknown scene name: " + sceneName, DebugUIManager::LogType::Error);
#endif
  }

  return newScene;
}
