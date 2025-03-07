# Simple_Json_Cpp
简单的库，可以对json实现大幅度控制与修改，一个特别轻量级的json修改头

# JSON 处理库开发文档

## 目录
1. [简介](#简介)  
2. [功能特性](#功能特性)  
3. [快速开始](#快速开始)  
4. [API 详解](#api-详解)  
5. [示例代码](#示例代码)  
6. [注意事项](#注意事项)  
7. [依赖与兼容性](#依赖与兼容性)  

---

## 简介
本库提供了一套轻量级的 C++ JSON 处理工具，包含以下两个核心组件：
- **Json.h**：实现 JSON 数据的解析、序列化与基础操作，支持多种数据类型（对象、数组、字符串、数字等）。  
- **JsonEditor.h**：提供基于键路径的 JSON 数据编辑功能，支持自动创建嵌套结构。  

适用于需要动态修改 JSON 配置、处理网络请求响应或实现数据持久化的场景。

---

## 功能特性
### Json.h
- **数据类型支持**：  
  - 字符串（`wstring`）、数字（`double`）、布尔值、`null`、对象（`map`）、数组（`vector`）。  
- **解析与序列化**：  
  - 支持从字符串/文件解析 JSON，支持格式化或紧凑序列化输出。  
- **类型安全操作**：  
  - 提供 `as_*()` 方法安全获取值，`is_*()` 方法检查类型。  
- **动态访问**：  
  - 通过 `operator[]` 访问对象键或数组索引，支持自动类型转换。  

### JsonEditor.h
- **路径式编辑**：  
  - 支持通过 `a.b.c` 格式的键路径修改嵌套值。  
- **自动创建中间路径**：  
  - 若路径中的中间节点不存在，自动创建为对象类型。  
- **快捷方法**：  
  - 提供 `set_string`、`set_number` 等简化常见类型的赋值操作。  

---

## 快速开始
### 1. 解析 JSON
```cpp
#include "Json.h"

// 从字符串解析
JsonValue root = JsonValue::parse(LR"({"name": "Alice", "scores": [90, 85]})");

// 从文件解析
JsonValue file_root = JsonValue::parse_file(L"data.json");

```
### 2.修改 JSON
```cpp
#include "JsonEditor.h"

JsonEditor editor(root);
editor.set_string(L"address.city", L"Shanghai"); // 自动创建 address 对象
editor.set_number(L"scores[1]", 95.5);          // 修改数组第二个元素（需确保是数组）
```
### 3. 序列化与保存
```cpp
// 序列化为字符串
wstring json_str = root.serialize(true); // 格式化输出

// 保存到文件
root.serialize_to_file(L"output.json");
```
## API详解
### JsonValue 类
#### 构造函数
```cpp
JsonValue();                      // 空值（null）
JsonValue(const wstring& val);    // 字符串
JsonValue(double val);            // 数字
JsonValue(bool val);              // 布尔值
JsonValue(nullptr_t);             // null
JsonValue(const Object& obj);     // 对象
JsonValue(const Array& arr);      // 数组
```
#### 类型检查
```cpp
bool is_object() const;  // 是否为对象
bool is_array() const;   // 是否为数组
// 其他类型检查方法类似...
```
#### 值获取
```cpp
wstring as_string() const;  // 获取字符串值（类型不匹配时抛出异常）
double as_number() const;   // 获取数字值
// 其他类型类似...
```
### 操作符重载
```cpp
JsonValue& operator[](const wstring& key);  // 访问对象键（自动创建对象）
JsonValue& operator[](size_t index);        // 访问数组索引（需确保是数组）
```
#### 解析与序列化
```cpp
static JsonValue parse(const wstring& content);     // 从字符串解析
static JsonValue parse_file(const wstring& filename); // 从文件解析
wstring serialize(bool formatted = true, int indent_level = 0) const; // 序列化
void serialize_to_file(const wstring& filename, bool formatted = true) const; // 保存到文件
```
### JsonEditor 类
#### 初始化
```cpp
explicit JsonEditor(JsonValue& root);  // 绑定到根节点
```
#### 修改方法
```cpp
bool set_value(const std::wstring& key_path, const JsonValue& value); // 通用赋值
bool set_string(const std::wstring& key_path, const std::wstring& value); 
bool set_number(const std::wstring& key_path, double value);
// 其他快捷方法类似...
```
#### 内部逻辑
键路径分割：split_key_path 将 a.b.c 分割为 ["a", "b", "c"]。<br>

类型强制转换：ensure_object 确保节点为对象类型。<br>

## 示例代码
### 1. 创建嵌套 JSON
```cpp
JsonValue root;
JsonEditor editor(root);

// 自动创建路径并赋值
editor.set_string(L"user.profile.name", L"Bob");
editor.set_number(L"user.profile.age", 30);
editor.set_value(L"user.tags", JsonValue::Array{ L"dev", L"gamer" });

// 输出结果：
// {
//     "user": {
//         "profile": {
//             "name": "Bob",
//             "age": 30
//         },
//         "tags": ["dev", "gamer"]
//     }
// }
```
### 2. 修改并保存文件
```cpp
JsonValue config = JsonValue::parse_file(L"config.json");
JsonEditor(config).set_number(L"settings.volume", 80.0);
config.serialize_to_file(L"config_updated.json");
```
</hr>

## 注意事项
### 宽字符编码
1. [所有字符串操作均使用 wstring，文件读写时需使用宽字符流（如 wofstream）]
2. [文件解析需确保 UTF-8 编码，通过 codecvt_utf8 实现转换]
### 异常处理
1. [as_*() 方法在类型不匹配时抛出 bad_variant_access]
2. [文件操作失败时抛出 runtime_error]
### 性能建议
1. [频繁修改时建议复用 JsonEditor 实例]
2. [序列化时若不需要格式化，设置 formatted=false 以提高性能]

## 依赖与兼容性
C++ 标准：C++17（依赖 std::variant 和结构化绑定）。

编译器支持：MSVC 2019+、GCC 9+、Clang 10+。

第三方库：仅依赖 C++ 标准库。
