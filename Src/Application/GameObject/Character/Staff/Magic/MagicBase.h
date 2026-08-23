#pragma once

#include "../../CharaBase.h"

#include <string>
#include <vector>

class EnemyBase;

enum class MagicType
{
	Fire,
	Ice,
	Volt,
	None
};

enum class MagicState
{
	Chant,	// 詠唱中：その場で演出だけ行い、まだ飛ばない。
	Fly,	// 飛行中：敵へ向かって移動し、当たり判定を行う。
	Hit		// 命中中：命中演出を行い、終わったら消える。
};

class MagicBase : public CharaBase
{
public:
	MagicBase() { Init(); }
	~MagicBase() {}

	void Init();
	void Update();
	void PostUpdate();
	void DrawLit();

	// 魔法を発射するための初期設定。
	void Shot(
		const Math::Vector3& startPos,
		const Math::Vector3& dir,
		MagicType type,
		float damage,
		float speed,
		const std::shared_ptr<KdGameObject>& chantTarget,
		const std::shared_ptr<KdGameObject>& flyTarget,
		int voltChainCount,
		const std::shared_ptr<KdGameObject>& ignoreTarget,
		bool isChainShot,
		float fireExplosionRadius,
		int icePierceCount,
		int iceSplitCount,
		bool isIceSplitShot);

private:
	// 魔法の種類に応じて、寿命・当たり判定・画像・アニメーションを設定する。
	void SetupMagic();

	// 状態ごとの更新処理。
	void UpdateChant();
	void UpdateFly();
	void UpdateHit();

	// 魔法の状態を切り替える処理。
	void StartFly();
	void StartHit();

	// 複数画像を使う魔法のアニメーションを進める。
	void UpdateFrameAnimation();

	// 現在位置・向きから描画用ワールド行列を作る。
	void UpdateWorldMatrix();

	// 画像パス配列の指定番号をm_spPolyへ反映する。
	void SetFrameTexture(int frameIndex);

	// 魔法ごとの音再生処理。
	void PlayShotSound();
	void PlayHitSound();
	const char* GetShotSoundPath() const;
	const char* GetHitSoundPath() const;

	// 雷魔法の連鎖処理。
	void CreateVoltChain(const std::shared_ptr<EnemyBase>& hitEnemy);
	std::shared_ptr<EnemyBase> SearchVoltChainTarget(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 氷魔法の派生弾生成処理。
	void CreateIceSplit(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 炎魔法の爆発処理。
	void ApplyFireExplosion(const std::shared_ptr<EnemyBase>& hitEnemy);

	// 氷の貫通処理で、同じ敵に毎フレーム当たり続けないように確認する。
	bool HasHitObject(const std::shared_ptr<KdGameObject>& obj) const;
	void AddHitObject(const std::shared_ptr<KdGameObject>& obj);

	MagicType m_magicType = MagicType::None;
	MagicState m_state = MagicState::Chant;

	// 詠唱中だけ追従する対象。
	std::weak_ptr<KdGameObject> m_wpChantTarget;
	Math::Vector3 m_chantOffset = Math::Vector3::Zero;

	// 発射時に向き直す対象。
	std::weak_ptr<KdGameObject> m_wpFlyTarget;

	// 連鎖魔法で、直前に当たった敵をもう一度狙わないための除外対象。
	std::weak_ptr<KdGameObject> m_wpIgnoreTarget;

	// 雷の残り連鎖回数。
	int m_voltChainCount = 0;
	float m_voltChainRadius = 8.0f;

	// trueなら雷の連鎖用に生成された魔法。
	bool m_isChainShot = false;

	// 炎の爆発範囲。
	float m_fireExplosionRadius = 3.0f;

	// 氷の残り貫通数。
	int m_icePierceCount = 1;

	// 氷の派生弾数。
	int m_iceSplitCount = 1;

	// trueなら氷の派生弾として生成された魔法。
	bool m_isIceSplitShot = false;

	// 通常の氷弾が、すでに派生弾を出したかどうか。
	bool m_hasCreatedIceSplit = false;

	// すでに当たった敵の記録。
	std::vector<std::weak_ptr<KdGameObject>> m_hitObjectList;

	float m_damage = 0.0f;
	float m_speed = 0.0f;
	float m_lifeTime = 0.0f;
	float m_radius = 0.0f;

	// 2.5DOriginalと同じく、1.0から0.0へ減らして詠唱完了を表す。
	float m_chant = 1.0f;
	float m_chantSpeed = 0.05f;

	// 複数画像のアニメーション用。
	std::vector<std::string> m_framePathList;
	float m_frame = 0.0f;
	float m_frameSpeed = 0.15f;
	int m_nowFrame = -1;

	// Fire.pngは横一列のスプライトシートなので、UV番号で表示部分を切り替える。
	int m_fireFlyFrameStart = 0;
	int m_fireFlyFrameEnd = 4;
	int m_fireHitFrameStart = 5;
	int m_fireHitFrameEnd = 10;
};















