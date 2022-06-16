#pragma once


#include <vector>
#include <windows.h>

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windef.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>  
#include <string>
#include <Psapi.h>
#include "mmsystem.h"
#include "Model_x.h"
#include "vector3.h"
#include "mesh.h"
#include "vertex.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

class Octree
{
public:
	struct Face 
	{
		DWORD v1;
		DWORD v2;
		DWORD v3;	
		unsigned long timer;
		unsigned long group;
	};

	struct Node 
	{
		Node()
		{
			for( short i = 0; i < 8; i++ )
			{
				node[i] = NULL;
			}		
			faces		= NULL;
			numFaces	= 0;
			numVerts	= 0;	
		}
		~Node()
		{
			for( int i = 0; i < 8; i++ )
			{
				SAFE_DELETE( node[i] );
			}
			SAFE_DELETE_ARRAY( faces );
		}

		Node *node[8];			// Pointer to 8 nodes
		D3DXVECTOR3 min, max;	// Max and min extent of the bounding box
		D3DXVECTOR3 center;		// Center of a the bounding sphere
		float   radius;			// Radius of the bounding sphere
		DWORD	numFaces;		// Number of faces this node contains
		DWORD	numVerts;		// Number of vertices this node contains
		DWORD	*faces;			// Array to store faces for this node
		int		index;			// Current node index number, use for retrieving a node
	};	

	struct Group
	{
		Group()
		{
			indexBuffer			= NULL;
			indices				= NULL;
			numFaces			= 0;
			numFacesToRender	= 0;
		}

		~Group()
		{
			SAFE_RELEASE(indexBuffer);			
		}

		LPDIRECT3DINDEXBUFFER9 indexBuffer;	// Holds all the indices for a particular group that would be visible in a frame render
		WORD	*indices;			// Variable use to fill in the index buffer
		DWORD	numFaces;			// Number of faces for this group
		DWORD   numFacesToRender;	// Number of faces to render this frame
	};


public:
	Octree();
	~Octree(void);
	
	// Loads a mesh into the octree
	HRESULT Create( LPDIRECT3DDEVICE9 device, Model_X *mesh );	
	void Destroy();
	void Lost();
	void Reset();

	// Recursively build the octree
	void AddNode( Node *node, D3DXVECTOR3 position, float size );	
	
	// Recursively check which nodes are currently visible and fill their indices with the visible faces
	void PrepareNodes(/* Camera *camera,*/ Node *node );

	// Render's the octree 
	void Render( /*Camera *camera, Camera *camera2,*/ bool showOctree );

	//bool IsFaceInBox( VertexPos *v1, VertexPos *v2, VertexPos *v3, D3DXVECTOR3 min, D3DXVECTOR3 max );
	bool IsFaceInBox( Vertex *v1, Vertex *v2, Vertex *v3, D3DXVECTOR3 min, D3DXVECTOR3 max );

	Node *GetNode( int index, Node *node );
	DWORD GetNumNodesRendered() { return mNumNodesRendered; }
	DWORD GetNumFacesRendered() { return mNumFacesRendered; }

	LPDIRECT3DDEVICE9	mDevice;
	Node				*mRootNode;	// Pointer to the root octree node
	Group				*mGroups;	// Array to holds all the texture, materials (attributes) and meshes together
	std::vector<Model_X *> mMeshes;
	Model_X				*mMeshClass;

	Vertex				*mVertices;		// Array to store all the scene mesh vertices
	Face				*mFaces;		// Array to store all the scene mesh faces	
	LPDIRECT3DVERTEXBUFFER9 mVertexBuffer;		// Vertex buffer to hold the entire mesh	

	// 2 variables use to decide whether to split the node or not
	DWORD mMaxFaces;		// Maximum number of faces per node.
	float mMaxHalfSize;		// Maximum half size of a node.

	int					mNumNodes;		// Total nodes
	int					mNumGroups;		// Total groups (attributes in mesh)
	unsigned long		mFrameCount;	// Accumulator to prevent redrawing of faces

	// Stats variable
	DWORD	mNumNodesRendered;
	DWORD	mNumFacesRendered;

	public:
	Node *getNodefromPos(Vector3 pos , Node *node )
	{
		for(int i = 0;i<8 ;i++)
		{
			if( node->node[i] != NULL )
			{
				if( node->node[i]->max.x < pos.x  || node->node[i]->min.x > pos.x || node->node[i]->max.y < pos.y ||node->node[i]->min.y > pos.y || node->node[i]->max.z < pos.z  ||  node->node[i]->min.z > pos.z );
				else
				{
					return getNodefromPos( pos , node->node[i] );
				}
			}
		}
		return node;
	}


};
