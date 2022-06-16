
class Polygen
{
public:
	Vector3 *vertix;
	int total_verts;
	Vector3 normal;

	Polygen( int i)
	{
		vertix = new Vector3[i]; 
		total_verts = i;
		this->normal = getNormal()		;
	}

	~Polygen()
	{
		//for(int i=0;i< total_verts ; i++)
		SAFE_DELETE_ARRAY(vertix);
	}


	bool PointInsidePolygon(Vector3 vIntersection)
	{
		const double MATCH_FACTOR = 0.99;		// Used to cover up the error in floating point
		double Angle = 0.0;						// Initialize the angle
		Vector3 vA, vB;						// Create temp vectors
	
		for (int i = 0; i < total_verts; i++)		// Go in a circle to each vertex and get the angle between
		{	
			vA = vertix[i] - vIntersection;			// Subtract the intersection point from the current vertex
													// Subtract the point from the next vertex
			vB = vertix[(i + 1) % total_verts] - vIntersection;
												
			Angle += vA.angleBetween( vB);	// Find the angle between the 2 vectors and add them all up as we go along
		}
											
		if(Angle >= (MATCH_FACTOR * (2.0 * PI)) )	// If the angle is greater than 2 PI, (360 degrees)
			return true;							// The point is inside of the polygon
		
		return false;								// If you get here, it obviously wasn't inside the polygon, so Return FALSE
	}

	Vector3 getNormal()					
	{														// Get 2 vectors from the polygon (2 sides), Remember the order!
		Vector3 vVector1 = vertix[2] - vertix[0];
		Vector3 vVector2 = vertix[1] - vertix[0];

		Vector3 vNormal = vVector1.cross(vVector2);		// Take the cross product of our 2 vectors to get a perpendicular vector

		vNormal = vNormal.normalize();						// Use our function we created to normalize the normal (Makes it a length of one)

		return vNormal;										// Return our normal at our desired length
	}
	
	virtual void Draw()
	{


	}

};

class Triangle : public Polygen
{
public:
	Triangle()
		:Polygen(3)
	{ 
		
	}
	Triangle( Vector3 vertix1,Vector3 vertix2,Vector3 vertix3 ) 
		:Polygen(3)
	{
		this->vertix[0] = vertix1;
		this->vertix[1] = vertix2;
		this->vertix[2] = vertix3;
	}

	void Draw()
	{
		#if defined ___OPENGL
			glBegin(GL_TRIANGLES);
				glVertex3f(vertix[0].x,vertix[0].y,vertix[0].z);   
				glVertex3f( vertix[1].x,vertix[1].y,vertix[1].z); 
				glVertex3f( vertix[2].x,vertix[2].y,vertix[2].z); 
			glEnd();
		#else

			Vector3 normal = this->getNormal();
			normal.y = -normal.y;
			float vertices[] = { vertix[0].x,vertix[0].y,vertix[0].z, normal(0),normal(1),normal(2),
								vertix[1].x,vertix[1].y,vertix[1].z , normal(0),normal(1),normal(2),
								vertix[2].x,vertix[2].y,vertix[2].z	, normal(0),normal(1),normal(2),	};

			g_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL  );
			g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP , 1 ,&vertices , 24 );

		#endif
	}

	bool IfPointInTriangle(Vector3 point)
	{
		return this->PointInsidePolygon(point);
	}
};

class Rect : public Polygen
{
public:
	Rect()
		:Polygen(4)
	{ 
		
	}
	Rect( Vector3 vertix1,Vector3 vertix2,Vector3 vertix3,Vector3 vertix4 ) 
		:Polygen(4)
	{
		this->vertix[0] = vertix1;
		this->vertix[1] = vertix2;
		this->vertix[2] = vertix3;
		this->vertix[3] = vertix4;
	}

