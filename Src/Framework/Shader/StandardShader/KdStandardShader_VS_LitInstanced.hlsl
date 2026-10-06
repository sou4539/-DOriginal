// インスタンシング描画10

#include "inc_KdStandardShader.hlsli"
#include "../inc_KdCommon.hlsli"

//==================================================
// 陰影付きモデル用インスタンシング頂点シェーダー
//==================================================
VSOutput main
(
	float4 pos : POSITION,
	float2 uv : TEXCOORD0,
	float4 color : COLOR,
	float3 normal : NORMAL,
	float3 tangent : TANGENT,
	uint4 skinIndex : SKININDEX,
	float4 skinWeight : SKINWEIGHT,

	// インスタンスごとのワールド行列
	float4 instanceWorld0 : INSTANCEWORLD0,
	float4 instanceWorld1 : INSTANCEWORLD1,
	float4 instanceWorld2 : INSTANCEWORLD2,
	float4 instanceWorld3 : INSTANCEWORLD3
)
{
	// スキンメッシュの場合はボーン行列を適用する
	if (g_IsSkinMeshObj)
	{
		row_major float4x4 boneMatrix = 0;

		[unroll]
		for (int i = 0; i < 4; ++i)
		{
			boneMatrix +=
				g_mBones[skinIndex[i]] * skinWeight[i];
		}

		pos = mul(pos, boneMatrix);
		normal = mul(normal, (float3x3) boneMatrix);
		tangent = mul(tangent, (float3x3) boneMatrix);
	}

	// スロット1から受け取った4行をワールド行列へ組み立てる
	row_major float4x4 instanceWorld = float4x4
	(
		instanceWorld0,
		instanceWorld1,
		instanceWorld2,
		instanceWorld3
	);

	// モデル内部のノード行列と、各個体の行列を合成する
	row_major float4x4 combinedWorld =
		mul(g_mWorld, instanceWorld);

	VSOutput output;

	// ローカル座標をワールド座標へ変換する
	output.Pos = mul(pos, combinedWorld);
	output.wPos = output.Pos.xyz;

	// ワールド座標を画面座標へ変換する
	output.Pos = mul(output.Pos, g_mView);
	output.Pos = mul(output.Pos, g_mProj);

	// 頂点カラー
	output.Color = color;

	// 法線
	output.wN = normalize
	(
		mul(normal, (float3x3) combinedWorld)
	);

	// 接線
	output.wT = normalize
	(
		mul(tangent, (float3x3) combinedWorld)
	);

	// 従接線
	float3 binormal = cross(normal, tangent);

	output.wB = normalize
	(
		mul(binormal, (float3x3) combinedWorld)
	);

	// UV座標
	output.UV = uv * g_UVTiling + g_UVOffset;

	return output;
}
