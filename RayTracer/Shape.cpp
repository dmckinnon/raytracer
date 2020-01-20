#include "Shape.h"
#include <string>
#include <fstream>

using namespace Eigen;
using namespace std;

/*
	The Sphere class

	Simplest shape
	.. more notes
*/
Sphere::Sphere()
{
	// Default is unit sphere at origin
	centre = Vector3f(0,0,0);
	colour = Vector3f(0.5f, 0.5f, 0.5f);
	coeffs = Vector3f(1, 1, 1);
	radius = 1;
}

Sphere::Sphere(
	_In_ const Eigen::Vector3f& centre,
	_In_ const Eigen::Vector3f& colour,
	_In_ const float r)
{
	this->centre = centre;
	this->colour = colour;
	coeffs = Vector3f(1, 1, 1);
	this->radius = r;
}

Sphere::~Sphere()
{
}

void Sphere::SetSphere(_In_ const Eigen::Vector3f& centre,
	_In_ const Eigen::Vector3f& colour,
	_In_ const float r)
{
	this->centre = centre;
	this->colour = colour;
	coeffs = Vector3f(1, 1, 1);
	this->radius = r;
}

/*
	The mathematics behind this function are surprisingly simple,
	and are described here: 
	http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/

	The idea is to project the centre of the sphere onto the ray in question,
	which makes checking distance to ray easy.

	One thing we should probably also return is the angle of striking
	so as to allow for computing the reflections easier
*/
bool Sphere::DoesRayIntersect(
	_In_ Eigen::Vector3f ray,
	_Out_ float& distance,
	_Out_ Eigen::Vector3f& reflectedRay,
	_Out_ Eigen::Vector3f& refractedRay,
	_Out_ Eigen::Vector3f& colour)
{
	// sanity check
	if (radius == BAD_RADIUS)
	{
		return false;
	}

	// Project centre of sphere on to ray:
	ray.normalize();
	Vector3f proj = (centre.dot(ray)) * ray;
	Vector3f dist = centre - proj;

	float distToCentre = dist.norm();
	if (distToCentre < radius + EPSILON)
	{
		if (distToCentre > radius - EPSILON)
		{
			// ray hits at radius, perfect tangent intersection
			distance = proj.norm();
		}
		else
		{
			// ray passes within circle; find closest point of intersection
			// Note that the ray could be within the circle
			// But for now I'm ignoring this case, since I'm just not going to
			// make scenes that create this case
			// TODO: verify that the first term in the following line is correct ... 
			distance = proj.norm() - sqrt(radius * radius - distToCentre * distToCentre);
		}

		// Dummy rays for now
		reflectedRay = Vector3f(0,0,0);
		refractedRay = Vector3f(0, 0, 0);

		colour = this->colour;
		return true;
	}

	// Ray passed outside sphere: no intersection
	return false;
}

/*
	I/O overloads
*/
std::ostream& operator << (std::ostream& os, const Sphere& s)
{
	os << /*"SPHERE " <<*/ s.radius << " ";
	for (int i = 0; i < 3; ++i)
		os << s.centre[i] << " ";
	for (int i = 0; i < 3; ++i)
		os << s.colour[i] << " ";
	return os;
}
std::istream & operator >> (std::istream & is, Sphere & s)
{
	std::string shapeType;
	Eigen::Vector3f centre, colour;
	float radius;

	/*is >> shapeType;
	if (strcmp(shapeType.c_str(), "SPHERE") != 0)
	{
		s.SetSphere(centre, colour, -1.f);
	}*/

	is >> radius;
	is >> centre[0] >> centre[1] >> centre[2];
	is >> colour[0] >> colour[1] >> colour[2];
	s.SetSphere(centre, colour, radius);
	return is;
}

/*
	Read shapes from the file

	Currently the only supported shapes are technically ellipsoids, and actually spheres
*/
bool ReadShapes(
	_In_ const string& shapeFile,
	_Out_ vector<Sphere>& shapes)
{
	ifstream file;
	file.open(shapeFile);
	if (file.is_open())
	{
		// TODO:
		// Read an entire scene
		// Shapes + background

		while (!file.eof())
		{
			Sphere s;
			file >> s;
			shapes.push_back(s);
		}
		file.close();
		return true;
	}

	return false;
}