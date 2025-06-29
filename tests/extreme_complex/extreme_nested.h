#pragma once
#include <stdint.h>
#include "base_types.h"
#include "network_types.h"
#include "game_types.h"
#include "physics_types.h"

#define MAX_ENTITIES 1024
#define MAX_COMPONENTS 32
#define MAX_WORLDS 4
#define MAX_LAYERS 16
#define CHUNK_SIZE 64

// エンティティコンポーネント
typedef struct {
    uint32_t entity_id;
    uint32_t component_mask;
    
    union {
        struct {
            struct {
                Vector3 position;
                Quaternion rotation;
                Vector3 scale;
                Matrix4x4 world_matrix;
            } transform;
            
            struct {
                Color ambient;
                Color diffuse;
                Color specular;
                float shininess;
                uint32_t texture_ids[8];
                uint8_t texture_count;
                uint8_t blend_mode;
                uint16_t render_flags;
            } renderer;
            
            RigidBody physics;
            
            struct {
                PlayerStats stats;
                ConnectionInfo connection;
                char username[MAX_NAME_LENGTH];
                uint64_t user_id;
            } player;
        } components;
        
        uint8_t raw_data[4096];
    } data;
} Entity;

// ワールドチャンク
typedef struct {
    struct {
        int32_t x, y, z;
    } coordinates;
    
    struct {
        struct {
            uint16_t id;
            uint8_t metadata;
            uint8_t light_level : 4;
            uint8_t sky_light : 4;
        } blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    } voxels;
    
    struct {
        Entity entities[MAX_ENTITIES];
        uint32_t entity_count;
        uint32_t active_mask[MAX_ENTITIES / 32];
    } entity_storage;
    
    struct {
        Timestamp last_update;
        uint32_t tick_count;
        union {
            struct {
                uint32_t dirty : 1;
                uint32_t loading : 1;
                uint32_t unloading : 1;
                uint32_t generated : 1;
                uint32_t reserved : 28;
            };
            uint32_t flags;
        } state;
    } metadata;
} WorldChunk;

// レイヤー情報
typedef struct {
    char name[MAX_NAME_LENGTH];
    uint32_t layer_id;
    uint32_t parent_id;
    
    struct {
        uint32_t visible : 1;
        uint32_t locked : 1;
        uint32_t collision_enabled : 1;
        uint32_t physics_enabled : 1;
        uint32_t reserved : 28;
    } flags;
    
    struct {
        Color fog_color;
        float fog_density;
        float fog_start;
        float fog_end;
    } environment;
    
    struct {
        Vector3 gravity;
        float time_scale;
        PhysicsJoint global_joints[MAX_JOINTS];
        uint8_t joint_count;
        uint8_t padding[3];
    } physics_settings;
} Layer;

// 超複雑な最終構造体
typedef struct {
    ConfigHeader header;
    
    struct {
        char name[MAX_NAME_LENGTH];
        char description[256];
        Timestamp created;
        Timestamp modified;
        uint64_t world_seed;
    } metadata;
    
    struct {
        struct {
            Layer layers[MAX_LAYERS];
            uint8_t layer_count;
            uint8_t active_layer;
            uint16_t layer_mask;
        } layer_system;
        
        struct {
            WorldChunk chunks[MAX_WORLDS];
            uint8_t loaded_chunks;
            uint8_t active_world;
            uint16_t world_flags;
        } world_storage;
        
        struct {
            PacketHeader last_packets[MAX_CONNECTIONS];
            ConnectionInfo connections[MAX_CONNECTIONS];
            uint32_t connection_count;
            NetworkAddress server_address;
        } network_state;
        
        struct {
            struct {
                uint64_t frame_count;
                float delta_time;
                float total_time;
                float average_fps;
            } timing;
            
            struct {
                uint32_t draw_calls;
                uint32_t triangles;
                uint32_t texture_memory;
                uint32_t buffer_memory;
            } rendering;
            
            struct {
                uint32_t collision_checks;
                uint32_t active_bodies;
                float simulation_time;
                uint32_t solver_iterations;
            } physics;
        } performance_metrics;
    } runtime_data;
    
    union {
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            uint8_t build;
        } version;
        uint32_t version_number;
    } app_version;
    
    // 最後に巨大な予約領域
    uint8_t reserved[65536];
} ExtremelyComplexGameState;