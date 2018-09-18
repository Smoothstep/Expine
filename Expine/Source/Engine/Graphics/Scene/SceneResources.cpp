#include "Precompiled.h"

#include "Scene/SceneResources.h"

namespace D3D
{
	void CResourceTracker::AddResource(SharedPointer<RResource>& pResource)
	{
		ResourceLinkMap.insert_or_assign(pResource->GetId(), pResource);
	}

	void CResourceTracker::ProceedOperations()
	{
		for (auto & ResourcePair : ResourcesToReplace)
		{
			auto Iter = ResourceLinkMap.find(ResourcePair.first);

			if (ResourcePair.second == -1)
			{
				ResourceLinkMap.erase(Iter);
			}
			else
			{
				KResourceLinks & ResourceReplace = ResourceLinkMap[ResourcePair.second];

				if (Iter->second.LinkSRV)
				{

				}
			}
		}

		ResourcesToReplace.clear();
	}

	void CResourceTracker::PostReplaceAction(UINT64 ResourceIdx, UINT64 ResourceIdxNew)
	{
		auto Iter = ResourceLinkMap.find(ResourceIdx);

		if (Iter == ResourceLinkMap.end())
		{
			return;
		}

		auto IterRP = ResourceLinkMap.find(ResourceIdxNew);

		if (IterRP != ResourceLinkMap.end())
		{
			const D3D12_RESOURCE_DESC & ResourceDesc	= Iter->second.Resource->GetResourceDesc();
			const D3D12_RESOURCE_DESC & ResourceDescRP	= IterRP->second.Resource->GetResourceDesc();

			if (
				ResourceDesc.Dimension	== ResourceDescRP.Dimension &&
				ResourceDesc.MipLevels	== ResourceDescRP.MipLevels &&
				ResourceDesc.Height		== ResourceDescRP.Height &&
				ResourceDesc.Width		== ResourceDescRP.Width)
			{
				return;
			}
		}

		ResourcesToReplace.push_back(std::make_pair(ResourceIdx, ResourceIdxNew));
	}
}
