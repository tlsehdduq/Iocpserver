#pragma once
enum class CompType : char
{
	Accept,
	Recv,
	Send
};

class Over 
{
public:
	Over()
	{
		_wsaBuf.buf = _buf;
		_wsaBuf.len = 200;
		_type = CompType::Recv;
		ZeroMemory(&_over, sizeof(_over));
	}
	Over(char* packet)
	{
		_wsaBuf.buf = _buf;
		_wsaBuf.len = packet[0];
		ZeroMemory(&_over, sizeof(_over));
		_type = CompType::Send;
		memcpy(_buf, packet, _wsaBuf.len);

	}
public:
	WSAOVERLAPPED _over;
	WSABUF _wsaBuf;
	char _buf[200] = {};
	// ID ·Î? Session À¸·Î? 
	int _id;
	CompType _type;
};



