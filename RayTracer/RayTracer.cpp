#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
//#include <omp.h>
#include <limits>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <random>

#include "Scene.h"
#include "UnitTests.h"
//#include "geometry.h"

using namespace std;
using namespace Eigen;

const Eigen::Vector3f NO_COLOUR = Vector3f(0.2, 0.2, 0.2);

// TODO define this in a better spot
#define MAX_SCENE_DEPTH 10

/*
    This is an exercise in ray tracing/ray marching/path tracing/whatever. 

    The aim is to do high quality ray tracing, denoising, with extra effects
    like caustics and god rays

    I'm going to start from a base similar to the great Dmitry Sokolov
    https://github.com/ssloy
    and steal some code from his tiny ray tracer. 
    I'll improve on it, but the base code was hard to figure out and 
    it's easier to learn from this. Further working from 
    https://raytracing.github.io/books/RayTracingInOneWeekend.html#antialiasing

    As he, so I: OpenCV is large and unwieldy to use just for showing an image. 
    I'll avoid 3rd party libs, and try this first completely from scratch.
    Except for Eigen. But I don't feel that is cheating

    If it gets to GPU optimisations ... then we'll see

    My convention is x and y form the camera coords, and z is away from the camera

    TODO: 
    - 10. Materials
        - material absorbs
        - scatters
        - if scattered, how much does it attentuate
    - add refraction and transparency
    

    Bug:
    - light position isn't lining up with what's showing
    - refactor?
*/

/* ---------- Structures ----------- */
struct Params
{
    string sceneFile;
    string outputFile;
    int width = 640;
    int height = 480;
    int fov = 90;

    int samplesPerPixel = 1;

    int numBouncesPerRay = MAX_NUM_BOUNCES_PER_RAY;

    bool runUnitTests;
};

/* ---------- Function prototypes -------------- */
void FailBadArgs();
bool ParseArgs(const int argc, char** argv, Params& params);

bool WriteImageToFile(
    const vector<Vector3f>& frameBuffer,
    const Params& params);

bool RenderScene(
    Scene& scene,
    const Params& params);

Vector3f CastRay(
    const Vector3f& ray,
    Scene scene,
    int numBounces);

inline double random_double();

/* ------ Main --------*/
// Read in the scene to render
// Render
// Save to an image file
// Inputs: scene file, output file name
int main(int argc, char** argv)
{
    // Read shapes in from files
    Params params;
    if (argc < 2)
    {
        FailBadArgs();
        return -1;
    } 
    else
    {
        if (!ParseArgs(argc, argv, params))
        {
            return -1;
        }
    }

    if (params.runUnitTests)
    {
        // Run the unit tests, then return. No image is rendered.
        return RunUnitTests();
    }


    // right now only supported shapes are spheres but design the interface so that 
    // arbitrary shapes defined by equations work once I figure out the math

    // TODO: somehow read all the inherited classes as the base class?
    Scene scene;
    ReadScene(params.sceneFile, scene);

    // Use current ray tracing technique to render the scene
    RenderScene(scene, params);
    return 0;
}

/*
    Print an error message
*/
void FailBadArgs()
{
    cout << "Usage:" << endl;
    cout << "raytracer.exe -w <int> -h <int> -f <int> -i <input_shape_filename> -o <output_imagefilename>" << endl;
    cout << "-h <int>                       : image height" << endl;
    cout << "-w <int>                       : image width" << endl;
    cout << "-f <int>                       : field of view in degrees" << endl;
    cout << "-s <int>                       : samples per pixel" << endl;
    cout << "-b <int>                       : num bounces per ray, capped at 10." << endl;
    cout << "-i <input_scene_filename>      : contains the scene to be rendered." << endl;
    cout << "-o <output_image_filename>     : name of .ppm file to save output to." << endl;
    cout << "-u                             : runs unit tests instead of rendering image." << endl;
    cout << endl;
    cout << "Scene file format (last line is background colour):" << endl;
    cout << "numShapes numLights" << endl;
    cout << "Shapes listed, one per line" << endl;
    cout << "Lights listed, one per line" << endl;
    cout << "R G B" << endl;
    cout << endl;
    cout << "Currently supported lights: " << endl;
    cout << "POINTLIGHT Cx Cy Cz Cr Cg Cb intensity" << endl;
    cout << endl;
    cout << "Currently supported shapes: " << endl;
    cout << "SPHERE Cx Cy Cz Cr Cg Cb radius" << endl;
    cout << "PLANE Nx Ny Nz Cr Cg Cb offset" << endl;
}

