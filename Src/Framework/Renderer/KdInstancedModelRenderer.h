#pragma once

class KdModelData;
class KdModelWork;

// インスタンシング描画28
// Collect matching models and render them together at the end of each pass.
class KdModelInstanceBatcher
{
public:
	static KdModelInstanceBatcher& Instance()
	{
		static KdModelInstanceBatcher instance;
		return instance;
	}

	void SubmitLit(const std::shared_ptr<KdModelData>& model, const Math::Matrix& world,
		const Math::Color& color = kWhiteColor,
		const Math::Vector3& emissive = Math::Vector3::Zero,
		const float dissolve = 0.0f);

	void SubmitLit(const std::shared_ptr<KdModelWork>& model, const Math::Matrix& world,
		const Math::Color& color = kWhiteColor,
		const Math::Vector3& emissive = Math::Vector3::Zero,
		const float dissolve = 0.0f);

	void SubmitDepth(const std::shared_ptr<KdModelData>& model, const Math::Matrix& world);
	void SubmitDepth(const std::shared_ptr<KdModelWork>& model, const Math::Matrix& world);

	void FlushLit();
	void FlushDepth();
	void Clear();

private:
	struct StaticBatch
	{
		std::shared_ptr<KdModelData> Model;
		Math::Color Color = kWhiteColor;
		Math::Vector3 Emissive = Math::Vector3::Zero;
		std::vector<Math::Matrix> Worlds;
		float Dissolve = 0.0f;
	};

	struct SkinnedBatch
	{
		// Instances sharing one KdModelWork also share one animated bone pose.
		std::shared_ptr<KdModelWork> Model;
		Math::Color Color = kWhiteColor;
		Math::Vector3 Emissive = Math::Vector3::Zero;
		std::vector<Math::Matrix> Worlds;
		float Dissolve = 0.0f;
	};

	std::vector<StaticBatch> m_litStaticBatches;
	std::vector<SkinnedBatch> m_litSkinnedBatches;
	std::vector<StaticBatch> m_depthStaticBatches;
	std::vector<SkinnedBatch> m_depthSkinnedBatches;
};
