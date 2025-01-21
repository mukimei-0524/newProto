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

//�R���X�g���N�^
Player::Player()
{
	//model = new Model("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
	model = new Model("Data/Model/Jammo/Jammo.mdl");
	//model->PlayAnimation(0);

	hitEffect = new Effect("Data/Effect/Hit.efk");

	//���f�����傫���׃X�P�[�����O
	scale.x = scale.y = scale.z = 0.01f;

	//�ҋ@�X�e�[�g�֑J��
	TransitionState();

	//�C���X�^���X�|�C���^�擾
	instance = this;
}

//�f�X�g���N�^
Player::~Player()
{
	delete model;
	delete hitEffect;
}

//�X�V����
void Player::Update(float elapsedTime)
{
	{
		{
			//�I�u�W�F�N�g�s����X�V
			//UpdateTransform();

			//���f���s��X�V
			//model->UpdateTransform(transform);

			////�i�s�x�N�g���擾
			//DirectX::XMFLOAT3 moveVec = GetMoveVec();

			////�ړ�����
			//float moveSpeed = this->moveSpeed * elapsedTime;
			//position.x += moveVec.x * moveSpeed;
			//position.z += moveVec.z * moveSpeed;
		}
		//�ړ�����
		//InputMove(elapsedTime);
		{

			////���͏����擾
			//GamePad& gamePad = Input::Instance().GetGamePad();
			//float ax = gamePad.GetAxisLX();
			//float ay = gamePad.GetAxisLY();

			////�ړ�����
			//float moveSpeed = 5.0f * elapsedTime;
			//{
			//	//���X�e�B�b�N�̓��͏������Ƃ�XZ���ʂւ̈ړ�����
			//	position.x += ax * moveSpeed;
			//	position.z += ay * moveSpeed;
			//}

			////��]����
			//float rotateSpeed = DirectX::XMConvertToRadians(360) * elapsedTime;
			//if (gamePad.GetButton() & GamePad::BTN_A)
			//{
			//	//X����]����
			//	DirectX::CXMMATRIX rotationX = DirectX::XMMatrixRotationX(360);
			//}
			//if (gamePad.GetButton() & GamePad::BTN_B)
			//{
			//	//Y����]����
			//	DirectX::CXMMATRIX rotationY = DirectX::XMMatrixRotationY(360);
			//}
			//if (gamePad.GetButton() & GamePad::BTN_X)
			//{
			//	//Z����]����
			//	DirectX::CXMMATRIX rotationZ = DirectX::XMMatrixRotationZ(360);
			//}
		}

		//�W�����v���͏���
		//InputJump();

		//�e�ۓ��͏���
		//InputProjectile();
	}
	//�X�e�[�g���̏���
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

	//���͏����X�V
	UpdateVelocity(elapsedTime);

	//���G���ԍX�V����
	UpdateInvincibleTimer(elapsedTime);
	
	//�e�ۍX�V����
	projectileManager.Update(elapsedTime);

	//�v���C���[�ƓG�Ƃ̏Փˏ���
	CollisionPlayerVsEnemies();

	//�e�ۂƓG�̏Փˏ���
	CollistionProjectilesVsEnemies();

	//�I�u�W�F�N�g�s����X�V
	UpdateTransform();

	//���f���A�j���[�V�����X�V����
	model->UpdateAnimation(elapsedTime);

	//���f���s��X�V
	model->UpdateTransform(transform);
#if 0
	//B�{�^�������Ń����V���b�g�A�j���[�V�����Đ�
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_B)
	{
		//model->PlayAnimation(0, false);
		//model->PlayAnimation(0, false, 0.1f);
		model->PlayAnimation(Anim_Running, true);
	}
	//�����V���b�g�A�j���[�V�����Đ����I������烋�[�v�A�j���[�V�����Đ�
	if (!model->IsPlayAnimation())
	{
		model->PlayAnimation(5, true);
	}
#endif

}

DirectX::XMFLOAT3 Player::GetMoveVec() const
{
	//���͏����擾
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���
	//using namespace DirectX;
	//auto vecRight = DirectX::XMLoadFloat3(&cameraRight);
	//auto vecForward = DirectX::XMLoadFloat3(&cameraFront);
	//vecRight = DirectX::XMVector3Normalize(vecRight);

	DirectX::XMFLOAT3 vec;
	//DirectX::XMStoreFloat3(&vec, vecRight * ax + vecForward * ay);
	//return vec;

	//�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
	//�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A
	//�i�s�x�N�g�����v�Z����
	vec.x = cameraRightX * ax + cameraFrontX * ay;
	vec.z = cameraRightZ * ax + cameraFrontZ * ay;
	//Y�������ɂ͈ړ����Ȃ�
	vec.y = 0.0f;

	return vec;
}

