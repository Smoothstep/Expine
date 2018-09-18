#include "Precompiled.h"

#include "PostProcess/PostProcessBloom.h"
#include "Pipeline/PSOPostProcess.h"

namespace D3D
{
	namespace PostProcess
	{
		using namespace Pipelines::PostProcess::Bloom;

		ErrorCode CPostProcessBloom::Create(const RShaderResourceView & Color, const RShaderResourceView & Overlay, const RShaderResourceView & Combined)
		{
			ErrorCode Error;

			Properties.Threshold = 1.0f;

			const DescriptorHeapRange RangesSource[] =
			{
				Color.GetDescriptorHeapEntry(), Overlay.GetDescriptorHeapEntry(), Combined.GetDescriptorHeapEntry()
			};

			if (!DescriptorHeapSRV)
			{
				DescriptorHeapSRV = new RDescriptorHeap();

				if ((Error = DescriptorHeapSRV->Create_CBV_SRV_UAV(_countof(RangesSource), D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
				{
					return Error;
				}
			}

			RDescriptorHeap::CopyInto<_countof(RangesSource), 1>
			(
				RangesSource, &DescriptorHeapSRV->AsDescriptorHeapRange()
			);

			return S_OK;
		}

		void CPostProcessBloom::Process(const PassContext & Context, const PostProcessSettings & Settings, const PostProcessTarget & Target)
		{
			const CCommandListContext & CmdListCtx = Context.CommandListCtx.GetRef();

			RDescriptorHeap::CopyInto<1, 1>
			(
				&DescriptorHeapRange(DescriptorHeapSRV->AsDescriptorHeapRange()),
				&DescriptorHeapRange(
					Context.DescriptorHeapRangeSRV.DescriptorHeap, 
					Context.DescriptorHeapRangeSRV.Offset, 
					DescriptorHeapSRV->GetDescriptorsCount())
			);

			PipelineSetup::Instance().Apply(Context.CommandListCtx.Get());
			{
				CmdListCtx.SetGraphicsRootDescriptorTable(PipelineSetup::BloomInput,		Context.DescriptorHeapRangeSRV);
				CmdListCtx.SetGraphicsRootDescriptorTable(PipelineSetup::BloomPostProcess,	Context.DescriptorHeapRangeCBV);

				const Vector4f BloomThresholdValue(Properties.Threshold, 0, 0, CPostProcess::ComputeExposureScaleValue(Settings));
				{
					CmdListCtx.SetGraphicsRoot32BitConstants(PipelineSetup::BloomThreshold, 4, &BloomThresholdValue);
				}
			}

			CmdListCtx.SetViewport(Target.ViewPort);
			CmdListCtx.SetScissorRect(Target.ViewRect);
			CmdListCtx.SetRenderTarget(Target.RenderTarget.GetRef());
			CmdListCtx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CmdListCtx.DrawInstanced(1, 3);

			PipelineOverlay::Instance().Apply(Context.CommandListCtx.Get());
			{
				CmdListCtx.SetGraphicsRootDescriptorTable(PipelineOverlay::BloomInput,			Context.DescriptorHeapRangeSRV);
				CmdListCtx.SetGraphicsRootDescriptorTable(PipelineOverlay::BloomPostProcess,	Context.DescriptorHeapRangeCBV);

				const uint32_t ScaleFactor = Math::DivideAndRoundUp(Target.SizeDestination.Y, Target.SizeSource.Y);

				D3D12_RECT SrcRect	= Target.ViewRect;
				{
					SrcRect.bottom	/= ScaleFactor;
					SrcRect.left	/= ScaleFactor;
					SrcRect.right	/= ScaleFactor;
					SrcRect.top		/= ScaleFactor;
				}

				D3D12_RECT DestRect = SrcRect;

				Vector4f ColorScale(Properties.Intensity, Properties.Intensity, Properties.Intensity, 0);
				{
					CmdListCtx.SetGraphicsRoot32BitConstants(PipelineOverlay::BloomColorScale, 4, &ColorScale);
				}
			}

			CmdListCtx.DrawInstanced(1, 3);
		}
	}
}
