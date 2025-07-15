// SPDX-FileCopyrightText: 2024 PhysicsStudio
// SPDX-License-Identifier: MIT

#pragma once

#include <Application/Application.h>
#include <UI/UIManager.h>
#include <Application/DebugUI.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Tests/Test.h>
#include <Utils/ContactListenerImpl.h>
#include <Renderer/DebugRendererImp.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Layers.h>

namespace JPH {
	class JobSystem;
	class TempAllocator;
};

class UIComboBox;
class UISlider;
class UICheckBox;
class UIButton;
class UIStaticText;

// PhysicsStudio - 一个功能完整的物理引擎GUI工具
class PhysicsStudio : public Application
{
public:
	// Constructor / destructor
							PhysicsStudio();
	virtual					~PhysicsStudio() override;
		
	// Render the frame.
	virtual bool			RenderFrame(float inDeltaTime) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Get scale factor for this world, used to boost camera speed and to scale detail of the shadows
	virtual float			GetWorldScale() const override;

private:
	// 初始化物理世界
	void					InitializePhysics();
	
	// 清理物理世界
	void					CleanupPhysics();
	
	// 创建GUI界面
	void					CreateUI();
	
	// 更新GUI状态
	void					UpdateUI();
	
	// 启动新测试场景
	void					StartTest(const RTTI *inRTTI);
	
	// 重置当前测试
	void					ResetTest();
	
	// GUI回调函数
	void					OnSceneSelected();
	void					OnGravityChanged();
	void					OnTimeStepChanged();
	void					OnRestitutionChanged();
	void					OnFrictionChanged();
	void					OnDampingChanged();
	void					OnResetPressed();
	void					OnPausePressed();
	void					OnStepPressed();
	void					OnSaveScene();
	void					OnLoadScene();

	// 物理系统组件
	unique_ptr<JobSystem>				mJobSystem;
	unique_ptr<TempAllocator>			mTempAllocator;
	unique_ptr<PhysicsSystem>			mPhysicsSystem;
	unique_ptr<ContactListenerImpl>		mContactListener;
	
	// 测试相关
	unique_ptr<Test>					mTest;
	const RTTI *						mTestClass = nullptr;
	
	// GUI组件
	UIComboBox *						mSceneComboBox = nullptr;
	UISlider *							mGravitySlider = nullptr;
	UISlider *							mTimeStepSlider = nullptr;
	UISlider *							mRestitutionSlider = nullptr;
	UISlider *							mFrictionSlider = nullptr;
	UISlider *							mLinearDampingSlider = nullptr;
	UISlider *							mAngularDampingSlider = nullptr;
	UICheckBox *						mPauseCheckBox = nullptr;
	UIButton *							mResetButton = nullptr;
	UIButton *							mStepButton = nullptr;
	UIButton *							mSaveButton = nullptr;
	UIButton *							mLoadButton = nullptr;
	UIStaticText *						mStatusText = nullptr;
	UIStaticText *						mStatsText = nullptr;
	
	// 物理参数
	float								mGravity = -9.81f;
	float								mTimeStep = 1.0f / 60.0f;
	float								mDefaultRestitution = 0.0f;
	float								mDefaultFriction = 0.2f;
	float								mLinearDamping = 0.05f;
	float								mAngularDamping = 0.05f;
	
	// 状态
	bool								mIsPaused = false;
	bool								mSingleStep = false;
	float								mAccumulatedTime = 0.0f;
	
	// 统计信息
	uint32								mNumBodies = 0;
	uint32								mNumActiveBodies = 0;
	uint32								mNumContacts = 0;
};