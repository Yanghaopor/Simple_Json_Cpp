// Json.h
#pragma once

#ifdef _MSC_VER
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <fstream>
#include <sstream>
#include <locale>
#include <stdexcept>
#include <codecvt>
#include <iomanip>

using namespace std;

class JsonValue {
public:

    wstring serialize(bool formatted = true, int indent_level = 0) const;
    void serialize_to_file(const wstring& filename, bool formatted = true) const;

    using Object = map<wstring, JsonValue>;
    using Array = vector<JsonValue>;
    using Value = variant<wstring, double, bool, nullptr_t, Object, Array>;

    Value data;

    // 构造函数
    JsonValue() : data(nullptr) {}
    JsonValue(const wstring& val) : data(val) {}
    JsonValue(double val) : data(val) {}
    JsonValue(bool val) : data(val) {}
    JsonValue(nullptr_t) : data(nullptr) {}
    JsonValue(const Object& obj) : data(obj) {}
    JsonValue(const Array& arr) : data(arr) {}

    // 访问操作符重载（安全版本）
    // 修改后的 operator[] 实现
    // 修改后的 operator[] 实现（支持自动创建嵌套对象）
    JsonValue& operator[](const wstring& key) {
        // 如果当前不是对象，强制转换为空对象
        if (!is_object()) {
            data = Object();
        }

        // 获取对象引用
        Object& obj = get<Object>(data);

        // 查找键是否存在
        auto it = obj.find(key);
        if (it == obj.end()) {
            // 键不存在时插入新的空对象
            it = obj.emplace(key, Object()).first;
        }

        return it->second;
    }

    JsonValue& operator[](size_t index) {
        if (!is_array()) {
            throw runtime_error("Not a JSON array");
        }
        return get<Array>(data)[index];
    }

    // 类型检查
    bool is_object() const noexcept { return holds_alternative<Object>(data); }
    bool is_array() const noexcept { return holds_alternative<Array>(data); }
    bool is_string() const noexcept { return holds_alternative<wstring>(data); }
    bool is_number() const noexcept { return holds_alternative<double>(data); }
    bool is_bool() const noexcept { return holds_alternative<bool>(data); }
    bool is_null() const noexcept { return holds_alternative<nullptr_t>(data); }

    // 值获取（安全版本）
    wstring as_string() const {
        if (!is_string()) throw bad_variant_access();
        return get<wstring>(data);
    }

    double as_number() const {
        if (!is_number()) throw bad_variant_access();
        return get<double>(data);
    }

    bool as_bool() const {
        if (!is_bool()) throw bad_variant_access();
        return get<bool>(data);
    }

    Object as_object() const {
        if (!is_object()) throw bad_variant_access();
        return get<Object>(data);
    }

    Array as_array() const {
        if (!is_array()) throw bad_variant_access();
        return get<Array>(data);
    }

    // 解析入口
    static JsonValue parse(const wstring& content);
    static JsonValue parse_file(const wstring& filename);

    // 赋值操作符
    JsonValue& operator=(const wstring& val) { data = val; return *this; }
    JsonValue& operator=(double val) { data = val; return *this; }
    JsonValue& operator=(bool val) { data = val; return *this; }
    JsonValue& operator=(nullptr_t) { data = nullptr; return *this; }
    JsonValue& operator=(const Object& obj) { data = obj; return *this; }
    JsonValue& operator=(const Array& arr) { data = arr; return *this; }

    // 键值操作
    void insert(const wstring& key, const JsonValue& value) {
        if (!is_object()) data = Object();
        get<Object>(data).emplace(key, value);
    }

    void erase(const wstring& key) {
        if (is_object()) get<Object>(data).erase(key);
    }

private:

    static wstring serialize_string(const wstring& s);
    static wstring serialize_number(double d);
    wstring serialize_object(const Object& obj, bool formatted, int indent_level) const;
    wstring serialize_array(const Array& arr, bool formatted, int indent_level) const;
    static wstring generate_indent(int indent_level);

