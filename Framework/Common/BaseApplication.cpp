#include "BaseApplication.hpp"

int MyGame::BaseApplication::Initialize(){
    m_IsQuit = false;
    return 0;
}

void MyGame::BaseApplication::Finalize(){

}

void MyGame::BaseApplication::Tick(){

}

bool MyGame::BaseApplication::IsQuit(){
    return m_IsQuit;
}