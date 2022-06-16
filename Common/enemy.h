
class Enemy : public PlayerObject
{
public:
	CXFileEntity *		Ogro;
	Model_X*			Weapon;
	float		    died_time;
	Vector3		    scale;
	Gun *gun;

	int mAnimationState;

	float lastTime;

	
	Enemy(char *enem_model_path,char *enem_skin_path,char *weapon_model_path,char *weapon_skin_path)
	{
		//Ogro = g_resourceManager.AddAnimatedModel_X(  enem_model_path , "" );
		Ogro = new CXFileEntity( ::g_pd3dDevice );
		if(!Ogro->Load( enem_model_path ))
		{
			MessageBox(0,enem_model_path,"Error",0);
			exit(0);
		}

		Weapon = new Model_X(::g_pd3dDevice );
		if ( !Weapon->LoadXFile( weapon_model_path ) )
		{
			MessageBox(0,weapon_model_path,"Error",0);
			exit(0);
		}

		//Ogro.LoadModel( enem_model_path );
		//Ogro.LoadSkin( enem_skin_path );

		mAnimationState = 0;
		
		/*Weapon.LoadModel( weapon_model_path );
		Weapon.LoadSkin( weapon_skin_path );
		
		Weapon.SetAnim( STAND );*/

		died_time = 0;
		this->speedOffset = .05;

		this->pos.y = 1;

		gun = new LazerGun(.1);

		lastTime = timeGetTime();
	}

	~Enemy()
	{
		delete Ogro;
		delete Weapon;
		delete gun;
	}


	void SetAnim(int i)
	{
		mAnimationState = i ;
		//Weapon.SetAnim(i);
	}

	void Scale(Vector3 scale)
	{
		this->scale = scale; 
		//Ogro.ScaleModel( i );
		//Weapon.ScaleModel( i );
	}

	void Draw(float time)
	{
		Ogro->SetAnimationSet( mAnimationState );

		if( died_time > 0 )
			SetAnim( 5 /*animType_t::DEATH_FALLBACKSLOW*/ );

		gun->pos = this->pos + ( this->dir * .1 ) +  (this->dir.cross(Vector3(0,1,0)) * -.2);
		gun->pos.y += .25;
		//gun->dir = this->dir;
		//gun->SetPosAndDir( this->pos + ( this->dir * .1 ) , this->dir);

		stack->Push();
		
			D3DXMATRIX mX,mT,mS,mR;
			::D3DXMatrixScaling( &mS ,scale(0),scale(1),scale(2) );
			::D3DXMatrixTranslation( &mT ,pos.x,pos.y,pos.z );
			::D3DXMatrixRotationAxis(&mR , &D3DXVECTOR3( 0,1,0 ) , this->dir.negative().getAngleAlongY()*PI/180 ) ;
			mX =  mR * mS * mT  ;
			stack->MultMatrix(&mX);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());

			float timeElapsed=0.001f*(timeGetTime()-lastTime);
			lastTime=timeGetTime();
			Ogro->FrameMove(timeElapsed,stack->GetTop());
			Ogro->Render();

		stack->Pop();g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());


		stack->Push();
		{
			D3DXMATRIX mat = mX;
			static int bone=0,angle=0;
			static D3DXVECTOR3 tra(0,0,0);
			{
				D3DXMATRIX mat=Ogro->m_boneMatrices[1];
				D3DXVECTOR3 v;D3DXVec3TransformCoord( &v , &(D3DXVECTOR3( (tra.x -.02), (tra.y+.53),( tra.z-.68))/.015 ), &mat );
				mat._41=0;mat._42=0;mat._43=0;
				stack->RotateAxis(&D3DXVECTOR3(0,1,0),D3DXToRadian(60+angle));
				stack->MultMatrix(&mat);
				stack->Translate(v.x,v.y,v.z);
			}
			g_pd3dDevice->SetTransform( D3DTS_WORLD, stack->GetTop() );
			Weapon->Draw();
		}stack->Pop();g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());

	}

	void Run()
	{
		if( died_time > 0 )return;

		pos = pos+ (dir* speedOffset );
		SetAnim(  1 /*animType_t::RUN */);
	}

	void StepDieTimer()
	{
		died_time+=.025;
	}
};
