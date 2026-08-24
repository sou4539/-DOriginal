#pragma once

#include "../MagicBase.h"

class VoltMagic : public MagicBase
{
public:
	VoltMagic() { Init(); }
	~VoltMagic() override {}

	// 雷魔法専用の発射設定。連鎖回数と連鎖弾かどうかをここで受け取る.
	void Shot(
		const Math::Vector3& startPos,
		const Math::Vector3& dir,
		MagicType type,
		float damage,
		float speed,
		const std::shared_ptr<KdGameObject>& chantTarget,
		const std::shared_ptr<KdGameObject>& flyTarget,
		const std::shared_ptr<KdGameObject>& ignoreTarget,
		int chainCount,
		bool isChainShot);

protected:
	// 雷弾の画像、寿命、当たり判定サイズを設定する.
	void SetupMagic() override;

	// 詠唱中の雷画像アニメーションを進める.
	void UpdateChantMagic() override;

	// 飛行中の雷画像アニメーションを進める.
	void UpdateFlyMagic() override;

	// 命中後に、近くの別の敵へ連鎖弾を作る.
	void OnAfterDamage(const std::shared_ptr<EnemyBase>& hitEnemy) override;

	// 炎魔法の発射音を返す.
	const char* GetShotSoundPath() const override;

private:
	// 命中した敵から次の敵へ飛ぶ連鎖弾を生成する.
	void CreateChain(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 連鎖範囲内で一番近い次の敵を探す.
	std::shared_ptr<EnemyBase> SearchChainTarget(const std::shared_ptr<EnemyBase>& hitEnemy);

	// あと何回連鎖できるか.
	int m_chainCount = 0;

	// trueなら通常弾ではなく、連鎖で生まれた雷弾.
	bool m_isChainShot = false;

	// 次の連鎖対象を探す範囲.
	float m_chainRadius = 8.0f;
};
