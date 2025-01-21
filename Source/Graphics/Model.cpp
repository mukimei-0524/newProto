#include "Graphics/Graphics.h"
#include "Graphics/Model.h"

// コンストラクタ
Model::Model(const char* filename)
{
	// リソース読み込み
	resource = std::make_shared<ModelResource>();
	resource->Load(Graphics::Instance().GetDevice(), filename);

	// ノード
	const std::vector<ModelResource::Node>& resNodes = resource->GetNodes();

	nodes.resize(resNodes.size());
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		auto&& src = resNodes.at(nodeIndex);
		auto&& dst = nodes.at(nodeIndex);

		dst.name = src.name.c_str();
		dst.parent = src.parentIndex >= 0 ? &nodes.at(src.parentIndex) : nullptr;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.translate = src.translate;

		if (dst.parent != nullptr)
		{
			dst.parent->children.emplace_back(&dst);
		}
	}

	// 行列計算
	const DirectX::XMFLOAT4X4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	UpdateTransform(transform);
}

// 変換行列計算
void Model::UpdateTransform(const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);

	for (Node& node : nodes)
	{
		// ローカル行列算出
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotate));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translate.x, node.translate.y, node.translate.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;

		// ワールド行列算出
		DirectX::XMMATRIX ParentTransform;
		if (node.parent != nullptr)
		{
			ParentTransform = DirectX::XMLoadFloat4x4(&node.parent->worldTransform);
		}
		else
		{
			ParentTransform = Transform;
		}
		DirectX::XMMATRIX WorldTransform = LocalTransform * ParentTransform;

		// 計算結果を格納
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

