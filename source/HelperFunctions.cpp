#include "HelperFunctions.h"


bool HelperFunctions::numericalSort(const std::string a, const std::string b)
{
	return (a.length() < b.length() || (a.length() == b.length() && (a < b)));
}

bool HelperFunctions::isLess(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &center)
{
	if (a.x - center.x >= 0 && b.x - center.x < 0)
		return true;
	if (a.x - center.x < 0 && b.x - center.x >= 0)
		return false;
	if (a.x - center.x == 0 && b.x - center.x == 0) {
		if (a.y - center.y >= 0 || b.y - center.y >= 0)
			return a.y > b.y;
		return b.y > a.y;
	}

	// compute the cross product of vectors (center -> a) x (center -> b)
	int det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
	if (det < 0)
		return true;
	if (det > 0)
		return false;

	// points a and b are on the same line from the center
	// check which point is closer to the center
	int d1 = (a.x - center.x) * (a.x - center.x) + (a.y - center.y) * (a.y - center.y);
	int d2 = (b.x - center.x) * (b.x - center.x) + (b.y - center.y) * (b.y - center.y);
	return d1 > d2;
}
//http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
bool HelperFunctions::intersectRayTriangle(const glm::vec3 &va, const glm::vec3 &vb, const glm::vec3 &vc, bool doubleSided, const glm::vec3 &origin, const glm::vec3 &rayDir, glm::vec3 &uvtCoord)
{
	glm::vec3 e1 = vb - va;
	glm::vec3 e2 = vc - va;
	glm::vec3 pvec = glm::cross(rayDir, e2);
	float det = glm::dot(e1, pvec);
	if (doubleSided) {
		// ray and triangle are parallel if det is close to 0
		if (det > -EPSILON && det < EPSILON)
			return false;
	}
	else {
		// if the determinant is negative the triangle is backfacing
		// if the determinant is close to 0, the ray misses the triangle
		if (det < EPSILON)
			return false;
	}
	float invDet = 1 / det;
	float u,v,t;
	// prepare to compute u
	glm::vec3 tvec = origin - va;
	u = glm::dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1)
		return false;
	// prepare to compute v
	glm::vec3 qvec = glm::cross(tvec, e1);
	v = glm::dot(rayDir, qvec) * invDet;
	if (v < 0 || v > 1 || u + v > 1)
		return false;
	// calculate t, ray intersects triangle
	t = glm::dot(e2, qvec) * invDet;
	uvtCoord = glm::vec3(u,v,t);
	return true;
}
