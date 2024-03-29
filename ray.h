#pragma once
#include"surfel.h"
struct Ray
{
	Vec3f origin;
	Vec3f direction;
	Ray(Vec3f _from, Vec3f _to) : origin(_from), direction(normalize(_to - _from)) {};	
	Ray() : origin(Vec3f(0, 0, 0)), direction(Vec3f(0, 0, 1)) {};

	const bool testTriangleIntersection(const Vec3<Vec3f>& trianglePos, Vec3f& barCoord, float& parT, float threshold = 0.0001f) const
	{
		Vec3f e0 = trianglePos[1] - trianglePos[0];
		Vec3f e1 = trianglePos[2] - trianglePos[0];		
		Vec3f q = cross(direction, e1);
		float a = dot(e0, q);
		//check if triangle is parallel
		if (abs(a) < threshold)
		{			
			return false;
		}
		Vec3f s = (origin - trianglePos[0])/a;
		float b0 = dot(s, q);
		if (b0 < 0.f || b0 > 1.0f) return false;
		Vec3f r = cross(s, e0);
		float b1 = dot(r, direction);
		if (b1 < 0.0f || b0 + b1 > 1.0f) return false;
		float b2 = 1 - b0 - b1;
		float t = dot(e1, r);
		if (t > threshold)
		{
			barCoord = Vec3f(b0, b1, b2);
			parT = t;
			return true;
		}
		return false;
	}

	bool testPlaneIntersection(const Vec3f& planePos, const Vec3f& planeNormal, Vec3f& intersectionPos, float& parT, float threshold = 0.0001f)
	{
		normalize(planeNormal);
		float denom = dot(direction, planeNormal);
		if(abs(denom) > 1e-6)
		{
			parT = dot(planePos - origin, planeNormal)/denom;
			if (parT >= 0)
			{
				intersectionPos = origin + parT * direction;
				return true;
			}
		}
		return false;
	}

	bool testDiscIntersection(const Vec3f& discPos,const Vec3f& discNormal, float radius, Vec3f& intersectionPos, float& parT, float threshold = 0.0001f)
	{
		if (testPlaneIntersection(discPos, discNormal, intersectionPos, parT, threshold))
		{			
			if ((intersectionPos - discPos).squaredLength() <= radius*radius)
			{								
				return true;
			}
			else
			{				
				return false;
			}
		}
		else return false;
	}

	bool testSurfelIntersection(Surfel surfel, Vec3f& intersectionPos, float& parT, float threshold = 0.0001f)
	{
		return testDiscIntersection(surfel.position, surfel.normal, surfel.radius, intersectionPos, parT, threshold);
	}
};