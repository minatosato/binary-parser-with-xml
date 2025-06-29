#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>

class JsonValue {
public:
    using ArrayType = std::vector<std::shared_ptr<JsonValue>>;
    using ObjectType = std::map<std::string, std::shared_ptr<JsonValue>>;

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
    
    // Create empty array or object
    static JsonValue createArray();
    static JsonValue createObject();

    // 型の取得
    Type getType() const;

    // 文字列への変換
    std::string toString() const;

    // 配列操作
    void pushBack(const JsonValue& value);
    size_t size() const;
    JsonValue& operator[](size_t index);
    const JsonValue& operator[](size_t index) const;

    // オブジェクト操作
    void set(const std::string& key, const JsonValue& value);
    bool contains(const std::string& key) const;
    JsonValue& operator[](const std::string& key);
    const JsonValue& operator[](const std::string& key) const;

    // ファイル操作
    static JsonValue parseFile(const std::string& filename);
    void writeToFile(const std::string& filename) const;

    // 値の取得
    bool getBool() const;
    double getNumber() const;
    std::string getString() const;
    const ArrayType& getArray() const;
    const ObjectType& getObject() const;

private:
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
