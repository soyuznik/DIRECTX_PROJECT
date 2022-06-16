
#include "Octree.h"

// This is just use for debugging, so we can render a node instead of rendering them by groups
//#define FILL_NODE_INDEXBUFFER

#define ESTIMATE_NUM_NODES 100.f
#define MAX_NODE_FACES 50
#define MAX_NODE_SIZE .5f


#define LOG_DEBUG(m)  true;
#define LOG_INFO(m)   ;
#define LOG_WARN(m)   ;
#define LOG_ERROR(m)  ;
#define LOG_FATAL(m)  ;

Octree::Octree()
{
	mVertices = NULL;
	mVertexBuffer = NULL;
	mFaces    = NULL;   
	mGroups   = NULL;
	mRootNode = NULL;

	mMaxFaces			= MAX_NODE_FACES;
	mMaxHalfSize		= MAX_NODE_SIZE;
	mNumNodes			= 0;
	mFrameCount			= 0;	
}

Octree::~Octree(void)
{

}

HRESULT Octree::Create( LPDIRECT3DDEVICE9 device, Model_X *mesh )
{
	mDevice = device;
	mMeshClass = mesh;	

	LPD3DXMESH newMesh = mMeshClass->GetMesh();

	DWORD totalVerts = newMesh->GetNumVertices();
	DWORD totalFaces = newMesh->GetNumFaces();

	mFaces		= new Face[ totalFaces ];		
	mNumGroups	= mMeshClass->GetNumMaterials();
	mGroups		= new Group[ mNumGroups ];

	// Copy over all the faces and groups from the mesh			
	unsigned short *indices		= NULL;
	unsigned long  *attributes	= NULL;

	newMesh->LockIndexBuffer( D3DLOCK_READONLY, (void**)&indices );	
	newMesh->LockAttributeBuffer(D3DLOCK_READONLY, &attributes );

	for(DWORD i = 0; i < totalFaces; i++)
	{
		mFaces[i].v1 = *indices++; 
		mFaces[i].v2 = *indices++; 
		mFaces[i].v3 = *indices++;
		mFaces[i].group = attributes[i]; // Store the group (attribute) index - this is where the grouping happens

		mGroups[attributes[i]].numFaces++; // Keep count of the number of faces		
	}

	newMesh->UnlockIndexBuffer();
	newMesh->UnlockAttributeBuffer();

	// Initialize the groups index buffer
	for(int i = 0; i < mNumGroups; i++)
	{
		unsigned int size =  mGroups[i].numFaces * 3 * sizeof(unsigned short);
		mDevice->CreateIndexBuffer( size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mGroups[i].indexBuffer, NULL );	
	}

	// Store all the mesh vertices on a local vertex buffer. This would be the stream source. 
	// The groups index buffer are use to index the vertices in the VB	
	DWORD bufferSize = totalVerts * VERTEX_FVF_SIZE ;
	mDevice->CreateVertexBuffer( bufferSize, D3DUSAGE_WRITEONLY, VERTEX_FVF, D3DPOOL_MANAGED, &mVertexBuffer, NULL );	
	Vertex* pVertices;
	newMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**) &pVertices );
	mVertexBuffer->Lock( 0, 0, (void**)&mVertices, 0 );
	memcpy( mVertices, pVertices, bufferSize );
	mVertexBuffer->Unlock();
	newMesh->UnlockVertexBuffer();		

	// Calculate the halfsize of a node
	float halfSize = (float)max( fabs( mMeshClass->GetBoundingBox()->max.x ), max( fabs( mMeshClass->GetBoundingBox()->max.y ), fabs( mMeshClass->GetBoundingBox()->max.z ) ) );	
	halfSize = (float)max( halfSize, max( fabs(  mMeshClass->GetBoundingBox()->min.x ), max( fabs( mMeshClass->GetBoundingBox()->min.y ), fabs( mMeshClass->GetBoundingBox()->min.z ) ) ) );

	LOG_DEBUG( "Initializing octree" );

	// We're now ready to build the octree, start by creating and adding a root node
	mRootNode = new Node();	

	// Recusively add nodes to the octree
	AddNode( mRootNode, mMeshClass->GetCenter(), halfSize );	

	LOG_DEBUG( "Octree initialized. Total nodes: " << mNumNodes << " Max faces per node: " << mMaxFaces << " Max half size per node: " << mMaxHalfSize );	

	return S_OK;
}

void Octree::Lost()
{
	
}

void Octree::Reset()
{
	
}

void Octree::Destroy()
{
	mNumNodes = 0;
	SAFE_DELETE( mRootNode );
	SAFE_DELETE_ARRAY( mGroups );	
	SAFE_DELETE_ARRAY( mFaces );
	SAFE_RELEASE( mVertexBuffer );
}

