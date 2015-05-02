#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H
#pragma once
#include "Common.h"

class HelperFunctions
{
public:
	static bool numericalSort(const std::string a, const std::string b);
	//http://stackoverflow.com/questions/6989100/sort-points-in-clockwise-order
	static bool isLess(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &center);
	static bool intersectRayTriangle(const glm::vec3 &va, const glm::vec3 &vb, const glm::vec3 &vc, bool doubleSided, const glm::vec3 &origin, const glm::vec3 &rayDir, glm::vec3 &uvtCoord);
	static int iDivUp(int a, int b);
};


#endif