#pragma once

#include "../MagicBase.h"

class FireMagic : public MagicBase
{
public:
	FireMagic() { Init(); }
	~FireMagic() override {}

	// 炎魔法専用の発射設定。強化で変わる爆発範囲をここで受け取る.
	void Shot(
		const Math::Vector3& startPos,
		const Math::Vector3& dir,
		MagicType type,
		float damage,
		float speed,
		const std::shared_ptr<KdGameObject>& chantTarget,
		const std::shared_ptr<KdGameObject>& flyTarget,
		const std::shared_ptr<KdGameObject>& ignoreTarget,
		float explosionRadius);

protected:
	// 炎弾の画像、寿命、当たり判定サイズを設定する.
	void SetupMagic() override;

	// 詠唱中は飛行用の先頭フレームを表示する.
	void UpdateChantMagic() override;

	// 飛行中の炎スプライトシートをループ再生する.
	void UpdateFlyMagic() override;

	// 命中中の炎スプライトシートを最後まで再生する.
	void UpdateHitMagic() override;

	// 直撃ダメージ後に、周囲の敵へ爆発ダメージを与える.
	void OnAfterDamage(const std::shared_ptr<EnemyBase>& hitEnemy) override;

	// 命中演出へ入る準備を行う.
	bool StartHitAnimation() override;

	// 炎魔法の発射音を返す.
	const char* GetShotSoundPath() const override;

	// 炎魔法の命中音を返す.
	const char* GetHitSoundPath() const override;

	// 炎画像の向きを進行方向に合わせるための補正角度を返す.
	float GetDirectionAngleOffset() const override;

	// 炎らしく少し赤く発光させる.
	Math::Vector3 GetEmissiveColor() const override;

private:
	// 命中した敵を中心に、爆発範囲内の敵へ追加ダメージを与える.
	void ApplyExplosion(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 炎強化で広がる爆発範囲.
	float m_explosionRadius = 3.0f;

	// Fire.pngの飛行アニメーション範囲.
	int m_flyFrameStart = 0;
	int m_flyFrameEnd = 4;

	// Fire.pngの命中アニメーション範囲.
	int m_hitFrameStart = 5;
	int m_hitFrameEnd = 10;
};
