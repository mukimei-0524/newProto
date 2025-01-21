#pragma once

#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "ProjectileManager.h"
#include "Effect.h"

//�v���C���[
class Player :public Character
{
private:
	//�A�j���[�V����
	enum Animation
	{
		Anim_Attak,
		Anim_Death,
		Anim_Falling,
		Anim_GetHit1,
		Anim_GetHit2,
		Anim_Idle,
		Anim_Jump,
		Anim_Jump_Flip,
		Anim_Landing,
		Anim_Revive,
		Anim_Running,
		Anim_Walking
	};

	enum class State
	{
		Idle,
		Move,
		Jump,
		Land,
		Attack,
		Damage,
		Death,
		Revive
	};
public:
	Player();
	~Player() override;

	//�C���X�^���X�擾(��΂ɐ���ł͎g��Ȃ�)
	static Player& Instance();

	//�X�V����
	void Update(float elapsedTime);

	//�`�揈��
	void Render(ID3D11DeviceContext* dc, Shader* shader);

	//�f�o�b�O�pGUI�`��
	void DrawDebugGUI();

	//�f�o�b�O�v���~�e�B�u�`��
	void DrawDebugPrimitive();

	//�W�����v���͏���
	bool InputJump();

	//�e�ۂƓG�̏Փˏ���
	void CollistionProjectilesVsEnemies();

protected:
	//���n�����Ƃ��ɌĂ΂��
	void OnLanding() override;

	//�_���[�W���󂯂����ɌĂ΂��
	void OnDamaged()override;

	//���S�����Ƃ��ɌĂ΂��
	void OnDead()override;

private:
	Model* model = nullptr;
	Effect* hitEffect = nullptr;
	float turnSpeed = DirectX::XMConvertToRadians(720);
	float moveSpeed = 5.0f;

	float jumpSpeed = 20.0f;
	
	int jumpCount = 0;
	int jumpLimit = 2;

	State state = State::Idle;

	ProjectileManager projectileManager;

	float leftHandRadius = 0.4f;

	bool attackCollitionFlag = false;

	//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
	DirectX::XMFLOAT3 GetMoveVec() const;

	//���͏���
	//void InputMove(float elapsedTime);
	bool InputMove(float elapsedTime);

	//�U�����͏���
	bool InputAttack();

	//�v���C���[�ƃG�l�~�[�̏Փˏ���
	void CollisionPlayerVsEnemies();

	//�m�[�h�ƃG�l�~�[�̏Փˏ���
	void CollisionNodeVsEnemies(const char* nodeName, float nodeRadius);

	//�e�ۓ��͏���
	void InputProjectile();

	//�ҋ@�X�e�[�g�֑J��
	void TransitionState();

	//�ҋ@�X�e�[�g�X�V����
	void UpdateIdleState(float elapsedTime);

	//�ړ��X�e�[�g�֑J��
	void TransitionMoveState();

	//�ړ��X�e�[�g�X�V����
	void UpdateMoveState(float elapsedTime);

	//�W�����v�X�e�[�g�֑J��
	void TransitionJumpState();

	//�W�����v�X�e�[�g�X�V����
	void UpdateJumpState(float elapsedTime);

	//���n�X�e�[�g�֑J��
	void TransitionLandState();
	
	//���n�X�e�[�g�X�V����
	void UpdateLandState(float elapsedTime);

	//�U���X�e�[�g�֑J��
	void TransitionAttackState();

	//�U���X�e�[�g�X�V����
	void UpdateAttackState(float elapsedTime);
	
	//�_���[�W�X�e�[�g�֑J��
	void TransitionDamageState();

	//�_���[�W�X�e�[�g�X�V����
	void UpdateDamageState(float elapsedTime);
	
	//���S�X�e�[�g�֑J��
	void TransitionDeathState();

	//���S�X�e�[�g�X�V����
	void UpdateDeathState(float elapsedTime);
	
	//�����X�e�[�g�֑J��
	void TransitionReviveState();

	//�����X�e�[�g�X�V����
	void UpdateReviveState(float elapsedTime);
};

//elapsedTime:�o�ߎ���