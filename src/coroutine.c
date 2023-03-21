#include "coroutine.h"

#include <malloc.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct coroutine
{
	coroutine_proc *Proc;
	void           *Data;
	jmp_buf         At;
	void           *Stack;
	void           *StackAlloc;
	bool            Started;
	bool            Done;
	uint8_t         Padding[6];
};

struct engine
{
	size_t           CoroutineCount;
	struct coroutine Coroutine[];
};

static _Thread_local jmp_buf  EngineAt;
static _Thread_local jmp_buf *CoroutineAt;

static void
Continue(struct coroutine *E)
{
	if(E->Started == false)
	{
		/* NOTE: We need these to be global because we are moving our stack and
         * invalidating local variables and parameters. */
		static _Thread_local coroutine_proc *Proc;
		static _Thread_local void           *Data;
		E->Started  = true;
		CoroutineAt = &E->At;
		Proc        = E->Proc;
		Data        = E->Data;
		asm("movq %0, %%rsp " : : "rm"(E->Stack) :);
		Proc(Data);
		longjmp(EngineAt, 2);
	} else
	{
		CoroutineAt = &E->At;
		longjmp(*CoroutineAt, 1);
	}
}

static struct engine *
NewEngine(size_t MaxCoroutineCount)
{
	size_t Size =
	  sizeof(struct engine) + sizeof(struct coroutine) * MaxCoroutineCount;
	struct engine *E = malloc(Size);
	bzero(E, Size);
	return E;
}

static inline void
FreeCoroutine(struct coroutine *E)
{
	free(E->StackAlloc);
}

static void
FreeEngine(struct engine *E)
{
	for(size_t i = 0; i < E->CoroutineCount; ++i)
		FreeCoroutine(E->Coroutine + i);
	free(E);
}

static void
Add(struct engine *E, coroutine_proc *Proc, void *Data, size_t StackSize)
{
	struct coroutine *Co = E->Coroutine + E->CoroutineCount++;
	Co->StackAlloc       = malloc(StackSize);
	Co->Stack            = (uint8_t *)Co->StackAlloc + StackSize;
	Co->Proc             = Proc;
	Co->Data             = Data;
	Co->Started          = false;
}

static void
Start(struct engine *E)
{
	size_t i = 0;
	switch(setjmp(EngineAt))
	{
		case 0: break;
		case 1: /* coroutine yield */
			++i;
			if(i == E->CoroutineCount) i = 0;
			break;
		case 2: /* coroutine return */
			FreeCoroutine(E->Coroutine + i);
			E->Coroutine[i] = E->Coroutine[--E->CoroutineCount];
			if(i == E->CoroutineCount) i = 0;
			break;
		default: __builtin_unreachable();
	}
	if(E->CoroutineCount == 0) return;
	Continue(E->Coroutine + i);
}

/* Public interface */

void
CO_Yield()
{
	if(setjmp(*CoroutineAt) == 0) longjmp(EngineAt, 1);
}

static _Thread_local struct engine *Engine;

static void
FreeGlobalEngine(void)
{
	if(Engine != NULL) FreeEngine(Engine);
}

void
CO_Enable(size_t MaxCoroutineCount)
{
	if(Engine != NULL)
	{
		FreeEngine(Engine);
		atexit(FreeGlobalEngine);
	}
	Engine = NewEngine(MaxCoroutineCount);
}

void
CO_(coroutine_proc *Proc, void *Data, size_t StackSize)
{
	Add(Engine, Proc, Data, StackSize);
}

void
CO_Execute(void)
{
	Start(Engine);
}
