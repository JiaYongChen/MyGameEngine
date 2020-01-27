#ifndef _IAPPLICATION_H_
#define _IAPPLICATION_H_

#include "Interface.h"
#include "IRuntimeModule.h"

namespace GameEngine {
    Interface IApplication : implements IRuntimeModule{
        public:
            virtual ~IApplication(){}

            virtual int Initialize() = 0;

            virtual void Finalize() = 0;

            virtual void Tick() = 0;

            virtual bool IsQuit() = 0;
    };
}

#endif // !_IAPPLICATION_H_