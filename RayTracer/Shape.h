#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <vector>

#define EPSILON 0.01

class Shape
{
public:
	//Shape();
	virtual ~Shape() {};

	// This has an equation defined somehow?
	virtual bool DoesRayIntersect(
		_In_ Eigen::Vector3f ray,
		_Out_ float& distance,
		_Out_ Eigen::Vector3f& reflectedRay,
		_Out_ Eigen::Vector3f& refractedRay,
		_Out_ Eigen::Vector3f& colour) = 0;

	// To be overloaded
	//friend std::istream& operator >> (std::istream& is, Shape& s);
	//friend std::ostream& operator << (std::ostream& os, const Shape& s);

protected:
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
		_Out_ Eigen::Vector3f& reflectedRay,
		_Out_ Eigen::Vector3f& refractedRay,
		_Out_ Eigen::Vector3f& colour) override;

	void SetSphere(_In_ const Eigen::Vector3f& centre,
		_In_ const Eigen::Vector3f& colour,
		_In_ const float r);

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const Sphere& s);
	friend std::istream& operator >> (std::istream& is, Sphere& s);


private:
	float radius;
	Eigen::Vector3f coeffs;
	Eigen::Vector3f centre;
	Eigen::Vector3f colour;

	const float BAD_RADIUS = -1;
};

// --------------------------------- //
// Some general shape functions

bool ReadShapes(
	_In_ const std::string& shapeFile,
	_Out_ std::vector<Sphere>& shapes);