#pragma once
#include <stdint.h>
#include "extreme_nested.h"

#define ULTRA_MAX_DEPTH 20
#define INSANE_ARRAY_SIZE 512
#define CRAZY_UNION_VARIANTS 16

// 20層以上の深いネストを作る
typedef struct {
    struct {
        struct {
            struct {
                struct {
                    struct {
                        struct {
                            struct {
                                struct {
                                    struct {
                                        struct {
                                            struct {
                                                struct {
                                                    struct {
                                                        struct {
                                                            struct {
                                                                struct {
                                                                    struct {
                                                                        struct {
                                                                            struct {
                                                                                struct {
                                                                                    uint32_t deepest_value;
                                                                                    union {
                                                                                        uint64_t as_long;
                                                                                        struct {
                                                                                            uint32_t : 4;
                                                                                            uint32_t flag1 : 1;
                                                                                            uint32_t flag2 : 1;
                                                                                            uint32_t : 26;
                                                                                            uint32_t extra;
                                                                                        } bits;
                                                                                    } deepest_union;
                                                                                } level20;
                                                                            } level19;
                                                                        } level18;
                                                                    } level17;
                                                                } level16;
                                                            } level15;
                                                        } level14;
                                                    } level13;
                                                } level12;
                                            } level11;
                                        } level10;
                                    } level9;
                                } level8;
                            } level7;
                        } level6;
                    } level5;
                } level4;
            } level3;
        } level2;
    } level1;
} DeepestNest;

// 巨大なunionバリアント
typedef union {
    struct variant1 {
        uint8_t type;
        uint8_t data[511];
        struct {
            uint32_t flags : 16;
            uint32_t count : 16;
        } meta;
    } v1;
    
    struct variant2 {
        uint16_t header[4];
        union {
            float floats[128];
            uint32_t ints[128];
            struct {
                uint64_t a, b, c, d;
            } quad[32];
        } payload;
    } v2;
    
    struct variant3 {
        DeepestNest nested_horror;
        uint8_t padding[INSANE_ARRAY_SIZE - sizeof(DeepestNest)];
    } v3;
    
    struct variant4 {
        Matrix4x4 matrices[8];
        Quaternion rotations[16];
        Vector3 positions[32];
    } v4;
    
    // ... 他のバリアントも追加可能
    uint8_t raw[INSANE_ARRAY_SIZE];
} MassiveUnion;

// 循環参照風の複雑な構造（実際には循環しない）
typedef struct Node Node;
struct Node {
    uint32_t id;
    uint32_t parent_id;  // ポインタの代わりにID参照
    uint32_t children_ids[8];
    uint8_t child_count;
    uint8_t node_type;
    uint16_t flags;
    
    union {
        struct {
            MassiveUnion data;
            DeepestNest deep_data;
        } complex;
        
        struct {
            ExtremelyComplexGameState* game_state_ptr;  // ポインタは無視される
            uint64_t game_state_offset;  // 代わりにオフセットを使用
        } reference;
        
        uint8_t simple_data[2048];
    } content;
};

// 3次元配列を含む巨大構造体
typedef struct {
    // ヘッダー
    struct {
        uint32_t magic;
        uint32_t version;
        uint64_t timestamp;
        uint8_t uuid[16];
    } header;
    
    // 3次元配列の悪夢
    struct {
        struct {
            struct {
                uint32_t value : 24;
                uint32_t flags : 8;
            } cells[16][16][16];
        } chunks[4][4][4];
    } voxel_world;
    
    // 相互に参照し合う複雑な構造
    struct {
        Node nodes[INSANE_ARRAY_SIZE];
        uint32_t node_count;
        
        struct {
            uint32_t from_id;
            uint32_t to_id;
            float weight;
            uint8_t type;
            uint8_t flags;
            uint16_t metadata;
        } edges[INSANE_ARRAY_SIZE * 4];
        uint32_t edge_count;
    } graph_data;
    
    // 全種類の型を含むテストフィールド
    struct {
        // 基本型すべて
        uint8_t u8; int8_t i8;
        uint16_t u16; int16_t i16;
        uint32_t u32; int32_t i32;
        uint64_t u64; int64_t i64;
        float f32; double f64;
        char ch;
        
        // すべてのビット幅のbitfield
        struct {
            uint64_t b1 : 1;
            uint64_t b2 : 2;
            uint64_t b3 : 3;
            uint64_t b4 : 4;
            uint64_t b5 : 5;
            uint64_t b6 : 6;
            uint64_t b7 : 7;
            uint64_t b8 : 8;
            uint64_t b9 : 9;
            uint64_t b10 : 10;
            uint64_t b16 : 16;
            uint64_t b20 : 20;
            uint64_t b24 : 24;
            uint64_t b32 : 32;
        } all_bitfields;
        
        // 無名構造体/unionの嵐
        struct {
            union {
                struct {
                    uint32_t a;
                    union {
                        float f;
                        uint32_t i;
                    };
                };
                struct {
                    uint64_t x;
                };
            };
        };
    } type_torture_test;
    
    // 最後に既存の複雑な構造体も含める
    ExtremelyComplexGameState game_states[2];
    
} UltraExtremeStruct;