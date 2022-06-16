// Note: Originally created in 2008 but updated on 24/08/10


#include "XfileEntity.h"
#include "Utility.h"
#include "MeshHierarchy.h"

// The time to change from one animation set to another
// To see how the merging works - increase this time value to slow it down
const float kMoveTransitionTime=0.25f;

// Constructor
CXFileEntity::CXFileEntity(LPDIRECT3DDEVICE9 d3dDevice) : m_d3dDevice(d3dDevice),m_speedAdjust(1.0f), m_currentTrack(0),
	m_currentTime(0),m_numAnimationSets(0),m_currentAnimationSet(0),m_maxBones(0),m_sphereRadius(0),
	m_sphereCentre(0,0,0),m_boneMatrices(0)
{
}

// Destructor
CXFileEntity::~CXFileEntity(void)
{
	if (m_animController)
	{
		m_animController->Release();
		m_animController=0;
	}

	if (m_frameRoot)
	{
		// Create a mesh heirarchy class to control the removal of memory for the frame heirarchy
		CMeshHierarchy memoryAllocator;
		D3DXFrameDestroy(m_frameRoot, &memoryAllocator);
		m_frameRoot=0;
	}

	if (m_boneMatrices)
	{
		delete []m_boneMatrices;
		m_boneMatrices=0;
	}
}

/*
Load the x file
The function D3DXLoadMeshHierarchyFromX requires a support object to handle memeory allocation etc.
I have defined this in the class CMeshHierarchy
*/
bool CXFileEntity::Load(const std::string &filename)
{
	// Create our mesh hierarchy class to control the allocation of memory - only used temporarily
	CMeshHierarchy *memoryAllocator=new CMeshHierarchy;

	// To make it easier to find the textures change the current directory to the one containing the .x file
	// First though remember the current one to put it back afterwards
	std::string currentDirectory=CUtility::GetTheCurrentDirectory();

	std::string xfilePath;
	CUtility::SplitPath(filename,&xfilePath,&m_filename);

	SetCurrentDirectory(xfilePath.c_str());

	// This is the function that does all the .x file loading. We provide a pointer to an instance of our 
	// memory allocator class to handle memory allocationm during the frame and mesh loading
	HRESULT hr = D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, m_d3dDevice, 
		memoryAllocator, NULL, &m_frameRoot, &m_animController);

	delete memoryAllocator;
	memoryAllocator=0;

	// Set current directory back
	SetCurrentDirectory(currentDirectory.c_str());

	if (CUtility::FailedHr(hr))
		return false; 

	// if the x file contains any animation remember how many sets there are
	if(m_animController)
		m_numAnimationSets = m_animController->GetMaxNumAnimationSets();

	// Bones for skining
	if(m_frameRoot)
	{
		// Set the bones up
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)m_frameRoot);

		// Create the bone matrices array for use during FrameMove to hold the final transform
		m_boneMatrices  = new D3DXMATRIX[m_maxBones];
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		// Calculate the Bounding Sphere for this model (used in GetInitialCameraPosition to position camera correctly)
		D3DXFrameCalculateBoundingSphere(m_frameRoot, &m_sphereCentre, &m_sphereRadius);
	}

	return true;
}

/*
Since this demo can load many different sizes and shapes of 3d model setting the initial
camera position so the model can be seen is a tricky task. This function uses the model's bounding sphere
to come up with an initial position for the camera.
*/
D3DXVECTOR3 CXFileEntity::GetInitialCameraPosition() const
{
	D3DXVECTOR3 cameraPos(0.0f,m_sphereCentre.y,-(m_sphereRadius*3));
	return cameraPos;
}

