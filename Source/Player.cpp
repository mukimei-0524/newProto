#include <imgui.h>
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "Player.h"
#include "camera.h"
#include "EnemyManager.h"
#include "Collision.h"
#include "ProjectileStraight.h"
#include "ProjectileHoming.h"
#include <EnemySlime.h>


static Player* instance = nullptr;
Player& Player::Instance()
{
	return *instance;
}

//コンストラクタ
Player::Player()
{
	//model = new Model("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
	model = new Model("Data/Model/Jammo/Jammo.mdl");
	//model->PlayAnimation(0);

	hitEffect = new Effect("Data/Effect/Hit.efk");

	//モデルが大きい為スケーリング
	scale.x = scale.y = scale.z = 0.01f;

	//待機ステートへ遷移
	TransitionState();

	//インスタンスポインタ取得
	instance = this;
}

//デストラクタ
Player::~Player()
{
	delete model;
	delete hitEffect;
}

//更新処理
void Player::Update(float elapsedTime)
{
	{
		{
			//オブジェクト行列を更新
			//UpdateTransform();

			//モデル行列更新
			//model->UpdateTransform(transform);

			////進行ベクトル取得
			//DirectX::XMFLOAT3 moveVec = GetMoveVec();

			////移動処理
			//float moveSpeed = this->moveSpeed * elapsedTime;
			//position.x += moveVec.x * moveSpeed;
			//position.z += moveVec.z * moveSpeed;
		}
		//移動入力
		//InputMove(elapsedTime);
		{

			////入力情報を取得
			//GamePad& gamePad = Input::Instance().GetGamePad();
			//float ax = gamePad.GetAxisLX();
			//float ay = gamePad.GetAxisLY();

			////移動操作
			//float moveSpeed = 5.0f * elapsedTime;
			//{
			//	//左スティックの入力情報をもとにXZ平面への移動処理
			//	position.x += ax * moveSpeed;
			//	position.z += ay * moveSpeed;
			//}

			////回転操作
			//float rotateSpeed = DirectX::XMConvertToRadians(360) * elapsedTime;
			//if (gamePad.GetButton() & GamePad::BTN_A)
			//{
			//	//X軸回転操作
			//	DirectX::CXMMATRIX rotationX = DirectX::XMMatrixRotationX(360);
			//}
			//if (gamePad.GetButton() & GamePad::BTN_B)
			//{
			//	//Y軸回転操作
			//	DirectX::CXMMATRIX rotationY = DirectX::XMMatrixRotationY(360);
			//}
			//if (gamePad.GetButton() & GamePad::BTN_X)
			//{
			//	//Z軸回転操作
			//	DirectX::CXMMATRIX rotationZ = DirectX::XMMatrixRotationZ(360);
			//}
		}

		//ジャンプ入力処理
		//InputJump();

		//弾丸入力処理
		//InputProjectile();
	}
	//ステート毎の処理
	switch (state)
	{
	case State::Idle:
		UpdateIdleState(elapsedTime);
		break;

	case State::Move:
		UpdateMoveState(elapsedTime);
		break;

	case State::Jump:
		UpdateJumpState(elapsedTime);
		break;
		
	case State::Land:
		UpdateLandState(elapsedTime);
		break;

	case State::Attack:
		UpdateAttackState(elapsedTime);
		break;

	case State::Damage:
		UpdateDamageState(elapsedTime);
		break;

	case State::Death:
		UpdateDeathState(elapsedTime);
		break;

	case State::Revive:
		UpdateReviveState(elapsedTime);
		break;
	}

	//速力処理更新
	UpdateVelocity(elapsedTime);

	//無敵時間更新処理
	UpdateInvincibleTimer(elapsedTime);
	
	//弾丸更新処理
	projectileManager.Update(elapsedTime);

	//プレイヤーと敵との衝突処理
	CollisionPlayerVsEnemies();

	//弾丸と敵の衝突処理
	CollistionProjectilesVsEnemies();

	//オブジェクト行列を更新
	UpdateTransform();

	//モデルアニメーション更新処理
	model->UpdateAnimation(elapsedTime);

	//モデル行列更新
	model->UpdateTransform(transform);
#if 0
	//Bボタン押下でワンショットアニメーション再生
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_B)
	{
		//model->PlayAnimation(0, false);
		//model->PlayAnimation(0, false, 0.1f);
		model->PlayAnimation(Anim_Running, true);
	}
	//ワンショットアニメーション再生が終わったらループアニメーション再生
	if (!model->IsPlayAnimation())
	{
		model->PlayAnimation(5, true);
	}
