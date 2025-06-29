#include "json_value.h"
#include <sstream>
#include <iomanip>
#include <variant>

JsonValue::JsonValue() : value_(nullptr) {}

JsonValue::JsonValue(bool value) : value_(value) {}

JsonValue::JsonValue(int value) : value_(static_cast<double>(value)) {}

JsonValue::JsonValue(double value) : value_(value) {}

JsonValue::JsonValue(const char* value) : value_(std::string(value)) {}

JsonValue::JsonValue(const std::string& value) : value_(value) {}

JsonValue::Type JsonValue::getType() const {
    if (std::holds_alternative<std::nullptr_t>(value_)) return Type::NULL_TYPE;
    if (std::holds_alternative<bool>(value_)) return Type::BOOL;
    if (std::holds_alternative<double>(value_)) return Type::NUMBER;
    if (std::holds_alternative<std::string>(value_)) return Type::STRING;
    if (std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) return Type::ARRAY;
    return Type::OBJECT;
}

std::string JsonValue::escapeString(const std::string& str) const {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string JsonValue::toString() const {
    if (std::holds_alternative<std::nullptr_t>(value_)) {
        return "null";
    }
    if (std::holds_alternative<bool>(value_)) {
        return std::get<bool>(value_) ? "true" : "false";
    }
    if (std::holds_alternative<double>(value_)) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(15);
        oss << std::get<double>(value_);
        std::string str = oss.str();
        // 末尾の0を削除
        while (str.find('.') != std::string::npos && str.back() == '0') {
            str.pop_back();
        }
        if (str.back() == '.') {
            str.pop_back();
        }
        return str;
    }
    if (std::holds_alternative<std::string>(value_)) {
        return "\"" + escapeString(std::get<std::string>(value_)) + "\"";
    }
    // Array and Object will be implemented later
    return "";
}
