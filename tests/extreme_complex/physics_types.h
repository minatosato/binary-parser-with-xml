#pragma once
#include <stdint.h>
#include "base_types.h"

#define MAX_COLLIDERS 8
#define MAX_JOINTS 16

// 物理マテリアル
typedef struct {
    float friction;
    float restitution;
    float density;
    uint32_t flags;
} PhysicsMaterial;

// コライダー
typedef union {
    struct {
        float radius;
        Vector3 center;
    } sphere;
    
    struct {
        Vector3 half_extents;
        Vector3 center;
    } box;
    
    struct {
        float radius;
        float height;
        Vector3 center;
        uint32_t axis;  // 0=X, 1=Y, 2=Z
    } capsule;
    
    struct {
        Vector3 vertices[8];
        uint32_t vertex_count;
    } convex;
} ColliderData;

// コライダー情報
typedef struct {
    uint8_t type;  // 0=sphere, 1=box, 2=capsule, 3=convex
    uint8_t layer;
    uint16_t mask;
    ColliderData data;
    PhysicsMaterial material;
} Collider;

// リジッドボディ
typedef struct {
    Vector3 position;
    Quaternion rotation;
    Vector3 linear_velocity;
    Vector3 angular_velocity;
    
    struct {
        float mass;
        float drag;
        float angular_drag;
        uint32_t constraints;  // Freeze position/rotation flags
    } properties;
    
    struct {
        Vector3 center_of_mass;
        Matrix4x4 inertia_tensor;
    } mass_data;
    
    Collider colliders[MAX_COLLIDERS];
    uint8_t collider_count;
    uint8_t padding[3];
} RigidBody;

// ジョイント
typedef struct {
    uint32_t body_a_id;
    uint32_t body_b_id;
    
    union {
        struct {
            Vector3 anchor_a;
            Vector3 anchor_b;
            Vector3 axis;
            float min_angle;
            float max_angle;
        } hinge;
        
        struct {
            Vector3 anchor_a;
            Vector3 anchor_b;
            float max_distance;
            float spring_constant;
            float damping;
        } spring;
        
        struct {
            Vector3 anchor_a;
            Vector3 anchor_b;
            float break_force;
            float break_torque;
        } fixed;
    } config;
    
    uint8_t type;  // 0=hinge, 1=spring, 2=fixed
    uint8_t enabled;
    uint16_t flags;
} PhysicsJoint;