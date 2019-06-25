#ifndef _IRUNTIMEMODULE_H_
#define _IRUNTIMEMODULE_H_

#include "Interface.hpp"

namespace GameEngine {
	Interface IRuntimeModule{
		public:
			virtual ~IRuntimeModule() {};

			virtual int Initialize() = 0;

			virtual void Finalize() = 0;

			virtual void Tick() = 0;
	};
}

#endif // !_IRUNTIMEMODULE_H_