#pragma once

class BaseScene;
class EnemyBase;

class SceneManager
{
public :

	// シーンの種類。
	enum class SceneType
	{
		Title,
		Game,
	};

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	// 次のシーンを予約する。切り替えは次のフレームの先頭で行う。
	void SetNextScene(SceneType _nextScene)
	{
		m_nextSceneType = _nextScene;
	}

	// 現在のシーンのオブジェクト一覧を取得する。
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList();

	void SetActiveEnemies(const std::vector<std::weak_ptr<EnemyBase>>& enemies);
	const std::vector<std::weak_ptr<EnemyBase>>& GetActiveEnemies() const { return m_activeEnemies; }

	// 現在のシーンにオブジェクトを追加する。
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

private :

	// シーン管理を初期化する。
	void Init()
	{
		// 開始シーンを作成する。
		ChangeScene(m_currentSceneType);
	}

	// 指定された種類のシーンに切り替える。
	void ChangeScene(SceneType _sceneType);

	// 現在のシーンを保持する共有ポインター。
	std::shared_ptr<BaseScene> m_currentScene = nullptr;

	// 現在のシーンの種類。
	SceneType m_currentSceneType = SceneType::Title;
	
	// 次に切り替えるシーンの種類。
	SceneType m_nextSceneType = m_currentSceneType;

	std::vector<std::weak_ptr<EnemyBase>> m_activeEnemies;

private:

	SceneManager() { Init(); }
	~SceneManager() {}

public:

	// シーン管理を一か所で共有するための取得関数。
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};
