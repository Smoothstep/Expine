#include "Hyper/SSE.h"

namespace SSE
{
#define SIGN_BIT ((1 << 31))

	const __m128 SignBit = MakeVectorRegister
	(
		(unsigned long)SIGN_BIT, 
		(unsigned long)SIGN_BIT, 
		(unsigned long)SIGN_BIT, 
		(unsigned long)SIGN_BIT
	);

	const __m128 SignMask = MakeVectorRegister
	(
		(unsigned long)(~SIGN_BIT), 
		(unsigned long)(~SIGN_BIT), 
		(unsigned long)(~SIGN_BIT), 
		(unsigned long)(~SIGN_BIT)
	);

	const __m128 XYZMask = MakeVectorRegister
	(
		0xffffffffUL, 
		0xffffffffUL, 
		0xffffffffUL, 
		0x00000000UL
	);

	const __m128 FloatOne = MakeVectorRegister
	(
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	const __m128 FloatOneHalf = MakeVectorRegister
	(
		0.5f,
		0.5f,
		0.5f,
		0.5f
	);

	const __m128 Float111_Minus1 = MakeVectorRegister
	(
		 1.0f, 
		 1.0f, 
		 1.0f, 
		-1.0f
	);

	const __m128 Float0001 = MakeVectorRegister
	(
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);

	const __m128 FloatMinus1_111 = MakeVectorRegister
	(
		-1.0f, 
		 1.0f, 
		 1.0f, 
		 1.0f
	);

	const __m128 SmallLengthThreshold = MakeVectorRegister
	(
		1.e-8f, 
		1.e-8f,
		1.e-8f,
		1.e-8f
	);
}