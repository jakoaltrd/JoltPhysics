// SPDX-FileCopyrightText: 2024 PhysicsStudio
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// 基础游乐场测试 - 展示各种物理交互的场景
class BasicPlaygroundTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(BasicPlaygroundTest)

	// Destructor
	virtual					~BasicPlaygroundTest() override = default;

	// Initialize the test
	virtual void			Initialize() override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override		{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	// Update the test
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	// 创建不同类型的物理对象
	void					CreateFloorAndWalls();
	void					CreateStackOfBoxes();
	void					CreateBouncingBalls();
	void					CreateSeesaw();
	void					CreatePendulum();
	void					CreateRamp();
	
	// 添加交互功能
	void					SpawnRandomObject();
	void					ResetScene();

	// 场景参数
	bool					mEnableWind = false;
	float					mWindStrength = 5.0f;
	bool					mEnableRandomSpawn = false;
	float					mSpawnTimer = 0.0f;
	float					mSpawnInterval = 2.0f;
	
	// 场景对象
	vector<BodyID>			mDynamicBodies;
	BodyID					mFloorID;
	BodyID					mSeesawPivotID;
	BodyID					mSeesawPlankID;
	BodyID					mPendulumAnchorID;
	BodyID					mPendulumBobID;
};