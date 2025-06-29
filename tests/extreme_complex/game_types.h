#pragma once
#include <stdint.h>
#include "base_types.h"
#include "network_types.h"

#define MAX_NAME_LENGTH 32
#define MAX_INVENTORY_SLOTS 256
#define MAX_SKILLS 64
#define MAX_BUFFS 32
#define MAX_EQUIPMENT_SLOTS 16

// アイテム情報
typedef struct {
    uint32_t id;
    uint16_t type;
    uint16_t rarity;
    union {
        struct {
            uint16_t damage;
            uint16_t defense;
            uint16_t magic_power;
            uint16_t durability;
        } stats;
        uint64_t raw_stats;
    } attributes;
    char name[MAX_NAME_LENGTH];
} Item;

// スキル情報
typedef struct {
    uint16_t id;
    uint8_t level;
    uint8_t max_level;
    uint32_t experience;
    union {
        struct {
            uint32_t active : 1;
            uint32_t cooldown : 15;
            uint32_t cost : 16;
        };
        uint32_t packed_data;
    } state;
} Skill;

// バフ/デバフ
typedef struct {
    uint16_t id;
    uint16_t duration;
    float modifier;
    union {
        struct {
            uint8_t type;
            uint8_t stack_count;
            uint16_t source_id;
        } info;
        uint32_t raw_info;
    } metadata;
} Buff;

// プレイヤーステータス
typedef struct {
    struct {
        uint32_t level;
        uint64_t experience;
        struct {
            uint32_t health;
            uint32_t max_health;
            uint32_t mana;
            uint32_t max_mana;
            uint32_t stamina;
            uint32_t max_stamina;
        } vitals;
        struct {
            uint16_t strength;
            uint16_t dexterity;
            uint16_t intelligence;
            uint16_t wisdom;
            uint16_t constitution;
            uint16_t charisma;
        } attributes;
    } base_stats;
    
    struct {
        Item items[MAX_INVENTORY_SLOTS];
        uint32_t gold;
        uint32_t item_count;
    } inventory;
    
    struct {
        Item equipment[MAX_EQUIPMENT_SLOTS];
        uint32_t equipped_mask;
    } equipment;
    
    Skill skills[MAX_SKILLS];
    Buff active_buffs[MAX_BUFFS];
} PlayerStats;