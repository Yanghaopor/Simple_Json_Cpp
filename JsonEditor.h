#pragma once
// JsonEditor.h
#pragma once
#include "Json.h"
#include <vector>
#include <stdexcept>
#include <iostream>  // 添加头文件
#include <codecvt>   // 添加编码转换支持

class JsonEditor {
public:
    // 绑定到指定JSON根节点
    explicit JsonEditor(JsonValue& root) : m_root(root) {}

    // 通用修改方法（支持自动创建中间路径）
    //explicit JsonEditor(JsonValue& root) : m_root(root) {}

    bool set_value(const std::wstring& key_path, const JsonValue& value) {
        try {
            auto keys = split_key_path(key_path);
            JsonValue* current = &m_root;

            for (size_t i = 0; i < keys.size(); ++i) {
                const auto& key = keys[i];

                if (i == keys.size() - 1) {
                    // 修改为通过operator[]自动创建对象
                    (*current)[key] = value;
                    return true;
                }

                // 使用operator[]自动创建中间对象
                current = &((*current)[key]);
            }
            return true;
        }
        catch (const std::exception& e) {
            // 添加宽字符转换
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wcerr << L"修改失败: "
                << converter.from_bytes(e.what())
                << std::endl;
            return false;
        }
    }

    // 快捷方法：设置字符串值
    bool set_string(const std::wstring& key_path, const std::wstring& value) {
        return set_value(key_path, JsonValue(value));
    }

    // 快捷方法：设置数字值
    bool set_number(const std::wstring& key_path, double value) {
        return set_value(key_path, JsonValue(value));
    }

    // 其他类型快捷方法...

private:
    JsonValue& m_root; // 绑定的JSON根节点

    // 分割键路径（如 L"a.b.c" → [L"a", L"b", L"c"]）
    static std::vector<std::wstring> split_key_path(const std::wstring& path) {
        std::vector<std::wstring> keys;
        size_t start = 0, end;
        while ((end = path.find(L'.', start)) != std::wstring::npos) {
            if (end != start) { // 跳过空键名
                keys.push_back(path.substr(start, end - start));
            }
            start = end + 1;
            if (start >= path.size()) break;
        }
        if (start < path.size()) {
            keys.push_back(path.substr(start));
        }
        return keys;
    }

    // 强制转换为对象类型（自动转换现有类型）
    static void ensure_object(JsonValue& node) {
        if (!node.is_object()) {
            node = JsonValue::Object(); // 覆盖原有类型
        }
    }
};