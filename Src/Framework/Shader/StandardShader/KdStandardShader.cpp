#include "Framework/KdFramework.h"

#include "KdStandardShader.h"


//================================================
// 描画準備
//================================================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影をつけるオブジェクトの描画の直前処理（不透明な物体やキャラクタの板ポリゴン）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// LitShaderで使用するリソースのバッファー設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginLit()
{
	// ディザ用の点模様を描画開始時に戻す。
	if (m_ditherTex) { SetDitherTexture(*m_ditherTex); }
	// 頂点シェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetVertexShader(m_VS_Lit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ピクセルシェーダーのパイプライン変更
	if (KdShaderManager::Instance().SetPixelShader(m_PS_Lit))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}

	// ボーン情報をセット(スキンメッシュ対応)
	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());

	// シャドウマップのテクスチャをセット
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(10, 1, m_depthMapFromLightRTPack.m_RTTexture->WorkSRViewAddress());

	// 通常テクスチャ用サンプラーのセット
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Wrap, 0);

	// 影ぼかし用の比較機能付きサンプラーのセット
	KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Linear_Clamp_Cmp, 1);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影ありオブジェクトの描画修了
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// 影を書き込む用に使用していたGenDepthFromLightで生成した深度SRVの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndLit()
{
	// インスタンシング描画31
	// 通常Litパスの終了直前に、全オブジェクトから登録されたモデルを一括描画する。
	KdModelInstanceBatcher::Instance().FlushLit();

	ID3D11ShaderResourceView* pNullSRV = nullptr;
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(10, 1, &pNullSRV);
}

// インスタンシング描画16
void KdStandardShader::BeginLitInstanced()
{
	if (m_ditherTex) { SetDitherTexture(*m_ditherTex); }
	// インスタンシング用頂点シェーダーと入力レイアウトを設定
	if
		(
			KdShaderManager::Instance().SetVertexShader
			(
				m_VS_LitInstanced
			)
			)
	{
		KdShaderManager::Instance().SetInputLayout
		(
			m_inputLayoutInstanced
		);

		KdShaderManager::Instance().SetVSConstantBuffer
		(
			0,
			m_cb0_Obj.GetAddress()
		);

		KdShaderManager::Instance().SetVSConstantBuffer
		(
			1,
			m_cb1_Mesh.GetAddress()
		);
	}

	// ピクセルシェーダーは通常Lit描画と同じものを使用
	if
		(
			KdShaderManager::Instance().SetPixelShader
			(
				m_PS_Lit
			)
			)
	{
		KdShaderManager::Instance().SetPSConstantBuffer
		(
			0,
			m_cb0_Obj.GetAddress()
		);

		KdShaderManager::Instance().SetPSConstantBuffer
		(
			2,
			m_cb2_Material.GetAddress()
		);
	}

	// スキンメッシュ用ボーン情報
	KdShaderManager::Instance().SetVSConstantBuffer
	(
		3,
		m_cb3_Bone.GetAddress()
	);

	// 通常Lit描画と同じシャドウマップを設定
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources
	(
		10,
		1,
		m_depthMapFromLightRTPack
		.m_RTTexture
		->WorkSRViewAddress()
	);

	// 通常テクスチャ用サンプラー
	KdShaderManager::Instance().ChangeSamplerState
	(
		KdSamplerState::Anisotropic_Wrap,
		0
	);

	// 影比較用サンプラー
	KdShaderManager::Instance().ChangeSamplerState
	(
		KdSamplerState::Linear_Clamp_Cmp,
		1
	);
}

