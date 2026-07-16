#include "Bat.h"

void Bat::Init()
{

	//コウモリモデルを読み込む。

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Objects/Bat/Bat.gltf");
	}


	//モデルに入っているアニメーションを名前で取得して再生する。
	//第2引数をtrueにすると、アニメーションが最後まで行った後にループする。

	if (m_spModel)
	{
		m_animator.SetAnimation(m_spModel->GetAnimation("flap_loop"), true);
	}


	//仮表示用に、プレイヤーと同じ初期座標へ配置する。

	m_pos = { -25,3,0 };
	SetPos(m_pos);
}

void Bat::Update()
{

	//アニメーションを1フレーム進める。
	//WorkNodes()でモデル内部のノード行列を編集できる状態にする。
	//AdvanceTime()が現在のアニメーション時間に合わせて、羽などのノードを動かす。

	if (m_spModel)
	{
		m_animator.AdvanceTime(m_spModel->WorkNodes(), 1.0f);
	}


	//コウモリ全体のワールド行列を作る。
	//アニメーションはモデル内部を動かす処理で、
	//ここではコウモリ全体をどこに配置するかを決めている。

	Math::Matrix scaleMat = Math::Matrix::CreateScale(1);
	Math::Matrix transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = scaleMat * transMat;
}
