#include <coroutine.h>
#include <stdio.h>

static COROUTINE(Coroutine2)
{
	char const *Data_ = Data;
	printf("\t\t%s\n", Data_);
	CO_Yield();
	printf("\t\t%s\n", Data_);
}

static COROUTINE(Coroutine1)
{
	char const *Data_ = Data;
	printf("\t%s\n", Data_);
	CO_Yield();
	printf("\t%s\n", Data_);
	CO(Coroutine2, Data);
	CO_Yield();
	printf("\t%s\n", Data_);
}

static COROUTINE(Coroutine0)
{
	char const *Data_ = Data;
	printf("%s\n", Data_);
	CO_Yield();
	printf("%s\n", Data_);
	CO(Coroutine1, Data);
	CO_Yield();
	printf("%s\n", Data_);
}

int
main(void)
{
	CO_Enable(32);
	CO(Coroutine0, "a");
	CO(Coroutine1, "b");
	CO(Coroutine2, "c");
	CO_Execute();
	CO(Coroutine2, "d");
	CO_Execute();
}
