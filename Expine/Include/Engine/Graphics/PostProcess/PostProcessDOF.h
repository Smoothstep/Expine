#pragma once

#include "PostProcess.h"

namespace D3D
{
	namespace PostProcess
	{
		class CPostProcessDOF
		{
		private:
			
			SharedPointer<RRenderTargetView>	BlurSRV;
			SharedPointer<RShaderResourceView>	BlurRTV;

		public:

			ErrorCode Create
			(
				const IntPoint & BlurSize
			);

			void ProcessBlur
			(
				const PassContext		& PassContext,
				const PostProcessTarget & Target
			)
			{

			}

			void Process
			(
				const PostProcessTarget & Target
			)
			{

			}
		};
	}
}