void KdStandardShader::EndLitInstanced()
{
	// スロット1のインスタンスバッファを解除
	ClearInstanceDataFromDevice();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影をつけないオブジェクトの描画の直前処理（エフェクトや半透明物）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// UnLitShaderで使用するリソースのバッファー設定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginUnLit()
{
	if (KdShaderManager::Instance().SetVertexShader(m_VS_UnLit))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	if (KdShaderManager::Instance().SetPixelShader(m_PS_UnLit))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetPSConstantBuffer(2, m_cb2_Material.GetAddress());
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 陰影なしオブジェクトの描画終了
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndUnLit()
{}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 影を生み出すオブジェクトの情報描画（光を遮る物体）
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダーのパイプライン変更
// GenDepthMapFromLightShaderで使用するリソースのバッファー設定
// 書き込むテクスチャーを深度用の赤一色のテクスチャに切り替え
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::BeginGenerateDepthMapFromLight()
{
	if (KdShaderManager::Instance().SetVertexShader(m_VS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);

		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	// ボーン情報をセット(スキンメッシュ対応)
	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());

	if (KdShaderManager::Instance().SetPixelShader(m_PS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
	}

	m_depthMapFromLightRTPack.ClearTexture(kRedColor);
	m_depthMapFromLightRTChanger.ChangeRenderTarget(m_depthMapFromLightRTPack);
}

// インスタンシング描画24
void KdStandardShader::BeginGenerateDepthMapFromLightInstanced()
{
	if (KdShaderManager::Instance().SetVertexShader(m_VS_GenDepthFromLightInstanced))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayoutInstanced);
		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());

	if (KdShaderManager::Instance().SetPixelShader(m_PS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetPSConstantBuffer(0, m_cb0_Obj.GetAddress());
	}
}

void KdStandardShader::EndGenerateDepthMapFromLightInstanced()
{
	ClearInstanceDataFromDevice();

	// 深度テクスチャを消去せず、通常の影用パイプラインだけを復元する
	if (KdShaderManager::Instance().SetVertexShader(m_VS_GenDepthFromLight))
	{
		KdShaderManager::Instance().SetInputLayout(m_inputLayout);
		KdShaderManager::Instance().SetVSConstantBuffer(0, m_cb0_Obj.GetAddress());
		KdShaderManager::Instance().SetVSConstantBuffer(1, m_cb1_Mesh.GetAddress());
	}

	KdShaderManager::Instance().SetVSConstantBuffer(3, m_cb3_Bone.GetAddress());
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 影を生み出すオブジェクトの描画終了
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// レンダーターゲットを元に戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::EndGenerateDepthMapFromLight()
{
	KdModelInstanceBatcher::Instance().FlushDepth();
	m_depthMapFromLightRTChanger.UndoRenderTarget();
}


//================================================
// 描画関数
//================================================

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// メッシュを描画
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// メッシュの頂点データや3Dワールド情報・マテリアル情報をシェーダー(GPU)に転送する
// サブセットごとに描画命令を呼び出す：サブセットの個数分処理が重くなる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld,
	const std::vector<KdMaterial>& materials, const Math::Vector4& colRate, const Math::Vector3& emissive)
{
	if (mesh == nullptr) { return; }

	// メッシュの頂点情報転送
	mesh->SetToDevice();

	// 3Dワールド行列転送
	m_cb1_Mesh.Work().mW = mWorld;
	m_cb1_Mesh.Write();

	// 全サブセット
	for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++)
	{
		// 面が１枚も無い場合はスキップ
		if (mesh->GetSubsets()[subi].FaceCount == 0)continue;

		// マテリアルデータの転送
		const KdMaterial& material = materials[mesh->GetSubsets()[subi].MaterialNo];
		WriteMaterial(material, colRate, emissive);

		//-----------------------
		// サブセット描画
		//-----------------------
		mesh->DrawSubset(subi);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルデータを描画（スタティック(アニメーションをしない)なモデル専用
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュを描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawModel(const KdModelData& rModel, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	// オブジェクト単位の情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	auto& dataNodes = rModel.GetOriginalNodes();

	// 全描画用メッシュノードを描画
	for (auto& nodeIdx : rModel.GetDrawMeshNodeIndices())
	{
		// 描画
		DrawMesh(dataNodes[nodeIdx].m_spMesh.get(), dataNodes[nodeIdx].m_worldTransform * mWorld,
			rModel.GetMaterials(), colRate, emissive);
	}

	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// インスタンシング描画18
void KdStandardShader::DrawModelInstanced(
	const KdModelData& rModel,
	const std::vector<InstanceData>& instances,
	const Math::Color& colRate,
	const Math::Vector3& emissive)
{
	// 描画する個体がなければ終了
	if (instances.empty()) { return; }

	// 全個体のワールド行列をGPUへ送る
	if (!SetInstanceDataToDevice(instances)) { return; }

	// 今回はアニメーションしないモデルとして描画する
	SetIsSkinMeshObj(false);

	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	const auto& dataNodes = rModel.GetOriginalNodes();
	const auto& materials = rModel.GetMaterials();

	const UINT instanceCount =
		static_cast<UINT>(instances.size());

	// モデル内のメッシュノードを順番に描画
	for (const int nodeIdx : rModel.GetDrawMeshNodeIndices())
	{
		if (nodeIdx < 0 ||
			nodeIdx >= static_cast<int>(dataNodes.size()))
		{
			continue;
		}

		const auto& node = dataNodes[nodeIdx];
		const KdMesh* mesh = node.m_spMesh.get();

		if (!mesh) { continue; }

		// 頂点バッファとインデックスバッファをセット
		mesh->SetToDevice();

		// モデル内部のノード変換をセット
		m_cb1_Mesh.Work().mW = node.m_worldTransform;
		m_cb1_Mesh.Write();

		const auto& subsets = mesh->GetSubsets();

		for (UINT subsetIndex = 0;
			subsetIndex < static_cast<UINT>(subsets.size());
			++subsetIndex)
		{
			const auto& subset = subsets[subsetIndex];

			if (subset.FaceCount == 0) { continue; }

			if (subset.MaterialNo >= materials.size())
			{
				continue;
			}

			// サブセットに対応するマテリアルをセット
			WriteMaterial(
				materials[subset.MaterialNo],
				colRate,
				emissive);

			// 同一メッシュを全個体分まとめて描画
			mesh->DrawSubsetInstanced(
				static_cast<int>(subsetIndex),
				instanceCount);
		}
	}

	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// モデルワークを描画（ダイナミック(アニメーションをする)なモデルに対応
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュをワークの3D行列に従って描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawModel(KdModelWork& rModel, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	if (!rModel.IsEnable()) { return; }

	const std::shared_ptr<KdModelData>& data = rModel.GetData();

	// データがないときはスキップ
	if (data == nullptr) { return; }

	if (rModel.NeedCalcNodeMatrices())
	{
		rModel.CalcNodeMatrices();
	}

	// オブジェクト単位の情報転送(スキンメッシュ対応)
	SetIsSkinMeshObj(data->IsSkinMesh());
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	auto& workNodes = rModel.GetNodes();
	auto& dataNodes = data->GetOriginalNodes();

	// スキンメッシュモデルの場合：ボーン情報を書き込み(スキンメッシュ対応)
	if (data->IsSkinMesh())
	{
		// ノード内からボーン情報を取得
		for (auto&& nodeIdx : data->GetBoneNodeIndices())
		{
			if (nodeIdx >= KdStandardShader::maxBoneBufferSize) { assert(0 && "転送できるボーンの上限数を超えました"); return; }

			auto& dataNode = dataNodes[nodeIdx];
			auto& workNode = workNodes[nodeIdx];

			// ボーン情報からGPUに渡す行列の計算
			m_cb3_Bone.Work().mBones[dataNode.m_boneIndex] = dataNode.m_boneInverseWorldMatrix * workNode.m_worldTransform;

			m_cb3_Bone.Write();
		}
	}


	// 全描画用メッシュノードを描画
	for (auto& nodeIdx : data->GetDrawMeshNodeIndices())
	{
		// 描画
		DrawMesh(dataNodes[nodeIdx].m_spMesh.get(), workNodes[nodeIdx].m_worldTransform * mWorld,
			data->GetMaterials(), colRate, emissive);
	}

	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// インスタンシング描画25
void KdStandardShader::DrawModelInstanced(
	KdModelWork& rModel,
	const std::vector<InstanceData>& instances,
	const Math::Color& colRate,
	const Math::Vector3& emissive)
{
	if (!rModel.IsEnable() || instances.empty()) { return; }
	if (!SetInstanceDataToDevice(instances)) { return; }

	const std::shared_ptr<KdModelData> data = rModel.GetData();
	if (!data) { return; }

	if (rModel.NeedCalcNodeMatrices())
	{
		rModel.CalcNodeMatrices();
	}

	SetIsSkinMeshObj(data->IsSkinMesh());
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	const auto& dataNodes = data->GetOriginalNodes();
	const auto& workNodes = rModel.GetNodes();

	if (data->IsSkinMesh())
	{
		for (const int nodeIdx : data->GetBoneNodeIndices())
		{
			if (nodeIdx < 0 ||
				nodeIdx >= static_cast<int>(dataNodes.size()) ||
				nodeIdx >= static_cast<int>(workNodes.size()))
			{
				continue;
			}

			const auto& dataNode = dataNodes[nodeIdx];
			if (dataNode.m_boneIndex < 0 || dataNode.m_boneIndex >= maxBoneBufferSize)
			{
				assert(0 && "転送できるボーンの上限数を超えました");
				return;
			}

			m_cb3_Bone.Work().mBones[dataNode.m_boneIndex] =
				dataNode.m_boneInverseWorldMatrix * workNodes[nodeIdx].m_worldTransform;
		}

		m_cb3_Bone.Write();
	}

	const UINT instanceCount = static_cast<UINT>(instances.size());
	const auto& materials = data->GetMaterials();

	for (const int nodeIdx : data->GetDrawMeshNodeIndices())
	{
		if (nodeIdx < 0 ||
			nodeIdx >= static_cast<int>(dataNodes.size()) ||
			nodeIdx >= static_cast<int>(workNodes.size()))
		{
			continue;
		}

		const KdMesh* mesh = dataNodes[nodeIdx].m_spMesh.get();
		if (!mesh) { continue; }

		mesh->SetToDevice();
		m_cb1_Mesh.Work().mW = workNodes[nodeIdx].m_worldTransform;
		m_cb1_Mesh.Write();

		const auto& subsets = mesh->GetSubsets();
		for (UINT subsetIndex = 0; subsetIndex < static_cast<UINT>(subsets.size()); ++subsetIndex)
		{
			const auto& subset = subsets[subsetIndex];
			if (subset.FaceCount == 0 || subset.MaterialNo >= materials.size()) { continue; }

			WriteMaterial(materials[subset.MaterialNo], colRate, emissive);
			mesh->DrawSubsetInstanced(static_cast<int>(subsetIndex), instanceCount);
		}
	}

	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポリゴンを描画（モデル以外のプログラム上で生成された頂点の集合体
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データに所属する全ての描画用メッシュをワークの3D行列に従って描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::DrawPolygon(const KdPolygon& rPolygon, const Math::Matrix& mWorld,
	const Math::Color& colRate, const Math::Vector3& emissive)
{
	if (!rPolygon.IsEnable()) { return; }

	// ポリゴン描画用の頂点取得
	auto& vertices = rPolygon.GetVertices();

	// 頂点数が3より少なければポリゴンが形成できないので描画不能
	if (vertices.size() < 3) { return; }

	// オブジェクト単位の定数バッファで変更があった場合のみ情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	// 3Dワールド行列転送
	m_cb1_Mesh.Work().mW = mWorld;
	m_cb1_Mesh.Write();

	// マテリアルの転送
	if (rPolygon.GetMaterial())
	{
		WriteMaterial(*rPolygon.GetMaterial(), colRate, emissive);
	}
	else
	{
		WriteMaterial(KdMaterial(), colRate, emissive);
	}

	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);

	// サンプラーステートの変更:ポリゴンの描画なので、テクスチャの末端が繰り返されると不自然な描画になるため変更が必要
	if (KdShaderManager::Instance().IsPixelArtStyle())
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Point_Clamp);
	}
	else
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Clamp);
	}

	// 描画パイプラインのチェック
	ID3D11VertexShader* pNowVS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->VSGetShader(&pNowVS, nullptr, nullptr);
	bool isLitShader = m_VS_Lit == pNowVS;
	KdSafeRelease(pNowVS);

	// 陰影ありのシェーダーで2Dオブジェクトを描画する時
	if (isLitShader && rPolygon.Is2DObject())
	{
		std::vector<KdPolygon::Vertex> _2DVertices = vertices;

		// ポリゴンの法線を光に向ける処理：どの方向に向いていても光の影響を正面からに受けるように変換
		ConvertNormalsFor2D(_2DVertices, mWorld);

		// 2DObject用に変換した頂点配列を描画
		KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, (signed)_2DVertices.size(), &_2DVertices[0], sizeof(KdPolygon::Vertex));
	}
	else
	{
		// 頂点配列を描画
		KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, (signed)vertices.size(), &vertices[0], sizeof(KdPolygon::Vertex));
	}

	KdShaderManager::Instance().UndoSamplerState();

	KdShaderManager::Instance().UndoRasterizerState();

	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

void KdStandardShader::DrawVertices(const std::vector<KdPolygon::Vertex>& vertices, const Math::Matrix& mWorld,
	const Math::Color& colRate)
{
	// 頂点数が2より少なければポリゴンが形成できないので描画不能
	if (vertices.size() < 2) { return; }

	// オブジェクト単位の定数バッファで変更があった場合のみ情報転送
	if (m_dirtyCBObj)
	{
		m_cb0_Obj.Write();
	}

	// 3Dワールド行列転送
	m_cb1_Mesh.Work().mW = mWorld;
	m_cb1_Mesh.Write();

	// マテリアルの転送
	WriteMaterial(KdMaterial(), colRate, Math::Vector3::Zero);

	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZDisable);

	// サンプラーステートの変更:ポリゴンの描画なので、テクスチャの末端が繰り返されると不自然な描画になるため変更が必要
	if (KdShaderManager::Instance().IsPixelArtStyle())
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Point_Clamp);
	}
	else
	{
		KdShaderManager::Instance().ChangeSamplerState(KdSamplerState::Anisotropic_Clamp);
	}

	// 描画パイプラインのチェック
	ID3D11VertexShader* pNowVS = nullptr;
	KdDirect3D::Instance().WorkDevContext()->VSGetShader(&pNowVS, nullptr, nullptr);

	KdSafeRelease(pNowVS);

	// 頂点配列を描画
	KdDirect3D::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINELIST, (signed)vertices.size(), &vertices[0], sizeof(KdPolygon::Vertex));

	KdShaderManager::Instance().UndoSamplerState();

	KdShaderManager::Instance().UndoDepthStencilState();

	KdShaderManager::Instance().UndoRasterizerState();
	// 定数に変更があった場合は自動的に初期状態に戻す
	if (m_dirtyCBObj)
	{
		ResetCBObject();
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// KdShaderManagerの初期化時に呼び出される
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// シェーダー本体の生成
// シェーダーで利用する定数バッファの生成
// 影用の光からの深度情報テクスチャを生成
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool KdStandardShader::Init()
{
	//-------------------------------------
	// 頂点シェーダ(スキンメッシュ対応)
	//-------------------------------------
	{
		// コンパイル済みのシェーダーヘッダーファイルをインクルード
#include "KdStandardShader_VS_Lit.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_Lit))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}

		// １頂点の詳細な情報
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SKININDEX",	0, DXGI_FORMAT_R16G16B16A16_UINT,	0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SKINWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		// 頂点入力レイアウト作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateInputLayout(
			&layout[0],				// 入力エレメント先頭アドレス
			(UINT)layout.size(),	// 入力エレメント数
			&compiledBuffer[0],		// 頂点バッファのバイナリデータ
			sizeof(compiledBuffer),	// 上記のバッファサイズ
			&m_inputLayout))
			) {
			assert(0 && "CreateInputLayout失敗");
			Release();
			return false;
		}
	}

	// インスタンシング描画13
	{
		// インスタンシング用コンパイル済みシェーダー
#include "KdStandardShader_VS_LitInstanced.shaderInc"

		// インスタンシング用頂点シェーダーを作成
		if (FAILED
		(
			KdDirect3D::Instance().WorkDev()->CreateVertexShader
			(
				compiledBuffer,
				sizeof(compiledBuffer),
				nullptr,
				&m_VS_LitInstanced
			)
		))
		{
			assert
			(
				0 &&
				"インスタンシング用頂点シェーダー作成失敗"
			);

			Release();
			return false;
		}

		// スロット0はモデル頂点、スロット1はインスタンス行列
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
		{
			// モデル頂点：スロット0
			{
				"POSITION", 0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0, 0,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0,
				DXGI_FORMAT_R32G32_FLOAT,
				0, 12,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"COLOR", 0,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				0, 20,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0, 24,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0, 36,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"SKININDEX", 0,
				DXGI_FORMAT_R16G16B16A16_UINT,
				0, 48,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"SKINWEIGHT", 0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0, 56,
				D3D11_INPUT_PER_VERTEX_DATA, 0
			},

			// インスタンス行列：スロット1
			{
				"INSTANCEWORLD", 0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 0,
				D3D11_INPUT_PER_INSTANCE_DATA, 1
			},
			{
				"INSTANCEWORLD", 1,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 16,
				D3D11_INPUT_PER_INSTANCE_DATA, 1
			},
			{
				"INSTANCEWORLD", 2,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 32,
				D3D11_INPUT_PER_INSTANCE_DATA, 1
			},
			{
				"INSTANCEWORLD", 3,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 48,
				D3D11_INPUT_PER_INSTANCE_DATA, 1
			}
		};

		// インスタンシング用入力レイアウトを作成
		if (FAILED
		(
			KdDirect3D::Instance().WorkDev()->CreateInputLayout
			(
				layout.data(),
				static_cast<UINT>(layout.size()),
				compiledBuffer,
				sizeof(compiledBuffer),
				&m_inputLayoutInstanced
			)
		))
		{
			assert
			(
				0 &&
				"インスタンシング用入力レイアウト作成失敗"
			);

			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_VS_GenDepthMapFromLight.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_GenDepthFromLight))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}
	}

	// インスタンシング描画26
	{
#include "KdStandardShader_VS_GenDepthMapFromLightInstanced.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(
			compiledBuffer,
			sizeof(compiledBuffer),
			nullptr,
			&m_VS_GenDepthFromLightInstanced)))
		{
			assert(0 && "影生成用インスタンシング頂点シェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_VS_UnLit.shaderInc"

		// 頂点シェーダー作成
		if (FAILED(KdDirect3D::Instance().WorkDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_UnLit))) {
			assert(0 && "頂点シェーダー作成失敗");
			Release();
			return false;
		}
	}

	//-------------------------------------
	// ピクセルシェーダ
	//-------------------------------------
	{
#include "KdStandardShader_PS_Lit.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_Lit))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_PS_GenDepthMapFromLight.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_GenDepthFromLight))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}

	{
#include "KdStandardShader_PS_UnLit.shaderInc"

		if (FAILED(KdDirect3D::Instance().WorkDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS_UnLit))) {
			assert(0 && "ピクセルシェーダー作成失敗");
			Release();
			return false;
		}
	}
	//-------------------------------------
	// 定数バッファ作成
	//-------------------------------------
	m_cb0_Obj.Create();
	m_cb1_Mesh.Create();
	m_cb2_Material.Create();
	m_cb3_Bone.Create();

	// インスタンシング描画5
	// インスタンスごとのワールド行列を保存する動的頂点バッファを作成
	const UINT bufferBytes =
		static_cast<UINT>(sizeof(InstanceData) * MaxInstanceCount);

	if (!m_instanceBuffer.Create
	(
		D3D11_BIND_VERTEX_BUFFER,
		bufferBytes,
		D3D11_USAGE_DYNAMIC,
		nullptr
	)
		)
	{
		Release();
		return false;
	}

	std::shared_ptr<KdTexture> ds = std::make_shared<KdTexture>();
	ds->CreateDepthStencil(1024, 1024);
	D3D11_VIEWPORT vp = {
		0.0f, 0.0f,
		static_cast<float>(ds->GetWidth()),
		static_cast<float>(ds->GetHeight()),
		0.0f, 1.0f };

	m_depthMapFromLightRTPack.CreateRenderTarget(1024, 1024, true, DXGI_FORMAT_R32_FLOAT);
	m_depthMapFromLightRTPack.ClearTexture(kRedColor);

	SetDissolveTexture(*KdAssets::Instance().m_textures.GetData("Asset/Textures/System/WhiteNoise.png"));

	// 点模様を起動時に読み込み、通常・一括描画で共有する。
	m_ditherTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/System/dot.png");
	if (!m_ditherTex) { Release(); return false; }
	SetDitherTexture(*m_ditherTex);


	return true;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シェーダー本体の解放
