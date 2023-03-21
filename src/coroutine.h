#if !defined COROUTINE_H
#	define COROUTINE_H

#	include <stddef.h>

#	define COROUTINE(x) void x(void *Data)
typedef COROUTINE(coroutine_proc);

void CO_Enable(size_t MaxCoroutineCount);
void CO_(coroutine_proc *, void *Data, size_t StackSize);
static inline void
CO(coroutine_proc *Proc, void *Data)
{
	CO_(Proc, Data, 8192);
}
void CO_Execute(void);
void CO_Yield(void);

#endif
