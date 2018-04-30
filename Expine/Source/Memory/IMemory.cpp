#include "IMemory.h"

template<class T> volatile size_t MemoryObject<T>::AllocatedMemorySize = 0;