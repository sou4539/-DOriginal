#include "Framework/KdFramework.h"

#include "KdInstancedModelRenderer.h"

namespace
{
	bool IsSameColor(const Math::Color& lhs, const Math::Color& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
	}

	bool IsSameVector(const Math::Vector3& lhs, const Math::Vector3& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}

	// 効果が異なる個体を同じ描画にまとめると見た目が混ざるため比較する。
	bool IsSameEffects(const KdModelVisualEffects& a, const KdModelVisualEffects& b)
	{
		return a.RimLight == b.RimLight && IsSameVector(a.RimColor, b.RimColor) &&
			a.RimPower == b.RimPower && a.AlphaDither == b.AlphaDither &&
			a.Alpha == b.Alpha && a.DistanceFade == b.DistanceFade;
	}
	std::vector<KdStandardShader::InstanceData> MakeInstanceData(
		const std::vector<Math::Matrix>& worlds,
		size_t begin,
		size_t count)
	{
		std::vector<KdStandardShader::InstanceData> instances;
		instances.reserve(count);

		for (size_t i = 0; i < count; ++i)
		{
			KdStandardShader::InstanceData instance;
			instance.World = worlds[begin + i];
			instances.push_back(instance);
		}

		return instances;
	}
}

// インスタンシング描画29
void KdModelInstanceBatcher::SubmitLit(
	const std::shared_ptr<KdModelData>& model,
	const Math::Matrix& world,
	const Math::Color& color,
	const Math::Vector3& emissive,
	const float dissolve, const KdModelVisualEffects& effects)
{
	if (!model) { return; }

	for (StaticBatch& batch : m_litStaticBatches)
	{
		if (batch.Model.get() == model.get() &&
			IsSameColor(batch.Color, color) &&
			IsSameVector(batch.Emissive, emissive)&&
			batch.Dissolve == dissolve && IsSameEffects(batch.Effects, effects))
		{
			batch.Worlds.push_back(world);
			return;
		}
	}

	StaticBatch batch;
	batch.Model = model;
	batch.Color = color;
	batch.Emissive = emissive;
	batch.Worlds.push_back(world);
	batch.Dissolve = dissolve;
	batch.Effects = effects;
	m_litStaticBatches.push_back(std::move(batch));
}

void KdModelInstanceBatcher::SubmitLit(
	const std::shared_ptr<KdModelWork>& model,
	const Math::Matrix& world,
	const Math::Color& color,
	const Math::Vector3& emissive,
	const float dissolve, const KdModelVisualEffects& effects
)
{
	if (!model || !model->IsEnable()) { return; }

	for (SkinnedBatch& batch : m_litSkinnedBatches)
	{
		if (batch.Model.get() == model.get() &&
			IsSameColor(batch.Color, color) &&
			IsSameVector(batch.Emissive, emissive)&&
			batch.Dissolve == dissolve && IsSameEffects(batch.Effects, effects))
		{
			batch.Worlds.push_back(world);
			return;
		}
	}

	SkinnedBatch batch;
	batch.Model = model;
	batch.Color = color;
	batch.Emissive = emissive;
	batch.Worlds.push_back(world);
	batch.Dissolve = dissolve;
	batch.Effects = effects;
	m_litSkinnedBatches.push_back(std::move(batch));
}

void KdModelInstanceBatcher::SubmitDepth(
	const std::shared_ptr<KdModelData>& model,
	const Math::Matrix& world)
{
	if (!model) { return; }

	for (StaticBatch& batch : m_depthStaticBatches)
	{
		if (batch.Model.get() == model.get())
		{
			batch.Worlds.push_back(world);
			return;
		}
	}

	StaticBatch batch;
	batch.Model = model;
	batch.Worlds.push_back(world);
	m_depthStaticBatches.push_back(std::move(batch));
}

void KdModelInstanceBatcher::SubmitDepth(
	const std::shared_ptr<KdModelWork>& model,
	const Math::Matrix& world)
{
	if (!model || !model->IsEnable()) { return; }

	for (SkinnedBatch& batch : m_depthSkinnedBatches)
	{
		if (batch.Model.get() == model.get())
		{
			batch.Worlds.push_back(world);
			return;
		}
	}

	SkinnedBatch batch;
	batch.Model = model;
	batch.Worlds.push_back(world);
	m_depthSkinnedBatches.push_back(std::move(batch));
}