/*
    ... Parse args
*/
bool ParseArgs(const int argc, char** argv, Params& params)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp("-i", argv[i]) == 0)
        {
            params.sceneFile = argv[++i];
        }
        else if (strcmp("-o", argv[i]) == 0)
        {
            params.outputFile = argv[++i];
        }
        else if (strcmp("-h", argv[i]) == 0)
        {
            params.height = stoi(argv[++i]);
            if (params.height < 1)
            {
                cout << "Bad argument!" << endl;
                FailBadArgs();
                return false;
            }
        }
        else if (strcmp("-w", argv[i]) == 0)
        {
            params.width = stoi(argv[++i]);
            if (params.width < 1)
            {
                cout << "Bad argument!" << endl;
                FailBadArgs();
                return false;
            }
        }
        else if (strcmp("-f", argv[i]) == 0)
        {
            params.fov = stoi(argv[++i]);
            if (params.fov < 1)
            {
                cout << "Bad argument!" << endl;
                FailBadArgs();
                return false;
            }
        }
        else if (strcmp("-s", argv[i]) == 0)
        {
            params.samplesPerPixel = stoi(argv[++i]);
            if (params.samplesPerPixel< 1)
            {
                cout << "Bad argument!" << endl;
                FailBadArgs();
                return false;
            }
        }
        else if (strcmp("-b", argv[i]) == 0)
        {
            params.numBouncesPerRay = stoi(argv[++i]);
            if (params.numBouncesPerRay > MAX_NUM_BOUNCES_PER_RAY)
            {
                cout << "Bad argument!" << endl;
                FailBadArgs();
                return false;
            }
        }
        else if (strcmp("-u", argv[i]) == 0)
        {
            params.runUnitTests = true;
        }
        else
        {
            cout << "Bad argument: " << argv[i] << endl;
            FailBadArgs();
            return false;
        }
    }
    return true;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

/*
    Write the image to a ppm file

    This is ripped straight from
    https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing
*/
bool WriteImageToFile(
    const vector<Vector3f>& frameBuffer,
    const Params& params)
{
    ofstream ofs; // save the framebuffer to file
    ofs.open(params.outputFile, std::ofstream::out | std::ofstream::binary);
    if (ofs.is_open())
    {
        ofs << "P6\n" << params.width << " " << params.height << "\n255\n";
        for (int i = 0; i < params.height * params.width; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                ofs << (char)(255 * std::max(0.f, std::min(1.f, frameBuffer[i](j))));
            }
        }
        ofs.close();
        cout << "Written image to file " << params.outputFile << endl;
    }
    else
    {
        cout << "ERROR: image could not be written to " << params.outputFile << endl;
        return false;
    }
    
    return true;
}

