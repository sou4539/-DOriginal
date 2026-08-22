#pragma once

class Status : public KdGameObject
{
public:
	// Statusを作成した時に、自動でInit()を呼んでHPバー画像を読み込む。
	// GameSceneでは std::make_shared<Status>() するだけで使える。
	Status() { Init(); }

	// Status破棄時の処理。
	// shared_ptrで持っている画像は自動解放されるため、ここでは追加処理を持たせていない。
	~Status() {}

	// Statusの初期化処理。
	// HPバー画像の読み込みを行う。
	void Init();

	// 毎フレーム更新用。
	// 今は処理なし。今後、毒や自動回復など時間で変化するステータス処理を入れられる。
	void Update();

	// 2DのUI描画処理。
	// HPバー画像を画面左上に描画し、現在HPに合わせて赤ゲージを隠す。
	void DrawSprite() override;

	// Statusがプレイヤー情報を参照したい時に使う。
	// 現在はHP管理中心だが、今後位置や状態を見たい場合に使える。
	void SetPlayer(const std::weak_ptr<KdGameObject>& player) { m_player = player; }

	// Statusが敵情報を参照したい時に使う。
	// 現在は未使用だが、敵HPやボスHP表示を追加する時に使える。
	void SetEnemy(const std::weak_ptr<KdGameObject>& enemy) { m_enemy = enemy; }

	// プレイヤーのHPを減らす関数。
	// Player側でHPを直接変更せず、Statusに依頼する形にしている。
	// こうすると、HP表示や死亡判定をStatus側にまとめやすい。
	void DamagePlayer(float damage);

	// プレイヤーを復活させる時に、HPを最大値まで戻す。
	// 復活処理自体はPlayer側で行い、HPの管理だけStatus側で担当する。
	void ResetPlayerHp();

	// プレイヤーのHPが0になっているかを確認する。
	// 死亡判定の条件をStatus側にまとめることで、Player側の処理を読みやすくする。
	bool IsPlayerDead() const { return m_pHp <= 0.0f; }

	// UI表示やデバッグ表示で、現在のプレイヤーHPを確認したい時に使う。
	float GetPlayerHp() const { return m_pHp; }

	// 敵を倒した時に経験値を加算する。
	// 必要経験値を超えた場合は、内部でレベルアップ処理も行う。
	void AddExp(float exp);

	// 現在のレベルを確認したい時に使う。
	int GetLevel() const { return m_level; }

	// 現在の経験値を確認したい時に使う。
	float GetExp() const { return m_exp; }

	// 次のレベルまでに必要な経験値を確認したい時に使う。
	float GetNextExp() const { return m_nextExp; }

	// 魔法強化値を確認する関数。
	// StaffBaseやMagicBaseが、現在の強化状態に合わせて攻撃性能を変えるために使う。
	float GetFireExplosionRadius() const { return m_fireExplosionRadius; }
	int GetIceSplitCount() const { return m_iceSplitCount; }
	int GetIcePierceCount() const { return m_icePierceCount; }
	int GetVoltChainCount() const { return m_voltChainCount; }
	bool IsLevelUpSelect() const { return m_isLevelUpSelect; }

private:
	// レベルアップ処理。
	// AddExp()から呼び、プレイヤーの能力値を上げる。
	void LevelUp();

	// レベルアップ時に表示する魔法強化選択UIを描画する。
	// Back画像を3つ並べ、その上にFire/Ice/Voltの文字画像を1つずつ重ねる。
	void DrawLevelUpSelect();

	// レベルアップ選択UI中の入力処理。
	// 1:炎 2:氷 3:雷 を選んで、対応する強化値を伸ばす。
	void UpdateLevelUpSelect();
	void EnhanceFire();
	void EnhanceIce();
	void EnhanceVolt();
	void DrawExpBar();
	void DrawNumber(int value, int x, int y, int drawW, int drawH);

	// Status側からプレイヤーや敵の情報を参照したい時に使う。
	// weak_ptrにしておくことで、対象オブジェクトを勝手に生存させ続けない。
	std::weak_ptr<KdGameObject> m_player;
	std::weak_ptr<KdGameObject> m_enemy;

	// HPバー画像。
	// 今回はAsset/Textures/UIに置いた HP_Bar.png を左上のUIとして使う。
	std::shared_ptr<KdTexture> m_hpBarTex = nullptr;

	// レベルアップ選択UI用の画像。
	// m_levelUpBackTexを3枚描画し、その上に各魔法の文字画像を重ねる。
	std::shared_ptr<KdTexture> m_levelUpBackTex = nullptr;
	std::shared_ptr<KdTexture> m_fireUpTex = nullptr;
	std::shared_ptr<KdTexture> m_iceUpTex = nullptr;
	std::shared_ptr<KdTexture> m_voltUpTex = nullptr;
	std::shared_ptr<KdTexture> m_numberTex = nullptr;

	// プレイヤーのステータス。
	// m_pHp が現在HP、m_pMaxHp が最大HP。
	// HPバーの横幅は m_pHp / m_pMaxHp の割合で決める。
	float m_pHp = 100.0f;
	float m_pMaxHp = 100.0f;
	float m_pMp = 100.0f;
	float m_pAttack = 10.0f;
	float m_pDefense = 5.0f;
	float m_pSpeed = 1.0f;

	// プレイヤーの成長情報。
	// 敵を倒してm_expを増やし、m_nextExp以上になったらレベルアップする。
	int m_level = 1;
	float m_exp = 0.0f;
	float m_nextExp = 100.0f;

	// trueの間、レベルアップ時の魔法強化選択UIを画面に表示する。
	// 今は見た目確認用として表示だけ行い、選択処理は次の作業で追加する。
	bool m_isLevelUpSelect = false;

	// デバッグ用レベルアップキーの前フレーム状態。
	// 押しっぱなしで毎フレームLevelUpしないようにするために使う。
	bool m_prevDebugLevelUpKey = false;

	// レベルアップ選択キーの前フレーム状態。
	// 押しっぱなしで複数回強化されないようにする。
	bool m_prevSelectFireKey = false;
	bool m_prevSelectIceKey = false;
	bool m_prevSelectVoltKey = false;
	bool m_prevLeftClick = false;

	// 魔法ごとの強化値。
	// 最大値は設けず、選択するたびにそれぞれの特徴が伸びていく。
	float m_fireExplosionRadius = 3.0f;
	int m_iceSplitCount = 1;
	int m_icePierceCount = 1;
	int m_voltChainCount = 1;

	// 敵のステータス。
	// 今回はまだ使っていないが、今後コウモリやボスのHP管理に使える。
	float m_eHp = 50.0f;
	float m_eMp = 50.0f;
	float m_eAttack = 5.0f;
	float m_eDefense = 2.0f;
	float m_eSpeed = 0.5f;
};