void Octree::AddNode( Node *node, D3DXVECTOR3 position, float size )
{
	// Set this node's bounding box extent
	node->min.x = position.x - size;
	node->min.y = position.y - size;
	node->min.z = position.z - size;

	node->max.x = position.x + size;	
	node->max.y = position.y + size;	
	node->max.z = position.z + size;	

	// And its bounding sphere
	node->center = position; 
	node->radius = (float)sqrt( size * size + size * size + size * size );	

	DWORD totalFaces = mMeshClass->GetMesh()->GetNumFaces();
	DWORD curTotalFaces = 0;

	// Step 1: Count the total number of faces in this current node
	for(DWORD i = 0; i < totalFaces; i++)
	{
		if( IsFaceInBox( &mVertices[ mFaces[i].v1 ], &mVertices[ mFaces[i].v2 ], &mVertices[ mFaces[i].v3 ], node->min, node->max ) )
		{
			curTotalFaces++;
		}
	}

	// Step 2: Check if the size of the current box and the number of faces (triangles) exceeds the threshold
	LOG_DEBUG( "Checking for size: " << size << "/" << mMaxHalfSize << " and total faces: " << curTotalFaces << "/" << mMaxFaces);

	if( size > mMaxHalfSize && curTotalFaces > mMaxFaces )
	{
		LOG_DEBUG( "Size and total faces exceeded, need to split the node into 8 pieces." );

		for( int c = 0; c < 8; c++ )
		{
			D3DXVECTOR3 newPos, newMin, newMax;
			float newHalfSize = size / 2.0f;
			float mod;

			// Calculate the translation of the new node on the x axis.
			mod = 1.0f;
			if( c % 2 < 1 )
				mod = -1.0f;
			newPos.x = position.x + newHalfSize * mod;

			// Calculate the translation of the new node on the y axis.
			mod = 1.0f;
			if( c % 4 < 2 )
				mod = -1.0f;
			newPos.y = position.y + newHalfSize * mod;

			// Calculate the translation of the new node on the z axis.
			mod = 1.0f;
			if( c % 8 < 4 )
				mod = -1.0f;
			newPos.z = position.z + newHalfSize * mod;

			// Calculate the bounding box around the new node.
			newMin = D3DXVECTOR3( newPos.x - newHalfSize, newPos.y - newHalfSize, newPos.z - newHalfSize );
			newMax = D3DXVECTOR3( newPos.x + newHalfSize, newPos.y + newHalfSize, newPos.z + newHalfSize );

			// Check if the new node will have at least one face in it before creating the node
			for(DWORD f = 0; f < totalFaces; f++)
			{				
				if( IsFaceInBox( &mVertices[ mFaces[f].v1 ], &mVertices[ mFaces[f].v2 ], &mVertices[ mFaces[f].v3 ], newMin, newMax ) )
				{					
					LOG_DEBUG( "Creating Node: " << c << " Box extent: " << newPos.x << "," << newPos.y << "," << newPos.z << "," << " size: " << size / 2.0f );

					node->node[c] = new Node();	

					// A face has been found, create a new node to hold the face(s) for this node
					AddNode( node->node[c], newPos, size / 2.0f );

					break;
				}
			}
		}

		LOG_DEBUG( "Done splitting this current node. " );
		return;
	}	

	LOG_DEBUG( "No need to split node." );

	// At this point, it means that the process has finished splitting the nodes apart and its time to 
	// store the faces that makes up this node

	node->numFaces = curTotalFaces;
	node->faces = new DWORD[curTotalFaces];

	// Final step: Add the visible faces for this node.
	curTotalFaces = 0;
	for(DWORD f = 0; f < totalFaces; f++ )
	{
		if( IsFaceInBox( &mVertices[ mFaces[f].v1 ], &mVertices[ mFaces[f].v2 ], &mVertices[ mFaces[f].v3 ], node->min, node->max ) )
		{
			node->faces[curTotalFaces++] = f;
		}
	}

	node->index = mNumNodes++;	// Current node index, using the total number of nodes

	LOG_DEBUG( "Node created. Index: " << node->index << " Total faces in node: " << curTotalFaces );
}

bool Octree::IsFaceInBox( Vertex *v1, Vertex *v2, Vertex *v3, D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	// Find the minimum and maximum points of the face along the x axis. Then
	// check if these two points are within the box's x axis extents.
	float minX = min( v1->position.x, min( v2->position.x, v3->position.x ) );
	float maxX = max( v1->position.x, max( v2->position.x, v3->position.x ) );
	if( maxX < min.x )
		return false;
	if( minX > max.x )
		return false;

	// Find the minimum and maximum points of the face along the y axis. Then
	// check if these two points are within the box's y axis extents.
	float minY = min( v1->position.y, min( v2->position.y, v3->position.y ) );
	float maxY = max( v1->position.y, max( v2->position.y, v3->position.y ) );
	if( maxY < min.y )
		return false;
	if( minY > max.y )
		return false;

	// Find the minimum and maximum points of the face along the z axis. Then
	// check if these two points are within the box's z axis extents.
	float minZ = min( v1->position.z, min( v2->position.z, v3->position.z ) );
	float maxZ = max( v1->position.z, max( v2->position.z, v3->position.z ) );
	if( maxZ < min.z )
		return false;
	if( minZ > max.z )
		return false;

	return true;
}

