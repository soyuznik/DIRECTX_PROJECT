#include <ctime>

#if defined ___OPENGL
	GLUquadric *q = gluNewQuadric();
#endif


void DrawSprite( Vector3 pos , float size , D3DXCOLOR color=0x00ffffff )
{
	g_pd3dDevice->SetTexture( 0, g_pTexture_Particle );
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );    // Turn on point sprites
	g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );    // Allow sprites to be scaled with distance
	g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE,     FtoDW(size) );  // Float value that specifies the size to use for point size computation in cases where point size is not specified for each vertex.
	g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(1.0f) ); // Float value that specifies the minimum size of point primitives. Point primitives are clamped to this size during rendering. 
	g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.0f) ); // Default 1.0
	g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.0f) ); // Default 0.0
	g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(1.f) ); // Default 0.0

	//
	// Render point sprites...
	//
	g_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	float vec[] = {pos.x,pos.y,pos.z, color };
	g_pd3dDevice->DrawPrimitiveUP( D3DPT_POINTLIST, 1 , &vec ,16 );
	//
	// Reset render states...
	//
	g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );

	g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}


class Particle
{
public:
	Vector3 pos_init;
	Vector3 pos;
	Vector3 dir;

	int texture;
	float age;
	float size;
	float speed;
	bool stopped;

	Particle(){}
	Particle(Vector3 pos,Vector3 dir,int texture)
	{
		this->age = 0.0f;
		this->pos = pos;
		this->dir = dir;
		this->texture = texture ;
		this->stopped = false;

		this->pos_init = this->pos;
	}

	virtual void Step()
	{
		this->age++;

		if(  this->stopped  ) 
			return ; 

		this->pos.x += dir.x*speed;
		this->pos.y += dir.y*speed;
		this->pos.z += dir.z*speed;
	}

	void stop()
	{
		this->stopped = true;
	}

	virtual void Draw()
	{
		#if defined ___OPENGL
			glPushMatrix();
				glTranslatef(pos.x,pos.y,pos.z);            
				gluQuadricNormals(q, GL_SMOOTH);                            
				gluQuadricTexture(q, GL_TRUE);                              
				gluSphere(q, size, 32, 16);
			glPopMatrix();
		#endif	
		DrawSprite( pos , .5 ) ;
	}

};


float PlainLineIntersection(Vector3 LStartPoint,Vector3 LDir,Vector3 PNormal,Vector3 PPoint,OUT Vector3 &IntersectionPoint){
	
	//http://www.thepolygoners.com/tutorials/lineplane/lineplane.html
	float t = PNormal.dot( PPoint -  LStartPoint ) /  PNormal.dot( LDir );
	IntersectionPoint = LStartPoint+(LDir*t);
	return t;
}


class Sphere
{
public:
	Vector3 pos;
	float radius;
	bool died;

	Sphere( Vector3 pos,float radius )
	{
		this->pos = pos;
		this->radius = radius;
		this->died = false;
	}

	bool pointInSphere(Vector3 point)
	{
		return ((point.x-pos.x)*(point.x-pos.x) + (point.y-pos.y)*(point.y-pos.y) +    (point.z-pos.z)*(point.z-pos.z)) < radius*radius ; 
	}

	void Draw(int texture,float angle=0)
	{
		if(  !died )
		{
			#if defined ___OPENGL
				glPushMatrix();
					glTranslatef(pos(0), pos(1) ,pos(2));
					glRotatef(angle*100,0,1,0);                        
					gluQuadricNormals(q, GL_SMOOTH);                            
					gluQuadricTexture(q, GL_TRUE);         
					gluSphere(q, radius , 32, 16);
				glPopMatrix();
			#endif
		}
		else
		{
			Blast();
		}
	}

