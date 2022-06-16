class PlayerObject
{
public:
	Vector3 pos,dir,last_pos,lhw;
	float speedOffset;
	float health ;

	PlayerObject()
	{
		 health=100;
	}

	void checkCollosionWithTriangle(Triangle tri,float radius,int i=0)
	{
		float distance,max;
		//if( ClassifySphere(this->pos, tri.getNormal() , tri.vertix[0], radius,OUT distance) == INTERSECTS )
		if( ClassifyCuboid(this->pos, tri.getNormal() , tri.vertix[0], this->lhw ,OUT distance,OUT max) == INTERSECTS )
		{
			Vector3 vIntersection = this->pos - tri.getNormal()*distance;
			if( tri.IfPointInTriangle( vIntersection )  || EdgeSphereCollision(this->pos, &tri.vertix[0], 3 , radius/2 ))
			{
				Vector3 cOffset = GetCollisionOffset( tri.getNormal(), max , distance );
				this->pos = this->pos + cOffset;
			}
		}
	}
	void checkCollosionWithMap(Triangle *worldMap,int mapSize , float sphereSize=.5)
	{
		for( int i=0; i <mapSize;i++)
		{
			checkCollosionWithTriangle(worldMap[i],sphereSize,i);
		}
	}

	void checkCollosionWithMap(Octree map_octree)
	{
		Octree::Node *node = map_octree.getNodefromPos( this->pos , map_octree.mRootNode );
		for(int i=0;i< node->numFaces;i++)
		{
			Vector3 v1 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v1].position) );
			Vector3 v2 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v2].position) );
			Vector3 v3 = Vector3(  D3DVecToVector3(map_octree.mVertices[map_octree.mFaces[node->faces[i]].v3].position) );

			this->checkCollosionWithTriangle( Triangle( v1, v2, v3 ) , this->lhw.x );
		}
	}

	void ApplyGravity()
	{
		this->pos.y -= .98f * .1;  
	}

	Polygen *getSurroundedPolygen( Vector3 normal)
	{
		Vector3 lDir =  normal.cross(Vector3(0,-1,0).normalize() * this->lhw(0) ) ;
		Vector3 uDir =  normal.cross(Vector3(1,0,0)).normalize() * this->lhw(1) ;
		//Vector3 uDir =  Vector3(0,1,0) * this->lhw(1) ;
		uDir.printVal();
		Vector3 TL = this->pos - lDir - uDir;
		Vector3 TR = this->pos + lDir - uDir;
		Vector3 BL = this->pos - lDir + uDir;
		Vector3 BR = this->pos + lDir + uDir;

		return new Rect(TL,TR,BR,BL);
	}

	void DrawOuterCircle()
	{
		Sphere s( pos,.5 );
		s.Draw(0);
	}

	void DrawOuterCuboid()
	{
		vector<Vector3> verts;
		verts.push_back(Vector3( pos.x - lhw.x ,pos.y + lhw.y ,pos.z + lhw.z ));  //0
		verts.push_back(Vector3( pos.x + lhw.x ,pos.y + lhw.y ,pos.z + lhw.z ));  //1
		verts.push_back(Vector3( pos.x + lhw.x ,pos.y - lhw.y ,pos.z + lhw.z ));  //2
		verts.push_back(Vector3( pos.x - lhw.x ,pos.y - lhw.y ,pos.z + lhw.z ));  //3

		verts.push_back(Vector3( pos.x - lhw.x ,pos.y + lhw.y ,pos.z - lhw.z ));  //
		verts.push_back(Vector3( pos.x + lhw.x ,pos.y + lhw.y ,pos.z - lhw.z ));
		verts.push_back(Vector3( pos.x + lhw.x ,pos.y - lhw.y ,pos.z - lhw.z ));
		verts.push_back(Vector3( pos.x - lhw.x ,pos.y - lhw.y ,pos.z - lhw.z ));

		float vertices[24];
		for(int i=0;i<8;i++)
		{
			vertices[i*3 + 0] = verts[i](0);
			vertices[i*3 + 1] = verts[i](1);
			vertices[i*3 + 2] = verts[i](2);
		}

		GLubyte indices[] = {0,1,2,3,   // 24 of indices
							 0,3,7,4,
							 4,7,6,5,
							 1,2,6,5,
							 3,2,6,7,
							 0,1,5,4};

		#if defined ___OPENGL
			// activate and specify pointer to vertex array
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, vertices);

			// draw a cube
			glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);

			// deactivate vertex arrays after drawing
			glDisableClientState(GL_VERTEX_ARRAY);
		#endif
	}
};

class Player : public PlayerObject
{
public:

	float angle;
	float deltaAngle;
	float deltaMove;

	Gun *gun;
	Model_X *gunModel;

	Player()
	{
		this->speedOffset = .1;
		this->deltaAngle = 0.f;
		this->deltaMove = 0.f;
		this->angle = 0.0f;
		this->pos.y = 1;
		gun = new Gun(2.5f);
		gunModel = NULL;
	}
	Player(Vector3 pos,Vector3 dir)
	{
		Player();
		this->pos = pos;
		this->dir = dir;
	}

	void PressedKeysHandler(bool *gKeys,bool *gSpKeys)
	{
		this->last_pos = this->pos ;

		deltaAngle = .01;
		//Code for Left:36 Top:37 Right:38  Bottom :39

		if( gKeys[ 37 ] ) 
		{
			angle += deltaAngle;
			dir.x = sin(angle);
			dir.z = -cos(angle);
		}		
		else if( gKeys[ 39 ] ) 
		{
			angle -= deltaAngle;
			dir.x = sin(angle);
			dir.z = -cos(angle);
		}		

		if( gKeys[ 'D' ] )
		{
			Vector3 v = dir.cross( Vector3(0,1,0)  );
			pos.x -= (v.x* speedOffset);
			pos.z -= (v.z* speedOffset);
			//pos.y -= (v.y* speedOffset);
		}		
		else if( gKeys[ 'A' ] )
		{
			Vector3 v = dir.cross( Vector3(0,1,0)  );
			pos.x += (v.x* speedOffset);
			pos.z += (v.z* speedOffset);
			//pos.y += (v.y* speedOffset);
		}	

		if(  gKeys['W'])
		{
			pos.x += (dir.x* speedOffset);
			pos.z += (dir.z* speedOffset);
			//pos.y += (dir.y* speedOffset);
		}		
		else if(  gKeys['S'] )
		{
			pos.x -= (dir.x* speedOffset);
			pos.z -= (dir.z* speedOffset);
			//pos.y -= (dir.y* speedOffset);
		}		
	}

	void MouseKeyHandler(bool *gMouseButton)
	{
		const int GLUT_LEFT_BUTTON = 0;
		if( gMouseButton[GLUT_LEFT_BUTTON] )
		{
			gun->create();
		}
	}

	void Draw()
	{
			gunModel->Draw();
			//model->Draw();
	}

};