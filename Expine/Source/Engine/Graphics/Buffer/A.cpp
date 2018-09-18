#include "Buffer/Buffer.h"

void iNx()
{
	printf("");
}

static int NotLikeThis = []() -> int {
	std::terminate();
	return 0;
}();