    class Parser {
        wstring str;
        size_t pos = 0;

        // 调整声明顺序
        void skip_whitespace();
        wchar_t peek();
        wchar_t next();
        JsonValue parse_value();
        wstring parse_string();
        double parse_number();
        bool parse_bool();
        nullptr_t parse_null();
        Object parse_object();  // 后声明
        Array parse_array();    // 后声明

    public:
        Parser(const wstring& s) : str(s) {}
        JsonValue parse();
    };
};

// 解析器实现（调整实现顺序）
void JsonValue::Parser::skip_whitespace() {
    while (pos < str.size() && iswspace(str[pos])) ++pos;
}

wchar_t JsonValue::Parser::peek() {
    return (pos < str.size()) ? str[pos] : L'\0';
}

wchar_t JsonValue::Parser::next() {
    return (pos < str.size()) ? str[pos++] : L'\0';
}

JsonValue JsonValue::Parser::parse() {
    skip_whitespace();
    return parse_value();
}

// 首先实现parse_value
JsonValue JsonValue::Parser::parse_value() {
    skip_whitespace();
    switch (peek()) {
    case L'{': return parse_object();
    case L'[': return parse_array();
    case L'"': return parse_string();
    case L't': case L'f': return parse_bool();
    case L'n': parse_null(); return nullptr;
    default:
        if (iswdigit(peek()) || peek() == L'-') return parse_number();
        throw runtime_error("Unexpected character");
    }
}

wstring JsonValue::Parser::parse_string() {
    wstring result;
    next(); // 跳过"
    while (peek() != L'"') {
        if (peek() == L'\\') {
            next();
            switch (next()) {
            case L'"':  result += L'"'; break;
            case L'\\': result += L'\\'; break;
            case L'/':  result += L'/'; break;
            case L'b':  result += L'\b'; break;
            case L'f':  result += L'\f'; break;
            case L'n':  result += L'\n'; break;
            case L'r':  result += L'\r'; break;
            case L't':  result += L'\t'; break;
            default: throw runtime_error("Invalid escape sequence");
            }
        }
        else {
            result += next();
        }
    }
    next(); // 跳过结尾的"
    return result;
}

double JsonValue::Parser::parse_number() {
    wstring buffer;
    while (iswalnum(peek()) || peek() == L'.' || peek() == L'-' || peek() == L'e' || peek() == L'E') {
        buffer += next();
    }
    try {
        return stod(buffer);
    }
    catch (...) {
        throw runtime_error("Invalid number format");
    }
}

bool JsonValue::Parser::parse_bool() {
    if (str.compare(pos, 4, L"true") == 0) {
        pos += 4;
        return true;
    }
    if (str.compare(pos, 5, L"false") == 0) {
        pos += 5;
        return false;
    }
    throw runtime_error("Invalid boolean value");
}

nullptr_t JsonValue::Parser::parse_null() {
    if (str.compare(pos, 4, L"null") != 0) {
        throw runtime_error("Invalid null value");
    }
    pos += 4;
    return nullptr;
}

// 最后实现parse_object和parse_array
JsonValue::Object JsonValue::Parser::parse_object() {
    Object obj;
    next(); // 跳过{
    while (true) {
        skip_whitespace();
        if (peek() == L'}') {
            next();
            break;
        }
        wstring key = parse_string();
        skip_whitespace();
        if (next() != L':') throw runtime_error("Expected ':'");
        JsonValue value = parse_value();
        obj.emplace(move(key), move(value));
        skip_whitespace();
        if (peek() == L',') {
            next();
        }
        else if (peek() != L'}') {
            throw runtime_error("Expected ',' or '}'");
        }
    }
    return obj;
}

JsonValue::Array JsonValue::Parser::parse_array() {
    Array arr;
    next(); // 跳过[
    while (true) {
        skip_whitespace();
        if (peek() == L']') {
            next();
            break;
        }
        arr.emplace_back(parse_value());
        skip_whitespace();
        if (peek() == L',') {
            next();
        }
        else if (peek() != L']') {
            throw runtime_error("Expected ',' or ']'");
        }
    }
    return arr;
}