	vector<Particle> particles;
	void Blast()
	{
		static int counter =0;
		if( counter++ > 50 ) return;

		if( particles.size() == 0 )
		{
			srand(time(0));
			for(int i=0;i<100;i++)
			{
				float x = (float)(rand()%100)/100;
				float y = (float)(rand()%100)/100;
				float z = (float)(rand()%100)/100;

				if( i%2 ==0 )x = -x;
				if( i%3 ==2 )y = -y;
				if( i%4 ==3 )z = -z;


				Particle p( pos, Vector3(x,y,z) , 0 );
				p.speed = .05;
				p.size = .01;

				particles.push_back ( p );
			}
			return;
		}

		for(int i=0;i<particles.size();i++)
		{ 
			#if defined ___OPENGL
				glPushMatrix();
					//glTranslatef(pos(0), pos(1) ,pos(2));
					particles[i].Step();
					particles[i].Draw ();
				glPopMatrix();
			#endif
		}
	}
};

class Sprites
{
public:
	vector<Particle>parts;
	int mMaxParticles;
	int mMaxAge;
	float mpSize;
	float mpColor;
	
	float stop_timer;

	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;

	struct Vertex
	{
		D3DXVECTOR3 posit;
		D3DCOLOR    color;

		enum FVF
		{
			FVF_Flags = D3DFVF_XYZ|D3DFVF_DIFFUSE
		};
	};

	~Sprites()
	{
		SAFE_RELEASE(m_pVertexBuffer);
		//SAFE_DELETE(m_pVertexBuffer);
		
		parts.clear();
	}

	Sprites(int maxParticles=100 , int maxAge=100 )
	{
		mMaxParticles = maxParticles ;
		mMaxAge = maxAge;
		mpSize = .1;
		mpColor = 0x00ffff;
		stop_timer  = 0;

		m_pVertexBuffer=NULL;
		Init();
	}

	void Init()
	{
			srand(time(0));
			g_pd3dDevice->CreateVertexBuffer( mMaxParticles * sizeof(Vertex), 
												D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
												Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
												&m_pVertexBuffer, NULL );
			float fMaxPointSize = 0.0f;
			bool  bDeviceSupportsPSIZE = false;
	
			D3DCAPS9 d3dCaps;
			g_pd3dDevice->GetDeviceCaps( &d3dCaps );

			fMaxPointSize = d3dCaps.MaxPointSize;

			if( d3dCaps.FVFCaps & D3DFVFCAPS_PSIZE )
				bDeviceSupportsPSIZE = true;
			else
				bDeviceSupportsPSIZE = false;
			return ;
	}

	void Stop()
	{
		 stop_timer ++;
	}

	void Start()
	{
		 stop_timer =0;
	}

	bool isFullyStopped()
	{
		//if( stop_timer <= 0 )
		//	return false;

		bool flag = true;
		for(int i=0;i<parts.size();i++)
		{ 
			if( parts[i].age < mMaxAge-1 )
				flag = false;
		}
		return flag;
	}

	void Draw()
	{
		
		for(int i=0;i<parts.size();i++)
		{ 
			parts[i].Step();
			parts[i].age +=1;

			if( parts[i].age > mMaxAge )
			{
				parts[i].pos  = parts[i].pos_init;
				parts[i].age = 0;
			}
			Vertex *pPointVertices;

			m_pVertexBuffer->Lock( 0, mMaxParticles * sizeof(Vertex),
									(void**)&pPointVertices, D3DLOCK_DISCARD );

			for( int i = 0; i < mMaxParticles; ++i )
			{
				pPointVertices->posit = D3DXVECTOR3( &parts[i].pos.x );
				pPointVertices->color = mpColor;
				pPointVertices++;
			}

			m_pVertexBuffer->Unlock();
		}

		g_pd3dDevice->SetTexture( 0, g_pTexture_Particle );
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );    // Turn on point sprites
		g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );    // Allow sprites to be scaled with distance
		g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE,     FtoDW(mpSize) );  // Float value that specifies the size to use for point size computation in cases where point size is not specified for each vertex.
		g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(1.0f) ); // Float value that specifies the minimum size of point primitives. Point primitives are clamped to this size during rendering. 
		g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.0f) ); // Default 1.0
		g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.0f) ); // Default 0.0
		g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(1.f) ); // Default 0.0

		//
		// Render point sprites...
		//
		g_pd3dDevice->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof(Vertex) );
		g_pd3dDevice->SetFVF( Vertex::FVF_Flags );
		g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, mMaxParticles );
		//
		// Reset render states...
		//
		g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
		g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );

		g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}

};
