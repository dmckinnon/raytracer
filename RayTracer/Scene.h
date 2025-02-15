#pragma once
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <vector>

#define EPSILON 0.01

#define MAX_NUM_BOUNCES_PER_RAY 10

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

class Material;

// This struct stores the collision of light with a shape.
// It contains the point where the light hit,
// the surface normal, any reflections or refractions, the colour,
// and whether the light was on the back face or front face.
struct LightCollision
{
	Eigen::Vector3f point;
	Eigen::Vector3f surfaceNormal;
	Eigen::Vector3f reflectedRay;
	Eigen::Vector3f refractedRay;
	Eigen::Vector3f colour;

	// need a material here
	std::shared_ptr<Material> material;
	

	bool frontFace;
};

/// should material naturally be part of shape or separate?
// Should a shape have a material? Should a shape naturally be
// a material? No, can assign different shapes different materials
// if a shape has a material then act on that
class Material
{
	public:
	virtual ~Material() {};

	virtual bool scatter(
		Eigen::Vector3f& incomingRay,
		// Some sort of collision record
		LightCollision& collision,
		Eigen::Vector3f& attentuationColour,
		Eigen::Vector3f& scatteredRay)
		{
			return false;
		}
};

class Shape
{
public:
	virtual ~Shape() {};

	// This has an equation defined somehow?
	virtual bool DoesRayIntersect(
		Eigen::Vector3f ray,
		float& distance,
		LightCollision& collision) = 0;

	virtual Eigen::Vector3f GetSurfaceNormalAtPoint(
	    Eigen::Vector3f& point) = 0;

	// a property of the material itself
	virtual float GetDiffusionFactor() = 0;

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

	void AddShape(Shape* shape);
	void AddLight(Light* light);

	std::vector<Shape*> GetShapes() { return shapes; };
	std::vector<Light*> GetLights() { return lights; };

	// Currently backgrounds are just colours, nothing fancier
	void SetBackground(Eigen::Vector3f& colour);
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
		const Eigen::Vector3f& position,
		const Eigen::Vector3f& colour,
		const float& intensity);
	~PointLight() {};

	Eigen::Vector3f GetPosition() override;
	float GetIntensity() override;

	void SetLight(
		const Eigen::Vector3f& position,
		const Eigen::Vector3f& colour,
		const float& intensity);

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
	Sphere(const Eigen::Vector3f& centre, const Eigen::Vector3f& colour, Material& m, const float r);
	~Sphere();

	bool DoesRayIntersect(
		Eigen::Vector3f ray,
		float& distance,
		LightCollision& collision) override;

	float GetDiffusionFactor() override;

	Eigen::Vector3f GetSurfaceNormalAtPoint(
		Eigen::Vector3f& point) override;

	void SetSphere(const Eigen::Vector3f& centre,
		const Eigen::Vector3f& colour,
		const float r);

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

	float diffusionFactor;

	Material material;

	static const int BAD_RADIUS = -1;
};

class Plane : public Shape
{
public:
	Plane();
	Plane(const Eigen::Vector3f& normal, const float& offset, const Eigen::Vector3f& colour);
	~Plane() {};

	bool DoesRayIntersect(
		Eigen::Vector3f ray,
		float& distance,
		LightCollision& collision) override;

	float GetDiffusionFactor() override;

	Eigen::Vector3f GetSurfaceNormalAtPoint(
		Eigen::Vector3f& point) override;

	void SetPlane(const Eigen::Vector3f& normal,
		const float& offset,
		const Eigen::Vector3f& colour);

	// IO overloads
	friend std::ostream& operator << (std::ostream& os, const Plane& s);
	friend std::istream& operator >> (std::istream& is, Plane& s);


private:
	Eigen::Vector3f normal;
	float offset;
	Eigen::Vector3f colour;

	Material material;

	float diffusionFactor;
};

/**********************************************/
/*############ MATERIAL CLASSES ##############*/
/**********************************************/
class Metal : public Material
{
public :
	Metal();
	// need other properties
	~Metal() {};

	bool scatter(
		Eigen::Vector3f& incomingRay,
		// Some sort of collision record
		LightCollision& collision,
		Eigen::Vector3f& attentuationColour,
		Eigen::Vector3f& scatteredRay
	) override;

private:
// various attentuation factors reflectivity factors, etc etc
};

// --------------------------------- //
// Some general scene functions

bool ReadScene(
	const std::string& sceneFile,
	Scene& scene);