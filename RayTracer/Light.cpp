#include "Scene.h"
#include <string>
#include <fstream>

using namespace Eigen;
using namespace std;

//-----------------------------------------------------------
/*
	The Point Light class

	Simplest light
	We're not yet using light colour. Light is generic and has
	"no colour" - that is, it does not influence the colour
	of objects beyond shadowing, etc
*/
PointLight::PointLight()
{
	position = Vector3f(0,0,0);
	colour = Vector3f(1, 1, 1);
	intensity = 1;
}

PointLight::PointLight(
	_In_ const Eigen::Vector3f& position,
	_In_ const Eigen::Vector3f& colour,
	_In_ const float& intensity)
{
	this->position = position;
	this->colour = colour;
	if (intensity >= 0 && intensity <= 1)
	{
		this->intensity = intensity;
	}
}

void PointLight::SetLight(
	_In_ const Eigen::Vector3f& position,
	_In_ const Eigen::Vector3f& colour,
	_In_ const float& intensity)
{
	this->position = position;
	this->colour = colour;
	if (intensity >= 0 && intensity <= 1)
	{
		this->intensity = intensity;
	}
}

Vector3f PointLight::GetPosition()
{
	return position;
}

/*
	We may have other lights whose intensity is determined
	by a cardoid, or some other function of angle and space
*/
float PointLight::GetIntensity()
{
	return intensity;
}

/*
	I/O overloads
*/
std::ostream& operator << (std::ostream& os, const PointLight& pl)
{
	os << "POINTLIGHT ";
	for (int i = 0; i < 3; ++i)
		os << pl.position[i] << " ";
	for (int i = 0; i < 3; ++i)
		os << pl.colour[i] << " ";
	os << pl.intensity;
	return os;
}
std::istream & operator >> (std::istream & is, PointLight& pl)
{
	Eigen::Vector3f position, colour;
	float intensity;

	is >> position[0] >> position[1] >> position[2];
	is >> colour[0] >> colour[1] >> colour[2];
	is >> intensity;
	pl.SetLight(position, colour, intensity);
	return is;
}