#include "StageManager.h"

void StageManager::Update(float elapsedTime)
{
	for (Stage* stage : stages) 
	{
		stage->Update(elapsedTime);
	}
}

void StageManager::Render(ID3D11DeviceContext* dc, Shader* shader)
{
	for (Stage* stage : stages) 
	{
		stage->Render(dc, shader);
	}
}

void StageManager::Register(Stage* stage)
{
	stages.emplace_back(stage);
}

void StageManager::Clear()
{
	for (Stage* stage : stages) 
	{
		delete stage;
	}
	//イテレーターをクリアにする
	stages.clear();
}

bool StageManager::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
	bool result = false;

	hit.distance = FLT_MAX;

	for (Stage* stage : stages)
	{
		HitResult tmp;
		if (stage->RayCast(start, end, tmp)) 
		{
			if (hit.distance > tmp.distance) 
			{
				hit = tmp;
				result = true;
			}
		}

	}


	return result;
}
