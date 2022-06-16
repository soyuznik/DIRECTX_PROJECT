#include "vector3.h"

const int BEHIND = 0,FRONT = 1,INTERSECTS= 2;

float Absolute(float num)
{
	// If num is less than zero, we want to return the absolute value of num.
	// This is simple, either we times num by -1 or subtract it from 0.
	if(num < 0)
		return (0 - num);

	// Return the original number because it was already positive
	return num;
}

double AngleBetweenVectors(Vector3 Vector1, Vector3 Vector2)
{							
	// Get the dot product of the vectors
	float dotProduct = Vector1.dot(Vector2);				

	// Get the product of both of the vectors magnitudes
	float vectorsMagnitude = Vector1.magnitude() * Vector2.magnitude() ;

	// Get the angle in radians between the 2 vectors
	double angle = acos( dotProduct / vectorsMagnitude );

	// Here we make sure that the angle is not a -1.#IND0000000 number, which means indefinate
	if(_isnan(angle))
		return 0;
	
	// Return the angle in radians
	return( angle );
}

Vector3 Normal(Vector3 vPolygon[])					
{														// Get 2 vectors from the polygon (2 sides), Remember the order!
	Vector3 vVector1 = vPolygon[2] - vPolygon[0];
	Vector3 vVector2 = vPolygon[1] - vPolygon[0];

	Vector3 vNormal = vVector1.cross(vVector2);		// Take the cross product of our 2 vectors to get a perpendicular vector

	// Now we have a normal, but it's at a strange length, so let's make it length 1.

	vNormal = vNormal.normalize();						// Use our function we created to normalize the normal (Makes it a length of one)

	return vNormal;										// Return our normal at our desired length
}


float PlaneDistance(Vector3 Normal, Vector3 Point)
{	
	float distance = 0;									// This variable holds the distance from the plane tot he origin

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// So, we come up with D = -(Ax + By + Cz)
														// Basically, the negated dot product of the normal of the plane and the point. (More about the dot product in another tutorial)
	distance = - ((Normal.x * Point.x) + (Normal.y * Point.y) + (Normal.z * Point.z));

	return distance;									// Return the distance
}


int ClassifySphere(Vector3 &vCenter, 
				Vector3 &vNormal, Vector3 &vPoint, float radius, float &distance)
{
	// First we need to find the distance our polygon plane is from the origin.
	float d = (float)PlaneDistance(vNormal, vPoint);

	// Here we use the famous distance formula to find the distance the center point
	// of the sphere is from the polygon's plane.  
	distance = (vNormal.x * vCenter.x + vNormal.y * vCenter.y + vNormal.z * vCenter.z + d);

	// If the absolute value of the distance we just found is less than the radius, 
	// the sphere intersected the plane.
	if(abs(distance) < radius)
		return INTERSECTS;
	// Else, if the distance is greater than or equal to the radius, the sphere is
	// completely in FRONT of the plane.
	else if(distance >= radius)
		return FRONT;
	
	// If the sphere isn't intersecting or in FRONT of the plane, it must be BEHIND
	return BEHIND;
}

int ClassifyCuboid( Vector3 &vCenter, 
				Vector3 &vNormal, Vector3 &vPoint, Vector3 cuboid_lhw, float &distance,float &max)
{
	// First we need to find the distance our polygon plane is from the origin.
	float d = (float)PlaneDistance(vNormal, vPoint);

	// Here we use the famous distance formula to find the distance the center point
	// of the sphere is from the polygon's plane.  
	distance = (vNormal.x * vCenter.x + vNormal.y * vCenter.y + vNormal.z * vCenter.z + d);

	if(  abs(vNormal.x) > abs(vNormal.y) && abs(vNormal.x) > abs(vNormal.z) ) max = cuboid_lhw.x;
	else if( abs(vNormal.y) > abs(vNormal.z) ) max = cuboid_lhw.y;
	else max = cuboid_lhw.z;

	if(  ( abs(distance) < max ) )
		return INTERSECTS;
	else if( distance >= max  )
		return FRONT;

	return BEHIND ;

}


