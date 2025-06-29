#include "json_value.h"
#include <sstream>
#include <iomanip>
#include <variant>
#include <stdexcept>

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
    if (std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        std::ostringstream oss;
        oss << "[";
        auto arr = std::get<std::shared_ptr<ArrayType>>(value_);
        for (size_t i = 0; i < arr->size(); i++) {
            if (i > 0) oss << ",";
            oss << (*arr)[i]->toString();
        }
        oss << "]";
        return oss.str();
    }
    if (std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        std::ostringstream oss;
        oss << "{";
        auto obj = std::get<std::shared_ptr<ObjectType>>(value_);
        bool first = true;
        for (const auto& [key, val] : *obj) {
            if (!first) oss << ",";
            first = false;
            oss << "\"" << escapeString(key) << "\":" << val->toString();
        }
        oss << "}";
        return oss.str();
    }
    return "";
}

// Array operations
void JsonValue::pushBack(const JsonValue& value) {
    if (!std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        value_ = std::make_shared<ArrayType>();
    }
    auto arr = std::get<std::shared_ptr<ArrayType>>(value_);
    arr->push_back(std::make_shared<JsonValue>(value));
}

size_t JsonValue::size() const {
    if (std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        return std::get<std::shared_ptr<ArrayType>>(value_)->size();
    }
    throw std::runtime_error("Not an array");
}

JsonValue& JsonValue::operator[](size_t index) {
    if (!std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        throw std::runtime_error("Not an array");
    }
    auto arr = std::get<std::shared_ptr<ArrayType>>(value_);
    if (index >= arr->size()) {
        throw std::out_of_range("Array index out of range");
    }
    return *(*arr)[index];
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (!std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        throw std::runtime_error("Not an array");
    }
    auto arr = std::get<std::shared_ptr<ArrayType>>(value_);
    if (index >= arr->size()) {
        throw std::out_of_range("Array index out of range");
    }
    return *(*arr)[index];
}

// Object operations
void JsonValue::set(const std::string& key, const JsonValue& value) {
    if (!std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        value_ = std::make_shared<ObjectType>();
    }
    auto obj = std::get<std::shared_ptr<ObjectType>>(value_);
    (*obj)[key] = std::make_shared<JsonValue>(value);
}

bool JsonValue::contains(const std::string& key) const {
    if (!std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        return false;
    }
    auto obj = std::get<std::shared_ptr<ObjectType>>(value_);
    return obj->find(key) != obj->end();
}

JsonValue& JsonValue::operator[](const std::string& key) {
    if (!std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        value_ = std::make_shared<ObjectType>();
    }
    auto obj = std::get<std::shared_ptr<ObjectType>>(value_);
    if (obj->find(key) == obj->end()) {
        (*obj)[key] = std::make_shared<JsonValue>();
    }
    return *(*obj)[key];
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (!std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        throw std::runtime_error("Not an object");
    }
    auto obj = std::get<std::shared_ptr<ObjectType>>(value_);
    auto it = obj->find(key);
    if (it == obj->end()) {
        throw std::out_of_range("Key not found");
    }
    return *it->second;
}

// Value getters
bool JsonValue::getBool() const {
    if (!std::holds_alternative<bool>(value_)) {
        throw std::runtime_error("Not a boolean");
    }
    return std::get<bool>(value_);
}

double JsonValue::getNumber() const {
    if (!std::holds_alternative<double>(value_)) {
        throw std::runtime_error("Not a number");
    }
    return std::get<double>(value_);
}

std::string JsonValue::getString() const {
    if (!std::holds_alternative<std::string>(value_)) {
        throw std::runtime_error("Not a string");
    }
    return std::get<std::string>(value_);
}

const JsonValue::ArrayType& JsonValue::getArray() const {
    if (!std::holds_alternative<std::shared_ptr<ArrayType>>(value_)) {
        throw std::runtime_error("Not an array");
    }
    return *std::get<std::shared_ptr<ArrayType>>(value_);
}

const JsonValue::ObjectType& JsonValue::getObject() const {
    if (!std::holds_alternative<std::shared_ptr<ObjectType>>(value_)) {
        throw std::runtime_error("Not an object");
    }
    return *std::get<std::shared_ptr<ObjectType>>(value_);
}
