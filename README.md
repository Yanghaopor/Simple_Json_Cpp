
# JSON 处理库文档

## 概述

这个 JSON 处理库包含两个核心组件：
1. `Json.h` - JSON 值的核心实现
2. `JsonEditor.h` - 提供便捷的 JSON 编辑功能

## Json.h - JSON 核心实现

### 主要特性

- **支持多种数据类型**：
  - 字符串 (wstring)
  - 数字 (double)
  - 布尔值 (bool)
  - null
  - 对象 (map<wstring, JsonValue>)
  - 数组 (vector<JsonValue>)
- **完整的序列化/反序列化功能**
- **文件读写支持**
- **UTF-8 编码处理**

### 核心类：JsonValue

```cpp
class JsonValue {
public:
    // 构造函数
    JsonValue();
    JsonValue(const wstring& val);
    JsonValue(double val);
    JsonValue(bool val);
    JsonValue(nullptr_t);
    JsonValue(const Object& obj);
    JsonValue(const Array& arr);
    
    // 类型检查
    bool is_object() const noexcept;
    bool is_array() const noexcept;
    bool is_string() const noexcept;
    bool is_number() const noexcept;
    bool is_bool() const noexcept;
    bool is_null() const noexcept;
    
    // 值获取
    wstring as_string() const;
    double as_number() const;
    bool as_bool() const;
    Object as_object() const;
    Array as_array() const;
    
    // 访问操作符
    JsonValue& operator[](const wstring& key);
    JsonValue& operator[](size_t index);
    
    // 解析方法
    static JsonValue parse(const wstring& content);
    static JsonValue parse_file(const wstring& filename);
    
    // 序列化方法
    wstring serialize(bool formatted = true, int indent_level = 0) const;
    void serialize_to_file(const wstring& filename, bool formatted = true) const;
};
```

### 使用示例

```cpp
// 解析 JSON
JsonValue json = JsonValue::parse_file(L"data.json");

// 访问值
wstring name = json["person"]["name"].as_string();
double age = json["person"]["age"].as_number();

// 修改值
json["person"]["age"] = 31.5;

// 序列化并保存
json.serialize_to_file(L"updated_data.json", true);
```

## JsonEditor.h - JSON 编辑器

### 主要特性

- **基于路径的键值访问** (如 `"person.address.city"`)
- **自动创建中间对象**
- **类型安全的赋值方法**

### 核心类：JsonEditor

```cpp
class JsonEditor {
public:
    explicit JsonEditor(JsonValue& root);
    
    // 通用修改方法
    bool set_value(const std::wstring& key_path, const JsonValue& value);
    
    // 类型专用方法
    bool set_string(const std::wstring& key_path, const std::wstring& value);
    bool set_number(const std::wstring& key_path, double value);
};
```

### 使用示例

```cpp
JsonValue root;
JsonEditor editor(root);

// 设置值（自动创建路径）
editor.set_string(L"user.name", L"John Doe");
editor.set_number(L"user.age", 30);
editor.set_value(L"user.isAdmin", true);

// 结果相当于：
// {
//   "user": {
//     "name": "John Doe",
//     "age": 30,
//     "isAdmin": true
//   }
// }
```

## ⚠ 重要限制：数组路径支持问题

当前 `JsonEditor` 实现**不支持数组索引路径**，例如：

```cpp
// 以下用法将无法正常工作
editor.set_string(L"users[0].name", L"Alice");
editor.set_value(L"items[1].price", 19.99);
```

### 原因分析

1. `split_key_path` 函数仅支持点分隔符 (`.`)，无法识别方括号 `[]` 语法
2. 路径解析逻辑仅处理字符串键，不处理数字索引
3. 当前实现无法区分对象键和数组索引

### 推荐解决方案

1. 对于数组操作，直接使用 `JsonValue` 的数组接口：
   ```cpp
   JsonValue users = JsonValue::Array();
   JsonValue user1;
   user1["name"] = L"Alice";
   users[0] = user1; // 直接操作数组
   root["users"] = users;
   ```

2. 对于混合路径，分开处理对象和数组部分：
   ```cpp
   root["users"][0]["name"] = L"Alice"; // 使用JsonValue的索引操作符
   ```

## 文件编码说明

- 所有字符串使用 `wstring` (宽字符)
- 文件读写使用 `codecvt_utf8` 进行 UTF-8 编码转换
- 数字序列化使用 C 本地化保证格式一致

## 总结

这个 JSON 库提供了：
- 完整的 JSON 解析和序列化功能
- 便捷的 JSON 编辑接口
- UTF-8 文件支持

**注意**：当需要处理数组时，请直接使用 `JsonValue` 的数组接口，因为 `JsonEditor` 目前不支持数组索引路径语法。
