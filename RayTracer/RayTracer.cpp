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
#include <Eigen/Dense>
#include "Scene.h"
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
	it's easier to learn from this

	As he, so I: OpenCV is large and unwieldy to use just for showing an image. 
	I'll avoid 3rd party libs, and try this first completely from scratch.
	Except for Eigen. But I don't feel that is cheating

	If it gets to GPU optimisations ... then we'll see

	My convention is x and y form the camera coords, and z is away from the camera

	TODO: 
	- read in any shape as base class
	- reflections/refractions/shadows etc
	- ambient + diffuse + specular

	Bug:
	- light position isn't lining up with what's showing
*/

/* ---------- Structures ----------- */
struct Params
{
	string sceneFile;
	string outputFile;
	int width;
	int height;
	int fov;
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
	Scene scene);

/* ------ Main --------*/
// Read in the scene to render
// Render
// Save to an image file
// Inputs: scene file, output file name
int main(int argc, char** argv)
{
	// Read shapes in from files
	Params params;
	// Set defaults
	params.width = 640;
	params.height = 480;
	params.fov = 90;
	if (argc < 4)
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

	// right now only supported shapes are spheres but design the interface so that 
	// arbitrary shapes defined by equations work once I figure out the math

	// TODO: somehow read all the inherited classes as the base class?
	Scene scene;
	ReadScene(params.sceneFile, scene);

	// Use current ray tracing technique to render the scene
	// This is starting off as just hit then light
	// with a constant light source
	// Then we'll expand to several bounces + hopefuly light
	// then bidirectional
	// TODO: describe the whole scene plus light sources
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
	cout << "-i <input_scene_filename>      : contains the scene to be rendered." << endl;
	cout << "-o <output_image_filename>     : name of .ppm file to save output to." << endl;
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
		else
		{
			cout << "Bad argument: " << argv[i] << endl;
			FailBadArgs();
			return false;
		}
	}
	return true;
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

	// Send rays from each pixel, checking over all shapes for collisions
	// TODO: hold shapes in an oct-tree, or similar, to make this more efficient
	// TODO: parallelise
	//#pragma omp parallel for
	for (int h = 0; h < params.height; ++h)
	{
		for (int w = 0; w < params.width; ++w)
		{
			// get x and y from field of view equation
			const float x = (2 * (w + 0.5) / width - 1) * tan(fov / 2.) * width / height;
			const float y = -(2 * (h + 0.5) / height - 1) * tan(fov / 2.);
			Vector3f ray(x, y, 1.f);
			ray.normalize();

			Vector3f colour = CastRay(ray, scene);
			frameBuffer[w + h * params.width] = Vector3f(colour[0], colour[1], colour[2]);
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
	Scene scene)
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
		Vector3f reflect, refract, normal, curColour = NO_COLOUR;
		if (s->DoesRayIntersect(ray, distance, normal, reflect, refract, curColour))
		{
			if (distance < closestDist)
			{
				colour = Vector3f(curColour[0], curColour[1], curColour[2]);
				closestDist = distance;
				closestShapeIndex = i;
				surfaceNormal = Vector3f(normal[0], normal[1], normal[2]);
			}
		}
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

		colour = diffuseIntensity* colour;
	}

	return colour;
}