#pragma once
//============================================================
//
// 基本シェーダー
//
//============================================================
class KdStandardShader
{
public:
	// スキンメッシュ対応
	static const int maxBoneBufferSize = 300;

	// インスタンシング描画3
	// 1回の描画で扱える最大インスタンス数
	static constexpr UINT MaxInstanceCount = 2048;

	// 同じモデルを描画する各個体の情報
	struct InstanceData
	{
		Math::Matrix World = Math::Matrix::Identity;
	};

	// 定数バッファ(オブジェクト単位更新)
	struct cbObject
	{
		// UV操作
		Math::Vector2	UVOffset = { 0.0f, 0.0f };
		Math::Vector2	UVTiling = { 1.0f, 1.0f };

		// フォグ有効
		int				FogEnable = 1;

		// エミッシブのみの描画
		int				OnlyEmissie = 0;

		// スキンメッシュオブジェクトかどうか(スキンメッシュ対応)
		int				IsSkinMeshObj = 0;

		// ディゾルブ関連
		float			DissolveThreshold = 0.0f;	// 0 ～ 1
		float			DissolveEdgeRange = 0.03f;	// 0 ～ 1

		Math::Vector3	DissolveEmissive = { 1.0f, 0.0f, 0.0f };

		// 16バイト単位でHLSL側と配置を一致させる。
		int LimLightEnable = 0;
		Math::Vector3 limLightColor = { 1, 1, 1 };
		float limLightPow = 1.0f;
		int DitherEnable = 0;
		float DitherAlpha = 1.0f;
		float DitherDistance = 5.0f;
		Math::Vector3 DitherTarget = Math::Vector3::Zero;
		float DitherRadius = 3.0f;
	};

	// 定数バッファ(メッシュ単位更新)
	struct cbMesh
	{
		Math::Matrix	mW;
	};

	// 定数バッファ(マテリアル単位更新)
	struct cbMaterial
	{
		Math::Vector4	BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		Math::Vector3	Emissive = { 1.0f, 1.0f, 1.0f };
		float			Metallic = 0.0f;

		float			Roughness = 1.0f;
		float			_blank[3] = { 0.0f, 0.0f ,0.0f };
	};

	// 定数バッファ(ボーン単位更新：スキンメッシュ対応)
	struct cbBone {
		Math::Matrix mBones[300];
	};

	//================================================
	// 設定・取得
	//================================================

	// リムライト設定
	// 次のモデルの色画像だけ差し替える。共有モデルのマテリアルは変更しない。
	// nullptrなら元の画像を使い、DrawModel終了時に差し替えを解除する。
	void SetBaseColorTextureOverride(const std::shared_ptr<KdTexture>& texture)
	{
		m_baseColorTextureOverride = texture;
		m_dirtyCBObj = true;
	}

	// リムライト設定
	// 授業サンプルと同じ名前で、輪郭の発光を設定する。
	// 差し替え時にノーマル画像を省略した場合は、凹凸なしの画像を使う。
	void SetNormalTextureOverride(const std::shared_ptr<KdTexture>& texture)
	{
		m_normalTextureOverride = texture;
		m_overrideNormalTexture = true;
		m_dirtyCBObj = true;
	}

	void SetLimLightEnable(bool enable) { m_cb0_Obj.Work().LimLightEnable = enable; m_dirtyCBObj = true; }
	void SetLimLight(Math::Vector3 color, float power = 1.0f)
	{
		m_cb0_Obj.Work().limLightColor = color;
		m_cb0_Obj.Work().limLightPow = std::max(power, 0.001f);
		m_dirtyCBObj = true;
	}
	// alphaは残す割合。distanceFade=trueなら授業のカメラ距離による抜き方。
	// wallsOnlyは距離による抜きを壁面に限定する。床・天井には適用しない。
	void SetAlphaDither(bool enable, float alpha = 0.4f, bool distanceFade = false, bool wallsOnly = false)
	{
		m_cb0_Obj.Work().DitherEnable = enable ? (wallsOnly ? 3 : (distanceFade ? 2 : 1)) : 0;
		m_cb0_Obj.Work().DitherAlpha = std::clamp(alpha, 0.0f, 1.0f);
		m_dirtyCBObj = true;
	}
		// Limit wall fading to the view corridor ending before the player.
	void SetWallDitherTarget(const Math::Vector3& target, float radius = 3.0f)
	{
		m_cb0_Obj.Work().DitherEnable = 4;
		m_cb0_Obj.Work().DitherTarget = target;
		m_cb0_Obj.Work().DitherRadius = std::max(radius, 0.01f);
		m_dirtyCBObj = true;
	}

