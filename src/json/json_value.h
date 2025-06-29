#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>

class JsonValue {
public:
    enum class Type {
        NULL_TYPE,
        BOOL,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };

    // コンストラクタ
    JsonValue();  // null
    JsonValue(bool value);
    JsonValue(int value);
    JsonValue(double value);
    JsonValue(const char* value);
    JsonValue(const std::string& value);

    // 型の取得
    Type getType() const;

    // 文字列への変換
    std::string toString() const;

private:
    using ArrayType = std::vector<std::shared_ptr<JsonValue>>;
    using ObjectType = std::map<std::string, std::shared_ptr<JsonValue>>;
    using ValueType = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        std::shared_ptr<ArrayType>,
        std::shared_ptr<ObjectType>
    >;

    ValueType value_;
    std::string escapeString(const std::string& str) const;
};

#endif // JSON_VALUE_H
