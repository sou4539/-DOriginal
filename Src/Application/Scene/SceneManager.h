#pragma once

class BaseScene;
class EnemyBase;

class SceneManager
{
public :

	// �V�[�����
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

	// ���̃V�[�����Z�b�g (���̃t���[������؂�ւ��)
	void SetNextScene(SceneType _nextScene)
	{
		m_nextSceneType = _nextScene;
	}

	// ���݂̃V�[���̃I�u�W�F�N�g���X�g���擾
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList();

	void SetActiveEnemies(const std::vector<std::weak_ptr<EnemyBase>>& enemies);
	const std::vector<std::weak_ptr<EnemyBase>>& GetActiveEnemies() const { return m_activeEnemies; }

	// ���݂̃V�[���ɃI�u�W�F�N�g��ǉ�
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

private :

	// �}�l�[�W���[�̏�����
	void Init()
	{
		// �J�n�V�[���ɐ؂�ւ�
		ChangeScene(m_currentSceneType);
	}

	// �V�[���؂�ւ��֐�
	void ChangeScene(SceneType _sceneType);

	// ���݂̃V�[���̃C���X�^���X��ێ����Ă���|�C���^
	std::shared_ptr<BaseScene> m_currentScene = nullptr;

	// ���݂̃V�[���̎�ނ�ێ����Ă���ϐ�
	SceneType m_currentSceneType = SceneType::Title;
	
	// ���̃V�[���̎�ނ�ێ����Ă���ϐ�
	SceneType m_nextSceneType = m_currentSceneType;

	std::vector<std::weak_ptr<EnemyBase>> m_activeEnemies;

private:

	SceneManager() { Init(); }
	~SceneManager() {}

public:

	// �V���O���g���p�^�[��
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};
