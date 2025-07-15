// SPDX-FileCopyrightText: 2024 PhysicsStudio
// SPDX-License-Identifier: MIT

#include "PhysicsStudio.h"
#include <TestFramework.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <UI/UIComboBox.h>
#include <UI/UISlider.h>
#include <UI/UICheckBox.h>
#include <UI/UIButton.h>
#include <UI/UIStaticText.h>
#include <UI/UIVerticalStack.h>
#include <UI/UIHorizontalStack.h>
#include <Renderer/DebugRendererImp.h>
#include <Application/DebugUI.h>
#include <fstream>
#include <filesystem>

using namespace JPH;
using namespace std;

// 测试场景列表
static vector<const RTTI *> sTestClasses;

// 初始化测试场景列表
static void InitializeTestClasses()
{
	if (!sTestClasses.empty())
		return;
		
	// 获取所有Test类的派生类
	vector<const RTTI *> classes;
	Factory::sInstance->GetAllClasses(JPH_RTTI(Test), classes);
	
	// 过滤掉抽象类
	for (const RTTI *rtti : classes)
	{
		if (rtti->GetConstructor() != nullptr)
			sTestClasses.push_back(rtti);
	}
	
	// 按名称排序
	sort(sTestClasses.begin(), sTestClasses.end(), [](const RTTI *a, const RTTI *b) {
		return string(a->GetName()) < string(b->GetName());
	});
}

PhysicsStudio::PhysicsStudio()
{
	// 初始化Jolt
	RegisterDefaultAllocator();
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
	Factory::sInstance = new Factory();
	RegisterTypes();
	
	// 初始化测试场景列表
	InitializeTestClasses();
	
	// 初始化物理系统
	InitializePhysics();
	
	// 创建GUI
	CreateUI();
}

