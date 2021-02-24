#include "Mesh.h"

#include "../../Maths.h"
#include <array>
#include <glm/gtc/noise.hpp>
#include <iostream>
#include <ctime>

namespace {
    void addCubeToMesh(Mesh& mesh, const glm::vec3& dimensions,
                       const glm::vec3& offset = {0, 0, 0})
    {
        float w = dimensions.x + offset.x;
        float h = dimensions.y + offset.y;
        float d = dimensions.z + offset.z;

        float ox = offset.x;
        float oy = offset.y;
        float oz = offset.z;

        // Front, left, back, right, top, bottom

        // clang-format off
        mesh.positions.insert(mesh.positions.end(), {
            {w, h,    d}, {ox,  h,  d}, {ox, oy,  d}, {w,  oy,  d},
            {ox, h,   d}, {ox,  h, oz}, {ox, oy, oz}, {ox, oy,  d},
            {ox, h,  oz}, {w,   h, oz}, {w,  oy, oz}, {ox, oy, oz},
            {w, h,   oz}, {w,   h,  d}, {w,  oy,  d}, {w,  oy, oz},
            {w, h,   oz}, {ox,  h, oz}, {ox,  h,  d}, {w,   h,  d},
            {ox, oy, oz}, {w,  oy, oz}, {w,  oy,  d}, {ox, oy,  d}
        });

        mesh.textureCoords.insert(mesh.textureCoords.end(), {
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
           {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
        });
    
        mesh.normals.insert(mesh.normals.end(), {
            { 0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f,  1.0f},
            {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, 
            { 0.0f,  0.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, { 0.0f,  0.0f, -1.0f},
            { 1.0f,  0.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, 
            { 0.0f,  1.0f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 0.0f,  1.0f,  0.0f},
            { 0.0f, -1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f},
        });
        // clang-format on

        // For each cube face, add indice
        for (int i = 0; i < 6; i++) {
            mesh.indices.push_back(mesh.currentIndex);
            mesh.indices.push_back(mesh.currentIndex + 1);
            mesh.indices.push_back(mesh.currentIndex + 2);
            mesh.indices.push_back(mesh.currentIndex + 2);
            mesh.indices.push_back(mesh.currentIndex + 3);
            mesh.indices.push_back(mesh.currentIndex);
            mesh.currentIndex += 4;
        }
    }
} // namespace

Mesh createCubeMesh(const glm::vec3& dimensions)
{
    Mesh cube;
    addCubeToMesh(cube, dimensions);
    return cube;
}

Mesh createWireCubeMesh(const glm::vec3& dimensions, float wireThickness)
{
    Mesh cube;
    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;
    // Front
    addCubeToMesh(cube, {w, wireThickness, wireThickness});
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, h, 0});
    addCubeToMesh(cube, {wireThickness, h, wireThickness});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {w, 0, 0});

    // Back
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, 0, d});
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, h, d});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {0, 0, d});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {w, 0, d});

    // Right
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {0, h, 0});
    addCubeToMesh(cube, {wireThickness, wireThickness, d});

    // Left
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {w, h, 0});
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {w, 0, 0});

    return cube;
}

struct NoiseOptions {
    int octaves;
    float amplitude;
    float smoothness;
    float roughness;
    float offset;
};
constexpr float SIZE = 256;
constexpr float VERTS = 256;

float getNoiseAt(const glm::vec2& vertexPosition, const glm::vec2& terrainPosition,
                 const NoiseOptions& options, int seed)
{
    // Get voxel X/Z positions
    float vx = vertexPosition.x + terrainPosition.x * SIZE;
    float vz = vertexPosition.y + terrainPosition.y * SIZE;

    // Begin iterating through the octaves
    float value = 0;
    float accumulatedAmps = 0;
    for (int i = 0; i < options.octaves; i++) {
        float frequency = glm::pow(2.0f, i);
        float amplitude = glm::pow(options.roughness, i);

        float x = vx * frequency / options.smoothness;
        float y = vz * frequency / options.smoothness;

        float noise = glm::simplex(glm::vec3{seed + x, seed + y, seed});
        noise = (noise + 1.0f) / 2.0f;
        value += noise * amplitude;
        accumulatedAmps += amplitude;
    }
    return value / accumulatedAmps;
}

Mesh createTerrainMesh(bool createBumps)
{
    constexpr unsigned TOTAL_VERTS = VERTS * VERTS;

    std::vector<float> heights(TOTAL_VERTS);

    if (createBumps) {
        NoiseOptions ops;
        ops.amplitude = 50;
        ops.octaves = 5;
        ops.smoothness = 200;
        ops.roughness = 0.5;
        for (int y = 0; y < VERTS; y++) {
            for (int x = 0; x < VERTS; x++) {
                heights[y * VERTS + x] = getNoiseAt({0, 0}, {x, y}, ops, std::time(nullptr));
            }
        }
    }
    else {
        std::fill(heights.begin(), heights.end(), 0);
    }

    auto getHeight = [&](int x, int y) {
        if (x < 0 || x >= VERTS || y < 0 || y >= VERTS) {
            return 0.0f;
        }
        else {
            return heights[y * (int)VERTS + x];
        }
    };

    Mesh terrain;
    for (int y = 0; y < VERTS; y++) {
        for (int x = 0; x < VERTS; x++) {
            auto fx = static_cast<float>(x);
            auto fy = static_cast<float>(y);

            float vx = fx / (VERTS - 1) * SIZE;
            float vz = fy / (VERTS - 1) * SIZE;
            float vy = getHeight(x, y);
            terrain.positions.emplace_back(vx, vy, vz);

            float h1 = getHeight(x - 1, y);
            float h2 = getHeight(x + 1, y);
            float h3 = getHeight(x, y - 1);
            float h4 = getHeight(x, y + 1);
            glm::vec3 normal{h1 - h2, 2, h3 - h4};
            glm::vec3 n = glm::normalize(normal);
            terrain.normals.emplace_back(n.x, n.y, n.z);

            float u = y % (int)VERTS;
            float v = x % (int)VERTS;
            terrain.textureCoords.emplace_back(u, v);
        }
    }

    for (int y = 0; y < VERTS - 1; y++) {
        for (int x = 0; x < VERTS - 1; x++) {
            int topLeft = (y * VERTS) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((y + 1) * VERTS) + x;
            int bottomRight = bottomLeft + 1;

            terrain.indices.push_back(topLeft);
            terrain.indices.push_back(bottomLeft);
            terrain.indices.push_back(topRight);
            terrain.indices.push_back(topRight);
            terrain.indices.push_back(bottomLeft);
            terrain.indices.push_back(bottomRight);
        }
    }
    return terrain;
}
