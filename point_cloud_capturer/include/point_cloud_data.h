#pragma once
#include <cstdint>
struct Vertex {
    float x, y, z;
};
struct Color {
    uint8_t r, g, b;
};

struct PlyPoint {
    Vertex vertex;
    Color color;
};

// ----------------------------

struct Vector3 {
    float x;
    float y;
    float z;
};
struct Color32 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};
