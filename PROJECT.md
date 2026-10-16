# FPS 项目文档

> 基于 **Unreal Engine 5.8** 的多人联网第一人称射击（FPS）项目。
> 以 C++ 实现 Gameplay 框架、武器/战斗、淘汰计分、UI 与联网/反作弊；支持局域网（Listen Server）联机与专用服务器（Dedicated Server）测试。

| 项 | 内容 |
|---|---|
| 引擎版本 | Unreal Engine 5.8 |
| 主模块 | `FPS`（Type: Runtime，LoadingPhase: Default） |
| 目标构建 | `FPSEditor`（编辑器）/ `FPS`（Game） |
| 实现方式 | C++（Gameplay / 网络 / UI 基类）+ 蓝图（资产 / 表现 / 关卡） |
| 网络模型 | 移动：引擎 CMC 预测；命中：客户端权威；伤害/血量/弹药/统计：服务器权威 |
| 默认地图 | 菜单 `StartupMap`、游戏 `FPSMap` |

---

## 目录

1. [项目简介](#1-项目简介)
2. [技术栈与依赖](#2-技术栈与依赖)
3. [目录结构](#3-目录结构)
4. [架构总览（Gameplay 框架）](#4-架构总览gameplay-框架)
5. [核心系统](#5-核心系统)
6. [网络模型](#6-网络模型)
7. [反作弊](#7-反作弊)
8. [输入（Enhanced Input）](#8-输入enhanced-input)
9. [联机与测试](#9-联机与测试)
10. [配置说明](#10-配置说明)
11. [已知问题 / TODO / 路线图](#11-已知问题--todo--路线图)
12. [构建与运行](#12-构建与运行)
13. [附录 A：类与文件索引](#附录-a类与文件索引)

---

## 1. 项目简介

- **定位**：小型多人联网 FPS 学习/作品项目，覆盖完整的「角色 → 武器 → 战斗 → 淘汰计分 → UI → 联机 → 反作弊」链路。
- **玩法**：自由混战（Free-for-All）。核心指标为击杀 / 死亡 / 爆头 / 命中率 / 最高连杀，并有一套「特殊淘汰」提示系统（爆头、连续淘汰、连杀、复仇、终结连杀、首杀、领先/追平/失去领先）。
- **联机**：局域网 Listen Server 联机（Host / Join 菜单），并可在编辑器中以专用服务器模式（`Play As Client`）验证服务器权威逻辑。
- **范围**：本文档聚焦 `Source/` 的 C++ 与 `Config/`；`Content/` 中的蓝图、动画、材质、关卡只描述职责，不展开具体资产。

---

## 2. 技术栈与依赖

### 2.1 模块与依赖

`Source/FPS/FPS.Build.cs`：

| 类型 | 依赖 |
|---|---|
| Public | `Core`, `CoreUObject`, `Engine`, `InputCore`, `GameplayTags`, `PhysicsCore`, `UMG`, `Slate`, `SlateCore` |
| Private | `EnhancedInput` |
| PCH | `PCHUsageMode.UseExplicitOrSharedPCHs` |

目标规则（`FPS.Target.cs` / `FPSEditor.Target.cs`）：`DefaultBuildSettings = BuildSettingsVersion.V7`。

### 2.2 插件

`FPS.uproject`:

| 插件 | 说明 |
|---|---|
| `ModelingToolsEditorMode` | 仅编辑器（`TargetAllowList: Editor`），建模工具 |
| `AnimationWarping` | 动画变形（用于移动/动画表现） |
| `CommonUI` | UI 框架（`DefaultGame.ini` 有 `CommonUISettings`） |

### 2.3 构建与渲染设置

`Config/DefaultEngine.ini` 要点：

| 设置 | 值 |
|---|---|
| 渲染器 RHI | `DefaultGraphicsRHI_DX12`，SM6（`PCD3D_SM6`） |
| 全局光照 | `r.DynamicGlobalIlluminationMethod=1`（Lumen） |
| 反射 | `r.ReflectionMethod=1`（Lumen） |
| 阴影 | `r.Shadow.Virtual.Enable=1`（虚拟阴影贴图 VSM） |
| 材质 | `r.Substrate=True` |
| 光线追踪 | `r.RayTracing=True`，`RayTracingProxies.ProjectEnabled=True` |
| 静态光照 | `r.AllowStaticLighting=False` |
| 目标硬件 | Desktop / Maximum |

---

## 3. 目录结构

### 3.1 Source 目录

`Source/FPS/Public` 与 `Private` 镜像同名子目录：

| 目录 | 职责 |
|---|---|
| `Attributes/` | `UHealthComponent`（生命值 / 最大生命 / 死亡状态） |
| `Character/` | `AShooterCharacter`（角色：双网格、相机、原地转身、FABRIK、死亡重生） |
| `Combat/` | `UCombatComponent`（武器栏、开火/换弹/切枪/瞄准、反作弊、后坐力） |
| `Data/` | `UWeaponData`、`USpecialElimData` 数据资产 |
| `Elimination/` | `UEliminationComponent`（特殊淘汰判定与聚合） |
| `Game/` | `AShooterGameModeBase`、`AShooterGameStateBase` |
| `Interfaces/` | `IPlayerInterface`（角色与武器解耦的蓝图原生接口） |
| `Player/` | `AShooterPlayerController`、`AShooterPlayerState` |
| `ShooterTypes/` | `ETurningInPlace`、`FReticleParams`、`ESpecialElimType` |
| `Tags/` | `ShooterTags`（原生 GameplayTag） |
| `UI/` | 各类 `UUserWidget`（准星、备弹、比分、Ping、特殊淘汰、计分板、LAN 菜单） |
| `Weapon/` | `AWeapon`（武器 Actor） |
| 根 | `FPS.h`（碰撞通道常量）、`FPS.cpp`（模块实现）、`FPS.Build.cs`、`ShooterGameInstance` |

### 3.2 Config / Content

| 路径 | 说明 |
|---|---|
| `Config/DefaultEngine.ini` | 地图、GameInstance 类、碰撞配置、渲染设置、CoreRedirects |
| `Config/DefaultGame.ini` | CommonUI 设置、打包设置（Development、Oodle/Kraken、`MapsToCook`） |
| `Config/DefaultInput.ini` | Enhanced Input 输入类、灵敏度、鼠标捕获 |
| `Config/DefaultEditor.ini` | 编辑器预览场景配置 |
| `Content/` | 蓝图类（HUD / Widget / 武器 / 角色）、动画、材质、关卡等（只述职责） |

---

## 4. 架构总览（Gameplay 框架）

### 4.1 类清单与职责

| 类 | 基类 | 职责 |
|---|---|---|
| `UShooterGameInstance` | `UGameInstance` | 跨关卡持久化本地玩家名（菜单输入 → 联机时通过 URL 传给服务器） |
| `AShooterGameModeBase` | `AGameModeBase` | 重生调度 `RequestRespawn`；覆写 `InitNewPlayer`（解析自定义名 / 兜底 `Player{N}`） |
| `AShooterGameStateBase` | `AGameStateBase` | 领先者计算（`UpdateLeader` / `GetSoleLeader` / `IsTiedForTheLead`） |
| `AShooterPlayerController` | `APlayerController` | Enhanced Input 绑定、存活标记、玩家名/State 就绪事件、计分板显隐事件 |
| `AShooterPlayerState` | `APlayerState` | 统计字段（复制）、特殊淘汰 RPC / 弹窗队列 |
| `AShooterCharacter` | `ACharacter` + `IPlayerInterface` | 角色实体，挂载 `CombatComponent` / `HealthComponent` / `EliminationComponent` |
| `AWeapon` | `AActor` | 武器实体：双网格、弹道、弹药、准星/弹药材质 |
| `UCombatComponent` | `UActorComponent` | 战斗逻辑与网络同步核心 |
| `UHealthComponent` | `UActorComponent` | 生命值复制与死亡状态 |
| `UEliminationComponent` | `UActorComponent` | 特殊淘汰判定（服务器） |

### 4.2 对象关系与生命周期

- **GameInstance**：进程级单例，跨关卡存活；本项目用其缓存玩家名。
- **GameMode / GameState**：仅存在于服务器（含 Listen 主机）；GameState 的 `PlayerArray` 复制给所有客户端。
- **PlayerController**：本地/远程各一份；本地玩家输入通过 `UEnhancedInputComponent` 绑定。
- **PlayerState**：每玩家一份，承载分数与统计（复制）；玩家名来自登录流程。
- **Pawn（`AShooterCharacter`）**：由 GameMode 生成；死亡后 `Destroy()` 并延迟重生新 Pawn（旧 Pawn/组件销毁）。
- **AWeapon**：服务器在 `PossessedBy → SpawnInventory()` 时生成并按物品栏装备；`CurrentWeapon` 复制给客户端。

---

## 5. 核心系统

### 5.1 角色与移动（`AShooterCharacter`）

**组件/成员**：

| 成员 | 说明 |
|---|---|
| `Mesh1P` | 第一人称骨骼网格（`bOnlyOwnerSee`，仅本人可见） |
| `GetMesh()` | 第三人称骨骼网格（`bOwnerNoSee`，本人不可见） |
| `SpringArm` | 相机臂（`bUsePawnControlRotation`，长度 0，带相机滞后） |
| `FirstPersonCamera` | 第一人称相机（`bUsePawnControlRotation=false`） |
| `CombatComponent` / `HealthComponent` / `EliminationComponent` | 逻辑组件 |
| `CycleWeaponAction` / `FireWeaponAction` / `ReloadWeaponAction` / `AimWeaponAction` | 角色输入 |

**原地转身（Turn In Place）**：`CalculateTurnInPlaceParameters()` + `TurnInPlace()`，依据控制 yaw 与瞄准旋转差的 `AO_YAW`，切换 `ETurningInPlace`（Left/Right/NotTurning）并插值归零，供动画使用。

**FABRIK**：`CalculateFABRIKSocketTransform()` 每帧从当前武器 3P 网格取 `FABRIK_Socket` 世界变换，转到角色 `hand_r` 骨骼空间，写入 `FABRIKSocketTransform`（供左手 IK）。

**受伤/死亡/重生**：
- `DoDamage_Implementation()`（接口）：非致命则 `Multicast_HitReact()` 随机播放受击蒙太奇；致命则 `UHealthComponent::ChangeHealthByAmount` 触发 `OnDeathStarted`。
- `OnDeathStarted()`：服务器销毁物品栏并 `SetTimer(DeathTimer, RespawnTime)`；客户端播放 `DeathEffects()`、禁用输入、忽略 Weapon 通道碰撞。
- `DeathTimerFinished()` → `AShooterGameModeBase::RequestRespawn()`。
- **复活后 UI 刷新**：`PossessedBy()` 中 `SpawnInventory()` 后，用 `SetTimerForNextTick` 延迟一帧调用 `CombatComponent->InitializeWeaponWidgets()`，修复「主机在 `PossessedBy` 广播早于 UI 重绑」导致的准星残留。

### 5.2 武器与战斗（`AWeapon` + `UCombatComponent`）

#### 5.2.1 武器状态与数据

`EWeaponStatus`：`Idle` / `Firing` / `Reloading` / `Cycling` / `Unequipped`。
`EFireType`：`Auto` / `SemiAuto`。

`AWeapon` 关键属性：`WeaponTypeTag`、`Damage`、`HeadShotDamageMultiplier`、`RecoilPitch/Yaw/RecoverySpeed`、`MagCapacity`、`Ammo`、`StartingCarriedAmmo`、`FireTime`、`FireType`、`AimFOV`、`ReticleParams`、材质接口（准星/弹药图标）。

`UWeaponData`（数据资产）：`GripPoints`（GameplayTag→挂点名）、`WeaponMontages` / `FirstPersonMontages` / `ThirdPersonMontages`、`FirstPersonAnims` / `ThirdPersonAnims`（`FPlayerAnims`、`FMontageData`）。

#### 5.2.2 开火时序（客户端预测 + 服务器仲裁 + 广播）

`UCombatComponent::Local_FireWeapon()`（本地，拥有者客户端）：

1. 设置武器状态 `Firing`，播放 1P/3P 角色开火蒙太奇；
2. `WeaponTrace()` 本地射线（`SweepSingleByChannel`，自定义 `ECC_Weapon`，球形半径 `TraceRadius`）；
3. `Local_Fire()` 播放特效、扣本地弹药（本地控制时）、递增 `Sequence`；
4. 广播 `OnRoundFired`（UI 刷新），启动 `FireTimer`（射速节奏），自动武器在 `FireTimerFinished` 连发；
5. 调用 `Server_FireWeapon(HitResult)` 上报命中结果。

服务器 `Server_FireWeapon_Implementation()`：

1. 校验 `CurrentWeapon` 与弹药；
2. **反作弊**：射速校验（见 [7. 反作弊](#7-反作弊)）；
3. 命中判定：`bValidHit = 上报命中 && IsShotDistanceValid()`，`bHeadShot = bValidHit && IsHeadShot()`；
4. 命中则 `IPlayerInterface::Execute_DoDamage(受害者, 伤害, 攻击者)`（伤害由**服务器**用武器数值算出）；
5. 广播 `OnRoundReported`（驱动特殊淘汰统计）；
6. `Auth_Fire()` 扣服务器弹药；`Multicast_FireWeapon(HitResult, AuthAmmo)` 广播。

`Multicast_FireWeapon_Implementation()`：本地控制者用 `Rep_fire(AuthAmmo)` 对账；其他客户端重放 3P 表现与角色蒙太奇。

#### 5.2.3 弹药预测与对账

- `AWeapon::Local_Fire()`：本地控制时 `Ammo = Clamp(Ammo-1)`，非权威则 `Sequence++`。
- `AWeapon::Auth_Fire()`：服务器权威扣弹（`Clamp`）。
- `AWeapon::Rep_fire(AuthAmmo)`：本地客户端用服务器值校正，`Sequence--; Ammo -= Sequence;` 抵消预测误差。

#### 5.2.4 换弹 / 切枪 / 瞄准

| 行为 | 入口 | 说明 |
|---|---|---|
| 换弹 | `Initiate_ReloadWeapon()` → `Local_ReloadWeapon()` + `Server_ReloadWeapon()` | 播放角色+武器换弹蒙太奇；实际装填在**动画通知** `Notify_ReloadWeapon()` 中（服务器侧仅在 `Reloading` 状态执行一次，防刷弹） |
| 切枪 | `Initiate_CycleWeapon()` → `Local_CycleWeapon()` + `Server_CycleWeapon()` | 按 `Local_WeaponIndex` 切到下一把；`Multicast_CycleWeapon` 同步其他端 |
| 装备 | `Notify_CycleWeapon()` → `EquipWeapon()` → `SetCurrentWeapon()` | 解绑旧武器、`AttachToOwningPawn`；重置 `LastFireTime` |
| 瞄准 | `Initiate_AimPressed()` → `Local_Aim()` + `Server_Aim()` | `bAiming` 复制（`COND_SkipOwner`），广播 `OnAimingStatusChanged` |

#### 5.2.5 后坐力（客户端表现）

`ApplyRecoil()`（开火时）/ `RecoverRecoil()`（Tick）：

- 通过 `APlayerController::AddPitchInput/AddYawInput` 施加/回收视角偏移；记录 `AccumulatedRecoilPitch/Yaw`，使踢与归位精确抵消、不漂移。
- 参数在 `AWeapon` 上（`RecoilPitch` / `RecoilYaw` / `RecoilRecoverySpeed`），可在武器蓝图 Class Defaults 调。
- 纯客户端、无需复制。

### 5.3 生命（`UHealthComponent`）

| 字段 | 复制 | 说明 |
|---|---|---|
| `DeathState`（`EDeathState`） | `ReplicatedUsing=OnRep_DeathState` | `NotDead/DeathStarted/DeathFinished` |
| `Health` | `COND_OwnerOnly` | 仅本人可见（防信息泄露） |
| `MaxHealth` | `COND_OwnerOnly` | 同上 |

- `ChangeHealthByAmount()`：**仅服务器**（`HasAuthority` 断言），Clamp 到 `[0, MaxHealth]`，广播 `OnHealthChanged`，归零触发 `StartDeath()`。
- `GetAccuracy()` 等统计由 `AShooterPlayerState` 负责（见 5.4）。

### 5.4 特殊淘汰（`UEliminationComponent` + `AShooterPlayerState`）

**`ESpecialElimType`（`uint16` 位标志）**：

| 位 | 名称 | 含义 |
|---|---|---|
| 0 | `HeadShot` | 爆头击杀 |
| 1 | `Sequential` | 快速连续淘汰（时间窗 `SequentialElimInterval`） |
| 2 | `Streak` | 连杀（达 `ElimsNeededForStreak`） |
| 3 | `Revenge` | 复仇（击杀上一个击杀你的人） |
| 4 | `Dethrone` | 终结当前领先者 |
| 5 | `ShowStopper` | 终结正在连杀的人 |
| 6 | `FirstBlood` | 首杀 |
| 7 | `GainedTheLead` | 取得领先 |
| 8 | `TiedTheLeader` | 追平领先 |
| 9 | `LostTheLead` | 失去领先 |

**事件流**（服务器）：`CombatComponent::OnRoundReported` → `UEliminationComponent::OnRoundReported`：

1. `ProcessHitOrMiss` 累计命中/失误；
2. 若致命：`ProcessElimination` → `AddScoredElim`/`AddDefeat`，依次 `ProcessHeadShot` / `ProcessSequentialEliminations` / `ProcessStreaks`，再用 `AShooterGameStateBase` 处理首杀与领先状态，把多个标志聚合进**一个位掩码**；
3. 若有特殊类型：`AShooterPlayerState::Client_SpecialElim(位掩码, 连杀数, 连杀状态, 分数)`；否则 `Client_ScoredElim(分数)`；
4. 失去领先者对**本人**发 `Client_LostTheLead()`。

**客户端**：`DecodeSpecialElimBitmask()` 逐位解码 → 从 `USpecialElimData` 取文案/图标 → 入 `SpecialElimQueue` → `ProcessNextSpecialElim()` 按 `ElimDisplayTime` 逐个生成 `USpecialElimWidget` 弹窗。

> 设计要点：**状态用复制，事件用 RPC**；一次淘汰可能同时命中多个特殊类型，故用**位掩码**打包进单个 RPC（2 字节）。

### 5.5 UI 系统

| 控件 | 基类 | 数据来源 | 说明 |
|---|---|---|---|
| `UShooterReticleWidget` | `UUserWidget` | `UCombatComponent` 委托 | 准星材质（缩放/裁切/颜色），命中敌人变红；`NativeTick` 插值 |
| `UReserveAmmoWidget` | `UUserWidget` | `OnCurrentReserveAmmoChanged` / `OnRoundFired` | 当前弹匣 / 备弹 / 武器图标 |
| `UScoreWidget` | `UUserWidget` | `AShooterPlayerState::OnScoreChanged` | 本人分数 |
| `UPingWidget` | `UUserWidget` | `AShooterPlayerState::GetPingInMilliseconds()` | 定时轮询显示本人 Ping |
| `USpecialElimWidget` | `UUserWidget` | `InitializeWidget(文案, 图标)` | 特殊淘汰弹窗 |
| `UScoreboardWidget` | `UUserWidget` | `GameState->PlayerArray` | 按住 Tab 显示，遍历生成 `UScoreboardRowWidget` |
| `UScoreboardRowWidget` | `UUserWidget` | `AShooterPlayerState` | 单行：名字/击杀/死亡/爆头/命中率/最高连杀/Ping；本人行高亮绿色 |
| `ULANMenuWidget` | `UUserWidget` | — | Host / Join / Quit；输入玩家名与 IP |

**就绪/重绑模式**：多数控件在 `NativeOnInitialized` 订阅所在 Pawn 的组件委托；未就绪时挂 `AShooterPlayerController::OnPlayerStateReplicated` 延迟启动；并监听 `OnPossessedPawnChanged` 以应对重生。

### 5.6 数据资产与 GameplayTag

- `UWeaponData`：抓握点、武器/角色蒙太奇、1P/3P 动画（按武器类型 Tag 索引）。
- `USpecialElimData`：`ESpecialElimType → FSpecialElimInfo`（文案、图标、连杀数）。
- `ShooterTags`：`Weapon.Type.None` / `Weapon.Type.Rifle` / `Weapon.Type.Pistol`（原生 GameplayTag）。

---

## 6. 网络模型

### 6.1 权威划分

| 行为 | 权威方 | 机制 |
|---|---|---|
| 移动 | 服务器 | `UCharacterMovementComponent` 客户端预测 + 服务器校正 |
| 命中判定 | **客户端** | 客户端本地 `WeaponTrace`，把 `FHitResult` 交服务器采信（客户端权威） |
| 伤害数值 | 服务器 | 服务器用自己武器的 `Damage`/`HeadShotDamageMultiplier` 计算 |
| 血量 | 服务器 | `UHealthComponent`（`HasAuthority`） |
| 弹药 | 服务器 | `Auth_Fire` + 客户端 `Rep_fire` 对账 |
| 统计/淘汰 | 服务器 | `AShooterPlayerState` 复制 + Client RPC |
| 计时（受击/重生/换弹/射速） | 服务器/动画 | 服务器侧定时器与动画通知 |

> 取舍说明：本项目命中采用**客户端权威**（所见即所中、免回滚），代价是可作弊；见 [7. 反作弊](#7-反作弊) 的缓解与 [11. 路线图](#11-已知问题--todo--路线图) 的改进方向。

### 6.2 RPC 清单

| RPC | 方向 | 用途 |
|---|---|---|
| `Server_Aim(bool)` | 客户→服 | 同步瞄准状态 |
| `Server_FireWeapon(FHitResult)` | 客户→服 | 上报命中、服务器结算 |
| `Server_DryFire()` | 客户→服 | 空仓提示 |
| `Server_CycleWeapon(int32)` | 客户→服 | 切枪索引 |
| `Server_EquipWeapon(AWeapon*)` | 客户→服 | 请求装备 |
| `Server_ReloadWeapon()` | 客户→服 | 请求换弹 |
| `Client_ReloadWeapon(int32,int32)` | 服→客 | 换弹结果（弹匣/备弹） |
| `Multicast_FireWeapon(FHitResult,int32)` | 服→全体 | 广播开火表现/对账 |
| `Multicast_DryFire()` / `Multicast_CycleWeapon(int32)` / `Multicast_ReloadWeapon()` | 服→全体 | 表现同步 |
| `Multicast_HitReact(int32)` | 服→全体 | 受击蒙太奇 |
| `Client_ScoredElim(int32)` / `Client_SpecialElim(...)` / `Client_LostTheLead()` | 服→客 | 本人分数/特殊淘汰/失去领先 |

### 6.3 复制属性与条件

| 组件/类 | 属性 | 条件 |
|---|---|---|
| `UHealthComponent` | `DeathState` | 全部客户端 |
| `UHealthComponent` | `Health` / `MaxHealth` | `COND_OwnerOnly` |
| `UCombatComponent` | `Inventory` / `CurrentWeapon` | 复制（`OnRep_CurrentWeapon`） |
| `UCombatComponent` | `bAiming` | `COND_SkipOwner` |
| `UCombatComponent` | `CurrentReserveAmmo` | `COND_OwnerOnly`（`OnRep`） |
| `AShooterPlayerState` | `ScoredElims`/`HeadShotElims`/`Defeats`/`Hits`/`Misses`/`HighestStreak` | 全部客户端（供计分板） |
| `AWeapon` | 弹药/状态 | **不复制**，由 RPC 驱动 |

### 6.4 预测与对账

- **开火**：客户端立即表现并扣弹，服务器仲裁后 `Multicast` 回传 `AuthAmmo`，`Rep_fire` 校正。
- **射速**：客户端 `FireTimer` 控制连发节奏；服务器 `LastFireTime` 反作弊兜底。
- **复活**：新 Pawn 生成后延迟一帧刷新 UI，规避广播早于重绑。

### 6.5 Listen 与 Dedicated 差异

| 维度 | Listen Server | Dedicated Server |
|---|---|---|
| 本地玩家 | 有（主机） | 无 |
| 渲染 | 有 | 无 → **角色网格 `VisibilityBasedAnimTickOption` 需为 `OnlyTickMontagesWhenNotRendered`**，否则换弹等「动画通知」类服务器逻辑不触发 |
| 分支 | `GetNetMode()==NM_ListenServer` 相关分支会命中 | 走纯远程路径 |
| Ping | 主机自身 ≈ 0 | 无本地玩家 |

> 测试提示：验证网络/联机请用 **Play As Listen Server / Standalone**；验证服务器权威逻辑用 **Play As Client（专用）**。

### 6.6 玩家名字与登录流程

1. 菜单输入名字 → 存入 `UShooterGameInstance`（跨关卡保留）。
2. Host：`OpenLevelBySoftObjectPtr(..., "/Game/Maps/FPSMap", true, "Listen?PlayerAlias=<名>")`；Join：`OpenLevel(this, FName(IP), false, "PlayerAlias=<名>")`。
3. 引擎 `AGameModeBase::InitNewPlayer` 解析 URL 选项；本项目覆写 `AShooterGameModeBase::InitNewPlayer`：
   - 先 `Super::InitNewPlayer(...)`；
   - 再 `ParseOption(Options, "PlayerAlias")`，为空则 `Player{N}`（自增）；
   - `ChangeName(...)` 覆盖，保证最终名。

> **坑**：引擎在 `ULocalPlayer::SpawnPlayActor` 与 `UPendingNetGame` 连接挑战时，会用**平台昵称（机器名）覆盖选项 `Name`**，且 `ULocalPlayer` 无 `SetNickname`。故自定义名**不能**用 `Name` 这个 key，改用 `PlayerAlias`。

---

## 7. 反作弊

服务器端校验（`UCombatComponent::Server_FireWeapon_Implementation`）：

| 项 | 实现 | 说明 |
|---|---|---|
| 射速 | `FireRateTolerance`（默认 0.8） | `Now - LastFireTime < FireTime * Tolerance` → 拒绝 |
| 距离 | `IsShotDistanceValid()` | 服务器眼位到命中点 ≤ `TraceLength * ShotDistanceTolerance`（默认余量 ~20m，远大于延迟误差） |
| 爆头 | `IsHeadShot()` | 命中组件须为受害者骨骼网格、`BoneName` 真实存在且等于 `HeadBoneName`（默认 `head`）；不直接采信客户端字符串 |
| 换弹防刷 | `Notify_ReloadWeapon()` | 服务器侧仅 `Reloading` 状态执行一次装填 |
| 权威断言 | `UHealthComponent::Change(Health/MaxHealth)ByAmount`、`AShooterCharacter::DoDamage_Implementation` | 非 `HasAuthority` 直接返回 |

**残余风险**：命中仍是客户端权威；`BoneName` 由客户端上报；无服务器 LOS/回滚校验。详见路线图。

---

## 8. 输入（Enhanced Input）

- 输入类：`DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`；`DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`（`DefaultInput.ini`）。
- 映射上下文：`AShooterPlayerController::ShooterIMC`（`BeginPlay` 添加，优先级 0）。

| Action（C++ 属性） | 绑定位置 | 事件 |
|---|---|---|
| `MoveAction` / `LookAction` | `AShooterPlayerController` | `Triggered` |
| `JumpAction` / `CrouchAction` | `AShooterPlayerController` | `Started` |
| `ScoreboardAction` | `AShooterPlayerController` | `Started`（显示）/ `Completed`（隐藏） |
| `CycleWeaponAction` / `FireWeaponAction` / `ReloadWeaponAction` / `AimWeaponAction` | `AShooterCharacter` | `Started` / `Completed` |

- 计分板采用**常驻控件 + 切可见性**：控制器广播 `OnScoreboardDisplayChanged(bool)`，`UScoreboardWidget` 订阅后显示/隐藏并起停刷新 Timer；可见时设为不拦截输入（`HitTestInvisible`），按住 Tab 仍可移动。

---

## 9. 联机与测试

### 9.1 局域网菜单（Host / Join）

- **Host**：`OpenLevelBySoftObjectPtr(this, HostingLevel, true, Options)`，`Options` 含 `Listen` 与可选 `PlayerAlias`（多项以 `?` 分隔，如 `Listen?PlayerAlias=Bob`）。
- **Join**：`OpenLevel(this, FName(IP), false, Options)`，`Options` 为 `PlayerAlias=Bob`（`OpenLevel` 会自动补 `?`）。
- 前置：项目设置 **GameInstance Class = `ShooterGameInstance`**（`DefaultEngine.ini` 的 `GameInstanceClass=/Script/FPS.ShooterGameInstance`）；菜单 WBP 里把 `HostingLevel` 指向 `FPSMap`，并绑定 `TextBox_PlayerName` / `TextBox_IpAddress` / 各按钮。

### 9.2 PIE 模式选择

| 模式 | 说明 | 适用 |
|---|---|---|
| `Play Standalone` | 单机 | 单机/表现 |
| `Play As Listen Server` | 主机 + 客户端 | **联机菜单（Host/Join）** |
| `Play As Client` | 专用服务器 + 客户端（服务器无本地玩家） | **验证服务器权威/复制**（换弹、反作弊等） |

> 联机菜单走 `?listen` 路径，只能在**有本地玩家**的世界生效；用 `Play As Client` 点 Host 是起不来监听服务器的。

### 9.3 延迟/丢包模拟（引擎控制台命令）

| 命令 | 作用 |
|---|---|
| `Net PktLag=200` | 每包延迟 ~200ms |
| `Net PktLagVariance=50` | 抖动 ±50ms |
| `Net PktLoss=2` | 2% 丢包 |
| `Net PktOrder=1` | 乱序 |
| `Net PktDup=…` | 重复包 |

复位：`Net PktLag=0` 等。配合 `stat net` / `stat netpackets` / `stat ping` 观察。命令作用于**敲命令的那个进程**的连接。

---

## 10. 配置说明

### 10.1 `DefaultEngine.ini`

| 段 | 关键项 |
|---|---|
| `[/Script/EngineSettings.GameMapsSettings]` | `GameDefaultMap`/`EditorStartupMap`/`ServerDefaultMap = /Game/Maps/StartupMap`；`GameInstanceClass=/Script/FPS.ShooterGameInstance` |
| `[/Script/Engine.CollisionProfile]` | 自定义通道 `ECC_GameTraceChannel1 = "Weapon"`；`Pawn` 配置对 Weapon 忽略；物理表面 `SurfaceType1=Character`、`SurfaceType2=Concrete`、`SurfaceType3=Glass` |
| `[/Script/Engine.RendererSettings]` | DX12/SM6、Lumen、VSM、Substrate、RayTracing 等 |
| `[CoreRedirects]` | 历史属性/函数/枚举重定向 |

### 10.2 碰撞通道（`FPS.h`）

```cpp
namespace FPSTraceChannels
{
    constexpr ECollisionChannel ECC_Weapon = ECC_GameTraceChannel1;
}
```
武器射线走自定义 `Weapon` 通道；角色/武器相关逻辑据此设置响应。

### 10.3 `DefaultInput.ini` / `DefaultGame.ini`

- 输入：EnhancedInput 默认类、鼠标灵敏度、鼠标捕获模式。
- 游戏：CommonUI 设置；打包配置（`Development`、Oodle/Kraken、`MapsToCook` 含 `StartupMap`/`FPSMap`）。

---

## 11. 构建与运行

### 11.1 环境

- Unreal Engine 5.8；Windows + Visual Studio 工具链（MSVC 14.4x）。
- 生成项目文件后即可在 IDE 构建；或命令行：

```
& "<引擎目录>\Engine\Build\BatchFiles\Build.bat" FPSEditor Win64 Development -Project="<项目路径>\FPS.uproject" -WaitMutex
```

> 若编辑器开启 **Live Coding**，命令行构建会失败（`Unable to build while Live Coding is active`）。请关闭编辑器，或在编辑器中按 `Ctrl+Alt+F11`。

### 11.2 运行与联机

1. 打开 `FPS.uproject`，运行 `StartupMap`（菜单）。
2. **本机自测**：PIE 用 `Play As Listen Server` + 2 玩家；或 `Play As Client`（专用）验证服务器权威。
3. **局域网**：主机以 `Standalone Game`/打包（Development）运行，输入名字点 Host（进 `FPSMap?Listen`）；另一台机输入主机 IP 点 Join。
4. **延迟测试**：客户端控制台 `Net PktLag=200` 等。

---

## 附录 A：类与文件索引

| 类 | 头文件 | 源文件 |
|---|---|---|
| `FPSTraceChannels`（命名空间） | `FPS.h` | `FPS.cpp` |
| `UShooterGameInstance` | `ShooterGameInstance.h` | `ShooterGameInstance.cpp` |
| `AShooterGameModeBase` | `Game/ShooterGameModeBase.h` | `Game/ShooterGameModeBase.cpp` |
| `AShooterGameStateBase` | `Game/ShooterGameStateBase.h` | `Game/ShooterGameStateBase.cpp` |
| `AShooterPlayerController` | `Player/ShooterPlayerController.h` | `Player/ShooterPlayerController.cpp` |
| `AShooterPlayerState` | `Player/ShooterPlayerState.h` | `Player/ShooterPlayerState.cpp` |
| `AShooterCharacter` | `Character/ShooterCharacter.h` | `Character/ShooterCharacter.cpp` |
| `AWeapon` | `Weapon/Weapon.h` | `Weapon/Weapon.cpp` |
| `UCombatComponent` | `Combat/CombatComponent.h` | `Combat/CombatComponent.cpp` |
| `UHealthComponent` | `Attributes/HealthComponent.h` | `Attributes/HealthComponent.cpp` |
| `UEliminationComponent` | `Elimination/EliminationComponent.h` | `Elimination/EliminationComponent.cpp` |
| `IPlayerInterface` | `Interfaces/PlayerInterface.h` | `Interfaces/PlayerInterface.cpp` |
| `UWeaponData` / `FPlayerAnims` / `FMontageData` | `Data/WeaponData.h` | `Data/WeaponData.cpp` |
| `USpecialElimData` / `FSpecialElimInfo` | `Data/SpecialElimData.h` | `Data/SpecialElimData.cpp` |
| `ShooterTags`（命名空间） | `Tags/ShooterGameplayTags.h` | `Tags/ShooterGameplayTags.cpp` |
| `ETurningInPlace` / `FReticleParams` / `ESpecialElimType` | `ShooterTypes/ShooterTypes.h` | `ShooterTypes/ShooterTypes.cpp` |
| `UShooterReticleWidget` | `UI/ShooterReticleWidget.h` | `UI/ShooterReticleWidget.cpp` |
| `UReserveAmmoWidget` | `UI/ReserveAmmoWidget.h` | `UI/ReserveAmmoWidget.cpp` |
| `UPingWidget` | `UI/PingWidget.h` | `UI/PingWidget.cpp` |
| `ULANMenuWidget` | `UI/LANMenuWidget.h` | `UI/LANMenuWidget.cpp` |
| `UScoreWidget` | `UI/Elims/ScoreWidget.h` | `UI/Elims/ScoreWidget.cpp` |
| `USpecialElimWidget` | `UI/Elims/SpecialElimWidget.h` | `UI/Elims/SpecialElimWidget.cpp` |
| `UScoreboardWidget` | `UI/Elims/ScoreboardWidget.h` | `UI/Elims/ScoreboardWidget.cpp` |
| `UScoreboardRowWidget` | `UI/Elims/ScoreboardRowWidget.h` | `UI/Elims/ScoreboardRowWidget.cpp` |

> 头文件位于 `Source/FPS/Public/`，源文件位于 `Source/FPS/Private/`。
