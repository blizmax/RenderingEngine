#include "BVHnode.h"

BVHnode::BVHnode(const std::vector<Surfel>& surfels)
{            
    Sphere sphere = computeBoundingSphere(surfels);
    new (this) BVHnode(surfels, sphere);
}

 BVHnode::BVHnode(const std::vector<Surfel>& surfels, Sphere sphere)
{
    //std::cout << "n_surfels : " << surfels.size() <<  " |  sphere - center : " << sphere.center() << " radius : " << sphere.radius() << std::endl;
    //stop condition
    if (surfels.size() <= 1)
    {
        m_surfels = surfels;
        m_sphere = sphere;
    }
    else
    {
        m_surfels = surfels;
        m_sphere = sphere;
        //determine in which dimension to split 
        AABB aabb{};
        for (int i = 0; i < surfels.size(); i++)
        {
            aabb.compareAndUpdate(surfels[i].position);
        }
        Vec3f diff = aabb.max() - aabb.min();
        int dimension = -1;
        if (diff[0] >= std::max(diff[1], diff[2])) dimension = 0;
        else if (diff[1] >= std::max(diff[0], diff[2])) dimension = 1;
        else if (diff[2] >= std::max(diff[1], diff[0])) dimension = 2;
        std::vector<float> centers;
        //compute triangle barycenter
        for (int i = 0; i < surfels.size(); i++)
        {
            centers.push_back(surfels[i].position[dimension]);
        }
        std::sort(centers.begin(), centers.end());
        float median = centers[centers.size() / 2];
        //sort surfels in two subsets
        std::vector<Surfel> surfelsLeft, surfelsRight;
        bool addLeft = false;
        for (int i = 0; i < surfels.size(); i++)
        {
            Vec3f center = surfels[i].position;      
            //altern left and right if positions are equal
            if (center[dimension] == median)
            {
                if (addLeft) surfelsLeft.push_back(surfels[i]);
                else surfelsRight.push_back(surfels[i]);                
                addLeft = !addLeft;
            }
            //first half
            else if (center[dimension] < median)
            {
                surfelsLeft.push_back(surfels[i]);
            }
            //second half
            else
            {
                surfelsRight.push_back(surfels[i]);
            }
        }
        //compute BoundingSphere
        Sphere leftSphere = computeBoundingSphere(surfelsLeft);
        Sphere rightSphere = computeBoundingSphere(surfelsRight);
        m_left = BVHptr(new BVHnode(surfelsLeft, leftSphere)); 
        m_right = BVHptr(new BVHnode(surfelsRight, rightSphere));
    }    
}

 Sphere BVHnode::computeBoundingSphere(std::vector<Surfel> surfels)
 {
     const Vec3f& init = surfels[0].position;
     //First step : going through the points to find extreme points along each direction
     Vec3f minX = init, minY = init, minZ = init, maxX = init, maxY = init, maxZ = init;
     for (int i = 0; i < surfels.size(); i++)
     {
         Vec3f center = surfels[i].position;
         //test center to find extreme points in each direction & filling the two subsets
         if (center[0] < minX[0]) minX = center;
         else if (center[0] > maxX[0]) maxX = center;
         if (center[1] < minY[1]) minY = center;
         else if (center[1] < maxY[1]) maxY = center;
         if (center[2] < minZ[2]) minZ = center;
         else if (center[2] < maxZ[2]) maxZ = center;
     }
     //Second step : create inital sphere for max direction and update it
     float xSpan = (maxX - minX).squaredLength();
     float ySpan = (maxY - minY).squaredLength();
     float zSpan = (maxZ - minZ).squaredLength();
     float maxSpan = xSpan; Vec3f minPt = minX, maxPt = maxX;
     if (ySpan > maxSpan)
     {
         maxSpan = ySpan;
         minPt = minY; maxPt = maxY;
     }
     if (zSpan > maxSpan)
     {
         maxSpan = zSpan;
         minPt = minZ; maxPt = maxZ;
     }
     Vec3f ctr = (minPt + maxPt) / 2.f;
     Sphere sphere(ctr, (maxPt - ctr).length());
     for (int i = 0; i < surfels.size(); i++)
     {
         Vec3f pos = surfels[i].position;
         float dist = (pos - sphere.center()).length();
         //if point is outside sphere
         if (dist > sphere.radius())
         {
             //update radius            
             float test = sphere.radius();
             sphere.setRadius((dist + sphere.radius()) / 2.f);
             //update center
             Vec3f centerShift = dist - sphere.radius();
             sphere.center() = (sphere.radius() * sphere.center() + centerShift * pos) / dist;
         }
     }
     return sphere;
 }

 //bool testTriangleIntersection(Ray ray, Vec3<Vec3f> trianglePos, Vec3f& barCoord, float& parT, float threshold = 0.0001f)
 //{
 //    Vec3f e0 = trianglePos[1] - trianglePos[0];
 //    Vec3f e1 = trianglePos[2] - trianglePos[0];
 //    Vec3f q = cross(ray.direction, e1);
 //    float a = dot(e0, q);
 //    if (fabs(a) < threshold)
 //    {
 //        return false;
 //    }
 //    Vec3f s = ray.origin - trianglePos[0];
 //    Vec3f r = cross(s, e0);
 //    float b0 = dot(s, q) / a;
 //    float b1 = dot(r, ray.direction) / a;
 //    float b2 = 1 - b0 - b1;
 //    if (b0 < 0 || b1 < 0 || b0 > 1 || b1>1)
 //    {
 //        return false;
 //    }
 //    float t = dot(e1, r) / a;
 //    if (t > 0 && b0 >= 0.f && b1 >= 0.f && b2 >= 0.f && b0 + b1 + b2 <= 1.f)
 //    {
 //        barCoord = Vec3f(b0, b1, b2);
 //        parT = t;
 //        return true;
 //    }
 //    return false;
 //}

 //bool BVHnode::hit(const Ray& ray, hitInfo& hitRecord, const std::vector<Mesh>& meshes)
 //{
 //    float tmin, tmax;
 //    //the root has no defined bounding box and its meshIndex is = to -1     
 //    if (m_aabb.hit(ray, tmin, tmax) || m_connectivity.size()==0)
 //    {         
 //        //Stop condition : node is leaf
 //        if (m_connectivity.size() == 1)
 //        {             
 //            bool test = false;
 //            for (int i = 0; i < m_connectivity.size(); i++)
 //            {
 //                float t; Vec3f barCoord;
 //                bool triangleIntersect = testTriangleIntersection(ray, meshes[meshIndex].triangle(m_connectivity[i]), barCoord, t, 0.0001f);
 //                if (triangleIntersect && t < hitRecord.parT)
 //                {                     
 //                    hitRecord.barCoord = barCoord;
 //                    hitRecord.parT = t;
 //                    hitRecord.meshIndex = meshIndex;
 //                    hitRecord.triangleIndices = m_connectivity[0];
 //                    test = true;
 //                }
 //            }
 //            return test;
 //        }
 //        else
 //        {             
 //            hitInfo hitInfo1(hitRecord), hitInfo2(hitRecord);
 //            bool hit1 = m_left->hit(ray, hitInfo1, meshes);
 //            bool hit2 = m_right->hit(ray, hitInfo2, meshes);
 //            if (hit1 && hit2)
 //            {
 //                if (hitInfo1.parT < hitInfo2.parT)
 //                {
 //                    hitRecord = hitInfo1;
 //                    return hit1;
 //                }
 //                else
 //                {
 //                    hitRecord = hitInfo2;
 //                    return hit2;
 //                }
 //            }
 //            else if (hit1) { hitRecord = hitInfo1; return hit1; }
 //            else if (hit2) { hitRecord = hitInfo2; return hit2; }
 //            else return false;
 //        }
 //    }
 //    else return false;
 //}