	void Draw(  LPDIRECT3DTEXTURE9 texture=NULL )
	{
		#if defined ___OPENGL
			glBegin(GL_QUADS);
				glVertex3f( vertix[0].x,vertix[0].y,vertix[0].z);   
				glVertex3f( vertix[1].x,vertix[1].y,vertix[1].z); 
				glVertex3f( vertix[2].x,vertix[2].y,vertix[2].z); 
				glVertex3f( vertix[3].x,vertix[3].y,vertix[3].z); 
			glEnd();
		#endif

			if( true ) //normaled
			{
				Vector3 normal = this->getNormal();

				float vertices[] = { vertix[0].x,vertix[0].y,vertix[0].z , normal(0),normal(1),normal(2),
									 vertix[1].x,vertix[1].y,vertix[1].z , normal(0),normal(1),normal(2),
									 vertix[2].x,vertix[2].y,vertix[2].z , normal(0),normal(1),normal(2),
									 vertix[3].x,vertix[3].y,vertix[3].z , normal(0),normal(1),normal(2),};

				short indices[] = {
										0,1,2,
										0,2,3,
								}	;
				g_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL  );
				//g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP , 2 ,&vertices , 24 );
				g_pd3dDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLESTRIP , 0 , 24 ,4 , &indices ,D3DFMT_INDEX16  , &vertices , 24 );
			}
	}

	bool IfPointInRect(Vector3 point)
	{
		return this->PointInsidePolygon(point);
	}
};


class Bullet : public Particle
{
public:
	Vector3 start_pos;
	
	float min_distance_from_objs ;
	Vector3 intersectPoint_Real ;
	Vector3 intersectPoint_Ideal ;
	Vector3 intersectPointNormal ;

	vector<Particle>parts;

	Bullet(Vector3 pos,Vector3 dir,float speed,int texture=0)
		:Particle(pos,dir,texture)
	{
		this->speed = speed;
		this->size = .001;
		this->min_distance_from_objs = 1000000.f;
		this->start_pos = pos;
		this->counter = 0;

		this->spriteEngine.mpSize = .02f;
	}

	bool CollideWithPolygen(Polygen *poly )
	{
		bool flag = false;
		Vector3 IntrsecPoint;  //IntSec point of the ray and the plain of perp. of dir of ray
		float u = PlainLineIntersection( pos ,
										dir, poly->getNormal() , 
										poly->vertix[0],
										IntrsecPoint);   //if ray intersect to plain
		if(  u > -(this->speed/2) && u < (this->speed/2) )		
		{
			if( poly->PointInsidePolygon(IntrsecPoint) )
			{
				if( this->min_distance_from_objs > u )
				{
					if( this->age ==0 && u < 0) 
						return false;

					this->min_distance_from_objs = u ;
					this->intersectPoint_Real = pos;
					this->intersectPoint_Ideal = IntrsecPoint;
					this->intersectPointNormal = poly->getNormal();

					flag = true;
				}
			}
		}
		return flag;
	}

