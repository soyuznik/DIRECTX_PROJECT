#pragma once
#include<float.h>

#define PI 3.14159265

class Vector3
{
public:
	float x,y,z,w;
	float verts;
	Vector3()
	{
		set(0,0,0,0);
	}
	~Vector3() {
		
	}
	Vector3(float x,float y,float z,float w=0)
	{
		 set( x,y,z,w);
	}
	void set(float x,float y,float z,float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	void set(float a[4])
	{
		this->set(a[0],a[1],a[2],a[3]);
	}
	void get( float a[4])
	{
		a[0]=this->x;
		a[1]=this->y;
		a[2]=this->z;
		a[3]=this->w;
	}
	void get( float &x,float &y,float &z,float &w)
	{
		this->get(x,y,z,w);
	}
	
	
	Vector3 cross(Vector3 vec)
	{
		Vector3 v;
		v.x = y*vec.z - z*vec.y;
		v.y = z*vec.x - x*vec.z;
		v.z = x*vec.y - y*vec.x;
		return v;
	}

	float dot(Vector3 vec)
	{
		return this->x * vec.x + this->y * vec.y + this->z * vec.z ;
	}

	float magnitude()
	{
		return (float)sqrt(this->x*this->x +this->y*this->y + this->z*this->z);
	}

	Vector3 normalize ()
	{
		//const float squaredLen = this->x*this->x +this->y*this->y + this->z*this->z;
		const float invLen = 1.f / this->magnitude() ;
		this->x *= invLen;
		this->y *= invLen;
		this->z *= invLen;

		return *this;
	}

	float operator() (int i)
	{
		if( i == 0 )
			return this->x;
		else if( i == 1 )
			return this->y;
		else if( i == 2 )
			return this->z;
	}

	Vector3 operator+=( Vector3 vec)
	{
		*this = Vector3( this->x+vec.x , this->y+vec.y , this->z+vec.z );
		return *this;
	}

	Vector3 operator+( Vector3 vec)
	{
		return Vector3( this->x+vec.x , this->y+vec.y , this->z+vec.z );
	}
	Vector3 operator-( Vector3 vec)
	{
		return Vector3( this->x-vec.x , this->y-vec.y , this->z-vec.z );
	}
	Vector3 operator*( float j)
	{
		return Vector3( this->x*j , this->y*j , this->z*j );
	}
	Vector3 operator/( float j)
	{
		return Vector3( this->x/j , this->y/j , this->z/j );
	}
	bool operator==( Vector3 vec)
	{
		return this->x==vec.x && this->y==vec.y && this->z==vec.z; 
	}
	Vector3 negative()
	{
		return Vector3(0,0,0)-*this;
	}

	float getAngleAlongY(   )
	{
		float angle= (atan(this->x/this->z)*180/PI);
		if( (this->x < 0 && this->z >= 0)  ) angle=180-abs(angle); 
		if( (this->x > 0 && this->z > 0)  )  angle=-180+angle; 
		return angle;
	}

	float GetDistance(Vector3 v)
	{
		return sqrt((this->x-v.x)*(this->x-v.x) + (this->y-v.y)*(this->y-v.y) + (this->z-v.z)*(this->z-v.z));
	}

	Vector3 getRandom()
	{
		return Vector3( (float)(::rand()%100)/100.f,(float)(::rand()%100)/100,(float)(::rand()%100)/100);
	}

	void printVal()
	{
		printf("%f %f %f \n",this->x,this->y,this->z);
	}

	char* ToString()
	{
		char *str= new char[100];
		sprintf(str,"%f,%f,%f",x,y,z);
		return str;
	}

	float angleBetween(Vector3 v)
	{
			float dotProduct = this->dot(v);				

			float vectorsMagnitude = this->magnitude() * v.magnitude() ;

			double angle = acos( dotProduct / vectorsMagnitude );

			if(_isnan(angle))
				return 0;

			return( angle );
	}
};
