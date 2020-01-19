#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <vector>

#define EPSILON 0.01

class Shape
{
public:
	Shape();
	~Shape();

	// This has an equation defined somehow?
	virtual bool DoesRayIntersect(_In_ Eigen::Vector3f ray, _Out_ float& distance);

	// To be overloaded
	friend std::istream& operator >> (std::istream& is, Shape& s);
	friend std::ostream& operator << (std::ostream& os, const Shape& s);

protected:
	// have all the parameters for material here
	Eigen::Vector3f centre;
	Eigen::Vector3f coeffs;
	Eigen::Vector3f colour;

};

class Sphere : Shape
{
public:
	Sphere();
	Sphere(_In_ const Eigen::Vector3f& centre, _In_ const Eigen::Vector3f& colour, _In_ const float r);
	~Sphere();

	bool DoesRayIntersect(
		_In_ Eigen::Vector3f ray,
		_Out_ float& distance,
		_Out_ Eigen::Vector3f reflectedRay,
		_Out_ Eigen::Vector3f refractedRay,
		_Out_ Eigen::Vector3f colour);

	void SetSphere(_In_ const Eigen::Vector3f& centre,
		_In_ const Eigen::Vector3f& colour,
		_In_ const float r);

	// overload file read write operators
	friend std::ostream& operator << (std::ostream& os, const Sphere& s)
	{
		os << /*"SPHERE " <<*/ s.radius << " ";
		for (int i = 0; i < 3; ++i)
			os << s.centre[i] << " ";
		for (int i = 0; i < 3; ++i)
			os << s.colour[i] << " ";
		return os;
	}
	friend std::istream& operator >> (std::istream& is, Sphere& s)
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
		is >> centre[0] >> centre[0] >> centre[0];
		is >> colour[0] >> colour[0] >> colour[0];
		return is;
	}

private:
	float radius;

	const float BAD_RADIUS = -1;
};

// --------------------------------- //
// Some general shape functions

bool ReadShapes(
	_In_ const std::string& shapeFile,
	_Out_ std::vector<Sphere>& shapes);