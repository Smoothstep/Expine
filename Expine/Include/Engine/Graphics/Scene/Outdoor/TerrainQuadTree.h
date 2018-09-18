#pragma once

#include "TerrainStructure.h"
#include "Scene/View/ViewFrustum.h"

#include "Hyper/Memory.h"

namespace D3D
{
	namespace Terrain
	{
		struct TerrainNode
		{
			IntPoint Position;

			inline TerrainNode
			(
				const IntPoint Position
			)
			{
				this->Position = Position;
			}
		};

		class CTerrainTree;
		class CTerrainPatch
		{
		public:

			IntPoint Position;
			IntPoint Size;

		private:

			ConstPointer<CTerrainTree> Tree;

		private:

			void CreatePatch();

		public:

			inline CTerrainPatch
			(
				const	IntPoint		Position,
				const	IntPoint		Size,
				const	CTerrainTree *	pTree
			)
			{
				this->Position	= Position;
				this->Size		= Size;
				this->Tree		= pTree;
			}
		};

		struct QueryResult
		{
			// Main tree

			const CTerrainTree * Tree = NULL;

			// Sub tree queries

			QueryResult	* SubQuery[4] = 
			{ 
				NULL,
				NULL,
				NULL,
				NULL
			};

			// Level of detail

			Float LOD = 0.0;

			inline QueryResult
			(
				const CTerrainTree * pTree
			)
			{
				Tree = pTree;
			}

			inline ~QueryResult()
			{
				for (auto & Query : SubQuery)
				{
					if (Query)
					{
						delete Query;
					}
				}
			}
		};

		class CTerrainTree
		{
			static constexpr unsigned NUM_SUB_TREES = 4;

		public:

			struct InitializeParameters
			{
				IntPoint Position;
				IntPoint Size;

				inline constexpr InitializeParameters
				(
					const IntPoint & Position,
					const IntPoint & Size
				) :
					Position(Position), Size(Size)
				{}
			};

		private:

			IntPoint Position;
			IntPoint Size;

			size_t Index;
			size_t IndexLocation;

		private:

			ConstPointer<CStructure>		TerrainStructure;
			ConstPointer<CTerrainTree>	TerrainTree;

		protected:

			UniquePointer<CTerrainTree>		SubTrees[NUM_SUB_TREES];
			UniquePointer<CTerrainPatch>	Patch;

		public:

			inline const CStructure * GetStructure() const
			{
				return TerrainStructure;
			}

			inline CTerrainPatch * GetPatch() const
			{
				return Patch.Get();
			}

			inline CTerrainTree * GetSubTree
			(
				const UINT Index
			)	const
			{
				return SubTrees[Index].Get();
			}

			inline bool HasChildren() const
			{
				return SubTrees[0] != NULL;
			}

			inline size_t GetIndex() const
			{
				return Index;
			}

			inline size_t GetIndexLocation() const
			{
				return IndexLocation;
			}

			inline const IntPoint & GetPosition() const
			{
				return Position;
			}

			inline const IntPoint & GetSize() const
			{
				return Size;
			}

		private:

			void CreateTree
			(
				const InitializeParameters & Parameters
			);

			ErrorCode CreateTreeMultiThreaded
			(
				const InitializeParameters & Parameters
			);

		public:

			CTerrainTree
			(
				const CTerrainTree			* pOwnerTree,
				const InitializeParameters	& Parameters
			);

			CTerrainTree
			(
				const CStructure			* pTerrainStructure,
				const InitializeParameters	& Parameters
			);

			QueryResult	* Query
			(
				const ViewFrustum & Frustum
			)	const;
		};
	}
}