#include "Scene.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace Eigen;
using namespace std;

//-----------------------------------------------------------
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
	radius = 1;
}

Sphere::Sphere(
	const Eigen::Vector3f& centre,
	const Eigen::Vector3f& colour,
	const float r)
{
	this->centre = centre;
	this->colour = colour;
	this->radius = r;
}

Sphere::~Sphere()
{
}

void Sphere::SetSphere( const Eigen::Vector3f& centre,
	const Eigen::Vector3f& colour,
	const float r)
{
	this->centre = centre;
	this->colour = colour;
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

	// TODO this should return surface normal at this point
*/
bool Sphere::DoesRayIntersect(
	 Eigen::Vector3f ray,
	float& distance,
	Eigen::Vector3f& surfaceNormal,
	Eigen::Vector3f& reflectedRay,
	Eigen::Vector3f& refractedRay,
	Eigen::Vector3f& colour)
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
			// Note that the ray could start within the circle
			// But for now I'm ignoring this case, since I'm just not going to
			// make scenes that create this case
			// TODO: verify that the first term in the following line is correct ... 
			distance = proj.norm() - sqrt(radius * radius - distToCentre * distToCentre);

			// intersection = p + d*di1
			// di1 = |pc - p| - dist
			// dist = sqrt(r*r - |pc - c|^2)
			// pc - p = projection of centre onto ray, minus ray origin = projection = proj.norm()
		}

		// Dummy rays for now
		reflectedRay = Vector3f(0,0,0);
		refractedRay = Vector3f(0, 0, 0);
		Vector3f normal = distance*ray - centre;
		surfaceNormal = Vector3f(normal[0], normal[1], normal[2]);
		
		colour = this->colour;
		return true;
	}

	// Ray passed outside sphere: no intersection
	return false;
}

Eigen::Vector3f Sphere::GetSurfaceNormalAtPoint(
	Eigen::Vector3f& point)
{
	// TODO: confirm point is on the surface
	Vector3f radial = point - this->centre;
	Vector3f surfaceNormal = 2 * radial - radial;
	return surfaceNormal;
}

/*
	I/O overloads
*/
std::ostream& operator << (std::ostream& os, const Sphere& s)
{
	os << "SPHERE " << s.radius << " ";
	for (int i = 0; i < 3; ++i)
		os << s.centre[i] << " ";
	for (int i = 0; i < 3; ++i)
		os << s.colour[i] << " ";
	return os;
}
std::istream & operator >> (std::istream & is, Sphere & s)
{
	Eigen::Vector3f centre, colour;
	float radius;

	is >> radius;
	is >> centre[0] >> centre[1] >> centre[2];
	is >> colour[0] >> colour[1] >> colour[2];
	s.SetSphere(centre, colour, radius);
	return is;
}

//-----------------------------------------------------------
/*
	The Plane class
*/
Plane::Plane()
{
	normal = Vector3f(1, 0, 0);
	colour = Vector3f(0.5f, 0.5f, 0.5f);
	offset = 0.f;
}

Plane::Plane(
	 const Eigen::Vector3f& normal,
	 const float& offset,
	 const Eigen::Vector3f& colour)
{
	this->normal = normal;
	this->offset = offset;
	this->colour = colour;
}

void Plane::SetPlane(
	 const Eigen::Vector3f& normal,
	 const float& offset,
	 const Eigen::Vector3f& colour)
{
	this->normal = normal;
	this->offset = offset;
	this->colour = colour;
}

/*
	This is considerably simpler than the sphere case. 
	Since the plane is defined by a normal and a constant,
	all points x on the plane fit dot(n,x) = c

	So we can just check if the ray ever fits this. If it does, we solve
	for the scalar factor that allows this and that's the distance. If 
	dot(n,ray) = 0, then we either have infinitely many solutions, or none. 
	In either case, this is not an intersection. 

	Reflection and refraction follow easily too. 
*/
bool Plane::DoesRayIntersect(
	 Eigen::Vector3f ray,
	float& distance,
	Eigen::Vector3f& surfaceNormal,
	Eigen::Vector3f& reflectedRay,
	Eigen::Vector3f& refractedRay,
	Eigen::Vector3f& colour)
{
	float dot = ray.dot(normal);
	surfaceNormal = this->normal;

	if (dot != 0)
	{
		distance = offset / dot;
		if (distance < 0)
		{
			// I think this logic is correct
			// It should mean that if the ray intersection is behind the camera
			// then reject this
			// since every ray should have a positive z
			return false;
		}

		colour = this->colour;

		// solve for reflected and refracted rays etc
		return true;
	}

	return false;
}

Eigen::Vector3f Plane::GetSurfaceNormalAtPoint([[maybe_unused]]Eigen::Vector3f& point)
{
	//point;
	return this->normal;
}

std::ostream& operator << (std::ostream& os, const Plane& p)
{
	os << "PLANE ";
	for (int i = 0; i < 3; ++i)
		os << p.normal[i] << " ";
	for (int i = 0; i < 3; ++i)
		os << p.colour[i] << " ";
	os << p.offset << " ";
	return os;
}
std::istream & operator >> (std::istream & is, Plane& p)
{
	Eigen::Vector3f normal, colour;
	float offset;

	is >> normal[0] >> normal[1] >> normal[2];
	is >> colour[0] >> colour[1] >> colour[2];
	is >> offset;
	p.SetPlane(normal, offset, colour);
	return is;
}