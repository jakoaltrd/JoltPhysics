// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT_BASE(ShapeSettings)
{
	JPH_ADD_BASE_CLASS(ShapeSettings, SerializableObject)

	JPH_ADD_ATTRIBUTE(ShapeSettings, mUserData)
}

#ifdef JPH_DEBUG_RENDERER
bool Shape::sDrawSubmergedVolumes = false;
#endif // JPH_DEBUG_RENDERER

ShapeFunctions ShapeFunctions::sRegistry[NumSubShapeTypes];

TransformedShape Shape::GetSubShapeTransformedShape(const SubShapeID &inSubShapeID, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, SubShapeID &outRemainder) const
{
	// We have reached the leaf shape so there is no remainder
	outRemainder = SubShapeID();

	// Just return the transformed shape for this shape
	TransformedShape ts(inPositionCOM, inRotation, this, BodyID());
	ts.SetShapeScale(inScale);
	return ts;
}

void Shape::CollectTransformedShapes(const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, const SubShapeIDCreator &inSubShapeIDCreator, TransformedShapeCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	// Test shape filter
	if (!inShapeFilter.ShouldCollide(inSubShapeIDCreator.GetID()))
		return;

	TransformedShape ts(inPositionCOM, inRotation, this, TransformedShape::sGetBodyID(ioCollector.GetContext()), inSubShapeIDCreator);
	ts.SetShapeScale(inScale);
	ioCollector.AddHit(ts);
}

void Shape::TransformShape(Mat44Arg inCenterOfMassTransform, TransformedShapeCollector &ioCollector) const
{
	Vec3 scale;
	Mat44 transform = inCenterOfMassTransform.Decompose(scale);
	TransformedShape ts(transform.GetTranslation(), transform.GetRotation().GetQuaternion(), this, BodyID(), SubShapeIDCreator());
	ts.SetShapeScale(scale);
	ioCollector.AddHit(ts);
}

void Shape::SaveBinaryState(StreamOut &inStream) const
{ 
	inStream.Write(mShapeSubType);
	inStream.Write(mUserData);
}

void Shape::RestoreBinaryState(StreamIn &inStream)
{
	// Type hash read by sRestoreFromBinaryState
	inStream.Read(mUserData);
}

Shape::ShapeResult Shape::sRestoreFromBinaryState(StreamIn &inStream)
{
	ShapeResult result;

	// Read the type of the shape
	EShapeSubType shape_sub_type;
	inStream.Read(shape_sub_type);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read type id");
		return result;
	}

	// Construct and read the data of the shape
	Ref<Shape> shape = ShapeFunctions::sGet(shape_sub_type).mConstruct();
	shape->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore shape");
		return result;
	}

	result.Set(shape);
	return result;
}

void Shape::SaveWithChildren(StreamOut &inStream, ShapeToIDMap &ioShapeMap, MaterialToIDMap &ioMaterialMap) const
{
	ShapeToIDMap::const_iterator shape_id_iter = ioShapeMap.find(this);
	if (shape_id_iter == ioShapeMap.end())
	{
		// Write shape ID of this shape
		uint32 shape_id = (uint32)ioShapeMap.size();
		ioShapeMap[this] = shape_id;
		inStream.Write(shape_id);

		// Write the shape itself
		SaveBinaryState(inStream);

		// Write the ID's of all sub shapes
		ShapeList sub_shapes;
		SaveSubShapeState(sub_shapes);
		inStream.Write(sub_shapes.size());
		for (const Shape *shape : sub_shapes)
		{
			if (shape == nullptr)
				inStream.Write(~uint32(0));
			else
				shape->SaveWithChildren(inStream, ioShapeMap, ioMaterialMap);
		}

		// Write the materials
		PhysicsMaterialList materials;
		SaveMaterialState(materials);
		inStream.Write(materials.size());
		for (const PhysicsMaterial *mat : materials)
		{
			if (mat == nullptr)
			{
				// Write nullptr
				inStream.Write(~uint32(0));
			}
			else
			{
				MaterialToIDMap::const_iterator material_id = ioMaterialMap.find(mat);
				if (material_id == ioMaterialMap.end())
				{
					// New material, write the ID
					uint32 new_material_id = (uint32)ioMaterialMap.size();
					ioMaterialMap[mat] = new_material_id;
					inStream.Write(new_material_id);

					// Write the material
					mat->SaveBinaryState(inStream);
				}
				else
				{
					// Known material, just write the ID
					inStream.Write(material_id->second);
				}
			}
		}
	}
	else
	{
		// Known shape, just write the ID
		inStream.Write(shape_id_iter->second);
	}
}

