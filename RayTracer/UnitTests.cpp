#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

#include "UnitTests.h"
#include "Scene.h"

/*
    Global unit test function. Runs all decided unit tests
*/
bool RunUnitTests()
{
    if (!SurfaceNormalTest())
    {
        std::cerr << "SurfaceNormal test failed!" << std::endl;
        return false;
    }

    if (!ReflectionTest())
    {
        std::cerr << "Reflection test failed!" << std::endl;
        return false;
    }


    return true;
}

/*
    Test Surface normals:
    
    Sphere
    - right direction
    - right side of sphere

    Plane
    - right direction
    - right side of plane
*/
bool SurfaceNormalTest()
{
    // Create a unit sphere at the origin
    Sphere s;

    // Surface normal at the point (1, 0, 0) should be a vector 
    // at (1, 0, 0) pointing to (2, 0, 0)
    Eigen::Vector3f v(1, 0, 0);
    auto normal = s.GetSurfaceNormalAtPoint(v);
    // check direction
    assert(normal(0) == 1);
    assert(normal(1) < 1e-3);
    assert(normal(2) < 1e-3);
    
    return true;
}

/*
    Test reflection angles
*/
bool ReflectionTest()
{
    // Create a unit sphere at the origin
    Sphere s;

    // send a light ray from inside to (1, 0, 0)
    // This should intersect. The normal should be (-1, 0, 0)
    Eigen::Vector3f ray(1, 0, 0);
    float distance = 0.f;
    LightCollision collision;
    s.DoesRayIntersect(ray, distance, collision);

    assert(-1.1 < collision.reflectedRay(0) && collision.reflectedRay(0) < -0.95);
    assert(collision.reflectedRay(1) < 1e-1);
    assert(collision.reflectedRay(2) < 1e-1);

    return true;
}