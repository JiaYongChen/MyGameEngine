#include "../Framework/Common/BaseApplication.hpp"

namespace MyGame {
    BaseApplication g_baseApplication;
    IApplication *g_pApplication = &g_baseApplication;
}