	void SetDitherTexture(KdTexture& texture)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(7, 1, texture.WorkSRViewAddress());
	}

	// 距離で抜く範囲を指定する。モデル描画後は既定値の5に戻る。
	void SetAlphaDitherDistance(float distance)
	{
		m_cb0_Obj.Work().DitherDistance = std::max(distance, 0.0f);
		m_dirtyCBObj = true;
	}

	// UVタイリング設定
	void SetUVTiling(const Math::Vector2& tiling)
	{
		m_cb0_Obj.Work().UVTiling = tiling;

		m_dirtyCBObj = true;
	}

	// UVオフセット設定
	void SetUVOffset(const Math::Vector2& offset)
	{
		m_cb0_Obj.Work().UVOffset = offset;

		m_dirtyCBObj = true;
	}

	// フォグ有効/無効
	void SetFogEnable(bool enable)
	{
		m_cb0_Obj.Work().FogEnable = enable;

		m_dirtyCBObj = true;
	}

	// ディゾルブ設定
	void SetDissolve(float threshold, const float* range = nullptr, const Math::Vector3* edgeColor = nullptr)
	{
		auto& cbObj = m_cb0_Obj.Work();

		cbObj.DissolveThreshold = threshold;

		if (range)
		{
			cbObj.DissolveEdgeRange = *range;
		}

		if (edgeColor)
		{
			cbObj.DissolveEmissive = *edgeColor;
		}

		m_dirtyCBObj = true;
	}

	// ディゾルブテクスチャ設定
	void SetDissolveTexture(KdTexture& dissolveMask)
	{
		KdDirect3D::Instance().WorkDevContext()->PSSetShaderResources(11, 1, dissolveMask.WorkSRViewAddress());
	}

	// デフォルトディゾルブテクスチャ設定
	void SetDefaultDissolveTexture(std::shared_ptr<KdTexture>& spDissolveMask)
	{
		if (!spDissolveMask) { return; }

		m_dissolveTex = spDissolveMask;

		SetDissolveTexture(*spDissolveMask);
	}

	// デフォルトのディゾルブテクスチャに戻す
	void ResetDissolveTexture()
	{
		if (!m_dissolveTex) { return; }

		SetDissolveTexture(*m_dissolveTex);
	}

	//================================================
	// 各定数バッファの取得
	//================================================
	const cbObject& GetObjectCB() const { return m_cb0_Obj.Get(); }

	const cbMesh& MeshCB() const { return m_cb1_Mesh.Get(); }

	const cbMaterial& WorkMaterialCB() const { return m_cb2_Material.Get(); }

	// スキンメッシュ対応
	const cbBone& WorkBoneCB() const { return m_cb3_Bone.Get(); }

	//================================================
	// 描画準備
	//================================================
	// 陰影をつけるオブジェクト等を描画する前後に行う
	void BeginLit();
	void EndLit();

	// インスタンシング描画15
	void BeginLitInstanced();
	void EndLitInstanced();

	// 陰影をつけないオブジェクト等を描画する前後に行う
	void BeginUnLit();
	void EndUnLit();

	// 最も初めに行う、光を遮るオブジェクトを描画する前後に行う
	void BeginGenerateDepthMapFromLight();
	void EndGenerateDepthMapFromLight();

	// インスタンシング描画21
	// 影マップの描画先を維持したまま、インスタンシング用パイプラインへ切り替える
	void BeginGenerateDepthMapFromLightInstanced();
	void EndGenerateDepthMapFromLightInstanced();

	//================================================
	// 描画関数
	//================================================
	// メッシュ描画
	void DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld, const std::vector<KdMaterial>& materials,
		const Math::Vector4& col, const Math::Vector3& emissive);

	// モデルデータ描画：アニメーションに非対応
	void DrawModel(const KdModelData& rModel, const Math::Matrix& mWorld = Math::Matrix::Identity, 
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// インスタンシング描画17
	void DrawModelInstanced(const KdModelData& rModel,const std::vector<InstanceData>& instances,
		const Math::Color& colRate = kWhiteColor,const Math::Vector3& emissive = Math::Vector3::Zero);

	// モデルワーク描画：アニメーションに対応
	void DrawModel(KdModelWork& rModel, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// インスタンシング描画22
	// 同じアニメーション姿勢を共有するスキンメッシュをまとめて描画
	void DrawModelInstanced(KdModelWork& rModel, const std::vector<InstanceData>& instances,
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// 任意の頂点群からなるポリゴン描画
	void DrawPolygon(const KdPolygon& poly, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor, const Math::Vector3& emissive = Math::Vector3::Zero);

	// 任意の頂点群からなるポリゴンライン描画
	void DrawVertices(const std::vector<KdPolygon::Vertex>& vertices, const Math::Matrix& mWorld = Math::Matrix::Identity,
		const Math::Color& colRate = kWhiteColor);

	//================================================
	// 初期化・解放
	//================================================

	// 初期化
	bool Init();
	// 解放
	void Release();

	~KdStandardShader()
	{
		Release();
	}

	std::shared_ptr<KdTexture>& GetDepthTex() { return m_depthMapFromLightRTPack.m_RTTexture; }

private:

	// インスタンシング描画7
	bool SetInstanceDataToDevice
	(
		const std::vector<InstanceData>& instances
	);

	void ClearInstanceDataFromDevice();

	// マテリアルのセット
	void WriteMaterial(const KdMaterial& material, const Math::Vector4& colRate, const Math::Vector3& emiRate);

	// ポリゴンの法線情報を2Dように書き換える
	void ConvertNormalsFor2D(std::vector<KdPolygon::Vertex>& target, const Math::Matrix& mWorld);

	// 定数バッファを初期状態に戻す
	void ResetCBObject();

	// スキンメッシュ有効かどうか(スキンメッシュ対応)
	void SetIsSkinMeshObj(bool isSkinMEshObj)
	{
		if (m_cb0_Obj.Work().IsSkinMeshObj != (int)isSkinMEshObj)
		{
			m_cb0_Obj.Work().IsSkinMeshObj = isSkinMEshObj;
			m_dirtyCBObj = true;
		}
	}

	// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
	// Lit：陰影をつけるオブジェクトの描画用（不透明な物体やキャラクタの板ポリなど
	// 平行光・点光源などの影響を受け角度によって色を変化させるオブジェクトを描画するシェーダー
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// UnLit：陰影のつかないオブジェクトの描画用（エフェクトや透明な物体など
	// 光の計算を行わずマテリアルの色をそのまま出力するシェーダー
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// GenDepthFromLight：光から見たオブジェクトの距離を赤で出力用
	// Litシェーダーで影の描画を行うために必要な情報テクスチャを作成するシェーダー
	// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////

	// 頂点シェーダー
	ID3D11VertexShader* m_VS_Lit = nullptr;

	// インスタンシング描画11
	ID3D11VertexShader* m_VS_LitInstanced = nullptr;

	ID3D11VertexShader* m_VS_UnLit = nullptr;				// 陰影なし
	ID3D11VertexShader* m_VS_GenDepthFromLight = nullptr;	// 光からの深度

	// インスタンシング描画23
	ID3D11VertexShader* m_VS_GenDepthFromLightInstanced = nullptr;

	// 頂点入力レイアウト
	ID3D11InputLayout* m_inputLayout = nullptr;

	// インスタンシング描画12
	ID3D11InputLayout* m_inputLayoutInstanced = nullptr;
	
	// ピクセルシェーダー
	ID3D11PixelShader* m_PS_Lit = nullptr;					// 陰影あり
	ID3D11PixelShader* m_PS_UnLit = nullptr;				// 陰影なし
	ID3D11PixelShader* m_PS_GenDepthFromLight = nullptr;	// 光からの深度

	// テクスチャ
	std::shared_ptr<KdTexture>	m_dissolveTex = nullptr;	// ディゾルブで使用するデフォルトテクスチャ
	std::shared_ptr<KdTexture> m_ditherTex; // 授業サンプルの点模様を保持する
	std::shared_ptr<KdTexture> m_baseColorTextureOverride;
	std::shared_ptr<KdTexture> m_normalTextureOverride;
	bool m_overrideNormalTexture = false;

	// 定数バッファ
	KdConstantBuffer<cbObject>		m_cb0_Obj;				// オブジェクト単位で更新
	KdConstantBuffer<cbMesh>		m_cb1_Mesh;				// メッシュ毎に更新
	KdConstantBuffer<cbMaterial>	m_cb2_Material;			// マテリアル毎に更新
	KdConstantBuffer<cbBone>		m_cb3_Bone;				// ボーン事に更新(スキンメッシュ対応「)

	// インスタンシング描画4
	// 各インスタンスのワールド行列をGPUへ渡す頂点バッファ
	KdBuffer m_instanceBuffer;

	KdRenderTargetPack	m_depthMapFromLightRTPack;
	KdRenderTargetChanger m_depthMapFromLightRTChanger;

	bool		m_dirtyCBObj = false;						// 定数バッファのオブジェクトに変更があったかどうか
};
