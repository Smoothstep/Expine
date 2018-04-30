#pragma once

namespace D3D
{
	enum EBlendMode
	{
		BlendNone,
		BlendMasked
	};

	struct KMaterialExpression
	{

	};

	struct KMaterialInput
	{
		KMaterialExpression * Expression;
	};

	struct KMaterialExpressionTextureCoordinate
	{
		uint32_t CoordinateIdx;
	};

	struct KMaterialExpressionComponentMask
	{
		Uint8 R;
		Uint8 G;
		Uint8 B;
		Uint8 A;
		KMaterialInput MaterialInput;
	};

	struct KMaterialExpressionPower
	{
		float Exponent;
		
		struct
		{
			KMaterialExpression * Expression;
		} Base;
	};

	struct KMaterialExpressionClamp
	{

		KMaterialInput MaterialInput;
	};

	struct KMaterial
	{
		EBlendMode										BlendMode;
		bool											TwoSided;
		bool											CastShadowAsMasked;
		TVector<UniquePointer<KMaterialExpression> >	Expressions;
	};

	struct KMaterialStatic
	{
		KMaterial		* Material;
		KMaterialStatic * Parent;

		KMaterialStatic(
			KMaterialStatic * Parent,
			KMaterial		* Material) :
			Material(Material), Parent(Parent)
		{}

		virtual void FreeComponents() = 0;
	};

	class CMaterialLab
	{
	private:

		THashMap<String, SharedPointer<KMaterial> >			MaterialMap;
		THashMap<String, SharedPointer<KMaterialStatic> >	MaterialMapStatic;

	public:

		SharedPointer<KMaterial> CreateMaterial
		(
			const String & Name
		)
		{
			return MaterialMap.insert_or_assign(Name, new KMaterial()).first.value();
		}
	};
}