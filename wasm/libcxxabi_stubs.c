#include <stdlib.h> 

// -fno-exceptions is not an option with libsass yet,
// stubbing libc++abi functions

void __cxa_throw(void *thrown_exception, void *tinfo,
                 void (*dest)(void *))
{
    abort();
}

void *__cxa_allocate_exception(size_t thrown_size)
{
    abort();
}

void __cxa_rethrow()
{
    abort();
}

void* __cxa_begin_catch(void* exceptionObject)
{
    abort();
}
