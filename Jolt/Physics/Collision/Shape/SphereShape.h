// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/ConvexShape.h>

JPH_NAMESPACE_BEGIN

/// Class that constructs a SphereShape
class SphereShapeSettings final : public ConvexShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(SphereShapeSettings)

	/// Default constructor for deserialization
							SphereShapeSettings() = default;

	/// Create a sphere with radius inRadius
							SphereShapeSettings(float inRadius, const PhysicsMaterial *inMaterial = nullptr)	: ConvexShapeSettings(inMaterial), mRadius(inRadius) { }

	// See: ShapeSettings
	virtual ShapeResult		Create() const override;

	float					mRadius = 0.0f;
};

/// A sphere, centered around the origin.
/// Note that it is implemented as a point with convex radius.
class SphereShape final : public ConvexShape
{
public:
	/// Constructor
							SphereShape() : ConvexShape(EShapeSubType::Sphere) { }
							SphereShape(const SphereShapeSettings &inSettings, ShapeResult &outResult);

	/// Create a sphere with radius inRadius
							SphereShape(float inRadius, const PhysicsMaterial *inMaterial = nullptr)			: ConvexShape(EShapeSubType::Sphere, inMaterial), mRadius(inRadius) { JPH_ASSERT(inRadius > 0.0f); }

	/// Radius of the sphere
	float					GetRadius() const																	{ return mRadius; }

	// See Shape::GetLocalBounds
	virtual AABox			GetLocalBounds() const override;
		
	// See Shape::GetWorldSpaceBounds
	virtual AABox			GetWorldSpaceBounds(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const override;

	// See Shape::GetInnerRadius
	virtual float			GetInnerRadius() const override														{ return mRadius; }

	// See Shape::GetMassProperties
	virtual MassProperties	GetMassProperties() const override;

	// See Shape::GetSurfaceNormal
	virtual Vec3			GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

	// See ConvexShape::GetSupportFunction
	virtual const Support *	GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const override;

	// See ConvexShape::GetSupportingFace
	virtual void			GetSupportingFace(Vec3Arg inDirection, Vec3Arg inScale, SupportingFace &outVertices) const override	{ /* Hit is always a single point, no point in returning anything */ }

	// See Shape::GetSubmergedVolume
	virtual void			GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override;

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void			Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	virtual bool			CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void			CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector, const ShapeFilter &inShapeFilter = { }) const override;

	// See: Shape::CollidePoint
	virtual void			CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector, const ShapeFilter &inShapeFilter = { }) const override;

	// See Shape::TransformShape
	virtual void			TransformShape(Mat44Arg inCenterOfMassTransform, TransformedShapeCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void			GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override;

	// See Shape::GetTrianglesNext
	virtual int				GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override;

	// See Shape
	virtual void			SaveBinaryState(StreamOut &inStream) const override;

	// See Shape::GetStats
	virtual Stats			GetStats() const override															{ return Stats(sizeof(*this), 0); }

	// See Shape::GetVolume
	virtual float			GetVolume() const override															{ return 4.0f / 3.0f * JPH_PI * Cubed(mRadius); }

	// See Shape::IsValidScale
	virtual bool			IsValidScale(Vec3Arg inScale) const override;

	// Register shape functions with the registry
	static void				sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void			RestoreBinaryState(StreamIn &inStream) override;

private:
	// Get the radius of this sphere scaled by inScale
	inline float			GetScaledRadius(Vec3Arg inScale) const;

	// Classes for GetSupportFunction
	class 					SphereNoConvex;
	class 					SphereWithConvex;

	float					mRadius = 0.0f;
};

JPH_NAMESPACE_END
