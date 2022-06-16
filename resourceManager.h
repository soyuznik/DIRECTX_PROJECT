#include<vector>
using namespace std;

class ResourceManager
{
public:
	vector<CXFileEntity *>models_AnimX;
	CXFileEntity  *AddAnimatedModel_X (char *fileName,char *model_name="")
	{
		CXFileEntity  *model;
		if( (model=GetModelAnimByPath(fileName))!=0)
			return model;

		model=new CXFileEntity( ::g_pd3dDevice );
		if( !model->Load(fileName) )
		{
			MessageBox(0,"Some Error Occured while Loading","Error",0);
			delete model;
			return 0;
		}
		model->m_filePath = fileName ;
		//model->modelname=model_name;
		
		models_AnimX.push_back(model);
		return model;
	}
	
	CXFileEntity  *GetModelAnimByPath(char *model_path)
	{
		for(int i=0;i<models_AnimX.size();i++)
		{
			if( !stricmp(models_AnimX[i]->m_filePath.c_str(),model_path))
			{
				return models_AnimX[i];
			}
		}
		return NULL;
	}

	ResourceManager()
	{
	
	}
};

ResourceManager g_resourceManager;