// 利用していたコンスタントバッファの解放
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::Release()
{
	m_ditherTex.reset();
	m_normalTextureOverride.reset();
	m_overrideNormalTexture = false;
	m_baseColorTextureOverride.reset();
	KdSafeRelease(m_VS_Lit);

	// インスタンシング描画14
	KdSafeRelease(m_VS_LitInstanced);
	KdSafeRelease(m_inputLayoutInstanced);

	KdSafeRelease(m_VS_GenDepthFromLight);

	// インスタンシング描画27
	KdSafeRelease(m_VS_GenDepthFromLightInstanced);
	KdSafeRelease(m_VS_UnLit);

	KdSafeRelease(m_inputLayout);

	KdSafeRelease(m_PS_Lit);
	KdSafeRelease(m_PS_GenDepthFromLight);
	KdSafeRelease(m_PS_UnLit);

	m_cb0_Obj.Release();
	m_cb1_Mesh.Release();
	m_cb2_Material.Release();
	// スキンメッシュ対応
	m_cb3_Bone.Release();

	// インスタンシング描画6
	// インスタンスバッファ解放
	m_instanceBuffer.Release();
}

// インスタンシング描画8
bool KdStandardShader::SetInstanceDataToDevice
(
	const std::vector<InstanceData>& instances
)
{
	// 描画するインスタンスがない場合は失敗
	if (instances.empty())
	{
		return false;
	}

	// 作成済みバッファの最大数を超える場合は失敗
	if (instances.size() > MaxInstanceCount)
	{
		return false;
	}

	// CPU側の行列一覧を動的インスタンスバッファへ書き込む
	const UINT dataBytes = static_cast<UINT>
		(
			sizeof(InstanceData) * instances.size()
			);

	m_instanceBuffer.WriteData
	(
		instances.data(),
		dataBytes
	);

	// スロット1へインスタンスバッファを設定する
	UINT stride = sizeof(InstanceData);
	UINT offset = 0;

	KdDirect3D::Instance().WorkDevContext()->IASetVertexBuffers
	(
		1,
		1,
		m_instanceBuffer.GetAddress(),
		&stride,
		&offset
	);

	return true;
}

