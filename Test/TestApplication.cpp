#include "../Framework/Common/BaseApplication.hpp"

namespace GameEngine {
    BaseApplication g_baseApplication;
    IApplication *g_pApplication = &g_baseApplication;
}