void Model::UpdateAnimation(float elapsedTime)
{
	//再生中でないなら処理しない
	if (!IsPlayAnimation()) return;

	//ブレンド率の計算
	float blendRate = 1.0f;
	if (animationBlendTime < animationBlendSeconds) {
		animationBlendTime += elapsedTime;
		if (animationBlendTime >= animationBlendSeconds) {
			animationBlendTime = animationBlendSeconds;
		}
		blendRate = animationBlendTime / animationBlendSeconds;
		blendRate *= blendRate;
	}

	//指定のアニメーションデータを取得
	const std::vector<ModelResource::Animation>& animations = resource->GetAnimations();
	const ModelResource::Animation& animation = animations.at(currentAnimationIndex);
	
	//アニメーションデータからキーフレームデータリストを取得
	const std::vector<ModelResource::Keyframe>& keyframes = animation.keyframes;
	int keyCount = static_cast<int>(keyframes.size());
	for (int keyIndex = 0; keyIndex < keyCount - 1; ++keyIndex)
	{
		//現在の時間がどのキーフレームの間にいるか判定する
		const ModelResource::Keyframe& keyframe0 = keyframes.at(keyIndex);
		const ModelResource::Keyframe& keyframe1 = keyframes.at(keyIndex + 1);
		if (currentAnimationSceconds >= keyframe0.seconds &&
			currentAnimationSceconds < keyframe1.seconds)
		{
			//再生時間とキーフレームの時間から補完率を算出する
			float rate = (currentAnimationSceconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);

			int nodeCount = static_cast<int>(nodes.size());
			for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				//2つのキーフレーム間の補完計算
				const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
				const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);
				Node& node = nodes[nodeIndex];

				// スケールの補完
				DirectX::XMVECTOR scale0 = DirectX::XMLoadFloat3(&key0.scale);
				DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&key1.scale);
				DirectX::XMVECTOR interpolatedScale = DirectX::XMVectorLerp(scale0, scale1, rate);
				DirectX::XMStoreFloat3(&node.scale, interpolatedScale);

				// 回転の補完 (クォータニオンを線形補間ではなく球面線形補間 (Slerp) で補間する)
				DirectX::XMVECTOR rotate0 = DirectX::XMLoadFloat4(&key0.rotate);
				DirectX::XMVECTOR rotate1 = DirectX::XMLoadFloat4(&key1.rotate);
				DirectX::XMVECTOR interpolatedRotate = DirectX::XMQuaternionSlerp(rotate0, rotate1, rate);
				DirectX::XMStoreFloat4(&node.rotate, interpolatedRotate);

				// 移動の補完
				DirectX::XMVECTOR translate0 = DirectX::XMLoadFloat3(&key0.translate);
				DirectX::XMVECTOR translate1 = DirectX::XMLoadFloat3(&key1.translate);
				DirectX::XMVECTOR interpolatedTranslate = DirectX::XMVectorLerp(translate0, translate1, rate);
				DirectX::XMStoreFloat3(&node.translate, interpolatedTranslate);

				//ブレンド補完処理
				if (blendRate < 1.0f)
				{
					//現在の姿勢と次のキーフレームとの姿勢の補完
					//拡大率の値をXMVECTORに変換
					DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&node.scale);
					DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);

					//回転の値をXMVECTORに変換
					DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&node.rotate);
					DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);

					//位置の値をXMVECTORに変換
					DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&node.translate);
					DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

					//拡大の値を線形補完で計算
					DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, blendRate);
					//回転の値を球面線形補完で算出
					DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, blendRate);
					//位置の値を線形補完で計算
					DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, blendRate);

					//計算結果（拡大、回転、位置）をボーン（node）に格納
					DirectX::XMStoreFloat3(&node.scale, S);
					DirectX::XMStoreFloat4(&node.rotate, R);
					DirectX::XMStoreFloat3(&node.translate, T);
				}
				//通常の処理
				else
				{
					//前のキーフレームと次のキーフレームの姿勢を補完
					//拡大率の値(key0とkey1のscale)をXMVECTORに変換
					DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&key0.scale);
					DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);

					//回転の値(key0とkey1のrotate)をXMVECTORに変換
					DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);

					//位置の値(key0とkey1のtranslate)をXMVECTORに変換
					DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&key0.translate);
					DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

					//拡大の値を線形補完で計算(拡大縮小の線形補完をrateで計算)
					DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, rate);
					//回転の値を球面線形補完で算出(回転率の球面線形補完をrateで計算)
					DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, rate);
					//位置の値を線形補完で計算(位置の線形補完をrateで計算)
					DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, rate);

					//計算結果（拡大、回転、位置）をボーン（node）に格納
					DirectX::XMStoreFloat3(&node.scale, S);
					DirectX::XMStoreFloat4(&node.rotate, R);
					DirectX::XMStoreFloat3(&node.translate, T);
				}
			}
			break;
		}
	}
	//最終フレーム処理
	if (animationEndFlag)
	{
		animationEndFlag = false;
		currentAnimationIndex = -1;
		return;
	}
	//時間経過
	currentAnimationSceconds += elapsedTime;

	//再生時間が終端時間を超えたら
	if (currentAnimationSceconds >= animation.secondsLength)
	{
		if (animationLoopFlag)
		{
			// ループする場合、再生時間を巻き戻す
			currentAnimationSceconds -= animation.secondsLength;
		}
		//再生終了時間にする
		else
		{
			// ループしない場合、アニメーションを停止する
			currentAnimationSceconds = animation.secondsLength; // 終端で固定
			animationEndFlag = true;  // アニメーション終了フラグを立てる
		}
	}
}

//アニメーション再生
void Model::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSceconds = 0.0f;
	animationLoopFlag = loop;
	animationEndFlag = false;
	animationBlendTime = 0.0f;
	animationBlendSeconds = blendSeconds;

}

bool Model::IsPlayAnimation() const
{
	//if (currentAnimationIndex < 0)return false;
	if (currentAnimationIndex >= resource->GetAnimations().size())return false;
	return true;
}

Model::Node* Model::FindNode(const char* name)
{
	//すべてのノードを総当たりで名前比較する
	for (auto& node : nodes)
	{
		//引数の名前とノードの名前を比較し、一致するかチェック
		if (strcmp(node.name, name) == 0)
		{
			//一致したらそのノードをリターン
			return &node;
		}
	}
	//見つからなかったら
	return nullptr;
}