// インスタンシング描画9
void KdStandardShader::ClearInstanceDataFromDevice()
{
	ID3D11Buffer* nullBuffer = nullptr;
	UINT stride = 0;
	UINT offset = 0;

	KdDirect3D::Instance().WorkDevContext()->IASetVertexBuffers
	(
		1,
		1,
		&nullBuffer,
		&stride,
		&offset
	);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画用マテリアル情報の転送
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// それぞれのマテリアルの影響倍率値とテクスチャを設定
// BaseColor：基本色 / Emissive：自己発光色 / Metalic：金属性(テカテカ) / Roughness：粗さ(材質の色の反映度)
// テクスチャは法線マップ以外は未設定なら白1ピクセルのシステムテクスチャを指定
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::WriteMaterial(const KdMaterial& material, const Math::Vector4& colRate, const Math::Vector3& emiRate)
{
	//-----------------------
	// マテリアル情報を定数バッファへ書き込む
	//-----------------------
	m_cb2_Material.Work().BaseColor = material.m_baseColorRate * colRate;
	m_cb2_Material.Work().Emissive = material.m_emissiveRate * emiRate;
	m_cb2_Material.Work().Metallic = material.m_metallicRate;
	m_cb2_Material.Work().Roughness = material.m_roughnessRate;
	m_cb2_Material.Write();

	//-----------------------
	// テクスチャセット
	//-----------------------
	ID3D11ShaderResourceView* srvs[4];

	srvs[0] = material.m_baseColorTex ? material.m_baseColorTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	// モデルの共有データを触らず、この描画の色画像だけ置き換える。
	if (m_baseColorTextureOverride) { srvs[0] = m_baseColorTextureOverride->WorkSRView(); }
	srvs[1] = material.m_metallicRoughnessTex ? material.m_metallicRoughnessTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	srvs[2] = material.m_emissiveTex ? material.m_emissiveTex->WorkSRView() : KdDirect3D::Instance().GetWhiteTex()->WorkSRView();
	srvs[3] = material.m_normalTex ? material.m_normalTex->WorkSRView() : KdDirect3D::Instance().GetNormalTex()->WorkSRView();
	if (m_overrideNormalTexture)
	{
		srvs[3] = m_normalTextureOverride ? m_normalTextureOverride->WorkSRView() : KdDirect3D::Instance().GetNormalTex()->WorkSRView();
	}

	// セット
	KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(0, _countof(srvs), srvs);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ポリゴンがどの方向に向いていても光の影響を正面からに受けるように頂点の法線を変換
// 2Dキャラクタを描画する時などは必要
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::ConvertNormalsFor2D(std::vector<KdPolygon::Vertex>& target, const Math::Matrix& mWorld)
{
	// 平行光の向き
	const Math::Vector3& dirLight_Dir = KdShaderManager::Instance().GetLightCB().DirLight_Dir;

	// どの角度を向いていても表面は常に光の方向を向いている状態：横向きの板ポリが暗くならない対策
	Math::Vector3 normal = Math::Vector3::TransformNormal(-dirLight_Dir, mWorld.Invert());
	Math::Vector3 tangent = (normal != Math::Vector3::Up) ?
		normal.Cross(Math::Vector3::Up) : normal.Cross(Math::Vector3::Right);

	for (size_t i = 0; i < target.size(); ++i)
	{
		target[i].normal = normal;
		target[i].tangent = tangent;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// オブジェクト定数バッファを初期状態に戻す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void KdStandardShader::ResetCBObject()
{
	m_normalTextureOverride.reset();
	m_overrideNormalTexture = false;
	m_baseColorTextureOverride.reset();
	m_cb0_Obj.Work() = cbObject();

	m_cb0_Obj.Write();

	m_dirtyCBObj = false;
}
