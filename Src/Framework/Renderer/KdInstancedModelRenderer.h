#pragma once

class KdModelData;
class KdModelWork;

// 一括描画では設定が同じ個体だけをまとめる。省略すると両方とも無効。
struct KdModelVisualEffects
{
	bool RimLight = false;
	Math::Vector3 RimColor = { 1, 1, 1 };
	float RimPower = 1.0f;
	bool AlphaDither = false;
	float Alpha = 0.4f; // 残す割合：0で消え、1で全部表示する
	bool DistanceFade = false; // trueならカメラの近くで点模様に抜く
};

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
		const float dissolve = 0.0f,
		const KdModelVisualEffects& effects = {});

	void SubmitLit(const std::shared_ptr<KdModelWork>& model, const Math::Matrix& world,
		const Math::Color& color = kWhiteColor,
		const Math::Vector3& emissive = Math::Vector3::Zero,
		const float dissolve = 0.0f,
		const KdModelVisualEffects& effects = {});

	void SubmitDepth(const std::shared_ptr<KdModelData>& model, const Math::Matrix& world);
	void SubmitDepth(const std::shared_ptr<KdModelWork>& model, const Math::Matrix& world);

	void FlushLit();
	void FlushDepth();
	void Clear();

private:
	struct StaticBatch
	{
		KdModelVisualEffects Effects;
		std::shared_ptr<KdModelData> Model;
		Math::Color Color = kWhiteColor;
		Math::Vector3 Emissive = Math::Vector3::Zero;
		std::vector<Math::Matrix> Worlds;
		float Dissolve = 0.0f;
	};

	struct SkinnedBatch
	{
		KdModelVisualEffects Effects;
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
