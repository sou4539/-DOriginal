#pragma once

class StageBase : public KdGameObject
{
public:
	StageBase(){}
	~StageBase(){}

	void DrawLit() override;

protected:

	std::shared_ptr<KdModelWork> m_spModel = nullptr;
};