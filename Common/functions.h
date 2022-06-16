
int rayIntersectsTriangle(float *p, float *d,
			float *v0, float *v1, float *v2) {  //v0,v1,v2 are tri vertics , p is point in line, d is dir http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/

	#define vector(a,b,c) \
	(a)[0] = (b)[0] - (c)[0];	\
	(a)[1] = (b)[1] - (c)[1];	\
	(a)[2] = (b)[2] - (c)[2];

	#define crossProduct(a,b,c) \
		(a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
		(a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
		(a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];

	#define innerProduct(v,q) \
			((v)[0] * (q)[0] + \
			(v)[1] * (q)[1] + \
			(v)[2] * (q)[2])

	float e1[3],e2[3],h[3],s[3],q[3];
	float a,f,u,v;
	vector(e1,v1,v0);
	vector(e2,v2,v0);

	crossProduct(h,d,e2);
	a = innerProduct(e1,h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1/a;
	vector(s,p,v0);
	u = f * (innerProduct(s,h));

	if (u < 0.0 || u > 1.0)
		return(false);

	crossProduct(q,s,e1);
	v = f * innerProduct(d,q);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	// at this stage we can compute t to find out where
	// the intersection point is on the line

	/*Vector3 ve( e2[0],e2[1],e2[2] ); ve.normalize();
	Vector3 vq( q[0],q[1],q[2] ); vq.normalize();*/

	float t = f * /*ve.dot(vq);*/innerProduct(e2,q);

	if (t > 0.00001 ) // ray intersection
	{
		return(true);
	}
	else
	{
		//printf("%f , ",(float)t );
		// this means that there is a line intersection
		 // but not a ray intersection
		 return (false);
	}
}



#if defined ___OPENGL
AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}
	return NULL;										// If Load Failed Return NULL
}

int LoadGLTexture(GLuint *texture,char *filename)									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[1];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0]=LoadBMP(filename))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}
	return Status;										// Return The Status
}

bool AttatchSubTexture( GLuint &g_textureID,char *filename,int xoffset,int yoffset )
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( filename );

    if( pTextureImage != NULL )
	{
		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, pTextureImage->sizeX,
			pTextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
	}
	else return false;

	if( pTextureImage )
	{
		if( pTextureImage->data )
			free( pTextureImage->data );

		free( pTextureImage );
	}
	return true;
}

static void drawCube(float size)
{
    glPushMatrix();
        glScalef(size, size, size);
        glBegin(GL_QUADS);															// Start Drawing Quads
        // Front Face
        glNormal3f(0.0f, 0.0f, 1.0f);											// Normal Pointing Towards Viewer
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);				// Point 1 (Front)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);				// Point 2 (Front)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);				// Point 3 (Front)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);				// Point 4 (Front)
        // Back Face
        glNormal3f(0.0f, 0.0f, -1.0f);											// Normal Pointing Away From Viewer
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);				// Point 1 (Back)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);				// Point 2 (Back)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);				// Point 3 (Back)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);				// Point 4 (Back)
        // Top Face
        glNormal3f(0.0f, 1.0f, 0.0f);											// Normal Pointing Up
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);				// Point 1 (Top)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);				// Point 2 (Top)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);				// Point 3 (Top)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);				// Point 4 (Top)
        // Bottom Face
        glNormal3f(0.0f, -1.0f, 0.0f);											// Normal Pointing Down
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);				// Point 1 (Bottom)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);				// Point 2 (Bottom)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);				// Point 3 (Bottom)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);				// Point 4 (Bottom)
        // Right face
        glNormal3f(1.0f, 0.0f, 0.0f);											// Normal Pointing Right
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);				// Point 1 (Right)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);				// Point 2 (Right)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);				// Point 3 (Right)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);				// Point 4 (Right)
        // Left Face
        glNormal3f(-1.0f, 0.0f, 0.0f);											// Normal Pointing Left
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);				// Point 1 (Left)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);				// Point 2 (Left)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);				// Point 3 (Left)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);				// Point 4 (Left)
        glEnd();
    glPopMatrix();
}

Vector3 GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( (int)x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    return Vector3(posX, posY, posZ);
}