PhysicsStudio::~PhysicsStudio()
{
	CleanupPhysics();
	
	UnregisterTypes();
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

void PhysicsStudio::InitializePhysics()
{
	// 创建任务系统
	mJobSystem = make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
	
	// 创建临时分配器
	mTempAllocator = make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
	
	// 创建接触监听器
	mContactListener = make_unique<ContactListenerImpl>();
	
	// 创建物理系统
	const uint cMaxBodies = 10240;
	const uint cNumBodyMutexes = 0; // 自动检测
	const uint cMaxBodyPairs = 65536;
	const uint cMaxContactConstraints = 20480;
	
	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;
	
	mPhysicsSystem = make_unique<PhysicsSystem>();
	mPhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, 
		broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
	
	// 设置重力
	mPhysicsSystem->SetGravity(Vec3(0, mGravity, 0));
	
	// 设置接触监听器
	mPhysicsSystem->SetContactListener(mContactListener.get());
}

void PhysicsStudio::CleanupPhysics()
{
	mTest.reset();
	mPhysicsSystem.reset();
	mContactListener.reset();
	mTempAllocator.reset();
	mJobSystem.reset();
}

void PhysicsStudio::CreateUI()
{
	// 创建主菜单
	UIElement *main_menu = mDebugUI->CreateMenu();
	
	// 场景选择
	UIVerticalStack *scene_stack = new UIVerticalStack(mDebugUI, 0, 0, 300, 600);
	main_menu->AddChild(scene_stack);
	
	scene_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 300, 20, "场景选择", UIStaticText::EAlign::Center));
	
	mSceneComboBox = new UIComboBox(mDebugUI, 0, 0, 300, 20);
	mSceneComboBox->AddItem("选择场景...", 0);
	for (size_t i = 0; i < sTestClasses.size(); ++i)
	{
		mSceneComboBox->AddItem(sTestClasses[i]->GetName(), (int)(i + 1));
	}
	mSceneComboBox->SetSelectedItem(0);
	mSceneComboBox->SetOnSelectionChanged([this]() { OnSceneSelected(); });
	scene_stack->AddChild(mSceneComboBox);
	
	// 物理参数调节
	scene_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 300, 20, "物理参数", UIStaticText::EAlign::Center));
	
	// 重力设置
	UIHorizontalStack *gravity_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	gravity_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "重力:"));
	mGravitySlider = new UISlider(mDebugUI, 0, 0, 200, 20, -50.0f, 50.0f, mGravity);
	mGravitySlider->SetOnValueChanged([this](float value) { mGravity = value; OnGravityChanged(); });
	gravity_stack->AddChild(mGravitySlider);
	scene_stack->AddChild(gravity_stack);
	
	// 时间步长设置
	UIHorizontalStack *timestep_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	timestep_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "时间步长:"));
	mTimeStepSlider = new UISlider(mDebugUI, 0, 0, 200, 20, 1.0f/120.0f, 1.0f/30.0f, mTimeStep);
	mTimeStepSlider->SetOnValueChanged([this](float value) { mTimeStep = value; OnTimeStepChanged(); });
	timestep_stack->AddChild(mTimeStepSlider);
	scene_stack->AddChild(timestep_stack);
	
	// 反弹系数设置
	UIHorizontalStack *restitution_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	restitution_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "反弹系数:"));
	mRestitutionSlider = new UISlider(mDebugUI, 0, 0, 200, 20, 0.0f, 1.0f, mDefaultRestitution);
	mRestitutionSlider->SetOnValueChanged([this](float value) { mDefaultRestitution = value; OnRestitutionChanged(); });
	restitution_stack->AddChild(mRestitutionSlider);
	scene_stack->AddChild(restitution_stack);
	
	// 摩擦系数设置
	UIHorizontalStack *friction_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	friction_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "摩擦系数:"));
	mFrictionSlider = new UISlider(mDebugUI, 0, 0, 200, 20, 0.0f, 2.0f, mDefaultFriction);
	mFrictionSlider->SetOnValueChanged([this](float value) { mDefaultFriction = value; OnFrictionChanged(); });
	friction_stack->AddChild(mFrictionSlider);
	scene_stack->AddChild(friction_stack);
	
	// 线性阻尼设置
	UIHorizontalStack *linear_damping_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	linear_damping_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "线性阻尼:"));
	mLinearDampingSlider = new UISlider(mDebugUI, 0, 0, 200, 20, 0.0f, 1.0f, mLinearDamping);
	mLinearDampingSlider->SetOnValueChanged([this](float value) { mLinearDamping = value; OnDampingChanged(); });
	linear_damping_stack->AddChild(mLinearDampingSlider);
	scene_stack->AddChild(linear_damping_stack);
	
	// 角阻尼设置
	UIHorizontalStack *angular_damping_stack = new UIHorizontalStack(mDebugUI, 0, 0, 300, 20);
	angular_damping_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 100, 20, "角阻尼:"));
	mAngularDampingSlider = new UISlider(mDebugUI, 0, 0, 200, 20, 0.0f, 1.0f, mAngularDamping);
	mAngularDampingSlider->SetOnValueChanged([this](float value) { mAngularDamping = value; OnDampingChanged(); });
	angular_damping_stack->AddChild(mAngularDampingSlider);
	scene_stack->AddChild(angular_damping_stack);
	
	// 控制按钮
	scene_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 300, 20, "模拟控制", UIStaticText::EAlign::Center));
	
	UIHorizontalStack *control_stack1 = new UIHorizontalStack(mDebugUI, 0, 0, 300, 30);
	mPauseCheckBox = new UICheckBox(mDebugUI, 0, 0, 150, 30, "暂停", mIsPaused);
	mPauseCheckBox->SetOnValueChanged([this](bool value) { mIsPaused = value; OnPausePressed(); });
	control_stack1->AddChild(mPauseCheckBox);
	
	mStepButton = new UIButton(mDebugUI, 0, 0, 150, 30, "单步");
	mStepButton->SetOnPressed([this]() { OnStepPressed(); });
	control_stack1->AddChild(mStepButton);
	scene_stack->AddChild(control_stack1);
	
	UIHorizontalStack *control_stack2 = new UIHorizontalStack(mDebugUI, 0, 0, 300, 30);
	mResetButton = new UIButton(mDebugUI, 0, 0, 150, 30, "重置");
	mResetButton->SetOnPressed([this]() { OnResetPressed(); });
	control_stack2->AddChild(mResetButton);
	
	mSaveButton = new UIButton(mDebugUI, 0, 0, 150, 30, "保存场景");
	mSaveButton->SetOnPressed([this]() { OnSaveScene(); });
	control_stack2->AddChild(mSaveButton);
	scene_stack->AddChild(control_stack2);
	
	mLoadButton = new UIButton(mDebugUI, 0, 0, 300, 30, "加载场景");
	mLoadButton->SetOnPressed([this]() { OnLoadScene(); });
	scene_stack->AddChild(mLoadButton);
	
	// 状态信息
	scene_stack->AddChild(new UIStaticText(mDebugUI, 0, 0, 300, 20, "状态信息", UIStaticText::EAlign::Center));
	
	mStatusText = new UIStaticText(mDebugUI, 0, 0, 300, 20, "就绪");
	scene_stack->AddChild(mStatusText);
	
	mStatsText = new UIStaticText(mDebugUI, 0, 0, 300, 60, "统计信息:\n物体: 0\n活跃物体: 0\n接触点: 0");
	scene_stack->AddChild(mStatsText);
}

