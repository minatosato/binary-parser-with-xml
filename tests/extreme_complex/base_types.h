#pragma once
#include <stdint.h>

// 基本的な型定義
typedef struct {
    uint8_t r, g, b, a;
} Color;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    double x, y, z, w;
} Quaternion;

// 基本的な設定構造体
typedef struct {
    uint32_t version : 8;
    uint32_t flags : 24;
} ConfigHeader;

// マトリックス型
typedef struct {
    float m[16];  // 4x4 matrix
} Matrix4x4;

// 時刻情報
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t padding;
    uint32_t microseconds;
} Timestamp;