/**
* \brief we need to go through the hierarchy and set the combined matrices
* calls itself recursively as it tareverses the hierarchy
* \param device - the Direct3D device object
* \param pFrame - current frame
* \param pParentMatrix - the parent frame matrix
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame)
{
	// Cast to our extended structure first
	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)pFrame->pMeshContainer;

	// If this frame has a mesh and there is skin info, then setup the bone matrices
	// Note: handles multiple mesh (// Added 24/08/10
	while(pMesh && pMesh->pSkinInfo)
	{
		// Create a copy of the mesh to skin into later
		D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
		if (FAILED(pMesh->MeshData.pMesh->GetDeclaration(Declaration)))
			return;

		pMesh->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, 
			Declaration, m_d3dDevice, 
			&pMesh->exSkinMesh);

		// Max bones is calculated for later use (to know how big to make the temp bone matrices array)
		m_maxBones=max(m_maxBones,(UINT)pMesh->pSkinInfo->GetNumBones());

		// For each bone work out its matrix
		for (unsigned int i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
		{   
			// Find the frame containing the bone
			// Must do this from root as skinned mesh and bone frame are not together
			D3DXFRAME_EXTENDED* pTempFrame = (D3DXFRAME_EXTENDED*)D3DXFrameFind(m_frameRoot, 
				pMesh->pSkinInfo->GetBoneName(i));
			assert(pTempFrame);

			// set the bone part - Note just point it at the transformation matrix of the relevant frame (aliase)
			pMesh->exFrameCombinedMatrixPointer[i] = &pTempFrame->exCombinedTransformationMatrix;										
		}

		pMesh=(D3DXMESHCONTAINER_EXTENDED*)pMesh->pNextMeshContainer;
	}

	// Pass on to sibblings
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameSibling);

	// Pass on to children
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameFirstChild);
}

/**
* \brief Called each frame update with the time and the current world matrix
* \param elapsedTime - time passed
* \param matWorld - current world matrix for the model
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::FrameMove(float elapsedTime,const D3DXMATRIX *matWorld)
{
	// Adjust animation speed
	elapsedTime/=m_speedAdjust;

	// Advance the time and set in the controller
	if (m_animController != NULL)
		m_animController->AdvanceTime(elapsedTime, NULL);

	m_currentTime+=elapsedTime;

	// Now update the model matrices in the hierarchy
	UpdateFrameMatrices(m_frameRoot, matWorld);

	// If the model contains a skinned mesh update the vertices
	UpdateSkinnedMesh(m_frameRoot);
}

// to handle when a model has multiple skinned mesh under different frames
// Added 24/08/10
// Requires exFrameCombinedMatrixPointer to be calculated prior to here (actually exCombinedTransformationMatrix as exFrameCombinedMatrixPointer aliasis it)
void CXFileEntity::UpdateSkinnedMesh(const D3DXFRAME *frameBase)
{
	D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)currentFrame->pMeshContainer;
	while(pMesh && pMesh->pSkinInfo) // handle chained  skinned mesh added 24/08/10
	{			
		unsigned int Bones = pMesh->pSkinInfo->GetNumBones();
		assert(Bones<=m_maxBones);

		// Create the bone matrices that transform each bone from bone space into character space
		// (via exFrameCombinedMatrixPointer) and also wraps the mesh around the bones using the bone offsets
		// in exBoneOffsetsArray
		for (unsigned int i = 0; i < Bones; ++i)
		{
			assert(i<m_maxBones);
			// Note: during set up exFrameCombinedMatrixPointer is made to point to the correct frame matrix (exCombinedTransformationMatrix)
			// So it does not directly get updated but uses the existing frame calculated value from the correct 'controller' frame
			D3DXMatrixMultiply(&m_boneMatrices[i],&pMesh->exBoneOffsets[i], pMesh->exFrameCombinedMatrixPointer[i]);				
		}

		// We need to modify the vertex positions based on the new bone matrices. This is achieved
		// by locking the vertex buffers and then calling UpdateSkinnedMesh. UpdateSkinnedMesh takes the
		// original vertex data (in pMesh->MeshData.pMesh), applies the matrices and writes the new vertices
		// out to skin mesh (pMesh->exSkinMesh). 

		// UpdateSkinnedMesh uses software skinning which is the slowest way of carrying out skinning 
		// but is easiest to describe and works on the majority of graphic devices. 
		// Other methods exist that use hardware to do this skinning - see the notes and the 
		// DirectX SDK skinned mesh sample for more details
		void *srcPtr=0;
		HRESULT hr=pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);
		if (CUtility::FailedHr(hr))
			return;

		void *destPtr=0;
		hr=pMesh->exSkinMesh->LockVertexBuffer(0, (void**)&destPtr);
		if (CUtility::FailedHr(hr))
			return;

		// Update the skinned mesh 
		hr=pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);
		if (CUtility::FailedHr(hr))
			return;

		// Note: bounds may have changed due to skinning! Need to recalc
		D3DXVECTOR3 min,max;
		hr=D3DXComputeBoundingBox((D3DXVECTOR3*)destPtr,pMesh->exSkinMesh->GetNumVertices(),
			D3DXGetFVFVertexSize(pMesh->exSkinMesh->GetFVF()),&min,&max);		
		if (CUtility::FailedHr(hr))
			return;

		// Unlock the meshes vertex buffers
		pMesh->exSkinMesh->UnlockVertexBuffer();
		pMesh->MeshData.pMesh->UnlockVertexBuffer();		

		// If we have more than one mesh
		pMesh=(D3DXMESHCONTAINER_EXTENDED*)pMesh->pNextMeshContainer;
	}		

	// If we have a sibling recurse 
	if (currentFrame->pFrameSibling != NULL)
		UpdateSkinnedMesh(currentFrame->pFrameSibling);

	// If we have a child recurse 
	if (currentFrame->pFrameFirstChild != NULL)
		UpdateSkinnedMesh(currentFrame->pFrameFirstChild);
}

/**
* \brief Called to update the frame matrices in the hierarchy to reflect current animation stage
* \param frameBase - frame being looked at
* \param parentMatrix - the matrix of our parent (if we have one)
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix)
{
	D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	// If parent matrix exists multiply our frame matrix by it
	if (parentMatrix != NULL)
		D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, &currentFrame->TransformationMatrix, parentMatrix);
	else
		currentFrame->exCombinedTransformationMatrix = currentFrame->TransformationMatrix;

	// If we have a sibling recurse 
	if (currentFrame->pFrameSibling != NULL)
		UpdateFrameMatrices(currentFrame->pFrameSibling, parentMatrix);

	// If we have a child recurse 
	if (currentFrame->pFrameFirstChild != NULL)
		UpdateFrameMatrices(currentFrame->pFrameFirstChild, &currentFrame->exCombinedTransformationMatrix);
}

/**
* \brief Render our mesh.
* Call the DrawFrame recursive fn on render with the root frame (see notes diagram)
* \param device - the Direct3D device object
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::Render() const
{
	if (m_frameRoot)
		DrawFrame(m_frameRoot);
}

/**
* \brief Called to render a frame in the hierarchy
* \param device - the Direct3D device object
* \param frame - frame to render
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::DrawFrame(LPD3DXFRAME frame) const
{
	// Draw all mesh containers in this frame
	LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
	while (meshContainer)
	{
		DrawMeshContainer(meshContainer, frame);
		meshContainer = meshContainer->pNextMeshContainer;
	}

	// Recurse for sibblings
	if (frame->pFrameSibling != NULL)
		DrawFrame(frame->pFrameSibling);

	// Recurse for children
	if (frame->pFrameFirstChild != NULL)
		DrawFrame(frame->pFrameFirstChild);
}

/**
* \brief Called to render a mesh
* \param device - the Direct3D device object
* \param meshContainerBase - the mesh container
* \param frameBase - frame containing the mesh
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase) const
{
	// Cast to our extended frame type
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameBase;		

	// Cast to our extended mesh container
	D3DXMESHCONTAINER_EXTENDED *meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;

	// Set the world transform But only if it is not a skinned mesh. 
	// The skinned mesh has the transform built in (the vertices are already transformed into world space) so we set identity
	// Added 24/08/10
	if (meshContainer->pSkinInfo)
	{
		D3DXMATRIX mat;
		D3DXMatrixIdentity(&mat);
		m_d3dDevice->SetTransform(D3DTS_WORLD, &mat);
	}
	else
		m_d3dDevice->SetTransform(D3DTS_WORLD, &frame->exCombinedTransformationMatrix);
	

	// Loop through all the materials in the mesh rendering each subset
	for (unsigned int iMaterial = 0; iMaterial < meshContainer->NumMaterials; iMaterial++)
	{
		// use the material in our extended data rather than the one in meshContainer->pMaterials[iMaterial].MatD3D
		m_d3dDevice->SetMaterial( &meshContainer->exMaterials[iMaterial] );
		m_d3dDevice->SetTexture( 0, meshContainer->exTextures[iMaterial] );

		// Select the mesh to draw, if there is skin then use the skinned mesh else the normal one
		LPD3DXMESH pDrawMesh = (meshContainer->pSkinInfo) ? meshContainer->exSkinMesh: meshContainer->MeshData.pMesh;

		// Finally Call the mesh draw function
		pDrawMesh->DrawSubset(iMaterial);
	}
}

/**
* \brief Change to a different animation set
* Handles transitions between animations to make it smooth and not a sudden jerk to a new position
* \param index - new animation set index
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::SetAnimationSet(unsigned int index)
{
	if (index==m_currentAnimationSet)
		return;

	if (index>=m_numAnimationSets)
		index=0;

	// Remember current animation
	m_currentAnimationSet=index;

	// Get the animation set from the controller
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );	

	// Note: for a smooth transition between animation sets we can use two tracks and assign the new set to the track
	// not currently playing then insert Keys into the KeyTrack to do the transition between the tracks
	// tracks can be mixed together so we can gradually change into the new animation

	// Alternate tracks
	DWORD newTrack = ( m_currentTrack == 0 ? 1 : 0 );

	// Assign to our track
	m_animController->SetTrackAnimationSet( newTrack, set );
	set->Release();	

	// Clear any track events currently assigned to our two tracks
	m_animController->UnkeyAllTrackEvents( m_currentTrack );
	m_animController->UnkeyAllTrackEvents( newTrack );

	// Add an event key to disable the currently playing track kMoveTransitionTime seconds in the future
	m_animController->KeyTrackEnable( m_currentTrack, FALSE, m_currentTime + kMoveTransitionTime );
	// Add an event key to change the speed right away so the animation completes in kMoveTransitionTime seconds
	m_animController->KeyTrackSpeed( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
	// Add an event to change the weighting of the current track (the effect it has blended with the secon track)
	m_animController->KeyTrackWeight( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

	// Enable the new track
	m_animController->SetTrackEnable( newTrack, TRUE );
	// Add an event key to set the speed of the track
	m_animController->KeyTrackSpeed( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
	// Add an event to change the weighting of the current track (the effect it has blended with the first track)
	// As you can see this will go from 0 effect to total effect(1.0f) in kMoveTransitionTime seconds and the first track goes from 
	// total to 0.0f in the same time.
	m_animController->KeyTrackWeight( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

	// Remember current track
	m_currentTrack = newTrack;
}

/**
* \brief Go to the next animation
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::NextAnimation()
{	
	unsigned int newAnimationSet=m_currentAnimationSet+1;
	if (newAnimationSet>=m_numAnimationSets)
		newAnimationSet=0;

	SetAnimationSet(newAnimationSet);
}

/**
* \brief Get the name of the animation
* Note: altered 24/09/07 to solve a D3DX memory leak caused because I was not releasing the set after getting it
* \param index - the animation set index
* \return the name
* \author Keith Ditchburn \date 18 July 2005
*/
std::string CXFileEntity::GetAnimationSetName(unsigned int index)
{
	if (index>=m_numAnimationSets)
		return "Error: No set exists";

	// Get the animation set
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	std::string nameString(set->GetName());

	set->Release();

	return nameString;
}

/**
* \brief Slow down animation
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::AnimateSlower()
{
	m_speedAdjust+=0.1f;
}

/**
* \brief Speed up animation
* \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::AnimateFaster()
{
	if (m_speedAdjust>0.1f)
		m_speedAdjust-=0.1f;
}