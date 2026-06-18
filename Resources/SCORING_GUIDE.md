# JSON 计分系统使用指南

## 概述

Angry Birds 游戏现在支持通过 JSON 配置文件来管理计分系统。这允许你在不重新编译游戏的情况下调整计分规则。

## JSON 配置文件位置

配置文件位于: `Resources/scoring_config.json`

## 配置结构

### 1. 基础分数配置 (basic_points)

```json
"basic_points": {
  "pig_destroyed": 5000,      // 摧毁一只猪得到的分数
  "leftover_bird": 10000,     // 剩余每只鸟得到的分数
  "special_item": 3000        // 摧毁特殊物品得到的分数
}
```

### 2. 材料分数配置 (material_points)

针对不同物质类型的分数规则：

```json
"material_points": {
  "glass": {
    "damage_budget": 500,        // 伤害预算 (造成伤害时的得分上限)
    "destroyed_points": 500,     // 完全摧毁时的分数
    "damage_multiplier": 1.0     // 伤害倍数
  },
  "ice": {
    "damage_budget": 500,
    "destroyed_points": 500,
    "damage_multiplier": 1.0
  },
  "wood": {
    "damage_budget": 800,
    "destroyed_points": 900,
    "damage_multiplier": 1.0
  },
  "stone": {
    "damage_budget": 1200,
    "destroyed_points": 1300,
    "damage_multiplier": 1.0
  }
}
```

**材料类型说明:**
- **Glass (玻璃)**: 易碎材料，最低伤害预算
- **Ice (冰)**: 冰冻材料，伤害预算同玻璃
- **Wood (木头)**: 中等硬度，伤害预算 800
- **Stone (石头)**: 最坚硬，伤害预算 1200

### 3. 难度倍数 (difficulty_multipliers)

```json
"difficulty_multipliers": {
  "easy": 1.0,      // 简单难度，正常得分
  "normal": 1.0,    // 普通难度，正常得分
  "hard": 1.5,      // 困难难度，得分乘以 1.5
  "extreme": 2.0    // 极难，得分乘以 2.0
}
```

## 如何在代码中使用

### 在游戏加载时加载配置

配置在 `GameScene::LoadLevel()` 中自动加载:

```cpp
m_ScoringSystem.LoadConfig(RESOURCE_DIR "/scoring_config.json");
```

### 获取计分信息

```cpp
// 获取伤害预算
int budget = m_ScoringSystem.GetDamageBudget(materialType);

// 获取摧毁分数
int points = m_ScoringSystem.GetDestroyedPoints(materialType);

// 获取当前分数
int currentScore = m_ScoringSystem.GetScore();

// 获取最高分
int highScore = m_ScoringSystem.GetHighScore();

// 设置难度倍数
m_ScoringSystem.SetDifficultyMultiplier(1.5f);
```

### 增加分数

```cpp
// 猪被摧毁
m_ScoringSystem.AwardPigDestroyed();

// 剩余鸟奖励 (参数: 剩余鸟数)
m_ScoringSystem.AwardLeftoverBirds(3);

// 特殊物品被摧毁
m_ScoringSystem.AwardSpecialItemDestroyed();

// 方块伤害奖励
// 参数: 物质类型, 伤害比例 (0.0-1.0), 剩余预算
m_ScoringSystem.AwardBlockDamage(
  Character::MaterialType::Stone, 
  0.75f,  // 75% 伤害
  1000    // 剩余预算
);

// 方块完全摧毁
m_ScoringSystem.AwardBlockDestroyed(
  Character::MaterialType::Wood
);
```

## 扩展配置示例

### 示例 1: 提高难度等级的分数

```json
{
  "difficulty_multipliers": {
    "easy": 0.8,
    "normal": 1.0,
    "hard": 1.5,
    "extreme": 2.5
  }
}
```

### 示例 2: 石头更难摧毁

```json
{
  "material_points": {
    "stone": {
      "damage_budget": 1500,       // 增加预算
      "destroyed_points": 1500,    // 增加完全摧毁分数
      "damage_multiplier": 1.2     // 增加伤害倍数
    }
  }
}
```

### 示例 3: 更高的猪分数

```json
{
  "basic_points": {
    "pig_destroyed": 7500,      // 从 5000 增加到 7500
    "leftover_bird": 15000,     // 从 10000 增加到 15000
    "special_item": 5000        // 从 3000 增加到 5000
  }
}
```

## 注意事项

1. **JSON 格式**: 确保 JSON 格式完全正确，否则加载会失败
2. **文件位置**: 配置文件必须在 `Resources/` 目录中
3. **默认值**: 如果某些字段缺失，系统会使用默认值
4. **难度倍数**: 在游戏运行时可以通过 `SetDifficultyMultiplier()` 动态改变
5. **日志**: 配置加载成功会输出日志信息

## 调试

配置加载时，系统会输出:

```
Scoring config loaded successfully from: Resources/scoring_config.json
```

如果加载失败:

```
Failed to open scoring config file: Resources/scoring_config.json
```

## 性能考虑

- 配置在关卡加载时只读取一次
- 对性能无影响
- 建议在开发时调整配置，生产环境使用最终版本