#endif

}

DirectX::XMFLOAT3 Player::GetMoveVec() const
{
	//入力情報を取得
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//カメラ方向とスティックの入力値によって進行方向を計算する
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//移動ベクトルはXZ平面に水平なベクトルになるようにする
	//using namespace DirectX;
	//auto vecRight = DirectX::XMLoadFloat3(&cameraRight);
	//auto vecForward = DirectX::XMLoadFloat3(&cameraFront);
	//vecRight = DirectX::XMVector3Normalize(vecRight);

	DirectX::XMFLOAT3 vec;
	//DirectX::XMStoreFloat3(&vec, vecRight * ax + vecForward * ay);
	//return vec;

	//カメラ右方向ベクトルをXZ単位ベクトルに変換
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//単位ベクトル化
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//カメラ前方向ベクトルをXZ単位ベクトルに変換
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//単位ベクトル化
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//スティックの水平入力値をカメラ右方向に反映し、
	//スティックの垂直入力値をカメラ前方向に反映し、
	//進行ベクトルを計算する
	vec.x = cameraRightX * ax + cameraFrontX * ay;
	vec.z = cameraRightZ * ax + cameraFrontZ * ay;
	//Y軸方向には移動しない
	vec.y = 0.0f;

	return vec;
}

//プレイヤーとエネミーの衝突処理
void Player::CollisionPlayerVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//全ての敵と総当たりで衝突処理
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//衝突処理
		DirectX::XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsSphere(
			position,
			radius,
			enemy->GetPosition(),
			enemy->GetRadius(),
			outPosition
		))
		{
			//敵の真上付近に当たったか
			DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
			DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
			DirectX::XMVECTOR V = DirectX::XMVectorSubtract(P, E);
			DirectX::XMVECTOR N = DirectX::XMVector3Normalize(V);
			DirectX::XMFLOAT3 nomal;
			DirectX::XMStoreFloat3(&nomal, N);

			//上から分図ける場合は小ジャンプ
			if (nomal.y > 0.8f)
			{
				//小ジャンプする
				Jump(jumpSpeed * 0.5f);
			}
			else
			{
				//押し出し後の位置設定
				enemy->SetPosition(outPosition);
			}
		}
	}
}

void Player::CollisionNodeVsEnemies(const char* nodeName, float nodeRadius)
{
	//ノード取得
	Model::Node* node = model->FindNode(nodeName);

	//ノード位置取得
	DirectX::XMFLOAT3 nodePosition;
	nodePosition.x = node->worldTransform._41;	//ボーンのワールドのx位置
	nodePosition.y = node->worldTransform._42;	//ボーンのワールドのy位置
	nodePosition.z = node->worldTransform._43;	//ボーンのワールドのz位置	

	//指定のノードと全ての敵を総当たりで衝突処理
	EnemyManager& enemyManager = EnemyManager::Instance();

	//敵の数を取得
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		//i番目の敵を取得
		Enemy* enemy = enemyManager.GetEnemy(i);

		//衝突処理
		DirectX::XMFLOAT3 outPosition;
		if (
			Collision::IntersectSphereVsCylinder(
				nodePosition,
				nodeRadius,
				enemy->GetPosition(),
				enemy->GetRadius(),
				enemy->GetHeight(),
				outPosition
			))
		{


			//ダメージを与える
			if (enemy->ApplyDamage(1, 0.5f))
			{
				//吹っ飛ばす(弾が当たったときのふっ飛ばす処理と同じ)
				{
					DirectX::XMFLOAT3 impulse;	//impulse：衝撃

					// どれぐらいの力で吹き飛ぶか
					float power = 10.0f;
					
					//敵の位置
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					//敵への方向ベクトル
					float vx = e.x - nodePosition.x;
					float vz = e.z - nodePosition.z;
					//方向ベクトルを正規化
					float XZLength = sqrtf(vx * vx + vz * vz);
					vx /= XZLength;
					vz /= XZLength;

					// 吹き飛ばす方向ベクトルに力をかける（スケーリング)
					impulse.x = vx * power;
					impulse.y = power * 0.5f;
					impulse.z = vz * power;

					enemy->AddImpulse(impulse);
				}

				//ヒットエフェクト再生(弾が当たったときのヒットエフェクト再生と同じ)
				{
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					e.y += enemy->GetHeight() * 0.5f;
					hitEffect->Play(e);
				}
			}
		}
	}
}