void Octree::PrepareNodes( /*Camera *camera,*/ Node *node )
{
	short num;	
	
	/*if( ! camera->FrustumBoxCheck(node->min, node->max) )
	{
		return;
	}*/
	num = 0;	
	for( int i = 0; i < 8; i++ )
	{
		if(node->node[i] != NULL)
		{
			num++;
			PrepareNodes( /*camera,*/ node->node[i] );
		}
	}

	if( num )
		return;
	
	mNumNodesRendered++;
	for(DWORD i = 0; i < node->numFaces; i++)
	{
		Face *pFace = &mFaces[node->faces[i]];

		if( pFace->timer != mFrameCount )
		{
			pFace->timer = mFrameCount;

			// Extract which group (attribute) this face belongs to
			unsigned long group = pFace->group;

			if( (int)group < mNumGroups)
			{
				*mGroups[group].indices++ = mFaces[ node->faces[i] ].v1;
				*mGroups[group].indices++ = mFaces[ node->faces[i] ].v2;
				*mGroups[group].indices++ = mFaces[ node->faces[i] ].v3;				

				mGroups[group].numFacesToRender++;
				mNumFacesRendered++;
			}
		}
	}
}

Octree::Node * Octree::GetNode( int index, Node *node )
{	
	if( node && node->index == index )
		return node;

	for( int i = 0; i < 8; i++ )
	{
		if( node->node[i] )
		{
			Node *pTempNode = GetNode( index, node->node[i] );
			if( pTempNode && pTempNode->index == index )
				return pTempNode;
		}
	}

	return NULL;
}

void Octree::Render( /*Camera *camera, Camera *camera2,*/ bool showOctree )
{	
	mFrameCount++;

	if( showOctree )
	{	
		mDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		mDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		mDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		struct NodeVertex
		{
			float x, y, z;  // The transformed(screen space) position for the vertex.
			DWORD colour;   // The vertex colour.
		};

		// Draw the entire Octree
		for(int nodeIndex = 0; nodeIndex < mNumNodes; nodeIndex++ )
		{
			Node *node = GetNode(nodeIndex, mRootNode );
			if( node->numFaces )
			{
				// draw the octree
				NodeVertex nodeVertices[] = 
				{
					// Front face
					{ node->min.x,node->min.y,node->min.z,0xFF0000FF}, {node->min.x, node->max.y,node->min.z,0xFF0000FF},{node->max.x, node->max.y,node->min.z,0xFF0000FF},
					{ node->max.x, node->max.y,node->min.z,0xFF0000FF},{ node->max.x,node->min.y,node->min.z,0xFF0000FF},{node->min.x,node->min.y,node->min.z,0xFF0000FF},
					// Back face
					{ node->max.x,node->min.y, node->max.z,0xFF0000FF},{ node->max.x, node->max.y, node->max.z,0xFF0000FF},{node->min.x,node->max.y, node->max.z,0xFF0000FF},
					{ node->min.x, node->max.y, node->max.z,0xFF0000FF},{node->min.x,node->min.y, node->max.z,0xFF0000FF},{ node->max.x,node->min.y, node->max.z,0xFF0000FF},
					// Top face
					{ node->min.x, node->max.y,node->min.z,0xFFFF0000},{node->min.x, node->max.y, node->max.z,0xFFFF0000},{ node->max.x, node->max.y, node->max.z,0xFFFF0000},
					{ node->max.x, node->max.y, node->max.z,0xFFFF0000},{ node->max.x, node->max.y,node->min.z,0xFFFF0000},{node->min.x, node->max.y,node->min.z,0xFFFF0000},
					// Bottom face
					{ node->max.x,node->min.y,node->min.z,0xFFFF0000},{ node->max.x,node->min.y, node->max.z,0xFFFF0000},{node->min.x,node->min.y, node->max.z,0xFFFF0000},
					{ node->min.x,node->min.y, node->max.z,0xFFFF0000},{node->min.x,node->min.y,node->min.z,0xFFFF0000},{ node->max.x,node->min.y,node->min.z,0xFFFF0000},
					// Left face
					{ node->min.x,node->min.y, node->max.z,0xFF00FF00},{node->min.x, node->max.y, node->max.z,0xFF00FF00},{node->min.x, node->max.y,node->min.z,0xFF00FF00},
					{ node->min.x, node->max.y,node->min.z,0xFF00FF00},{node->min.x,node->min.y,node->min.z,0xFF00FF00},{node->min.x,node->min.y, node->max.z,0xFF00FF00},
					// Right face
					{ node->max.x,node->min.y,node->min.z,0xFF00FF00},{ node->max.x, node->max.y,node->min.z,0xFF00FF00},{ node->max.x, node->max.y, node->max.z,0xFF00FF00},
					{ node->max.x, node->max.y, node->max.z,0xFF00FF00},{ node->max.x,node->min.y, node->max.z,0xFF00FF00},{ node->max.x,node->min.y,node->min.z,0xFF00FF00},
				};

				mDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 12, (LPVOID) &nodeVertices, sizeof(NodeVertex)  );
			}

		}

	} // if showOctree

	mDevice->SetRenderState( D3DRS_CULLMODE,	D3DCULL_CCW );
	mDevice->SetRenderState( D3DRS_FILLMODE,	D3DFILL_SOLID );	
}