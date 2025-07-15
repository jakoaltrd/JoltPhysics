// SPDX-FileCopyrightText: 2024 PhysicsStudio
// SPDX-License-Identifier: MIT

#include "BasicPlaygroundTest.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Application/DebugUI.h>
#include <UI/UICheckBox.h>
#include <UI/UISlider.h>
#include <random>

JPH_IMPLEMENT_RTTI_VIRTUAL(BasicPlaygroundTest)
{
	JPH_ADD_BASE_CLASS(BasicPlaygroundTest, Test)
}

void BasicPlaygroundTest::Initialize()
{
	// 创建场景组件
	CreateFloorAndWalls();
	CreateStackOfBoxes();
	CreateBouncingBalls();
	CreateSeesaw();
	CreatePendulum();
	CreateRamp();
}

void BasicPlaygroundTest::CreateFloorAndWalls()
{
	// 创建地面
	ShapeRefC floor_shape = new BoxShape(Vec3(50.0f, 1.0f, 50.0f));
	BodyCreationSettings floor_settings(floor_shape, Vec3(0, -1, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mFloorID = mBodyInterface->CreateAndAddBody(floor_settings, EActivation::DontActivate);
	
	// 创建墙壁
	ShapeRefC wall_shape = new BoxShape(Vec3(1.0f, 10.0f, 50.0f));
	
	// 左墙
	BodyCreationSettings left_wall(wall_shape, Vec3(-50, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mBodyInterface->CreateAndAddBody(left_wall, EActivation::DontActivate);
	
	// 右墙
	BodyCreationSettings right_wall(wall_shape, Vec3(50, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mBodyInterface->CreateAndAddBody(right_wall, EActivation::DontActivate);
	
	// 后墙
	ShapeRefC back_wall_shape = new BoxShape(Vec3(50.0f, 10.0f, 1.0f));
	BodyCreationSettings back_wall(back_wall_shape, Vec3(0, 10, -50), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mBodyInterface->CreateAndAddBody(back_wall, EActivation::DontActivate);
}

void BasicPlaygroundTest::CreateStackOfBoxes()
{
	// 创建一个盒子堆叠
	ShapeRefC box_shape = new BoxShape(Vec3(1.0f, 1.0f, 1.0f));
	
	for (int y = 0; y < 5; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			Vec3 position(-10.0f + x * 2.2f, 1.0f + y * 2.2f, -10.0f);
			BodyCreationSettings box_settings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			box_settings.mRestitution = 0.3f;
			box_settings.mFriction = 0.5f;
			BodyID box_id = mBodyInterface->CreateAndAddBody(box_settings, EActivation::Activate);
			mDynamicBodies.push_back(box_id);
		}
	}
}

void BasicPlaygroundTest::CreateBouncingBalls()
{
	// 创建不同大小的弹球
	static mt19937 rng(42);
	uniform_real_distribution<float> size_dist(0.5f, 2.0f);
	uniform_real_distribution<float> pos_dist(-20.0f, 20.0f);
	uniform_real_distribution<float> height_dist(10.0f, 30.0f);
	
	for (int i = 0; i < 8; ++i)
	{
		float radius = size_dist(rng);
		Vec3 position(pos_dist(rng), height_dist(rng), pos_dist(rng));
		
		ShapeRefC sphere_shape = new SphereShape(radius);
		BodyCreationSettings sphere_settings(sphere_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		sphere_settings.mRestitution = 0.8f;  // 高反弹
		sphere_settings.mFriction = 0.1f;     // 低摩擦
		
		BodyID sphere_id = mBodyInterface->CreateAndAddBody(sphere_settings, EActivation::Activate);
		mDynamicBodies.push_back(sphere_id);
	}
}

void BasicPlaygroundTest::CreateSeesaw()
{
	// 创建跷跷板的支点
	ShapeRefC pivot_shape = new BoxShape(Vec3(0.5f, 2.0f, 0.5f));
	BodyCreationSettings pivot_settings(pivot_shape, Vec3(0, 2, 10), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mSeesawPivotID = mBodyInterface->CreateAndAddBody(pivot_settings, EActivation::DontActivate);
	
	// 创建跷跷板的板子
	ShapeRefC plank_shape = new BoxShape(Vec3(8.0f, 0.2f, 1.0f));
	BodyCreationSettings plank_settings(plank_shape, Vec3(0, 4.2f, 10), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	plank_settings.mRestitution = 0.2f;
	plank_settings.mFriction = 0.7f;
	mSeesawPlankID = mBodyInterface->CreateAndAddBody(plank_settings, EActivation::Activate);
	mDynamicBodies.push_back(mSeesawPlankID);
	
	// 创建铰链约束
	HingeConstraintSettings hinge_settings;
	hinge_settings.mPoint1 = Vec3(0, 2.2f, 0);  // 在支点上的连接点
	hinge_settings.mHingeAxis1 = Vec3(0, 0, 1);  // Z轴为旋转轴
	hinge_settings.mPoint2 = Vec3(0, 0, 0);      // 在板子上的连接点
	hinge_settings.mHingeAxis2 = Vec3(0, 0, 1);
	
	mPhysicsSystem->AddConstraint(hinge_settings.Create(*mBodyInterface->GetBody(mSeesawPivotID), 
													   *mBodyInterface->GetBody(mSeesawPlankID)));
}

void BasicPlaygroundTest::CreatePendulum()
{
	// 创建摆锤的锚点
	ShapeRefC anchor_shape = new SphereShape(0.5f);
	BodyCreationSettings anchor_settings(anchor_shape, Vec3(20, 15, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mPendulumAnchorID = mBodyInterface->CreateAndAddBody(anchor_settings, EActivation::DontActivate);
	
	// 创建摆锤
	ShapeRefC bob_shape = new SphereShape(2.0f);
	BodyCreationSettings bob_settings(bob_shape, Vec3(25, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bob_settings.mRestitution = 0.9f;
	bob_settings.mFriction = 0.1f;
	mPendulumBobID = mBodyInterface->CreateAndAddBody(bob_settings, EActivation::Activate);
	mDynamicBodies.push_back(mPendulumBobID);
	
	// 创建距离约束（绳子）
	DistanceConstraintSettings rope_settings;
	rope_settings.mPoint1 = Vec3(0, 0, 0);       // 锚点
	rope_settings.mPoint2 = Vec3(0, 0, 0);       // 摆锤
	rope_settings.mMinDistance = 10.0f;          // 最小距离
	rope_settings.mMaxDistance = 10.0f;          // 最大距离（刚性绳子）
	
	mPhysicsSystem->AddConstraint(rope_settings.Create(*mBodyInterface->GetBody(mPendulumAnchorID), 
													   *mBodyInterface->GetBody(mPendulumBobID)));
}

void BasicPlaygroundTest::CreateRamp()
{
	// 创建斜坡
	ShapeRefC ramp_shape = new BoxShape(Vec3(10.0f, 0.5f, 5.0f));
	Quat ramp_rotation = Quat::sRotation(Vec3(0, 0, 1), 0.3f);  // 绕Z轴旋转
	BodyCreationSettings ramp_settings(ramp_shape, Vec3(-30, 5, 20), ramp_rotation, EMotionType::Static, Layers::NON_MOVING);
	mBodyInterface->CreateAndAddBody(ramp_settings, EActivation::DontActivate);
	
	// 在斜坡顶部放置一些物体
	ShapeRefC cylinder_shape = new CapsuleShape(1.0f, 1.0f);
	for (int i = 0; i < 3; ++i)
	{
		Vec3 position(-35.0f, 12.0f, 18.0f + i * 2.0f);
		BodyCreationSettings cylinder_settings(cylinder_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		cylinder_settings.mRestitution = 0.4f;
		cylinder_settings.mFriction = 0.3f;
		
		BodyID cylinder_id = mBodyInterface->CreateAndAddBody(cylinder_settings, EActivation::Activate);
		mDynamicBodies.push_back(cylinder_id);
	}
}

void BasicPlaygroundTest::SpawnRandomObject()
{
	static mt19937 rng(time(nullptr));
	uniform_real_distribution<float> pos_dist(-40.0f, 40.0f);
	uniform_real_distribution<float> size_dist(0.5f, 2.0f);
	uniform_int_distribution<int> type_dist(0, 2);
	
	Vec3 position(pos_dist(rng), 20.0f, pos_dist(rng));
	float size = size_dist(rng);
	
	ShapeRefC shape;
	switch (type_dist(rng))
	{
	case 0: // 盒子
		shape = new BoxShape(Vec3(size, size, size));
		break;
	case 1: // 球体
		shape = new SphereShape(size);
		break;
	case 2: // 胶囊
		shape = new CapsuleShape(size * 0.5f, size);
		break;
	}
	
	BodyCreationSettings settings(shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	settings.mRestitution = 0.5f;
	settings.mFriction = 0.4f;
	
	BodyID body_id = mBodyInterface->CreateAndAddBody(settings, EActivation::Activate);
	mDynamicBodies.push_back(body_id);
}

void BasicPlaygroundTest::ResetScene()
{
	// 移除所有动态物体
	for (BodyID id : mDynamicBodies)
	{
		mBodyInterface->RemoveBody(id);
		mBodyInterface->DestroyBody(id);
	}
	mDynamicBodies.clear();
	
	// 重新创建场景
	CreateStackOfBoxes();
	CreateBouncingBalls();
	CreateSeesaw();
	CreatePendulum();
	CreateRamp();
}

void BasicPlaygroundTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inSubMenu->AddChild(new UICheckBox(inUI, 0, 0, 200, 20, "启用风效果", mEnableWind, 
		[this](bool inValue) { mEnableWind = inValue; }));
	
	inSubMenu->AddChild(new UISlider(inUI, 0, 0, 200, 20, 0.0f, 20.0f, mWindStrength, 
		[this](float inValue) { mWindStrength = inValue; }));
	
	inSubMenu->AddChild(new UICheckBox(inUI, 0, 0, 200, 20, "随机生成物体", mEnableRandomSpawn, 
		[this](bool inValue) { mEnableRandomSpawn = inValue; }));
	
	inSubMenu->AddChild(new UISlider(inUI, 0, 0, 200, 20, 0.5f, 5.0f, mSpawnInterval, 
		[this](float inValue) { mSpawnInterval = inValue; }));
}

void BasicPlaygroundTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// 应用风效果
	if (mEnableWind)
	{
		Vec3 wind_force(mWindStrength, 0, 0);
		for (BodyID id : mDynamicBodies)
		{
			if (mBodyInterface->IsActive(id))
			{
				mBodyInterface->AddForce(id, wind_force);
			}
		}
	}
	
	// 随机生成物体
	if (mEnableRandomSpawn)
	{
		mSpawnTimer += inParams.mDeltaTime;
		if (mSpawnTimer >= mSpawnInterval)
		{
			SpawnRandomObject();
			mSpawnTimer = 0.0f;
		}
	}
	
	// 键盘控制
	if (inParams.mKeyboard->IsKeyPressed(Key::R))
	{
		ResetScene();
	}
	
	if (inParams.mKeyboard->IsKeyPressed(Key::Space))
	{
		SpawnRandomObject();
	}
}