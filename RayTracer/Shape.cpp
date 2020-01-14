#include "Shape.h"

using namespace Eigen;

/*
	The Sphere class

	Simplest shape
	.. more notes
*/
Sphere::Sphere()
{
	// Default is unit sphere at origin
	centre = Vector3f(0,0,0);
	coeffs = Vector3f(1, 1, 1);
	radius = 1;
}

Sphere::Sphere(
	_In_ const Eigen::Vector3f& centre,
	_In_ const float r)
{
	this->centre = centre;
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
	_Out_ float& distance)
{
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
		
		return true;
	}

	// Ray passed outside sphere: no intersection
	return false;
}