JsonValue JsonValue::parse(const wstring& content) {
    return Parser(content).parse();
}

JsonValue JsonValue::parse_file(const wstring& filename) {
    wifstream file(filename, ios::binary);
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));
    if (!file.is_open()) throw runtime_error("File open failed");
    wstringstream ss;
    ss << file.rdbuf();
    return parse(ss.str());
}

wstring JsonValue::serialize(bool formatted, int indent_level) const {
    if (is_string()) {
        return serialize_string(get<wstring>(data));
    }
    else if (is_number()) {
        return serialize_number(get<double>(data));
    }
    else if (is_bool()) {
        return get<bool>(data) ? L"true" : L"false";
    }
    else if (is_null()) {
        return L"null";
    }
    else if (is_object()) {
        return serialize_object(get<Object>(data), formatted, indent_level);
    }
    else if (is_array()) {
        return serialize_array(get<Array>(data), formatted, indent_level);
    }
    throw runtime_error("Unknown JSON type during serialization");
}


wstring JsonValue::serialize_string(const wstring& s) {
    wstring result = L"\"";
    for (wchar_t c : s) {
        switch (c) {
        case L'\"': result += L"\\\""; break;
        case L'\\': result += L"\\\\"; break;
        case L'\b': result += L"\\b"; break;
        case L'\f': result += L"\\f"; break;
        case L'\n': result += L"\\n"; break;
        case L'\r': result += L"\\r"; break;
        case L'\t': result += L"\\t"; break;
        default:
            if (c < 0x20) {
                wostringstream oss;
                oss << L"\\u" << hex << setw(4) << setfill(L'0') << static_cast<int>(c);
                result += oss.str();
            }
            else {
                result += c;
            }
            break;
        }
    }
    result += L"\"";
    return result;
}

wstring JsonValue::serialize_number(double d) {
    wostringstream oss;
    oss.imbue(locale("C")); // 避免本地化数字格式
    if (floor(d) == d && d < 1e15) { // 整数且不超出精度范围
        oss << static_cast<long long>(d);
    }
    else {
        oss << d;
    }
    return oss.str();
}

wstring JsonValue::serialize_object(const Object& obj, bool formatted, int indent_level) const {
    wstring result = L"{";
    if (formatted) result += L'\n';

    bool first = true;
    for (const auto& [key, value] : obj) {
        if (!first) {
            result += formatted ? L",\n" : L",";
        }

        if (formatted) {
            result += generate_indent(indent_level + 1) +
                serialize_string(key) +
                L": " +
                value.serialize(formatted, indent_level + 1);
        }
        else {
            result += serialize_string(key) + L":" + value.serialize(false);
        }

        first = false;
    }

    if (formatted) {
        result += L'\n' + generate_indent(indent_level) + L"}";
    }
    else {
        result += L"}";
    }
    return result;
}


wstring JsonValue::serialize_array(const Array& arr, bool formatted, int indent_level) const {
    wstring result = L"[";
    if (formatted) result += L'\n';

    bool first = true;
    for (const auto& item : arr) {
        if (!first) {
            result += formatted ? L",\n" : L",";
        }

        if (formatted) {
            result += generate_indent(indent_level + 1) +
                item.serialize(formatted, indent_level + 1);
        }
        else {
            result += item.serialize(false);
        }

        first = false;
    }

    if (formatted) {
        result += L'\n' + generate_indent(indent_level) + L"]";
    }
    else {
        result += L"]";
    }
    return result;
}

// 修改文件写入方法
void JsonValue::serialize_to_file(const wstring& filename, bool formatted) const {
    wofstream file(filename, ios::binary);
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for writing");
    }
    file << serialize(formatted);
    file.close();
}

// 辅助函数：生成缩进字符串
wstring JsonValue::generate_indent(int indent_level) {
    return wstring(indent_level * 4, L' '); // 每级缩进4个空格（可改为\t）
}