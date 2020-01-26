#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <vector>

#define EPSILON 0.01

/**********************************************/
/*############# SCENE CLASSES ################*/
/**********************************************/
class Light
{
public:
	virtual ~Light() {};

	virtual Eigen::Vector3f GetPosition() = 0;
	virtual float GetIntensity() = 0;

	// angle for directional?

protected:
	// What does a light need?
	// Colour, intensity, whether it's a point source or directional
	// all lights currently shall be point sources

};

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

protected:
};

class Scene
{
public:
	Scene() 
	{
		backgroundColour = Eigen::Vector3f(0.1f, 0.1f, 0.1f);
	};
	~Scene() {};

	// other constructors

	void AddShape(_In_ Shape* shape);
	void AddLight(_In_ Light* light);

	std::vector<Shape*> GetShapes() { return shapes; };
	std::vector<Light*> GetLights() { return lights; };

	// Currently backgrounds are just colours, nothing fancier
	void SetBackground(_In_ Eigen::Vector3f& colour);
	Eigen::Vector3f GetBackground();

	// What else does a scene have? Objects and lighting, really

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const Scene& s);
	friend std::istream& operator >> (std::istream& is, Scene& s);

private:
	std::vector<Shape*> shapes;
	std::vector<Light*> lights;

	Eigen::Vector3f backgroundColour;
};

/**********************************************/
/*############# Light CLASSES ################*/
/**********************************************/
class PointLight : public Light
{
public:
	PointLight();
	PointLight(
		_In_ const Eigen::Vector3f& position,
		_In_ const Eigen::Vector3f& colour,
		_In_ const float& intensity);
	~PointLight() {};

	Eigen::Vector3f GetPosition() override;
	float GetIntensity() override;

	void SetLight(
		_In_ const Eigen::Vector3f& position,
		_In_ const Eigen::Vector3f& colour,
		_In_ const float& intensity);

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const PointLight& pl);
	friend std::istream& operator >> (std::istream& is, PointLight& pl);

private:
	Eigen::Vector3f position;
	float intensity;

	// This colour is not going to be relevant
	// for a while. For simplicity, I'll start with
	// white light, or rather, general light. 
	Eigen::Vector3f colour;
};

/**********************************************/
/*############# SHAPE CLASSES ################*/
/**********************************************/
class Sphere : public Shape
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

	Eigen::Vector3f GetCentre() { return centre; };
	float           GetRadius() { return radius; };
	Eigen::Vector3f GetColour() { return colour; };

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const Sphere& s);
	friend std::istream& operator >> (std::istream& is, Sphere& s);

private:
	float radius;
	Eigen::Vector3f centre;
	Eigen::Vector3f colour;

	static const int BAD_RADIUS = -1;
};

class Plane : public Shape
{
public:
	Plane();
	Plane(_In_ const Eigen::Vector3f& normal, _In_ const float& offset, _In_ const Eigen::Vector3f& colour);
	~Plane() {};

	bool DoesRayIntersect(
		_In_ Eigen::Vector3f ray,
		_Out_ float& distance,
		_Out_ Eigen::Vector3f& reflectedRay,
		_Out_ Eigen::Vector3f& refractedRay,
		_Out_ Eigen::Vector3f& colour) override;

	void SetPlane(_In_ const Eigen::Vector3f& normal,
		_In_ const float& offset,
		_In_ const Eigen::Vector3f& colour);

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const Plane& s);
	friend std::istream& operator >> (std::istream& is, Plane& s);


private:
	Eigen::Vector3f normal;
	float offset;
	Eigen::Vector3f colour;
};

// --------------------------------- //
// Some general scene functions

bool ReadScene(
	_In_ const std::string& sceneFile,
	_Out_ Scene& scene);