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
#include "Shape.h"
//#include "geometry.h"

using namespace std;
using namespace Eigen;

const Eigen::Vector3f BACKGROUND_COLOUR = Vector3f(0.2, 0.7, 0.8);
const Eigen::Vector3f NO_COLOUR = Vector3f(0.2, 0.2, 0.2);

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
	- specify a background colour

	TO REALLY DO:
	- make the shape an interface, not an abstrract class. No members
	- implement joost's feedback


*/

/* ---------- Structures ----------- */
struct Params
{
	string shapeFile;
	string outputFile;
	int width;
	int height;
};

/* ---------- Function prototypes -------------- */
void FailBadArgs();
bool ParseArgs(_In_ const int argc, _In_ char** argv, _Out_ Params& params);

bool WriteImageToFile(
	_In_ const vector<Vector3f>& frameBuffer,
	_In_ const Params& params);

bool RenderScene(
	_In_ vector<Sphere>& shapes,
	_In_ const Params& params);

/* ------ Main --------*/
// Read in the scene to render
// Render
// Save to an image file
// Inputs: scene file, output file name
int main(int argc, char** argv)
{
	// Read shapes in from files
	Params params;
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
	vector<Sphere> shapes;
	ReadShapes(params.shapeFile, shapes);

	// Use current ray tracing technique to render the scene
	// This is starting off as just hit then light
	// with a constant light source
	// Then we'll expand to several bounces + hopefuly light
	// then bidirectional
	// TODO: describe the whole scene plus light sources
	RenderScene(shapes, params);

	return 0;
}

/*
	Print an error message
*/
void FailBadArgs()
{
	cout << "Usage:" << endl;
	cout << "raytracer.exe -w <int> -h <int> -i <input_shape_filename> -o <output_imagefilename>" << endl;
	cout << "-h <int>                       : image height" << endl;
	cout << "-w <int>                       : image width" << endl;
	cout << "-i <input_shape_filename>      : contains the shapes to be rendered." << endl;
	cout << "-o <output_image_filename>     : name of .ppm file to save output to." << endl;
	cout << endl;
	cout << "Currently supported shapes: " << endl;
	cout << "ELLIPSOID x y z x_coeff y_coeff z_coeff" << endl;
	cout << "PLANE Nx Ny Nz" << endl;
}

/*
	... Parse args
*/
bool ParseArgs(_In_ const int argc, _In_ char** argv, _Out_ Params& params)
{
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp("-i", argv[i]) == 0)
		{
			params.shapeFile = argv[++i];
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
	_In_ const vector<Vector3f>& frameBuffer,
	_In_ const Params& params)
{
	ofstream ofs; // save the framebuffer to file
	ofs.open(params.outputFile, fstream::out);
	if (ofs.is_open())
	{
		ofs << "P6\n" << params.width << " " << params.height << "\n255\n";
		for (int i = 0; i < params.height * params.width; ++i) {
			for (int j = 0; j < 3; j++) {
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

	Next step is to add this to the scene file
*/
bool RenderScene(
	_In_ vector<Sphere>& shapes,
	_In_ const Params& params)
{
	// This will be the image
	vector<Vector3f> frameBuffer(params.width*params.height);
	const float fov = 1.570796; // 90 degrees
	float width = (float)params.width;
	float height = (float)params.height;

	// Send rays from each pixel, checking over all shapes for collisions
	// TODO: hold shapes in an oct-tree, or similar, to make this more efficient
	// TODO: parallelise
	for (int h = 0; h < params.height; ++h)
	{
		for (int w = 0; w < params.width; ++w)
		{
			// get x and y from field of view equation
			float x = (2 * (w + 0.5) / width - 1) * tan(fov / 2.) * width / height;
			float y = -(2 * (h + 0.5) / height - 1) * tan(fov / 2.);
			Vector3f ray(x, y, 1.f);
			ray.normalize();

			Vector3f colour = NO_COLOUR;
			for (auto& s : shapes)
			{
				float distance;
				Vector3f reflect, refract;
				if (!s.DoesRayIntersect(ray, distance, reflect, refract, colour))
				{
					// why aren't we getting into here? Does this function always succeed?
					// every ray cannot possibly be hitting it?
					colour = BACKGROUND_COLOUR;
				}
			}

			frameBuffer[w + h * params.width] = colour;
		}
	}

	return WriteImageToFile(frameBuffer, params);
}