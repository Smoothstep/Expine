#include "Precompiled.h"

#include "Scene/Outdoor/TerrainQuadTree.h"
#include "Utils/Routine/Pool.h"

namespace D3D
{
	namespace Terrain
	{
		static constexpr unsigned WorkerThreadCount = 4;

		static inline IntPoint GetPreferableSize(const IntPoint & P, const IntPoint & S)
		{
			Int XL = Math::PreviousPowerOfTwo(S.X / 2);
			Int YL = Math::PreviousPowerOfTwo(S.Y / 2);
			Int XB = XL * 2;
			Int YB = YL * 2;

			IntPoint R;

			if (S.X - XB > XL)
			{
				R.X = XB;
			}
			else
			{
				R.X = XL;
			}

			if (S.Y - YB > YL)
			{
				R.Y = YB;
			}
			else
			{
				R.Y = YL;
			}

			return R;
		}

		void CTerrainTree::CreateTree(const InitializeParameters & Parameters)
		{
			if (Parameters.Size.Y <= TerrainStructure->GetPatchSize().Y || 
				Parameters.Size.X <= TerrainStructure->GetPatchSize().X)
			{
				Patch = new CTerrainPatch(Position, Size, this);
				{
					IndexLocation = TerrainStructure->IncrementTileOffset(Size);
				}
			}
			else
			{
				InitializeParameters Params(Position, Size);

				IntPoint S = GetPreferableSize(Position, Size);

				if (S.Y < TerrainStructure->GetPatchSize().Y)
				{
					S.Y = TerrainStructure->GetPatchSize().Y;
				}

				if (S.X < TerrainStructure->GetPatchSize().X)
				{
					S.X = TerrainStructure->GetPatchSize().X;
				}

				// Top Left

				Params.Position = Position;
				Params.Size = S;
				SubTrees[0] = new CTerrainTree(this, Params);

				// Top Right

				Params.Position.Y = Position.Y;
				Params.Position.X = Position.X + S.X;
				Params.Size.X = Size.X - S.X;
				Params.Size.Y = S.Y;
				SubTrees[1] = new CTerrainTree(this, Params);

				// Bottom Left

				Params.Position.X = Position.X;
				Params.Position.Y = Position.Y + S.Y;
				Params.Size.Y = Size.Y - S.Y;
				Params.Size.X = S.X;
				SubTrees[2] = new CTerrainTree(this, Params);

				// Bottom Right

				Params.Position = Position + S;
				Params.Size = Size - S;
				SubTrees[3] = new CTerrainTree(this, Params);
			}
		}

		ErrorCode CTerrainTree::CreateTreeMultiThreaded(const InitializeParameters & Parameters)
		{
			Thread::CThreadPool WorkerPool(WorkerThreadCount);

			try
			{
				if (Parameters.Size.X <= TerrainStructure->GetPatchSize().X ||
					Parameters.Size.Y <= TerrainStructure->GetPatchSize().Y)
				{
					Patch = new CTerrainPatch(Position, Size, this);
				}
				else
				{
					InitializeParameters Params = InitializeParameters(Position, Size);

					auto F0 = new CFunction([this](InitializeParameters Parameters)
					{
						IntPoint S = GetPreferableSize(Parameters.Position, Parameters.Size);

						Parameters.Size = S;
						SubTrees[0] = new CTerrainTree(this, Parameters);
					}, Params);

					auto F1 = new CFunction([this](InitializeParameters Parameters)
					{
						IntPoint S = GetPreferableSize(Parameters.Position, Parameters.Size);

						Parameters.Position.X += S.X;
						Parameters.Size.Y = S.Y;
						Parameters.Size.X-= S.X;
						SubTrees[1] = new CTerrainTree(this, Parameters);
					}, Params);

					auto F2 = new CFunction([this](InitializeParameters Parameters)
					{
						IntPoint S = GetPreferableSize(Parameters.Position, Parameters.Size);

						Parameters.Position.Y += S.Y;
						Parameters.Size.X = S.X;
						Parameters.Size.Y-= S.Y;
						SubTrees[2] = new CTerrainTree(this, Parameters);
					}, Params);

					auto F3 = new CFunction([this](InitializeParameters Parameters)
					{
						IntPoint S = GetPreferableSize(Parameters.Position, Parameters.Size);

						Parameters.Position += S;
						Parameters.Size	-= S;
						SubTrees[3] = new CTerrainTree(this, Parameters);
					}, Params);

					WorkerPool.PostTask(F0);
					WorkerPool.PostTask(F1);
					WorkerPool.PostTask(F2);
					WorkerPool.PostTask(F3);
				}
			}
			catch (ErrorCode Error)
			{
				return Error;
			}

			WorkerPool.JoinAllWork();

			return S_OK;
		}
		
		CTerrainTree::CTerrainTree(const CTerrainTree * pOwnerTree, const InitializeParameters & Parameters)
		{
			Ensure(pOwnerTree);

			Size = Parameters.Size;
			Position = Parameters.Position;
			TerrainTree = pOwnerTree;
			TerrainStructure = pOwnerTree->GetStructure();

			Index = TerrainStructure->GetTreeIndexByPosition(Position);
			IndexLocation = TerrainStructure->GetTreeOffsetByPosition(Position);

			CreateTree(Parameters);
		}
		
		CTerrainTree::CTerrainTree(const CStructure * pTerrainStructure, const InitializeParameters & Parameters)
		{
			Ensure(pTerrainStructure);

			Size = Parameters.Size;
			Position = Parameters.Position;
			TerrainStructure = pTerrainStructure;

			Index = 0;

			CreateTreeMultiThreaded(Parameters);
		}
		
		QueryResult * CTerrainTree::Query(const ViewFrustum & Frustum) const
		{
			if (!Frustum.Intersects(Vector2f(Position), Vector2f(Position + Size)))
			{
				return NULL;
			}

			QueryResult * Result = new QueryResult(this);

			if (!HasChildren())
			{
				return Result;
			}

#pragma unroll(1)
			for (UINT N = 0; N < 4; ++N)
			{
				Result->SubQuery[N] = SubTrees[N]->Query(Frustum);
			}

			return Result;
		}
	}
}

