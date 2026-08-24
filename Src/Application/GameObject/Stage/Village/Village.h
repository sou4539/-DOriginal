#pragma once

#include "../StageBase.h"

class Village :public StageBase
{
public:
	Village() { Init(); }
	~Village() override {}

	void Update() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;
	void DrawEffect() override;

	// ���͕ǔ���ɂ��g���B
	bool EnableSphereCollision() const override { return true; }

	// ���S�n�т̒��S���W�B
	const Math::Vector3& GetSafeAreaCenter() const { return m_safeAreaCenter; }

	// ���S�n�т̔��a�B
	float GetSafeAreaRadius() const { return m_safeAreaRadius; }

	// �����f�����\������Ă���Ԃ͖����������߂Ɏg���B
	float GetVisibleRadius() const { return m_visibleRadius; }

	// ���̕\������Ɏg���Ώۂ�ݒ肷��B
	void SetTarget(const std::shared_ptr<KdGameObject>& target)
	{
		m_wpTarget = target;
	}

	// ����\�����鋗����ݒ肷��B
	void SetVisibleRadius(float radius) { m_visibleRadius = radius; }

private:
	void Init() override;

	// �����\���͈͓����m�F����B
	bool IsInVisibleRange() const;

	// ���S�̂𕢂����S�n�сB
	Math::Vector3 m_safeAreaCenter = Math::Vector3::Zero;
	float m_safeAreaRadius = 46.0f;

	// �\���������m�F����ΏہB
	std::weak_ptr<KdGameObject> m_wpTarget;

	// ����\�����锼�a�B
	float m_visibleRadius = 85.0f;
};