void RenderSkybox(Vector3 position,Vector3 size, GLTexture *SkyBox)
{	
	// djoubert187 _at_ hotmail.com
	// Begin DrawSkybox
	glColor4f(0.5, 0.5, 0.5,0.5f);

	glDisable(GL_DEPTH_TEST);
	// Save Current Matrix
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL_FACE );
 
	// Second Move the render space to the correct position (Translate)
	
	// First apply scale matrix
	//float modelview[16];
	//glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
	//glTranslatef(modelview[3],modelview[7],modelview[11]);

	glTranslatef(position(0),position(1),position(2));

	glScalef(size(0),size(1),size(2));
	glRotatef(-90,1,0,0);
	float cz = -0.0f,cx = 1.0f;
	float r = 1.005f; // If you have border issues change this to 1.005f
	// Common Axis Z - FRONT Side

	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	SkyBox[4].Use();
	glBegin(GL_QUADS);	
		glTexCoord2f(cx, cz); glVertex3f(-r ,1.0f,-r);
		glTexCoord2f(cx,  cx); glVertex3f(-r,1.0f,r);
		glTexCoord2f(cz,  cx); glVertex3f( r,1.0f,r); 
		glTexCoord2f(cz, cz); glVertex3f( r ,1.0f,-r);
	glEnd();
 
	// Common Axis Z - BACK side
	SkyBox[5].Use();
	glBegin(GL_QUADS);		
		glTexCoord2f(cx,cz);  glVertex3f(-r,-1.0f,-r);
		glTexCoord2f(cx,cx);  glVertex3f(-r,-1.0f, r);
		glTexCoord2f(cz,cx);  glVertex3f( r,-1.0f, r); 
		glTexCoord2f(cz,cz);  glVertex3f( r,-1.0f,-r);
	glEnd();
 
	// Common Axis X - Left side
	SkyBox[3].Use();
	glBegin(GL_QUADS);		
		glTexCoord2f(cx,cx); glVertex3f(-1.0f, -r, r);	
		glTexCoord2f(cz,cx); glVertex3f(-1.0f,  r, r); 
		glTexCoord2f(cz,cz); glVertex3f(-1.0f,  r,-r);
		glTexCoord2f(cx,cz); glVertex3f(-1.0f, -r,-r);		
	glEnd();
 
	// Common Axis X - Right side
	SkyBox[2].Use();
	glBegin(GL_QUADS);		
		glTexCoord2f( cx,cx); glVertex3f(1.0f, -r, r);	
		glTexCoord2f(cz, cx); glVertex3f(1.0f,  r, r); 
		glTexCoord2f(cz, cz); glVertex3f(1.0f,  r,-r);
		glTexCoord2f(cx, cz); glVertex3f(1.0f, -r,-r);
	glEnd();
 
	// Common Axis Y - Draw Up side
	SkyBox[0].Use();
	glBegin(GL_QUADS);		
		glTexCoord2f(cz, cz); glVertex3f( r, -r,1.0f);
		glTexCoord2f(cx, cz); glVertex3f( r,  r,1.0f); 
		glTexCoord2f(cx, cx); glVertex3f(-r,  r,1.0f);
		glTexCoord2f(cz, cx); glVertex3f(-r, -r,1.0f);
	glEnd();
 
	// Common Axis Y - Down side
	SkyBox[1].Use();
	glBegin(GL_QUADS);		
		glTexCoord2f(cz,cz);  glVertex3f( r, -r,-1.0f);
		glTexCoord2f( cx,cz); glVertex3f( r,  r,-1.0f); 
		glTexCoord2f( cx,cx); glVertex3f(-r,  r,-1.0f);
		glTexCoord2f(cz, cx); glVertex3f(-r, -r,-1.0f);
	glEnd();
 
	// Load Saved Matrix
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
 
};

void SDL_GL_RenderText(char *text, 
                      TTF_Font *font,
					  Vector3 color,
                      SDL_Rect *location)
{
	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect rect;
	int w,h;
	GLuint texture;
	
	/* Use SDL_TTF to render our text */
	SDL_Color colorX = {(int)color(0),(int)color(1),(int)color(2)} ;
	initial = TTF_RenderText_Blended(font, text, colorX);
	
	/* Convert the rendered text to a known format */
	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);
	
	/* Tell GL about our new texture */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, intermediary->pixels );
	
	/* GL_NEAREST looks horrible, if scaled... */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	/* prepare to render our texture */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1.0f, 1.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_COLOR);

	
	glEnable2D();

	/* Draw a quad at location */
	glBegin(GL_QUADS);
		/* Recall that the origin is in the lower-left corner
		   That is why the TexCoords specify different corners
		   than the Vertex coors seem to. */
		glTexCoord2f(0.0f, 1.0f); 
			glVertex2f(location->x    , location->y);
		glTexCoord2f(1.0f, 1.0f); 
			glVertex2f(location->x + w, location->y);
		glTexCoord2f(1.0f, 0.0f); 
			glVertex2f(location->x + w, location->y + h);
		glTexCoord2f(0.0f, 0.0f); 
			glVertex2f(location->x    , location->y + h);
	glEnd();

	glDisable2D();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	/* Bad things happen if we delete the texture before it finishes */
	glFinish();
	
	/* return the deltas in the unused w,h part of the rect */
	location->w = initial->w;
	location->h = initial->h;
	
	/* Clean up */
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
	glDeleteTextures(1, &texture);
}


