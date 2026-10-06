// インスタンシング描画20

#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

//==================================================
// 静的モデル・スキンメッシュ共通
// 影生成用インスタンシング頂点シェーダー
//==================================================
VSOutputGenShadow main
(
	float4 pos : POSITION,
	float2 uv : TEXCOORD0,
	float4 color : COLOR,
	float3 normal : NORMAL,
	float3 tangent : TANGENT,
	uint4 skinIndex : SKININDEX,
	float4 skinWeight : SKINWEIGHT,

	float4 instanceWorld0 : INSTANCEWORLD0,
	float4 instanceWorld1 : INSTANCEWORLD1,
	float4 instanceWorld2 : INSTANCEWORLD2,
	float4 instanceWorld3 : INSTANCEWORLD3
)
{
	// 蝙蝠などのスキンメッシュではボーン変形を行う
	if (g_IsSkinMeshObj)
	{
		row_major float4x4 boneMatrix = 0;

		[unroll]
		for (int i = 0; i < 4; ++i)
		{
			boneMatrix +=
				g_mBones[skinIndex[i]] *
				skinWeight[i];
		}

		pos = mul(pos, boneMatrix);
	}

	// インスタンスごとのワールド行列
	row_major float4x4 instanceWorld = float4x4
	(
		instanceWorld0,
		instanceWorld1,
		instanceWorld2,
		instanceWorld3
	);

	// モデル内のノード行列と個体ごとの行列を合成
	row_major float4x4 combinedWorld =
		mul(g_mWorld, instanceWorld);

	VSOutputGenShadow output;

	// ローカル座標からワールド座標へ変換
	output.Pos = mul(pos, combinedWorld);

	// ワールド座標からライト視点へ変換
	output.Pos = mul(output.Pos, g_DL_mLightVP);

	output.pPos = output.Pos;
	output.UV = uv;
	output.Color = color;

	return output;
}
