#pragma once

#include "../MagicBase.h"

class IceMagic : public MagicBase
{
public:
	IceMagic() { Init(); }
	~IceMagic() override {}

private:
	// BaseのShotから呼ばれる専用設定。戻り値trueなら詠唱を省略する。
	bool ConfigureShot(const MagicShotParams& params) override;

	// 氷弾の画像、寿命、当たり判定サイズを設定する.
	void SetupMagic() override;

	// 詠唱中の氷画像アニメーションを進める.
	void UpdateChantMagic() override;

	// 氷は3枚目の画像になったタイミングで発射する.
	bool IsReadyToFly() const override;

	// 通常氷弾が敵に当たった瞬間、ダメージ前に派生弾を作る.
	void OnBeforeDamage(const std::shared_ptr<EnemyBase>& hitEnemy) override;

	// 派生弾や貫通中の弾を、命中後も飛ばし続けるか判定する.
	bool ShouldKeepFlyingAfterHit(const std::shared_ptr<EnemyBase>& hitEnemy) override;

	// 発射音を返す.
	const char* GetShotSoundPath() const override;

	// 命中音を返す.
	const char* GetHitSoundPath() const override;


	// 命中した敵の位置から、左右へ広がる派生氷弾を生成する.
	bool CreateSplit(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 現在の進行方向をY軸回転させ、分散用の方向を作る.
	Math::Vector3 RotateDirY(const Math::Vector3& dir, float angle) const;

	// 通常氷弾があと何体まで貫通できるか.
	int m_pierceLeft = 1;

	// 通常氷弾が命中時に作る派生弾の数.
	int m_splitCount = 0;

	// trueなら通常弾ではなく、分散で生まれた派生弾.
	bool m_isSplitShot = false;

	// 通常氷弾が、この弾で分散を済ませたか.
	bool m_hasSplit = false;
};
