#pragma once
#include <Eigen/Dense>

#define EPSILON 0.01

class Shape
{
public:
	Shape();
	~Shape();

	// This has an equation defined somehow?
	virtual bool DoesRayIntersect(_In_ Eigen::Vector3f ray, _Out_ float& distance);
protected:
	// have all the parameters for material here
	Eigen::Vector3f centre;
	Eigen::Vector3f coeffs;

};

class Sphere : Shape
{
public:
	Sphere();
	Sphere(_In_ const Eigen::Vector3f& centre, _In_ const float r);
	~Sphere();

	bool DoesRayIntersect(_In_ Eigen::Vector3f ray, _Out_ float& distance);

	// overload file read write operators

private:
	float radius;
};