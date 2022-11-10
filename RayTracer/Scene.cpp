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
void Scene::AddShape(Shape* shape)
{
	shapes.push_back(shape);
}

void Scene::AddLight(Light* light)
{
	lights.push_back(light);
}

void Scene::SetBackground(Eigen::Vector3f& colour)
{
	backgroundColour = colour;
}

Eigen::Vector3f Scene::GetBackground()
{
	return backgroundColour;
}

/*
	I/O overloads
*/
std::ostream& operator << (std::ostream& os, const Scene& s)
{
	os << s.shapes.size() << " " << s.lights.size() << endl;
	for (int i = 0; i < s.shapes.size(); ++i)
		os << s.shapes[i] << endl;
	for (int i = 0; i < s.lights.size(); ++i)
		os << s.lights[i] << endl;
	os << s.backgroundColour[0] << " " << s.backgroundColour[1] << " " << s.backgroundColour[2] << endl;
	return os;
}
std::istream & operator >> (std::istream & is, Scene& scene)
{
	Eigen::Vector3f colour;
	int numShapes, numLights;
	string shapeType, lightType;
	
	is >> numShapes >> numLights;
	for (int i = 0; i < numShapes; ++i)
	{
		// Check shape type
		is >> shapeType;
		if (strcmp(shapeType.c_str(), "SPHERE") == 0)
		{
			Sphere* s = new Sphere();
			is >> *s;
			scene.AddShape(s);
		}
		else if (strcmp(shapeType.c_str(), "PLANE") == 0)
		{
			Plane* p = new Plane();
			is >> *p;
			scene.AddShape(p);
		}
	}
	for (int i = 0; i < numLights; ++i)
	{
		// Check light type
		is >> lightType;
		if (strcmp(lightType.c_str(), "POINTLIGHT") == 0)
		{
			PointLight* pl = new PointLight();
			is >> *pl;
			scene.AddLight(pl);
		}
	}

	is >> colour[0] >> colour[1] >> colour[2];
	scene.SetBackground(colour);
	return is;
}

//-----------------------------------------------------------
/*
	Read a scene from a file
*/
bool ReadScene(
	const string& sceneFile,
	Scene& scene)
{
	ifstream file;
	file.open(sceneFile);
	if (file.is_open())
	{
		while (!file.eof())
		{
			file >> scene;
		}
		file.close();
		return true;
	}

	return false;
}