/*
    Render the scene using ray tracing. 

    Currently this assumes a constant hardcoded background colour
    and a constant hardcoded light source.
    and a hardcoded field of view

    Currently this is not a function of the scene class
    The scene class is purely for organisational purposes

    We want, in future, this to probably be stochastic progressive path mapping 
    or whatever the name is
    Where you do BDRT for a few rays at a time, to reduce memory overhead
    but keep quality
*/
bool RenderScene(
    Scene& scene,
    const Params& params)
{
    // This will be the image
    vector<Vector3f> frameBuffer(params.width*params.height);
    const float fov = (3.141592 / 180.f) * (float)params.fov;
    const float width = (float)params.width;
    const float height = (float)params.height;

    float oneHorizontalPixel = (2 * (1 + 0.5) / width - 1) * tan(fov / 2.) * width / height;
    float oneVerticalPixel = -(2 * (1 + 0.5) / height - 1) * tan(fov / 2.);

    // Send rays from each pixel, checking over all shapes for collisions
    // TODO: hold shapes in a BVH, or similar, to make this more efficient
    //       given that we just have a small number of spheres and planes, don't worry about this for now
    // TODO: parallelise
    //#pragma omp parallel for
    for (int h = 0; h < params.height; ++h)
    {
        for (int w = 0; w < params.width; ++w)
        {
            // get x and y from field of view equation
            const float x = (2 * (w + 0.5) / width - 1) * tan(fov / 2.) * width / height;
            const float y = -(2 * (h + 0.5) / height - 1) * tan(fov / 2.);
            auto finalColour = Vector3f(0, 0, 0);
            for (int n = 0; n < params.samplesPerPixel; ++n)
            {
                // perturb x and y by an amount not larger than a quarter pixel
                // since pixel intensities are gaussian-distributed at the centre of the pixel
                // [0, 0.5] + 0.25 = [0.25, 0.75]
                double r1 = 0.5*random_double() - 0.25;
                double r2 = 0.5*random_double() - 0.25;
                Vector3f ray(x+r1, y+r2, 1.f);
                ray.normalize();

                // Accumulate the colour over all sampled rays
                finalColour += CastRay(ray, scene, params.numBouncesPerRay);
            }

            // Get the average pixel colour. Values are clamped when written out.
            finalColour /= (float)params.samplesPerPixel;
            
            frameBuffer[w + h * params.width] = Vector3f(finalColour[0], finalColour[1], finalColour[2]);
        }
    }

    return WriteImageToFile(frameBuffer, params);
}

/*
    Cast a single ray into a scene

    This checks all shapes for collisions, and does lighting equations. Returns a colour
*/
Vector3f CastRay(
    const Vector3f& ray,
    Scene scene,
    int numBounces)
{
    vector<Light*> lights = scene.GetLights();
    vector<Shape*> shapes = scene.GetShapes();

    Vector3f colour = scene.GetBackground();

    Vector3f surfaceNormal(0, 0, 0);
    float closestDist = MAX_SCENE_DEPTH;
    int closestShapeIndex = -1;
    for (int i = 0; i < shapes.size(); ++i)
    {
        const auto s = shapes[i];
        float distance = MAX_SCENE_DEPTH;
        LightCollision collision;
        if (s->DoesRayIntersect(ray, distance, collision))
        {
                                        // don't use distances too close, in case reflection 
                                        // rays got spawned beneath the surface
            if (distance < closestDist && distance > EPSILON)
            {
                colour = Vector3f(collision.colour[0], collision.colour[1], collision.colour[2]);
                closestDist = distance;
                closestShapeIndex = i;
                surfaceNormal = collision.surfaceNormal;
            }
        }

        // add bounces, so adjust ray and call this recursively
    }

    if (closestShapeIndex != -1)
    {
        float diffuseIntensity = 0;
        for (const auto l : lights)
        {
            Vector3f point = closestDist * ray;
            Vector3f lightDir = l->GetPosition() - point;
            lightDir /= lightDir.norm();
            surfaceNormal /= surfaceNormal.norm();
            diffuseIntensity += l->GetIntensity() * max(0.f, lightDir.dot(surfaceNormal));
        }

        // How much light this reflects back toward the ray origin vs absorbs
        // Maybe should call this absorption factor
        float diffusionFactor = shapes[closestShapeIndex]->GetDiffusionFactor();
        colour = diffusionFactor * diffuseIntensity * colour;

        if (numBounces > 0)
        {
            // Compute reflection ray
            // reflected ray = R - 2 (dot(R, N)) * N
            // where R = ray, N = normal
            auto reflectedRay = ray - 2 * ray.dot(surfaceNormal) * surfaceNormal;
            // add a smidge of random to this vector for diffuse reflection

            // This 0.5 is the reflectance factor
            // final colour should contain some surface colour and some reflected colour
            colour += 0.5 * CastRay(reflectedRay, scene, numBounces-1);
        }
    }
    
    return colour;
}