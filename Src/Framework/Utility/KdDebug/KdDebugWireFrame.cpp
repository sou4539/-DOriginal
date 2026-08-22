#include "KdDebugWireFrame.h"

bool KdDebugWireFrame::s_enable = false;

void KdDebugWireFrame::AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Color& col)
{
	if (!s_enable) { return; }

	// ?f?o?b?O???C????n?_
	KdPolygon::Vertex v1;
	v1.color = col.RGBA().v;
	v1.UV = Math::Vector2::Zero;
	v1.pos = start;

	// ?f?o?b?O???C????I?_
	KdPolygon::Vertex v2;
	v2.color = col.RGBA().v;
	v2.UV = Math::Vector2::Zero;
	v2.pos = end;

	m_debugVertices.push_back(v1);
	m_debugVertices.push_back(v2);
}

void KdDebugWireFrame::AddDebugLine(const Math::Vector3& start, const Math::Vector3& dir, float length, const Math::Color& col)
{
	if (!s_enable) { return; }

	// ?f?o?b?O???C????n?_
	KdPolygon::Vertex v1;
	v1.color = col.RGBA().v;
	v1.UV = Math::Vector2::Zero;
	v1.pos = start;

	// ?f?o?b?O???C????I?_
	KdPolygon::Vertex v2;
	v2.color = col.RGBA().v;
	v2.UV = Math::Vector2::Zero;
	v2.pos = v1.pos + (dir * length);

	m_debugVertices.push_back(v1);
	m_debugVertices.push_back(v2);
}

void KdDebugWireFrame::AddDebugLineFromMatrix(const Math::Matrix& mat, float scale)
{
	if (!s_enable) { return; }

	Math::Vector3 start = mat.Translation();
	Math::Vector3 vAxis;

	// X???`??
	vAxis = mat.Right();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kRedColor);

	// Y???`??
	vAxis = mat.Up();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kGreenColor);

	// Z???`??
	vAxis = mat.Backward();
	vAxis.Normalize();
	AddDebugLine(start, start + vAxis * scale, kBlueColor);
}

// ?f?o?b?O?X?t?B?A??`??
void KdDebugWireFrame::AddDebugSphere(const Math::Vector3& pos, float radius, const Math::Color& col)
{
	if (!s_enable) { return; }

	KdPolygon::Vertex v;
	v.UV = Math::Vector2::Zero;
	v.color = col.RGBA().v;

	int detail = 16;
	for (int i = 0; i < detail + 1; ++i)
	{
		// XZ??
		v.pos = pos;
		v.pos.x += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.x += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		// XY??
		v.pos = pos;
		v.pos.x += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.y += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.x += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.y += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		// YZ??
		v.pos = pos;
		v.pos.y += cos((float)i * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)i * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);

		v.pos = pos;
		v.pos.y += cos((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		v.pos.z += sin((float)(i + 1) * (360 / detail) * KdToRadians) * radius;
		m_debugVertices.push_back(v);
	}
}

// ?f?o?b?O?{?b?N?X??`??
void KdDebugWireFrame::AddDebugBox(const Math::Matrix& matrix, const Math::Vector3& size, const Math::Vector3& offset, const bool isOriented, const Math::Color& col)
{
	if (!s_enable) { return; }

	KdPolygon::Vertex v;
	v.UV = Math::Vector2::Zero;
	v.color = col.RGBA().v;

	Math::Vector3					corners[8];
	DirectX::BoundingBox			aabb;
	DirectX::BoundingOrientedBox	obb;
	// ?{?b?N?X(AABB)
	if (!isOriented)
	{
		aabb.Center = matrix.Translation() + offset;
		
		// ?s????T?C?Y???M?????????
		aabb.Extents.x = size.x * (matrix.Right().Length());
		aabb.Extents.y = size.y * (matrix.Up().Length());
		aabb.Extents.z = size.z * (matrix.Backward().Length());

		aabb.GetCorners(corners);
	}
	// ?{?b?N?X(OBB)
	else
	{
		Math::Matrix createMat = matrix;
		createMat.Translation(createMat.Translation() + offset);
		obb.Transform(obb, createMat);
		
		// ?s????T?C?Y???M?????????
		obb.Extents.x = size.x * (matrix.Right().Length());
		obb.Extents.y = size.y * (matrix.Up().Length());
		obb.Extents.z = size.z * (matrix.Backward().Length());

		obb.GetCorners(corners);
	}	

	// ????`
	{
		//???1
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[1];
		m_debugVertices.push_back(v);

		//???2
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[4];
		m_debugVertices.push_back(v);

		//???3
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[1];
		m_debugVertices.push_back(v);

		//???4
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[4];
		m_debugVertices.push_back(v);
	}

	// ?????`
	{
		//????1
		v.pos = corners[0];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//????2
		v.pos = corners[1];
		m_debugVertices.push_back(v);

		v.pos = corners[2];
		m_debugVertices.push_back(v);

		//????3
		v.pos = corners[4];
		m_debugVertices.push_back(v);

		v.pos = corners[7];
		m_debugVertices.push_back(v);

		//????4
		v.pos = corners[5];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);
	}

	// ????`
	{
		//???1
		v.pos = corners[2];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//???2
		v.pos = corners[2];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);

		//???3
		v.pos = corners[7];
		m_debugVertices.push_back(v);

		v.pos = corners[3];
		m_debugVertices.push_back(v);

		//???4
		v.pos = corners[7];
		m_debugVertices.push_back(v);

		v.pos = corners[6];
		m_debugVertices.push_back(v);
	}
}

void KdDebugWireFrame::Draw()
{
	if (m_debugVertices.size() < 2) { return; }

	KdShaderManager::Instance().m_StandardShader.DrawVertices(m_debugVertices, Math::Matrix::Identity);

	m_debugVertices.clear();
}

void KdDebugWireFrame::Clear()
{
	m_debugVertices.clear();
}

void KdDebugWireFrame::Release()
{
	m_debugVertices.clear();
}







