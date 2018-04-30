#pragma once

#include "PostProcess.h"

namespace D3D
{
	class CSceneRenderer;

	namespace PostProcess
	{
		struct BloomProperties
		{
			Float	Threshold;
			Float	Intensity;
			Float4	Tint;
			Float4	Size;
		};

		class CPostProcess;
		class CPostProcessBloom
		{
		private:

			BloomProperties Properties;

		private:

			SharedPointer<RDescriptorHeap> DescriptorHeapSRV;

		public:

			inline BloomProperties & GetProperties()
			{
				return Properties;
			}

			inline void SetProperties
			(
				const BloomProperties & Properties
			)
			{
				this->Properties = Properties;
			}

			ErrorCode Create
			(
				const RShaderResourceView & Color,
				const RShaderResourceView & Overlay,
				const RShaderResourceView & Combined
			);

			void Process
			(
				const PassContext			& Context,
				const PostProcessSettings	& Settings,
				const PostProcessTarget		& Target
			);
		};
	}
}