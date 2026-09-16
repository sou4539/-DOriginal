#pragma once

struct RemotePlayerState
{
	int id;
	Math::Vector3 pos;
	float angle;
};

class Server
{
	// 接続
	bool Connect(const std::string& host, unsigned short port);

	// 送信
	// 座標
	void SendPos(const Math::Vector3& pos, float angle);

	// 更新
	void Update();


	void Disconnect();
};