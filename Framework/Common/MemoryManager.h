#ifndef _MEMORY_MANAGER_HPP_
#define _MEMORY_MANAGER_HPP_

#include "../Interface/IRuntimeModule.hpp"
#include "Allocator.hpp"
#include <new>

namespace GameEngine {
    class MemoryManager : implements IRuntimeModule{
        public:
            template<typename T, typename... Arguments>
            T* New(Arguments... parameters){
                return new (Allocator(sizeof(T))) T(parameters...);
            }

            template<typename T>
            void Delete(T* p){
                reinterpret_cast<T*>(p)->~T();
                Free(p, sizeof(T));
            }

        public:
            virtual ~MemoryManager(){}

            virtual int Initialize();
            virtual void Finalize();
            virtual void Tick();

            void* Allocate(size_t size);
            void Free(void* p, size_t size);

        private:
            static size_t* m_pBlockSizeLookup;
            static Allocator* m_pAllocators;
        private:
            static Allocator* LookupAllocator(size_t size);
    };
}

#endif