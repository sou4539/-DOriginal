#pragma once

#include "../CharaBase.h"

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

// 発射に必要な値をまとめた設定。呼び出し側は必要な項目だけ指定する。
// 共通項目はBase、専用項目は各魔法が読む。魔法の種類は生成するクラスで決まる。
struct MagicShotParams
{
	Math::Vector3 startPos = Math::Vector3::Zero; // 発射位置
	Math::Vector3 dir = Math::Vector3::Zero; // 方向。Shotで正規化する
	float damage = 0.0f; // ダメージ量（倍率ではない）
	float speed = 0.0f; // 1フレームの移動量
	std::shared_ptr<KdGameObject> chantTarget; // 詠唱中の追従先。未指定なら追従しない
	std::shared_ptr<KdGameObject> flyTarget; // 発射時の照準先。未指定ならdirを使う
	std::shared_ptr<KdGameObject> ignoreTarget; // 当たり判定から除外する敵

	// 氷専用：貫通数、分散数、分散で生まれた弾かどうか。
	int pierceCount = 1;
	int splitCount = 0;
	bool isSplitShot = false;
	// 雷専用：残り連鎖数、連鎖で生まれた弾かどうか。
	int chainCount = 0;
	bool isChainShot = false;
	// 炎専用：直撃した敵を中心とした爆発範囲。
	float explosionRadius = 3.0f;
};

class MagicBase : public CharaBase
{
public:
	MagicBase() { Init(); }
	~MagicBase() override {}

	void Init();
	void Update();
	void PostUpdate();
	void DrawLit() override;

	// 全魔法共通の入口。派生クラスではShotを再定義しない。
	// 共通設定 → 専用設定 → 画像・寿命設定 → 即時発射判定、の順序を保証する。
	void Shot(const MagicShotParams& params);

protected:
	// Shotから呼ぶ仮想関数。実体がIceMagicならIceMagicの実装が呼ばれる。
	// trueは詠唱省略を意味する。ここではStartFlyや画像設定は行わない。
	virtual bool ConfigureShot(const MagicShotParams&) { return false; }

	// 派生クラスで、寿命・当たり判定・画像などを設定する。
	virtual void SetupMagic();

	// 魔法ごとに必要な更新だけを派生クラスで追加する。
	virtual void UpdateChantMagic() {}
	virtual void UpdateFlyMagic() {}
	virtual void UpdateHitMagic();
	virtual bool IsReadyToFly() const { return m_chant <= 0.0f; }

	// 命中した時に、ダメージ前後へ魔法ごとの処理を差し込む。
	virtual void OnBeforeDamage(const std::shared_ptr<EnemyBase>&) {}
	virtual void OnAfterDamage(const std::shared_ptr<EnemyBase>&) {}
	virtual bool ShouldKeepFlyingAfterHit(const std::shared_ptr<EnemyBase>&) { return false; }

	// 命中演出を開始できる魔法だけtrueを返す。
	virtual bool StartHitAnimation() { return false; }

	// 魔法ごとの音パスと画像向き補正。
	virtual const char* GetShotSoundPath() const { return ""; }
	virtual const char* GetHitSoundPath() const { return ""; }
	virtual float GetDirectionAngleOffset() const { return 0.0f; }
	virtual Math::Vector3 GetEmissiveColor() const { return Math::Vector3::Zero; }

	// 派生クラスから使う共通処理。
	void StartFly();
	void StartHit();
	void UpdateFrameAnimation();
	void SetFrameTexture(int frameIndex);
	bool HasHitObject(const std::shared_ptr<KdGameObject>& obj) const;
	void AddHitObject(const std::shared_ptr<KdGameObject>& obj);

	// 状態ごとの更新処理。
	void UpdateChant();
	void UpdateFly();
	void UpdateHit();

	// 現在位置・向きから描画用ワールド行列を作る。
	void UpdateWorldMatrix();

	// 音再生の共通入口。
	void PlayShotSound();
	void PlayHitSound();

	MagicState m_state = MagicState::Chant;

	// 詠唱中だけ追従する対象。
	std::weak_ptr<KdGameObject> m_wpChantTarget;
	Math::Vector3 m_chantOffset = Math::Vector3::Zero;

	// 発射時に向き直す対象。
	std::weak_ptr<KdGameObject> m_wpFlyTarget;

	// 連鎖魔法で、直前に当たった敵をもう一度狙わないための除外対象。
	std::weak_ptr<KdGameObject> m_wpIgnoreTarget;

	// すでに当たった敵の記録。
	std::vector<std::weak_ptr<KdGameObject>> m_hitObjects;

	float m_damage = 0.0f;
	float m_speed = 0.0f;
	float m_lifeFrames = 0.0f;
	float m_radius = 0.0f;

	// 2.5DOriginalと同じく、1.0から0.0へ減らして詠唱完了を表す。
	float m_chant = 1.0f;
	float m_chantSpeed = 0.05f;

	// 複数画像のアニメーション用。
	std::vector<std::string> m_framePaths;
	float m_animFrame = 0.0f;
	float m_frameSpeed = 0.15f;
	int m_frameIndex = -1;

};
