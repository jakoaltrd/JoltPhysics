# PhysicsStudio 完整使用指南

## 项目概述

PhysicsStudio 是一个基于 Jolt Physics 引擎开发的完整图形用户界面工具，专门用于物理模拟的可视化和交互式参数调节。该工具提供了一个直观的界面，让用户能够：

- 🎮 **加载和运行各种物理场景**
- 🔧 **实时调节物理参数**  
- 📊 **可视化物理模拟过程**
- 💾 **保存和加载场景状态**
- ⏯️ **控制模拟播放和暂停**

## 核心功能特性

### 1. 场景管理系统
- **智能场景加载**：自动检测所有可用的Jolt Physics测试场景
- **场景切换**：通过下拉菜单快速切换不同的物理演示
- **状态保存**：将当前物理世界状态保存为`.jps`文件
- **状态恢复**：加载之前保存的场景状态

### 2. 实时物理参数调节
| 参数类型 | 调节范围 | 实时效果 |
|---------|---------|----------|
| 重力强度 | -50 到 50 m/s² | 立即影响所有物体的下落行为 |
| 时间步长 | 30fps 到 120fps | 调节模拟精度和速度 |
| 反弹系数 | 0 到 1 | 控制碰撞后的弹性程度 |
| 摩擦系数 | 0 到 2 | 影响物体间的摩擦力大小 |
| 线性阻尼 | 0 到 1 | 控制直线运动的阻力 |
| 角阻尼 | 0 到 1 | 控制旋转运动的阻力 |

### 3. 高级模拟控制
- **暂停/继续**：随时暂停或恢复物理模拟
- **单步执行**：在暂停状态下逐帧前进
- **场景重置**：一键恢复场景到初始状态
- **实时统计**：显示物体数量、活跃物体数和接触点数

### 4. 3D可视化引擎
- **实时渲染**：流畅的3D物理世界可视化
- **调试绘制**：显示物体形状、碰撞框、接触点等
- **自由视角**：支持鼠标控制的3D摄像机
- **材质着色**：不同材质的物体用不同颜色显示

## 安装和构建

### 系统要求
- **操作系统**：Windows 10/11
- **编译器**：Visual Studio 2019 或 2022
- **依赖**：CMake 3.16+
- **硬件**：支持OpenGL的显卡

### 构建步骤

1. **快速构建**（推荐）：
   ```batch
   # 运行自动构建脚本
   build_physics_studio.bat
   ```

2. **手动构建**：
   ```batch
   # 进入构建目录
   cd Build
   
   # 生成项目文件
   cmake_vs2022_cl.bat
   
   # 编译项目
   cmake --build ../build --config Release --target PhysicsStudio
   ```

3. **运行应用**：
   ```batch
   cd build/Release
   PhysicsStudio.exe
   ```

## 详细使用教程

### 基础操作流程

#### 步骤1：启动应用
- 双击运行 `PhysicsStudio.exe`
- 等待应用程序加载完成
- 界面左侧会显示控制面板

#### 步骤2：选择场景
- 在"场景选择"下拉菜单中选择一个测试场景
- 推荐从 `BasicPlaygroundTest` 开始体验
- 场景加载完成后，状态栏会显示"已加载"信息

#### 步骤3：调节参数
- 使用滑块调节各种物理参数
- 观察参数变化对物理行为的实时影响
- 尝试不同的参数组合，体验物理特性

#### 步骤4：控制模拟
- 使用"暂停"复选框暂停模拟
- 在暂停状态下使用"单步"按钮逐帧查看
- 使用"重置"按钮恢复场景初始状态

### 高级功能使用

#### 自定义场景创建
PhysicsStudio 包含一个演示自定义场景 `BasicPlaygroundTest`，展示了：
- **物体堆叠**：盒子塔的重力坍塌
- **弹球系统**：高反弹球的碰撞行为
- **跷跷板**：铰链约束的机械运动
- **单摆**：距离约束的周期运动
- **斜坡滚动**：摩擦力和重力的综合作用

#### 交互控制
- **R键**：重置当前场景
- **空格键**：随机生成新物体
- **鼠标拖拽**：旋转3D视角
- **滚轮**：缩放视图

#### 场景持久化
1. **保存场景**：
   - 点击"保存场景"按钮
   - 文件自动保存到 `scenes/` 目录
   - 文件名包含时间戳，格式：`scene_timestamp.jps`

2. **加载场景**：
   - 点击"加载场景"按钮
   - 自动加载最新保存的场景文件
   - 支持跨会话的状态恢复

## 界面布局说明

### 控制面板（左侧）

```
┌─────────────────────────┐
│      场景选择            │
├─────────────────────────┤
│      物理参数            │
│  ├ 重力 [-50, 50]       │
│  ├ 时间步长 [30,120fps] │
│  ├ 反弹系数 [0, 1]      │
│  ├ 摩擦系数 [0, 2]      │
│  ├ 线性阻尼 [0, 1]      │
│  └ 角阻尼 [0, 1]        │
├─────────────────────────┤
│      模拟控制            │
│  ├ [✓] 暂停             │
│  ├ [单步] [重置]        │
│  ├ [保存场景]           │
│  └ [加载场景]           │
├─────────────────────────┤
│      状态信息            │
│  ├ 当前状态: 运行中     │
│  ├ 物体数: 156          │
│  ├ 活跃物体: 23         │
│  └ 接触点: 89           │
└─────────────────────────┘
```