//弾丸入力処理
void Player::InputProjectile()
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	//直進弾丸発射
	if (gamePad.GetButtonDown() & GamePad::BTN_X)
	{
		//前方向
		DirectX::XMFLOAT3 dir;
		dir.x = sinf(angle.y);
		dir.y = 0;
		dir.z = cosf(angle.y);
		
		//発射位置(プレイヤーの腰あたり)
		DirectX::XMFLOAT3 pos;
		pos.x = position.x;
		pos.y = position.y + height * 0.5f;
		pos.z = position.z;

		//発射
		ProjectileStraight* projectile = new ProjectileStraight(&projectileManager);
		projectile->Launch(dir, pos);
	}
	// 追尾弾丸発射
	if (gamePad.GetButtonDown() & GamePad::BTN_Y)
	{
		//前方向
		DirectX::XMFLOAT3 dir;
		dir.x = sinf(angle.y);
		dir.y = 0;
		dir.z = cosf(angle.y);

		//発射位置(プレイヤーの腰あたり)
		DirectX::XMFLOAT3 pos;
		pos.x = position.x;
		pos.y = position.y + height * 0.5f;
		pos.z = position.z;

		//ターゲット(デフォルトではプレイヤーの前方)
		DirectX::XMFLOAT3 target;
		target.x = pos.x + dir.x * 1000.0f;
		target.y = pos.y + dir.y * 1000.0f;
		target.z = pos.z + dir.z * 1000.0f;

		//一番近くの敵をターゲットにする
		float dist = FLT_MAX;
		EnemyManager& enemyManager = EnemyManager::Instance();
		int enemyCount = enemyManager.GetEnemyCount();
		for (int i = 0; i < enemyCount; ++i)
		{
			//敵との距離判定
			Enemy* enemy = EnemyManager::Instance().GetEnemy(i);
			DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
			DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
			DirectX::XMVECTOR V = DirectX::XMVectorSubtract(E, P);
			DirectX::XMVECTOR D = DirectX::XMVector3LengthSq(V);
			float d;
			DirectX::XMStoreFloat(&d, D);
			if (d < dist)
			{
				dist = d;
				target = enemy->GetPosition();
				target.y = enemy->GetHeight() * 0.5f;
			}
		}

		ProjectileHoming* projectileH = new ProjectileHoming(&projectileManager);
		projectileH->Launch(dir, pos, target);
	}
}

/// <summary>
///ジャンプ入力処理 
/// </summary>
/// <returns>true...ジャンプした</returns>
bool Player::InputJump()
{
	//ボタン入力でジャンプ(ジャンプ回数制限付き)
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_A)
	{
		if (jumpCount < jumpLimit)
		{
			//ジャンプ
			jumpCount++;
			Jump(jumpSpeed);

			//ジャンプ入力した
			return true;
		}
	}
	return false;
}