bool PhysicsStudio::RenderFrame(float inDeltaTime)
{
	// 更新物理系统
	if (!mIsPaused || mSingleStep)
	{
		if (mPhysicsSystem && mTest)
		{
			// 更新测试
			Test::PreUpdateParams pre_update_params;
			pre_update_params.mDeltaTime = mTimeStep;
			pre_update_params.mKeyboard = GetKeyboard();
			pre_update_params.mCameraState = GetCamera()->GetCameraState();
			mTest->PrePhysicsUpdate(pre_update_params);
			
			// 物理更新
			mPhysicsSystem->Update(mTimeStep, 1, mTempAllocator.get(), mJobSystem.get());
			
			// 后更新
			mTest->PostPhysicsUpdate(mTimeStep);
		}
		
		mSingleStep = false;
	}
	
	// 渲染调试信息
	if (mPhysicsSystem && mDebugRenderer)
	{
		BodyManager::DrawSettings draw_settings;
		draw_settings.mDrawShape = true;
		draw_settings.mDrawShapeWireframe = false;
		draw_settings.mDrawShapeColor = BodyManager::EShapeColor::InstanceColor;
		mPhysicsSystem->DrawBodies(draw_settings, mDebugRenderer, &mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), &mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING));
	}
	
	// 更新统计信息
	UpdateUI();
	
	// 调用基类渲染
	return Application::RenderFrame(inDeltaTime);
}

void PhysicsStudio::UpdateUI()
{
	if (mPhysicsSystem)
	{
		const BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
		mNumBodies = mPhysicsSystem->GetNumBodies();
		mNumActiveBodies = mPhysicsSystem->GetNumActiveBodies();
		
		// 更新统计文本
		if (mStatsText)
		{
			string stats_text = "统计信息:\n";
			stats_text += "物体: " + to_string(mNumBodies) + "\n";
			stats_text += "活跃物体: " + to_string(mNumActiveBodies) + "\n";
			stats_text += "接触点: " + to_string(mNumContacts);
			mStatsText->SetText(stats_text);
		}
	}
}

void PhysicsStudio::StartTest(const RTTI *inRTTI)
{
	if (!inRTTI || !mPhysicsSystem)
		return;
		
	// 清除现有测试
	mTest.reset();
	
	// 清除所有物体
	BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
	vector<BodyID> body_ids;
	mPhysicsSystem->GetBodies(body_ids);
	for (BodyID id : body_ids)
	{
		body_interface.RemoveBody(id);
		body_interface.DestroyBody(id);
	}
	
	// 创建新测试
	mTestClass = inRTTI;
	mTest.reset(reinterpret_cast<Test *>(inRTTI->CreateObject()));
	
	if (mTest)
	{
		mTest->SetPhysicsSystem(mPhysicsSystem.get());
		mTest->SetJobSystem(mJobSystem.get());
		mTest->SetDebugRenderer(mDebugRenderer);
		mTest->SetTempAllocator(mTempAllocator.get());
		mTest->Initialize();
		
		if (mStatusText)
			mStatusText->SetText("已加载: " + string(inRTTI->GetName()));
	}
}

void PhysicsStudio::ResetTest()
{
	if (mTestClass)
	{
		StartTest(mTestClass);
	}
}

void PhysicsStudio::OnSceneSelected()
{
	if (!mSceneComboBox)
		return;
		
	int selected = mSceneComboBox->GetSelectedItem();
	if (selected > 0 && selected <= (int)sTestClasses.size())
	{
		StartTest(sTestClasses[selected - 1]);
	}
}

void PhysicsStudio::OnGravityChanged()
{
	if (mPhysicsSystem)
	{
		mPhysicsSystem->SetGravity(Vec3(0, mGravity, 0));
	}
}

void PhysicsStudio::OnTimeStepChanged()
{
	// 时间步长在下一帧生效
}