### 3D视图（右侧）
- **主渲染区域**：显示3D物理世界
- **调试信息**：物体轮廓、接触点等
- **状态指示器**：FPS、模拟时间等

## 实践案例

### 案例1：重力实验
1. 选择 `BasicPlaygroundTest` 场景
2. 将重力设置为 0，观察物体悬浮
3. 逐渐增加重力强度，观察下落加速
4. 设置负重力，观察物体"上浮"效果

### 案例2：摩擦力分析
1. 观察斜坡上圆柱体的滚动
2. 将摩擦系数调至最小（0），观察无摩擦滑动
3. 调至最大（2），观察高摩擦阻滞效果
4. 对比不同摩擦系数下的运动差异

### 案例3：碰撞弹性研究
1. 观察弹球的反弹行为
2. 将反弹系数设为0，观察完全非弹性碰撞
3. 设为1，观察完全弹性碰撞
4. 中间值观察真实世界的碰撞效果

### 案例4：阻尼效应
1. 观察单摆的摆动
2. 调节线性阻尼，观察摆动幅度衰减
3. 调节角阻尼，观察旋转运动的衰减
4. 找到最佳阻尼值模拟真实环境

## 技术架构

### 核心组件
```
PhysicsStudio
├── 物理引擎 (Jolt Physics)
├── 渲染系统 (TestFramework)
├── GUI系统 (UI Components)
├── 场景管理 (Test Framework)
└── 资源管理 (Assets)
```

### 性能特性
- **多线程支持**：利用多核CPU加速物理计算
- **内存优化**：高效的临时内存分配器
- **实时渲染**：60fps+ 的流畅可视化
- **大规模模拟**：支持10,000+物体的场景

### 扩展性
- **插件式测试**：易于添加新的物理场景
- **参数化设计**：所有物理属性可调节
- **状态序列化**：完整的场景保存/加载机制

## 故障排除

### 常见问题解决

#### 构建问题
**问题**：CMake配置失败
**解决**：
1. 确认Visual Studio 2019/2022已安装
2. 检查CMake版本是否为3.16+
3. 以管理员权限运行构建脚本

**问题**：编译错误
**解决**：
1. 清理构建目录：`rm -rf build/`
2. 重新运行构建脚本
3. 检查路径中是否包含中文字符

#### 运行问题
**问题**：启动闪退
**解决**：
1. 检查显卡驱动是否最新
2. 确认支持OpenGL 3.3+
3. 在项目根目录运行程序

**问题**：场景加载失败
**解决**：
1. 确认Test类已正确注册
2. 检查RTTI系统是否启用
3. 验证场景文件格式

#### 性能问题
**问题**：帧率过低
**解决**：
1. 减少物体数量
2. 降低时间步长精度
3. 关闭不必要的调试绘制

**问题**：内存占用过高
**解决**：
1. 定期重置场景
2. 避免过度生成随机物体
3. 调整临时分配器大小

## 开发扩展

### 添加自定义场景

1. **创建测试类**：
```cpp
// CustomTest.h
class CustomTest : public Test
{
public:
    JPH_DECLARE_RTTI_VIRTUAL(CustomTest)
    virtual void Initialize() override;
    virtual void PrePhysicsUpdate(const PreUpdateParams &inParams) override;
};

// CustomTest.cpp
JPH_IMPLEMENT_RTTI_VIRTUAL(CustomTest)
{
    JPH_ADD_BASE_CLASS(CustomTest, Test)
}
```

2. **注册到CMake**：
在 `PhysicsStudio.cmake` 中添加源文件

3. **实现场景逻辑**：
在 `Initialize()` 中创建物理对象
在 `PrePhysicsUpdate()` 中处理实时更新

### 自定义UI组件

1. **继承UI基类**：
```cpp
class CustomSlider : public UISlider
{
public:
    // 自定义滑块行为
};
```

2. **集成到主界面**：
在 `CreateUI()` 中添加自定义组件

## 未来发展

### 计划功能
- **材质编辑器**：可视化材质属性编辑
- **约束编辑器**：图形化约束创建和编辑  
- **性能分析器**：实时性能监控和优化建议
- **脚本系统**：Lua脚本支持自定义行为
- **网络同步**：多客户端物理同步

### 平台扩展
- **Linux支持**：使用GTK或Qt界面
- **macOS支持**：Cocoa原生界面
- **Web版本**：WebAssembly + WebGL

## 许可与贡献

PhysicsStudio 基于 MIT 许可证开源，欢迎社区贡献：

- **Bug报告**：提交Issue描述问题
- **功能建议**：讨论新功能需求
- **代码贡献**：提交Pull Request
- **文档完善**：改进使用文档

---

**联系方式**：
- 项目地址：基于Jolt Physics引擎
- 文档站点：参考项目README
- 技术支持：查看Jolt Physics官方文档

祝您使用PhysicsStudio愉快！🚀