void KdModelInstanceBatcher::FlushLit()
{
	if (m_litStaticBatches.empty() && m_litSkinnedBatches.empty()) { return; }

	auto& shader = KdShaderManager::Instance().m_StandardShader;
	shader.BeginLitInstanced();

	for (StaticBatch& batch : m_litStaticBatches)
	{
		for (size_t begin = 0; begin < batch.Worlds.size(); begin += KdStandardShader::MaxInstanceCount)
		{
			const size_t count = std::min<size_t>(KdStandardShader::MaxInstanceCount, batch.Worlds.size() - begin);
			const auto instances = MakeInstanceData(batch.Worlds, begin, count);
			// 描画の直前に保存した効果を戻す。無効なバッチには効果を引き継がない。
			shader.SetLimLightEnable(batch.Effects.RimLight);
			shader.SetLimLight(batch.Effects.RimColor, batch.Effects.RimPower);
			shader.SetAlphaDither(batch.Effects.AlphaDither, batch.Effects.Alpha, batch.Effects.DistanceFade);
			shader.SetDissolve(batch.Dissolve);
			shader.DrawModelInstanced(*batch.Model, instances, batch.Color, batch.Emissive);
		}
	}

	for (SkinnedBatch& batch : m_litSkinnedBatches)
	{
		for (size_t begin = 0; begin < batch.Worlds.size(); begin += KdStandardShader::MaxInstanceCount)
		{
			const size_t count = std::min<size_t>(KdStandardShader::MaxInstanceCount, batch.Worlds.size() - begin);
			const auto instances = MakeInstanceData(batch.Worlds, begin, count);
			// 描画の直前に保存した効果を戻す。無効なバッチには効果を引き継がない。
			shader.SetLimLightEnable(batch.Effects.RimLight);
			shader.SetLimLight(batch.Effects.RimColor, batch.Effects.RimPower);
			shader.SetAlphaDither(batch.Effects.AlphaDither, batch.Effects.Alpha, batch.Effects.DistanceFade);
			shader.SetDissolve(batch.Dissolve);
			shader.DrawModelInstanced(*batch.Model, instances, batch.Color, batch.Emissive);
		}
	}

	shader.EndLitInstanced();
	shader.BeginLit();

	m_litStaticBatches.clear();
	m_litSkinnedBatches.clear();
}

void KdModelInstanceBatcher::FlushDepth()
{
	if (m_depthStaticBatches.empty() && m_depthSkinnedBatches.empty()) { return; }

	auto& shader = KdShaderManager::Instance().m_StandardShader;
	shader.BeginGenerateDepthMapFromLightInstanced();

	for (StaticBatch& batch : m_depthStaticBatches)
	{
		for (size_t begin = 0; begin < batch.Worlds.size(); begin += KdStandardShader::MaxInstanceCount)
		{
			const size_t count = std::min<size_t>(KdStandardShader::MaxInstanceCount, batch.Worlds.size() - begin);
			const auto instances = MakeInstanceData(batch.Worlds, begin, count);
			shader.DrawModelInstanced(*batch.Model, instances);
		}
	}

	for (SkinnedBatch& batch : m_depthSkinnedBatches)
	{
		for (size_t begin = 0; begin < batch.Worlds.size(); begin += KdStandardShader::MaxInstanceCount)
		{
			const size_t count = std::min<size_t>(KdStandardShader::MaxInstanceCount, batch.Worlds.size() - begin);
			const auto instances = MakeInstanceData(batch.Worlds, begin, count);
			shader.DrawModelInstanced(*batch.Model, instances);
		}
	}

	shader.EndGenerateDepthMapFromLightInstanced();
	m_depthStaticBatches.clear();
	m_depthSkinnedBatches.clear();
}

void KdModelInstanceBatcher::Clear()
{
	m_litStaticBatches.clear();
	m_litSkinnedBatches.clear();
	m_depthStaticBatches.clear();
	m_depthSkinnedBatches.clear();
}
