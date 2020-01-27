#include "BaseApplication.h"
#include "stdio.h"

int GameEngine::BaseApplication::Initialize(){
    m_IsQuit = false;
    return 0;
}

void GameEngine::BaseApplication::Finalize(){

}

void GameEngine::BaseApplication::Tick(){
    printf("Trick\n");
}

bool GameEngine::BaseApplication::IsQuit(){
    return m_IsQuit;
}