	void checkCollosionWithMap(Octree map_octree)
	{
		Octree::Node *node = map_octree.getNodefromPos( this->pos , map_octree.mRootNode );
		for(int i=0;i< node->numFaces;i++)
		{
			Vector3 v1 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v1].position) );
			Vector3 v2 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v2].position) );
			Vector3 v3 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v3].position) );

			Triangle tri = Triangle( v1, v2, v3 );

			this->CollideWithPolygen( &tri  );
		}
		
	}

	bool CollideToPlane(Vector3 pNormal,Vector3 pPoint,float planeRange ,OUT float &distance,bool &nearest )
	{
		bool flag = false;
		Vector3 IntrsecPoint;  //IntSec point of the ray and the plain of perp. of dir of ray
		float u = PlainLineIntersection( pos ,
										dir, pNormal , 
										pPoint,
										IntrsecPoint);   //if ray intersect to plain
		if(  u > -(this->speed/2) && u < (this->speed/2) )		
		{
			if( pPoint.GetDistance( IntrsecPoint  ) <= planeRange )
			{
				if( this->min_distance_from_objs > u )
				{
					if( this->age ==0 && u < 0) 
						return false;

					nearest = true;
					this->min_distance_from_objs  = u;
					this->intersectPoint_Real = pos;
					this->intersectPoint_Ideal = IntrsecPoint;
					this->intersectPointNormal =  pNormal ;

					flag = true;
				}
			}
		}
		distance = u;
		return flag;
	}

	int counter;
	Sprites spriteEngine;

	void DivideToParts()
	{
		if( counter++ > 50 ) return;

		struct Vertex
		{
			D3DXVECTOR3 posit;
			D3DCOLOR    color;

			enum FVF
			{
				FVF_Flags = D3DFVF_XYZ|D3DFVF_DIFFUSE
			};
		};


		if( parts.size() == 0 )
		{
			srand(time(0));
			for(int i=0;i<10;i++)
			{
				
				float x = (float)(rand()%100)/100;
				float y = -(float)(rand()%100)/100;
				float z = (float)(rand()%100)/100;

				if( i%2 ==0 )x = -x;
				if( i%3 ==2 )y = -y;
				if( i%4 ==3 )z = -z;

				Vector3 pDir = this->intersectPointNormal;
				pDir = pDir.negative() + Vector3(x,y,z) ;
				Particle p( this->intersectPoint_Ideal , pDir.normalize() , 0 );
				p.speed = 0.005;
				p.size = .3;
				parts.push_back ( p );
			}
			spriteEngine.mMaxParticles = parts.size();
			spriteEngine.parts = parts ;
			spriteEngine.mpSize = .2f;
			return ;
		}
		spriteEngine.Draw();
	}
};

class LazerBullet : public Bullet
{
public:
	LazerBullet(Vector3 pos,Vector3 dir,float speed,int texture=0)
		:Bullet(pos,dir,speed,0)
	{
		this->size = .001;
	}
	virtual void Draw()
	{
		Vector3 point1_s = this->pos + (this->dir.cross(Vector3(0,1,0)) * -.01),
				point1_e = point1_s  + (this->dir * 1),
				point2_s = this->pos + (this->dir.cross(Vector3(0,1,0)) * .01),
				point2_e = point2_s  + (this->dir * 1);

		::DrawSprite( point1_s , .1 );
		::DrawSprite( point2_s , .1 );

		#if defined ___OPENGL
			glPushMatrix();
				//glTranslatef(pos.x,pos.y,pos.z);
			
				/*glBegin( GL_LINES);
					glVertex3f( point1_s.x,point1_s.y,point1_s.z );
					glVertex3f( point1_e.x,point1_e.y,point1_e.z );
				glEnd();

				glBegin( GL_LINES);
					glVertex3f( point2_s.x,point2_s.y,point2_s.z );
					glVertex3f( point2_e.x,point2_e.y,point2_e.z );
				glEnd();*/

				Sphere s(point1_s,this->size);
				s.Draw(0);

				Sphere s2(point2_s,this->size);
				s2.Draw(0);

			glPopMatrix();
		#endif
	}
};


class Gun
{
public:
	vector<Bullet*> bullets;
	Vector3 pos;
	Vector3 dir;
	float bulletSpeed;

	void SetPosAndDir( Vector3 pos,Vector3 dir)
	{
		this->pos = pos;
		this->dir = dir;
	}
	virtual void StepUpBullets()
	{
		for(int i=0;i< bullets.size();i++)
		{
			if( bullets[i]->age > 100 )
			{
				SAFE_DELETE(bullets[i]);
				bullets.erase( bullets.begin() + i );
				i--;
			}
			else
				bullets[i]->Step();
		}
	}

	Gun(float bulletSpeed=.1f)
	{
		this->bulletSpeed = bulletSpeed;
	}
	virtual void create()
	{
		if(bullets.size()!=0 &&  bullets[bullets.size()-1]->age < 5 ) return;

		Bullet *bullet = new Bullet(this->pos,this->dir,bulletSpeed);
		bullets.push_back(bullet);
	}

