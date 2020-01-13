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
//#include "geometry.h"

using namespace std;
using namespace Eigen;

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
	_In_ const vector<Matrix3f>& frameBuffer,
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
	// arbitrary shapes defined by euqations work

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
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp("-i", argv[i]) == 0)
		{
			params.shapeFile = argv[++i];
		}
		else if (strcmp("-o", argv[i]) == 0)
		{
			params.outputFile = argv[++i];
		}
		if (strcmp("-h", argv[i]) == 0)
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
			cout << "Bad argument!" << endl;
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
	_In_ const vector<Matrix3f>& frameBuffer,
	_In_ const Params& params)
{
	ofstream ofs; // save the framebuffer to file
	ofs.open("./" + params.outputFile + ".ppm");
	if (ofs.is_open())
	{
		ofs << "P6\n" << params.width << " " << params.height << "\n255\n";
		for (size_t i = 0; i < params.height * params.width; ++i) {
			for (size_t j = 0; j < 3; j++) {
				ofs << (char)(255 * std::max(0.f, std::min(1.f, frameBuffer[i](j))));
			}
		}
		ofs.close();
	}
	else
	{
		return false;
	}
	
	return true;
}