Shape::ShapeResult Shape::sRestoreWithChildren(StreamIn &inStream, IDToShapeMap &ioShapeMap, IDToMaterialMap &ioMaterialMap)
{
	ShapeResult result;

	// Read ID of this shape
	uint32 shape_id;
	inStream.Read(shape_id);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read shape id");
		return result;
	}

	// Check nullptr shape
	if (shape_id == ~uint32(0))
	{
		result.Set(nullptr);
		return result;
	}

	// Check if we already read this shape
	if (shape_id < ioShapeMap.size())
	{
		result.Set(ioShapeMap[shape_id]);
		return result;
	}

	// Read the shape
	result = sRestoreFromBinaryState(inStream);
	if (result.HasError())
		return result;
	JPH_ASSERT(ioShapeMap.size() == shape_id); // Assert that this is the next ID in the map
	ioShapeMap.push_back(result.Get());

	// Read the sub shapes
	size_t len;
	inStream.Read(len);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read stream");
		return result;
	}
	ShapeList sub_shapes;
	sub_shapes.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		ShapeResult sub_shape_result = sRestoreWithChildren(inStream, ioShapeMap, ioMaterialMap);
		if (sub_shape_result.HasError())
			return sub_shape_result;
		sub_shapes.push_back(sub_shape_result.Get());
	}
	result.Get()->RestoreSubShapeState(sub_shapes.data(), (uint)sub_shapes.size());

	// Read the materials
	inStream.Read(len);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read stream");
		return result;
	}
	PhysicsMaterialList materials;
	materials.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		Ref<PhysicsMaterial> material;

		uint32 material_id;
		inStream.Read(material_id);

		// Check nullptr
		if (material_id != ~uint32(0))
		{
			if (material_id >= ioMaterialMap.size())
			{
				// New material, restore material
				PhysicsMaterial::PhysicsMaterialResult material_result = PhysicsMaterial::sRestoreFromBinaryState(inStream);
				if (material_result.HasError())
				{
					result.SetError(material_result.GetError());
					return result;
				}
				material = material_result.Get();
				JPH_ASSERT(material_id == ioMaterialMap.size());
				ioMaterialMap.push_back(material);
			}
			else
			{
				// Existing material
				material = ioMaterialMap[material_id];
			}
		}

		materials.push_back(material);
	}
	result.Get()->RestoreMaterialState(materials.data(), (uint)materials.size());

	return result;
}

Shape::Stats Shape::GetStatsRecursive(VisitedShapes &ioVisitedShapes) const
{
	Stats stats = GetStats();

	// If shape is already visited, don't count its size again
	if (!ioVisitedShapes.insert(this).second)
		stats.mSizeBytes = 0;

	return stats;
}

Shape::ShapeResult Shape::ScaleShape(Vec3Arg inScale) const
{
	const Vec3 unit_scale = Vec3::sReplicate(1.0f);

	if (inScale.IsNearZero())
	{
		ShapeResult result;
		result.SetError("Can't use zero scale!");
		return result;
	}

	// First test if we can just wrap this shape in a scaled shape
	if (IsValidScale(inScale))
	{
		// Test if the scale is near unit
		ShapeResult result;
		if (inScale.IsClose(unit_scale))
			result.Set(const_cast<Shape *>(this));
		else
			result.Set(new ScaledShape(this, inScale));
		return result;
	}

	// Collect the leaf shapes and their transforms
	struct Collector : TransformedShapeCollector
	{
		virtual void				AddHit(const ResultType &inResult) override
		{
			mShapes.push_back(inResult);
		}

		vector<TransformedShape>	mShapes;
	};
	Collector collector;
	TransformShape(Mat44::sScale(inScale) * Mat44::sTranslation(GetCenterOfMass()), collector);

	// Construct a compound shape
	StaticCompoundShapeSettings compound;
	compound.mSubShapes.reserve(collector.mShapes.size());
	for (const TransformedShape &ts : collector.mShapes)
	{
		const Shape *shape = ts.mShape;

		// Construct a scaled shape if scale is not unit
		Vec3 scale = ts.GetShapeScale();
		if (!scale.IsClose(unit_scale))
			shape = new ScaledShape(shape, scale);

		// Add the shape
		compound.AddShape(ts.mShapePositionCOM - ts.mShapeRotation * shape->GetCenterOfMass(), ts.mShapeRotation, shape);
	}

	return compound.Create();
}

JPH_NAMESPACE_END
