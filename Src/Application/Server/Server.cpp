#include "Server.h"

Server::Server()
	: m_socket(m_ioContext)
{}

Server::~Server()
{
	Disconnect();
}

void Server::Disconnect()
{
	asio::error_code error;

	if (m_socket.is_open())
	{
		m_socket.shutdown
		(
			asio::ip::tcp::socket::shutdown_both,
			error
		);

		m_socket.close(error);
	}

	m_isConnected = false;
}