//�v���C���[�ƃG�l�~�[�̏Փˏ���
void Player::CollisionPlayerVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�S�Ă̓G�Ƒ�������ŏՓˏ���
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
		DirectX::XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsSphere(
			position,
			radius,
			enemy->GetPosition(),
			enemy->GetRadius(),
			outPosition
		))
		{
			//�G�̐^��t�߂ɓ���������
			DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
			DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
			DirectX::XMVECTOR V = DirectX::XMVectorSubtract(P, E);
			DirectX::XMVECTOR N = DirectX::XMVector3Normalize(V);
			DirectX::XMFLOAT3 nomal;
			DirectX::XMStoreFloat3(&nomal, N);

			//�ォ�番�}����ꍇ�͏��W�����v
			if (nomal.y > 0.8f)
			{
				//���W�����v����
				Jump(jumpSpeed * 0.5f);
			}
			else
			{
				//�����o����̈ʒu�ݒ�
				enemy->SetPosition(outPosition);
			}
		}
	}
}

void Player::CollisionNodeVsEnemies(const char* nodeName, float nodeRadius)
{
	//�m�[�h�擾
	Model::Node* node = model->FindNode(nodeName);

	//�m�[�h�ʒu�擾
	DirectX::XMFLOAT3 nodePosition;
	nodePosition.x = node->worldTransform._41;	//�{�[���̃��[���h��x�ʒu
	nodePosition.y = node->worldTransform._42;	//�{�[���̃��[���h��y�ʒu
	nodePosition.z = node->worldTransform._43;	//�{�[���̃��[���h��z�ʒu	

	//�w��̃m�[�h�ƑS�Ă̓G�𑍓�����ŏՓˏ���
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�G�̐����擾
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		//i�Ԗڂ̓G���擾
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
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


			//�_���[�W��^����
			if (enemy->ApplyDamage(1, 0.5f))
			{
				//������΂�(�e�����������Ƃ��̂ӂ���΂������Ɠ���)
				{
					DirectX::XMFLOAT3 impulse;	//impulse�F�Ռ�

					// �ǂꂮ�炢�̗͂Ő�����Ԃ�
					float power = 10.0f;
					
					//�G�̈ʒu
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					//�G�ւ̕����x�N�g��
					float vx = e.x - nodePosition.x;
					float vz = e.z - nodePosition.z;
					//�����x�N�g���𐳋K��
					float XZLength = sqrtf(vx * vx + vz * vz);
					vx /= XZLength;
					vz /= XZLength;

					// ������΂������x�N�g���ɗ͂�������i�X�P�[�����O)
					impulse.x = vx * power;
					impulse.y = power * 0.5f;
					impulse.z = vz * power;

					enemy->AddImpulse(impulse);
				}

				//�q�b�g�G�t�F�N�g�Đ�(�e�����������Ƃ��̃q�b�g�G�t�F�N�g�Đ��Ɠ���)
				{
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					e.y += enemy->GetHeight() * 0.5f;
					hitEffect->Play(e);
				}
			}
		}
	}
}