bool InsidePolygon(Vector3 vIntersection, Vector3 Poly[], long verticeCount)
{
	const double MATCH_FACTOR = 0.99;		// Used to cover up the error in floating point
	double Angle = 0.0;						// Initialize the angle
	Vector3 vA, vB;						// Create temp vectors
	
	for (int i = 0; i < verticeCount; i++)		// Go in a circle to each vertex and get the angle between
	{	
		vA = Poly[i] - vIntersection;			// Subtract the intersection point from the current vertex
												// Subtract the point from the next vertex
		vB = Poly[(i + 1) % verticeCount] - vIntersection;
												
		Angle += AngleBetweenVectors(vA, vB);	// Find the angle between the 2 vectors and add them all up as we go along
	}
											
	if(Angle >= (MATCH_FACTOR * (2.0 * PI)) )	// If the angle is greater than 2 PI, (360 degrees)
		return true;							// The point is inside of the polygon
		
	return false;								// If you get here, it obviously wasn't inside the polygon, so Return FALSE
}

Vector3 GetCollisionOffset(Vector3 &vNormal, float radius, float distance)
{
	Vector3 vOffset;
	if(distance > 0)
	{
		float distanceOver = radius - distance;
		vOffset = vNormal * distanceOver;
	}
	else // Else colliding from behind the polygon
	{
		float distanceOver = radius + distance;
		vOffset = vNormal * -distanceOver;
	}
	return vOffset;
}


Vector3 ClosestPointOnLine(Vector3 vA, Vector3 vB, Vector3 vPoint)
{
	// Create the vector from end point vA to our point vPoint.
	Vector3 vVector1 = vPoint - vA;

	// Create a normalized direction vector from end point vA to end point vB
	Vector3 vVector2 = (vB - vA).normalize();

	// Use the distance formula to find the distance of the line segment (or magnitude)
	float d = vA.GetDistance(vB);

	// Using the dot product, we project the vVector1 onto the vector vVector2.
	// This essentially gives us the distance from our projected vector from vA.
	float t = vVector2.dot(vVector1);

	// If our projected distance from vA, "t", is less than or equal to 0, it must
	// be closest to the end point vA.  We want to return this end point.
    if (t <= 0) 
		return vA;

	// If our projected distance from vA, "t", is greater than or equal to the magnitude
	// or distance of the line segment, it must be closest to the end point vB.  So, return vB.
    if (t >= d) 
		return vB;
 
	// Here we create a vector that is of length t and in the direction of vVector2
    Vector3 vVector3 = vVector2 * t;

	// To find the closest point on the line segment, we just add vVector3 to the original
	// end point vA.  
    Vector3 vClosestPoint = vA + vVector3;

	// Return the closest point on the line segment
	return vClosestPoint;
}


bool EdgeSphereCollision(Vector3 &vCenter, 
						 Vector3 vPolygon[], int vertexCount, float radius)
{
	Vector3 vPoint;

	// Go through all of the vertices in the polygon
	for(int i = 0; i < vertexCount; i++)
	{
		// This returns the closest point on the current edge to the center of the sphere.
		vPoint = ClosestPointOnLine(vPolygon[i], vPolygon[(i + 1) % vertexCount], vCenter);
		
		// Now, we want to calculate the distance between the closest point and the center
		float distance = vPoint.GetDistance(vCenter);

		// If the distance is less than the radius, there must be a collision so return true
		if(distance < radius)
			return true;
	}

	// The was no intersection of the sphere and the edges of the polygon
	return false;
}


//Others

//void checkCollosionWithTriangle(Triangle tri,Vector3 pos,float radius)
//{
//	float distance;
//	if( ClassifySphere(pos, tri.getNormal() , tri.vertix[0], radius, OUT distance) == INTERSECTS )
//	{
//		Vector3 vIntersection = pos - tri.getNormal()*distance;
//		if( InsidePolygon( vIntersection ,  &tri.vertix[0] , 3 )  || EdgeSphereCollision(pos, &tri.vertix[0], 3 , radius/2 ))
//		{
//			
//			Vector3 cOffset = GetCollisionOffset( tri.getNormal(), radius , distance );
//			pos = pos + cOffset;
//		}
//	}
//}

//void sphereCollosionWithTriangle( Vector3 sPos , Vector3 sDir ,  Vector3 pNormal , Vector3 pPoint ,    )
//{
//	Vector3 IntrsecPoint;  //IntSec point of the ray and the plain of sphere 
//	float u = PlainLineIntersection(player->gun->bullets[i]->pos ,
//								player->gun->bullets[i]->dir,Vector3(0,0,0)-player->gun->bullets[i]->dir , 
//								s.pos,
//								IntrsecPoint);   //if ray intersect to plain
//	if( u >= -(player->gun->bullets[i]->speed/2) && u < (player->gun->bullets[i]->speed/2)  ) 
//	{
//		if( s.pos.GetDistance(IntrsecPoint) <= s.radius )
//		{
//			s.died=true;
//		}
//	}
//}