#pragma once

class BaseScene
{
public:

	BaseScene() { Init(); }
	virtual ~BaseScene() {}

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	virtual void DrawSprite();
	void DrawDebug();

	const std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}

	void AddObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_objList.push_back(_obj);
	}

protected:

	virtual void Event();
	virtual void Init();
	virtual bool IsUpdatePaused() const { return false; }
	virtual bool CanUpdateWhenPaused(const std::shared_ptr<KdGameObject>&) const { return false; }

	std::list<std::shared_ptr<KdGameObject>> m_objList;

private:

	bool m_isDebugWireVisible = false;
	bool m_prevDebugWireKey = false;
};