//�e�ۓ��͏���
void Player::InputProjectile()
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	//���i�e�۔���
	if (gamePad.GetButtonDown() & GamePad::BTN_X)
	{
		//�O����
		DirectX::XMFLOAT3 dir;
		dir.x = sinf(angle.y);
		dir.y = 0;
		dir.z = cosf(angle.y);
		
		//���ˈʒu(�v���C���[�̍�������)
		DirectX::XMFLOAT3 pos;
		pos.x = position.x;
		pos.y = position.y + height * 0.5f;
		pos.z = position.z;

		//����
		ProjectileStraight* projectile = new ProjectileStraight(&projectileManager);
		projectile->Launch(dir, pos);
	}
	// �ǔ��e�۔���
	if (gamePad.GetButtonDown() & GamePad::BTN_Y)
	{
		//�O����
		DirectX::XMFLOAT3 dir;
		dir.x = sinf(angle.y);
		dir.y = 0;
		dir.z = cosf(angle.y);

		//���ˈʒu(�v���C���[�̍�������)
		DirectX::XMFLOAT3 pos;
		pos.x = position.x;
		pos.y = position.y + height * 0.5f;
		pos.z = position.z;

		//�^�[�Q�b�g(�f�t�H���g�ł̓v���C���[�̑O��)
		DirectX::XMFLOAT3 target;
		target.x = pos.x + dir.x * 1000.0f;
		target.y = pos.y + dir.y * 1000.0f;
		target.z = pos.z + dir.z * 1000.0f;

		//��ԋ߂��̓G���^�[�Q�b�g�ɂ���
		float dist = FLT_MAX;
		EnemyManager& enemyManager = EnemyManager::Instance();
		int enemyCount = enemyManager.GetEnemyCount();
		for (int i = 0; i < enemyCount; ++i)
		{
			//�G�Ƃ̋�������
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
///�W�����v���͏��� 
/// </summary>
/// <returns>true...�W�����v����</returns>
bool Player::InputJump()
{
	//�{�^�����͂ŃW�����v(�W�����v�񐔐����t��)
	GamePad& gamePad = Input::Instance().GetGamePad();
	if (gamePad.GetButtonDown() & GamePad::BTN_A)
	{
		if (jumpCount < jumpLimit)
		{
			//�W�����v
			jumpCount++;
			Jump(jumpSpeed);

			//�W�����v���͂���
			return true;
		}
	}
	return false;
}

//�e�ۂƓG�̏Փˏ���
void Player::CollistionProjectilesVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�S�Ă̒e�ۂƑS�Ă̓G�𑍓�����ŏՓˏ���
	int projectileCount = projectileManager.GetProjectileCount();
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < projectileCount; ++i)
	{
		Projectile* projectile = projectileManager.GetProjectile(i);

		for (int j = 0; j < enemyCount; ++j)
		{
			Enemy* enemy = enemyManager.GetEnemy(j);

			//�Փˏ���
			DirectX::XMFLOAT3 outPosition;
			if (Collision::IntersectSphereVsCylinder(
				projectile->GetPosition(),
				projectile->GetRadius(),
				enemy->GetPosition(),
				enemy->GetRadius(),
				enemy->GetHeight(),
				outPosition))
			{
				//�_���[�W��^����
				//enemy->ApplyDamage(1, 0.5f);
				if (enemy->ApplyDamage(1, 0.5f))
				{
					//������΂�
					{
						DirectX::XMFLOAT3 impulse;	//impulse�F�Ռ�

						// �ǂꂮ�炢�̗͂Ő�����Ԃ�
						float power = 10.0f;

						// �G�̈ʒu - �e�̈ʒu�A���ꂼ��� xz ����
						float impX = enemy->GetPosition().x - projectile->GetPosition().x;
						float impZ = enemy->GetPosition().z - projectile->GetPosition().z;


						// ���K������������΂������x�N�g��
						float impLength = sqrtf(impX * impX + impZ * impZ);
						float vx = impX / impLength;
						float vz = impZ / impLength;

						// ������΂������x�N�g���ɗ͂�������i�X�P�[�����O)
						impulse.x = vx * power;
						impulse.y = power * 0.5f;
						impulse.z = vz * power;

#if 0
						//���Ɖ�
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

					//�q�b�g�G�t�F�N�g�Đ�
					{
						DirectX::XMFLOAT3 e = enemy->GetPosition();
						e.y += enemy->GetHeight() * 0.5f;
						hitEffect->Play(e);
					}
					//�e�۔j��
					projectile->Destroy();
				}
			}
		}

	}
}


//�ړ����͏���
bool Player::InputMove(float elapsedTime)
{
	//�i�s�x�N�g���擾
	DirectX::XMFLOAT3 moveVec = GetMoveVec();

	//�ړ�����
	Move(moveVec.x, moveVec.z, moveSpeed);

	//���񏈗�
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

//���n�����Ƃ��ɌĂ΂��
void Player::OnLanding()
{
	jumpCount = 0;

	//�������̑��͂����ȏ�Ȃ璅�n�X�e�[�g��
	if (velocity.y < gravity * 5.0f)
	{
		if (state != State::Damage && state != State::Death)
		{
			//���n�X�e�[�g�֑J��
			TransitionLandState();
		}
	}
}

void Player::OnDamaged()
{
	//�_���[�W�X�e�[�g�֑J��
	TransitionDamageState();
}

void Player::OnDead()
{
	//���S�X�e�[�g�֑J��
	TransitionDeathState();
}

//�`�揈��
void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
{
	shader->Draw(dc, model);

	//�e�ە`�揈��
	projectileManager.Render(dc, shader);
}

//�f�o�b�O�v���~�e�B�u�`��
void Player::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//�Փ˔���p�̃f�o�b�O����`��
	//debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));

	//�Փ˔���p�̃f�o�b�O�~����`��
	debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));

	//�e�ۃf�o�b�O�v���~�e�B�u�`��
	debugRenderer->DrawSphere(position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));

	//�U���Փ˗p�̍���m�[�h�̃f�o�b�O����`��
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
		//�g�����X�t�H�[��
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::InputFloat3("Position", &position.x);
			ImGui::InputFloat3("Velocity", &velocity.x);
			//��]
			DirectX::XMFLOAT3 a;
			a.x = DirectX::XMConvertToDegrees(angle.x);
			a.y = DirectX::XMConvertToDegrees(angle.y);
			a.z = DirectX::XMConvertToDegrees(angle.z);
			ImGui::InputFloat3("Angle", &a.x);
			angle.x = DirectX::XMConvertToRadians(a.x);
			angle.y = DirectX::XMConvertToRadians(a.y);
			angle.z = DirectX::XMConvertToRadians(a.z);
			//�X�P�[��
			ImGui::InputFloat3("Scale", &scale.x);
		}
		
	}
	ImGui::End();
}