void PhysicsStudio::OnRestitutionChanged()
{
	// 更新所有物体的反弹系数
	if (mPhysicsSystem)
	{
		BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
		vector<BodyID> body_ids;
		mPhysicsSystem->GetBodies(body_ids);
		for (BodyID id : body_ids)
		{
			body_interface.SetRestitution(id, mDefaultRestitution);
		}
	}
}

void PhysicsStudio::OnFrictionChanged()
{
	// 更新所有物体的摩擦系数
	if (mPhysicsSystem)
	{
		BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
		vector<BodyID> body_ids;
		mPhysicsSystem->GetBodies(body_ids);
		for (BodyID id : body_ids)
		{
			body_interface.SetFriction(id, mDefaultFriction);
		}
	}
}

void PhysicsStudio::OnDampingChanged()
{
	// 更新所有物体的阻尼
	if (mPhysicsSystem)
	{
		BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
		vector<BodyID> body_ids;
		mPhysicsSystem->GetBodies(body_ids);
		for (BodyID id : body_ids)
		{
			Body *body = body_interface.TryGetBody(id);
			if (body && body->IsDynamic())
			{
				MotionProperties *motion = body->GetMotionProperties();
				motion->SetLinearDamping(mLinearDamping);
				motion->SetAngularDamping(mAngularDamping);
			}
		}
	}
}

void PhysicsStudio::OnResetPressed()
{
	ResetTest();
}

void PhysicsStudio::OnPausePressed()
{
	// 暂停状态已在UI回调中设置
}

void PhysicsStudio::OnStepPressed()
{
	if (mIsPaused)
	{
		mSingleStep = true;
	}
}

void PhysicsStudio::OnSaveScene()
{
	if (!mPhysicsSystem)
		return;
		
	// 保存当前场景状态
	StateRecorderImpl recorder;
	mPhysicsSystem->SaveState(recorder);
	
	// 写入文件
	filesystem::create_directories("scenes");
	string filename = "scenes/scene_" + to_string(time(nullptr)) + ".jps";
	ofstream file(filename, ios::binary);
	if (file.is_open())
	{
		recorder.WriteToStream(file);
		file.close();
		
		if (mStatusText)
			mStatusText->SetText("场景已保存: " + filename);
	}
}

void PhysicsStudio::OnLoadScene()
{
	// 这里可以添加文件对话框来选择场景文件
	// 目前简化为加载最新的场景文件
	if (!filesystem::exists("scenes"))
		return;
		
	string latest_file;
	time_t latest_time = 0;
	
	for (const auto &entry : filesystem::directory_iterator("scenes"))
	{
		if (entry.path().extension() == ".jps")
		{
			auto write_time = filesystem::last_write_time(entry);
			auto system_time = chrono::time_point_cast<chrono::system_clock::duration>(write_time - filesystem::file_time_type::clock::now() + chrono::system_clock::now());
			time_t file_time = chrono::system_clock::to_time_t(system_time);
			
			if (file_time > latest_time)
			{
				latest_time = file_time;
				latest_file = entry.path().string();
			}
		}
	}
	
	if (!latest_file.empty())
	{
		ifstream file(latest_file, ios::binary);
		if (file.is_open())
		{
			StateRecorderImpl recorder;
			recorder.ReadFromStream(file);
			file.close();
			
			// 清除现有物体
			BodyInterface &body_interface = mPhysicsSystem->GetBodyInterface();
			vector<BodyID> body_ids;
			mPhysicsSystem->GetBodies(body_ids);
			for (BodyID id : body_ids)
			{
				body_interface.RemoveBody(id);
				body_interface.DestroyBody(id);
			}
			
			// 恢复状态
			mPhysicsSystem->RestoreState(recorder);
			
			if (mStatusText)
				mStatusText->SetText("场景已加载: " + latest_file);
		}
	}
}

void PhysicsStudio::GetInitialCamera(CameraState &ioState) const
{
	if (mTest)
	{
		mTest->GetInitialCamera(ioState);
	}
	else
	{
		ioState.mPos = Vec3(0, 10, 30);
		ioState.mForward = Vec3(0, -0.3f, -1).Normalized();
	}
}

Mat44 PhysicsStudio::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	if (mTest)
	{
		return mTest->GetCameraPivot(inCameraHeading, inCameraPitch);
	}
	return Mat44::sIdentity();
}

float PhysicsStudio::GetWorldScale() const
{
	if (mTest)
	{
		return mTest->GetWorldScale();
	}
	return 1.0f;
}