	virtual void Draw()
	{
		for(int i=0;i< bullets.size();i++)
		{
			if( !bullets[i]->stopped )
				bullets[i]->Draw();
		}
	}

	virtual void fire()
	{
		create();
	}
};

class LazerGun:public Gun
{
public:
	LazerGun( float bulletSpeed)
		:Gun( bulletSpeed )
	{
		
	}
	virtual void create()
	{
		if(bullets.size()!=0 &&  bullets[bullets.size()-1]->age < 10 ) return;

		Bullet *bullet = new LazerBullet(this->pos,this->dir,bulletSpeed);
		bullets.push_back(bullet);
	}
};

Triangle *Loadmap(char *fileName,int *size)
{
	FILE *fp = fopen(fileName, "r");

	int numVerts=0;
	while(1)
	{
		Vector3 vTemp;
		int result = fscanf(fp, "%f %f %f\n", &vTemp.x, &vTemp.y, &vTemp.z);
		if(result == EOF) 
			break;
		 numVerts++;
	}
	rewind(fp);

	Triangle *t = new Triangle[numVerts];
	for(int i = 0; i < numVerts/3; i++)
	{
		fscanf(fp, "%f %f %f\n", &t[ i ].vertix[0].x, 
								 &t[ i ].vertix[0].y, 
								&t[ i ].vertix[0].z);
		
		fscanf(fp, "%f %f %f\n", &t[ i ].vertix[1].x, 
								 &t[ i ].vertix[1].y, 
								&t[ i ].vertix[1].z);
		
		fscanf(fp, "%f %f %f\n", &t[ i ].vertix[2].x, 
								 &t[ i ].vertix[2].y, 
								&t[ i ].vertix[2].z);

		t[ i ].vertix[0] = t[ i ].vertix[0]*.2f;
		t[ i ].vertix[1] = t[ i ].vertix[1]*.2f;
		t[ i ].vertix[2] = t[ i ].vertix[2]*.2f;
	}
	fclose(fp);
	*size =  numVerts/3;
	return t;
}

class Map
{
public:
	Triangle *tris;
	int totalMapTris;
	
	void Loadmap(char *fileName)
	{
		FILE *fp = fopen(fileName, "r");

		int numVerts=0;
		while(1)
		{
			Vector3 vTemp;
			int result = fscanf(fp, "%f %f %f\n", &vTemp.x, &vTemp.y, &vTemp.z);
			if(result == EOF) 
				break;
			 numVerts++;
		}
		rewind(fp);

		tris = new Triangle[numVerts];
		Triangle *t = tris;
		for(int i = 0; i < numVerts/3; i++)
		{
			fscanf(fp, "%f %f %f\n", &t[ i ].vertix[0].x, 
									 &t[ i ].vertix[0].y, 
									&t[ i ].vertix[0].z);
		
			fscanf(fp, "%f %f %f\n", &t[ i ].vertix[1].x, 
									 &t[ i ].vertix[1].y, 
									&t[ i ].vertix[1].z);
		
			fscanf(fp, "%f %f %f\n", &t[ i ].vertix[2].x, 
									 &t[ i ].vertix[2].y, 
									&t[ i ].vertix[2].z);

			t[ i ].vertix[0] = t[ i ].vertix[0]*.2f;
			t[ i ].vertix[1] = t[ i ].vertix[1]*.2f;
			t[ i ].vertix[2] = t[ i ].vertix[2]*.2f;
		}
		fclose(fp);

		totalMapTris = numVerts/3 ;
	}

	void Draw()
	{
		for(int i=0;i<totalMapTris;i++)
		{
			tris[i].Draw();
		}
	}

	void Scale(float size)
	{
		for(int i=0;i<totalMapTris;i++)
		{
			tris[i].vertix[0] = tris[i].vertix[0]*size; 
			tris[i].vertix[1] = tris[i].vertix[1]*size;
			tris[i].vertix[2] = tris[i].vertix[2]*size;
		}
	}
};