void Player::TransitionState()
{
	state = State::Idle;

	//�ҋ@�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Idle, true);
}

void Player::UpdateIdleState(float elapsedTime)
{
	//�ړ����͏���(true��������ړ��X�e�[�g�֑J��)
	if (InputMove(elapsedTime))
	{
		//�ړ��X�e�[�g�֑J��
		TransitionMoveState();
	}

	//�W�����v���͏���(true��������W�����v�X�e�[�g�֑J��)
	if (InputJump())
	{
		//�W�����v�X�e�[�g�֑J��
		TransitionJumpState();
	}

	//�e�ۓ��͏���
	InputProjectile();

	//�U�����͏���(true��������U���X�e�[�g�֑J��)
	if (InputAttack())
	{
		TransitionAttackState();
	}
}

void Player::TransitionMoveState()
{
	state = State::Move;

	//����A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Running, true);
}

void Player::UpdateMoveState(float elapsedTime)
{
	//�ړ����͏���(false��������ҋ@�X�e�[�g�֑J��)
	if (!InputMove(elapsedTime))
	{
		TransitionState();
	}

	//�W�����v���͏���
	if (InputJump())
	{
		//�W�����v�X�e�[�g�֑J��
		TransitionJumpState();
	}

	//�e�ۓ��͏���
	InputProjectile();

	//�U�����͏���
	if (InputAttack())
	{
		TransitionAttackState();
	}
}

void Player::TransitionJumpState()
{
	state = State::Jump;

	//�W�����v�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Jump, false);
}

void Player::UpdateJumpState(float elapsedTime)
{
	//�ړ����͏���(false��������ҋ@�X�e�[�g�֑J��)
	InputMove(elapsedTime);

	//�W�����v���͏���
	InputJump();

	//�W�����v�A�j���[�V�����I����(���Ă�����)
	if (!model->IsPlayAnimation())
	{
		//�����A�j���[�V�������Đ�
		model->PlayAnimation(Anim_Falling, true);
	}

	//�e�ۓ��͏���
	InputProjectile();
}

void Player::TransitionLandState()
{
	state = State::Land;

	//���n�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Landing, false);
}

void Player::UpdateLandState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		//�ҋ@�X�e�[�g�֑J��
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

	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	//���f�����猻�݂̃A�j���[�V�����Đ����Ԃ��擾
	float animationTime = model->GetCurrentAnimationSeconds();
	//animationTime��0.3���傫���A0.4��菬����
	attackCollitionFlag = animationTime > 0.3f && animationTime < 0.4f;

	if (attackCollitionFlag)
	{
		CollisionNodeVsEnemies("mixamorig:LeftHand", leftHandRadius);
	}

}

void Player::TransitionDamageState()
{
	state = State::Damage;
	//�_���[�W�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_GetHit1, false);
}

void Player::UpdateDamageState(float elapsedTime)
{
	if (!model->IsPlayAnimation())
	{
		//�ҋ@�X�e�[�g�֑J��
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
	//�{�^�����������畜���X�e�[�g�֑J��
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
		//�ҋ@�X�e�[�g�֑J��
		TransitionState();
	}
}