int CalculateFPS()
{
	static int iCurrentTick = 0,  iFps = 0, iFrames = 0;
	static int iStartTick = GetTickCount();

	iFrames++;
    iCurrentTick = GetTickCount();
	if ((iCurrentTick - iStartTick) >= 1000) 
	{
		iFps = (int)((float)iFrames/(iCurrentTick-iStartTick)*1000.0f);
		iFrames = 0;
		iStartTick = iCurrentTick;
	}
	return iFps;
}

void glutPrint(float x, float y, LPVOID font, char* text, float r, float g, float b, float a,float z=-6.0f)
{
	
}  

void glEnable2D()
{
	int vPort[4];
  
	glGetIntegerv(GL_VIEWPORT, vPort);
  
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
  
	glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void glDisable2D()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
}


int RenderTextToSurface(std::string Text, int x, int y, SDL_Surface *Dest)
{	
	SDL_Color TXT_Color;	
	TXT_Color.r = 0xFF;	
	TXT_Color.g = 0xFF;	
	TXT_Color.b = 0xFF;	
	SDL_Surface *TTF_Message;	
	TTF_Font *font = TTF_OpenFont("font.ttf", 24);	
	if(!(TTF_Message = TTF_RenderText_Solid(font, Text.c_str(), TXT_Color)))	
	{		
		SDL_FreeSurface(TTF_Message);		
		printf("Error in function 'RenderTextToSurface': TTF_Message could not be blitted: returned 1");		
		return 1;	
	}	
	//SDL_BlitSurface(x, y, TTF_Message, Dest);	
	SDL_FreeSurface(TTF_Message);	return 0;
}

void drawText(SDL_Surface *Dest,const std::string font_type, const std::string text,int size,int x, int y,int fR, int fG, int fB,int bR, int bG, int bB)
{   
	TTF_Font* font = TTF_OpenFont(font_type.c_str(), font_type.size());
	SDL_Color foregroundColor = { fR, fG, fB };    
	SDL_Color backgroundColor = { bR, bG, bB };   
	SDL_Surface* textSurface = TTF_RenderText_Shaded(font, text.c_str(),foregroundColor, backgroundColor);   
	SDL_Rect textLocation = { x, y, 100, 100 };   
	SDL_BlitSurface(textSurface, NULL, Dest , &textLocation);   
	SDL_FreeSurface(textSurface);   
	TTF_CloseFont(font);
}

int round(double x)
{
	return (int)(x + 0.5);
}

int nextpoweroftwo(int x)
{
	double logbase2 = log((double)x) / log((double)2);
	return round(pow(2,ceil(logbase2)));
}


void PosAndRotManually(bool *gKeys)
{
		static float yRot=0,xRot=0,zRot=0,yPos=0,xPos=0,zPos=0;
		if( gKeys['i'] ) xRot+=.1;
		if( gKeys['k'] ) xRot-=.1;
		if( gKeys['j'] ) yRot+=.1;
		if( gKeys['l'] ) yRot-=.1;
		if( gKeys['n'] ) zRot+=.1;
		if( gKeys['m'] ) zRot-=.1;

		if( gKeys['1'] ) zPos+=.1;
		if( gKeys['2'] ) zPos-=.1;
		if( gKeys['4'] ) yPos+=.1;
		if( gKeys['5'] ) yPos-=.1;
		if( gKeys['7'] ) xPos+=.1;
		if( gKeys['8'] ) xPos-=.1;

		glTranslatef(xPos,yPos,zPos);
		glRotatef(yRot,0,1,0);
		glRotatef(xRot,1,0,0);
		glRotatef(zRot,0,0,1);
		printf("%f %f %f %f %f %f\n",xRot,yRot,zRot,xPos,yPos,zPos);
}

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

#endif