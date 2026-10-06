#pragma once

#include <asio.hpp>

struct RemotePlayerState
{
	int id = -1;
	Math::Vector3 pos = Math::Vector3::Zero;
	float angle = 0.0f;
};

class Server
{
public:
	Server();
	~Server();

	// 指定したサーバーへ接続する
	bool Connect(const std::string& host, unsigned short port);

	// プレイヤーの位置と向きを送信する
	void SendPos(const Math::Vector3& pos, float angle);

	// サーバーから受信した情報をゲームへ反映する
	void Update();

	// サーバーとの接続を終了する
	void Disconnect();

	// 現在接続しているか取得する
	bool IsConnected() const
	{
		return m_isConnected;
	}

private:
	// Asioの通信処理を管理する
	asio::io_context m_ioContext;

	// サーバーとTCP通信するためのソケット
	asio::ip::tcp::socket m_socket;

	// 現在の接続状態
	bool m_isConnected = false;
};