//弾丸と敵の衝突処理
void Player::CollistionProjectilesVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//全ての弾丸と全ての敵を総当たりで衝突処理
	int projectileCount = projectileManager.GetProjectileCount();
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < projectileCount; ++i)
	{
		Projectile* projectile = projectileManager.GetProjectile(i);

		for (int j = 0; j < enemyCount; ++j)
		{
			Enemy* enemy = enemyManager.GetEnemy(j);

			//衝突処理
			DirectX::XMFLOAT3 outPosition;
			if (Collision::IntersectSphereVsCylinder(
				projectile->GetPosition(),
				projectile->GetRadius(),
				enemy->GetPosition(),
				enemy->GetRadius(),
				enemy->GetHeight(),
				outPosition))
			{
				//ダメージを与える
				//enemy->ApplyDamage(1, 0.5f);
				if (enemy->ApplyDamage(1, 0.5f))
				{
					//吹き飛ばす
					{
						DirectX::XMFLOAT3 impulse;	//impulse：衝撃

						// どれぐらいの力で吹き飛ぶか
						float power = 10.0f;

						// 敵の位置 - 弾の位置、それぞれの xz 成分
						float impX = enemy->GetPosition().x - projectile->GetPosition().x;
						float impZ = enemy->GetPosition().z - projectile->GetPosition().z;


						// 正規化した吹き飛ばす方向ベクトル
						float impLength = sqrtf(impX * impX + impZ * impZ);
						float vx = impX / impLength;
						float vz = impZ / impLength;

						// 吹き飛ばす方向ベクトルに力をかける（スケーリング)
						impulse.x = vx * power;
						impulse.y = power * 0.5f;
						impulse.z = vz * power;

#if 0
						//授業回答
						{
							const DirectX::XMFLOAT3& e = enemy->GetPosition();
							const DirectX::XMFLOAT3& p = projectile->GetPosition();
							float vx = e.x - p.x;
							float vz = e.z - p.z;
							float lengthXZ = sqrtf(vx * vx + vz * vz);
							vx /= lengthXZ;
							vz /= lengthXZ;
						}
#endif
						enemy->AddImpulse(impulse);
					}

					//ヒットエフェクト再生
					{
						DirectX::XMFLOAT3 e = enemy->GetPosition();
						e.y += enemy->GetHeight() * 0.5f;
						hitEffect->Play(e);
					}
					//弾丸破棄
					projectile->Destroy();
				}
			}
		}

	}
}


//移動入力処理
bool Player::InputMove(float elapsedTime)
{
	//進行ベクトル取得
	DirectX::XMFLOAT3 moveVec = GetMoveVec();

	//移動処理
	Move(moveVec.x, moveVec.z, moveSpeed);

	//旋回処理
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	return moveVec.x != 0.0f || moveVec.y != 0.0f || moveVec.z != 0.0f;
}

bool Player::InputAttack()
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	if (gamePad.GetButtonDown() & GamePad::BTN_B)
	{
		return true;
	}
	return false;
}

//着地したときに呼ばれる
void Player::OnLanding()
{
	jumpCount = 0;

	//下方向の速力が一定以上なら着地ステートへ
	if (velocity.y < gravity * 5.0f)
	{
		if (state != State::Damage && state != State::Death)
		{
			//着地ステートへ遷移
			TransitionLandState();
		}
	}
}

void Player::OnDamaged()
{
	//ダメージステートへ遷移
	TransitionDamageState();
}

void Player::OnDead()
{
	//死亡ステートへ遷移
	TransitionDeathState();
}

//描画処理
void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
{
	shader->Draw(dc, model);

	//弾丸描画処理
	projectileManager.Render(dc, shader);
}

//デバッグプリミティブ描画
void Player::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//衝突判定用のデバッグ球を描画
	//debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));

	//衝突判定用のデバッグ円柱を描画
	debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));

	//弾丸デバッグプリミティブ描画
	debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));

	//攻撃衝突用の左手ノードのデバッグ球を描画
	if (attackCollitionFlag)
	{
		Model::Node* leftHandBone = model->FindNode("mixamorig:LeftHand");
		debugRenderer->DrawSphere(DirectX::XMFLOAT3(
			leftHandBone->worldTransform._41,
			leftHandBone->worldTransform._42,
			leftHandBone->worldTransform._43),
			leftHandRadius,
			DirectX::XMFLOAT4(1, 0, 0, 1)
		);
	}
}

void Player::DrawDebugGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
	{
		//トランスフォーム
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::InputFloat3("Position", &position.x);
			ImGui::InputFloat3("Velocity", &velocity.x);
			//回転
			DirectX::XMFLOAT3 a;
			a.x = DirectX::XMConvertToDegrees(angle.x);
			a.y = DirectX::XMConvertToDegrees(angle.y);
			a.z = DirectX::XMConvertToDegrees(angle.z);
			ImGui::InputFloat3("Angle", &a.x);
			angle.x = DirectX::XMConvertToRadians(a.x);
			angle.y = DirectX::XMConvertToRadians(a.y);
			angle.z = DirectX::XMConvertToRadians(a.z);
			//スケール
			ImGui::InputFloat3("Scale", &scale.x);
		}
		
	}
	ImGui::End();
}


