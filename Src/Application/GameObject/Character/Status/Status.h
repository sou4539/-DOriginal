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

private:
	// レベルアップ処理。
	// AddExp()から呼び、プレイヤーの能力値を上げる。
	void LevelUp();

	// Status側からプレイヤーや敵の情報を参照したい時に使う。
	// weak_ptrにしておくことで、対象オブジェクトを勝手に生存させ続けない。
	std::weak_ptr<KdGameObject> m_player;
	std::weak_ptr<KdGameObject> m_enemy;

	// HPバー画像。
	// 今回はAsset/Textures/UIに置いた HP_Bar.png を左上のUIとして使う。
	std::shared_ptr<KdTexture> m_hpBarTex = nullptr;

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

	// 敵のステータス。
	// 今回はまだ使っていないが、今後コウモリやボスのHP管理に使える。
	float m_eHp = 50.0f;
	float m_eMp = 50.0f;
	float m_eAttack = 5.0f;
	float m_eDefense = 2.0f;
	float m_eSpeed = 0.5f;
};


