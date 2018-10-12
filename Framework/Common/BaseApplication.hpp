#ifndef _BASEAPPLICATION_H_
#define _BASEAPPLICATION_H_

#include "../Interface/IApplication.hpp"

namespace MyGame {
    class BaseApplication : implements IApplication{
        public:
            ~BaseApplication(){}

            virtual int Initialize();

            virtual void Finalize();

            virtual void Tick();

            virtual bool IsQuit();

        private:
            bool m_IsQuit;
    };
}

#endif // !_BASEAPPLICATION_H_