void Player::TransitionState()
{
	state = State::Idle;

	//待機アニメーション再生
	model->PlayAnimation(Anim_Idle, true);
}

void Player::UpdateIdleState(float elapsedTime)
{
	//移動入力処理(trueだったら移動ステートへ遷移)
	if (InputMove(elapsedTime))
	{
		//移動ステートへ遷移
		TransitionMoveState();
	}

	//ジャンプ入力処理(trueだったらジャンプステートへ遷移)
	if (InputJump())
	{
		//ジャンプステートへ遷移
		TransitionJumpState();
	}

	//弾丸入力処理
	InputProjectile();

	//攻撃入力処理(trueだったら攻撃ステートへ遷移)
	if (InputAttack())
	{
		TransitionAttackState();
	}
}

void Player::TransitionMoveState()
{
	state = State::Move;

	//走りアニメーション再生
	model->PlayAnimation(Anim_Running, true);
}

void Player::UpdateMoveState(float elapsedTime)
{
	//移動入力処理(falseだったら待機ステートへ遷移)
	if (!InputMove(elapsedTime))
	{
		TransitionState();
	}

	//ジャンプ入力処理
	if (InputJump())
	{
		//ジャンプステートへ遷移
		TransitionJumpState();
	}

	//弾丸入力処理
	InputProjectile();

	//攻撃入力処理
	if (InputAttack())
	{
		TransitionAttackState();
	}
}

void Player::TransitionJumpState()
{
	state = State::Jump;

	//ジャンプアニメーション再生
	model->PlayAnimation(Anim_Jump, false);
}

void Player::UpdateJumpState(float elapsedTime)
{
	//移動入力処理(falseだったら待機ステートへ遷移)
	InputMove(elapsedTime);

	//ジャンプ入力処理
	InputJump();

	//ジャンプアニメーション終了後(していたら)
	if (!model->IsPlayAnimation())
	{
		//落下アニメーションを再生
		model->PlayAnimation(Anim_Falling, true);
	}

	//弾丸入力処理
	InputProjectile();
}

void Player::TransitionLandState()
{
	state = State::Land;

	//着地アニメーション再生
	model->PlayAnimation(Anim_Landing, false);
}

void Player::UpdateLandState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		//待機ステートへ遷移
		TransitionState();
	}
}

void Player::TransitionAttackState()
{
	state = State::Attack;

	model->PlayAnimation(Anim_Attak, false);
}

void Player::UpdateAttackState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		TransitionState();
	}

	//任意のアニメーション再生区間でのみ衝突判定処理をする
	//モデルから現在のアニメーション再生時間を取得
	float animationTime = model->GetCurrentAnimationSeconds();
	//animationTimeが0.3より大きく、0.4より小さい
	attackCollitionFlag = animationTime > 0.3f && animationTime < 0.4f;

	if (attackCollitionFlag)
	{
		CollisionNodeVsEnemies("mixamorig:LeftHand", leftHandRadius);
	}

}

void Player::TransitionDamageState()
{
	state = State::Damage;
	//ダメージアニメーション再生
	model->PlayAnimation(Anim_GetHit1, false);
}

void Player::UpdateDamageState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		//待機ステートへ遷移
		TransitionState();
	}
}

void Player::TransitionDeathState()
{
	state = State::Death;

	model->PlayAnimation(Anim_Death, false);
}

void Player::UpdateDeathState(float elapsedTime)
{
	//ボタンを押したら復活ステートへ遷移
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_A)
	{
		TransitionReviveState();
	}
}

void Player::TransitionReviveState()
{
	state = State::Revive;

	health = maxHealth;

	model->PlayAnimation(Anim_Revive, false);
}

void Player::UpdateReviveState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		//待機ステートへ遷移
		TransitionState();
	}
}
