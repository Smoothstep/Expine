#pragma once

#include "RawResource.h"
#include "Scene.h"

namespace D3D
{
	class CResourceTracker : public CSingleton<CResourceTracker>
	{
		friend class RResource;
		friend class RShaderResourceView;
		friend class RRenderTargetView;
		friend class RUnorderedAccessView;
		friend class RDepthStencilView;

		struct KResourceLinks
		{
			inline KResourceLinks() = default;
			inline KResourceLinks
			(
				SharedPointer<RResource> & pResource
			)
			{
				Ensure(Resource = pResource);
			}

			SharedPointer<RResource>			Resource;
			SharedPointer<RShaderResourceView>	LinkSRV;
			SharedPointer<RRenderTargetView>	LinkRTV;
			SharedPointer<RUnorderedAccessView>	LinkUAV;
			SharedPointer<RDepthStencilView>	LinkDSV;
		};

	private:

		TMap<UINT64, KResourceLinks>	ResourceLinkMap;

	private:

		TVector<TPair<UINT64, UINT64> > ResourcesToReplace;

	protected:

		void AddResource
		(
			SharedPointer<RResource> & pResource
		);

		void ResourceToSRV
		(
			UINT64 ResourceIdx
		);

		void ResourceToRTV
		(
			UINT64 ResourceIdx
		);

		void ResourceToUAV
		(
			UINT64 ResourceIdx
		);

		void ResourceToDSV
		(
			UINT64 ResourceIdx
		);

	public:

		void PostReplaceAction
		(
			UINT64 ResourceIdx,
			UINT64 ResourceIdxNew
		);

		void ProceedOperations();
	};
}