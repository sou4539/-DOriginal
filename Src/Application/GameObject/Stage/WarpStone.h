#pragma once

// 村と城で共通のワープストーン。移動判定はステージ側で管理する。
class WarpStone final : public KdGameObject
{
public:
	explicit WarpStone(const Math::Vector3& position)
	{
		m_model = std::make_shared<KdModelWork>();
		m_model->SetModelData("Asset/Models/Objects/GameObjects/WarpStone/WarpStone.gltf");
		m_mWorld = Math::Matrix::CreateScale(0.3f)
			* Math::Matrix::CreateTranslation(position);
	}

	void DrawLit() override
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
	}

	void GenerateDepthMapFromLight() override
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
	}

private:
	std::shared_ptr<KdModelWork> m_model;
};
