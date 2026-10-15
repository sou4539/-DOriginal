#pragma once

class ReturnCircle : public KdGameObject
{
public:
	ReturnCircle()
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/GameObjects/Warp/WarpMagic.gltf");
	}
	void SetVisible(bool visible)
	{
		m_visible = visible;
	}

	// progress: 0で詠唱開始、1で詠唱完了
	void SetProgress(float progress)
	{
		progress = std::clamp(progress, 0.0f, 1.0f);
		m_dissolve = 1.0 - progress;
	}

	void DrawEffect() override
	{
		if (!m_visible || !m_spModel) { return; }

		auto& shader = KdShaderManager::Instance().m_StandardShader;

		const float edgeRenge = 0.05f;
		const Math::Vector3 edgeColor = { 0.3,0.7f,1.0f };
		
		m_mWorld = Math::Matrix::CreateScale(1.0f) * Math::Matrix::CreateTranslation(GetPos());

		shader.SetDissolve(m_dissolve, &edgeRenge, &edgeColor);
		auto& manager = KdShaderManager::Instance();

		manager.ChangeRasterizerState(KdRasterizerState::CullNone);
		shader.DrawModel(*m_spModel, m_mWorld);
		manager.UndoRasterizerState();

		// 描画後は元に戻す
		shader.SetDissolve(0.0f);
	}

private:
	std::shared_ptr<KdModelWork>m_spModel;
	bool m_visible = false;
	float m_dissolve = 1.0f;
};