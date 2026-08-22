#pragma once

#include "../../CharaBase.h"

#include <string>
#include <vector>

class Bat;

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
	// StaffBaseから、開始位置・方向・魔法種類・ダメージ・速度を受け取る。
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
	// 発射音やヒット音など、状態が変わった瞬間だけ行いたい処理はここに集める。
	void StartFly();
	void StartHit();

	// 複数画像を使う魔法のアニメーションを進める。
	void UpdateFrameAnimation();

	// 現在位置・向きから描画用ワールド行列を作る。
	void UpdateWorldMatrix();

	// 画像パス配列の指定番号をm_spPolyへ反映する。
	void SetFrameTexture(int frameIndex);

	// 魔法ごとの音再生処理。
	// 音素材を追加したら、GetShotSoundPath / GetHitSoundPath の戻り値にパスを入れる。
	void PlayShotSound();
	void PlayHitSound();
	const char* GetShotSoundPath() const;
	const char* GetHitSoundPath() const;

	// 雷魔法の連鎖処理。
	// 当たったコウモリの近くに別のコウモリがいれば、同じVolt画像の魔法弾を追加で飛ばす。
	void CreateVoltChain(const std::shared_ptr<Bat>& hitBat);
	std::shared_ptr<Bat> SearchVoltChainTarget(const std::shared_ptr<Bat>& hitBat);

	// 氷魔法の派生弾生成処理。
	// 通常の氷弾が敵に当たった時だけ、半分ダメージの氷弾を1世代だけ追加で出す。
	void CreateIceSplit(const std::shared_ptr<Bat>& hitBat);

	// 炎魔法の爆発処理。
	// 命中した敵の周囲にいるコウモリにも同じダメージを与える。
	void ApplyFireExplosion(const std::shared_ptr<Bat>& hitBat);

	// 氷の貫通処理で、同じ敵に毎フレーム当たり続けないように確認する。
	bool HasHitObject(const std::shared_ptr<KdGameObject>& obj) const;
	void AddHitObject(const std::shared_ptr<KdGameObject>& obj);

	MagicType m_magicType = MagicType::None;
	MagicState m_state = MagicState::Chant;

	// 詠唱中だけ追従する対象。
	// 現在は杖を渡し、杖の上で詠唱し続けるようにする。
	std::weak_ptr<KdGameObject> m_wpChantTarget;
	Math::Vector3 m_chantOffset = Math::Vector3::Zero;

	// 発射時に向き直す対象。
	// 詠唱中に敵や杖が動いても、発射する瞬間の位置から敵へ飛ばせるようにする。
	std::weak_ptr<KdGameObject> m_wpFlyTarget;

	// 連鎖魔法で、直前に当たった敵をもう一度狙わないための除外対象。
	std::weak_ptr<KdGameObject> m_wpIgnoreTarget;

	// 雷の残り連鎖回数。
	// 初期状態でも1回は連鎖するため、VoltStaffから1を渡す。
	int m_voltChainCount = 0;
	float m_voltChainRadius = 8.0f;

	// trueなら雷の連鎖用に生成された魔法。
	// 通常の雷弾はLightning画像、連鎖時は線のようなVolt画像を使い分ける。
	bool m_isChainShot = false;

	// 炎の爆発範囲。
	// レベルアップで炎を選ぶたびにStatus側の値が伸びる。
	float m_fireExplosionRadius = 3.0f;

	// 氷の残り貫通数。
	// 敵に当たるたびに減り、0になったら消える。
	int m_icePierceCount = 1;

	// 氷の派生弾数。
	// 通常の氷弾が初めて敵に当たった時、この数だけ半分ダメージの弾を出す。
	int m_iceSplitCount = 1;

	// trueなら氷の派生弾として生成された魔法。
	// 派生弾からさらに派生弾を出さないために使う。
	bool m_isIceSplitShot = false;

	// 通常の氷弾が、すでに派生弾を出したかどうか。
	// 貫通中に複数の敵へ当たっても、派生は1回だけにする。
	bool m_hasCreatedIceSplit = false;

	// すでに当たった敵の記録。
	// 特に氷の貫通弾が、同じ敵へ何度も連続ヒットするのを防ぐ。
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















