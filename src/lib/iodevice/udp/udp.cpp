/**
 * @file udp_client.cpp
 *
 */
#include "udp.h"

UdpSocket::UdpSocket(int port) {
	// 创建socket
	_sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (_sockfd < 0) {
		printf("create socket failed!!\n");
		return;
	}

	// 配置服务器地址信息
	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(port);
	_server_addr.sin_addr.s_addr = INADDR_ANY;

	int flags = fcntl(_sockfd, F_GETFL, 0);  // 获取当前状态
	if (flags == -1) {
		perror("fcntl");
		return;
	}
	fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);  // 设置为非阻塞模式

	// 绑定socket到本地地址
	if (bind(_sockfd, (struct sockaddr*) &_server_addr, sizeof(_server_addr)) < 0) {
		printf("bind socket failed!!\n");
		return;
	}
}

/**
 * 发送UDP数据
 * @param 1: 需要发送的数据
 * @param 2: 数据大小
 * @param 3: 目标IP地址
 * @param 4: 目标端口号
 * @return 成功-> ture ; 失败 -> false
 */
bool UdpSocket::send(void *message , size_t msg_len, const char* client_ip, int port) {
	// 配置目标地址信息
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(client_ip);

	// 发送数据
	if (sendto(_sockfd, message, msg_len, 0, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) < 0) {
		printf("send data failed!!\n");
		return false;
	}
		return true;
}

/**
 * 接收UDP数据
 * @param 1: 存放接收到的数据结构
 * @param 2: 数据大小
 * @return 成功-> ture ; 失败 -> false
 */
bool UdpSocket::receive(void *message , size_t msg_len) {
	socklen_t addrlen = sizeof(_client_addr);
	// 接收数据
	if (recvfrom(_sockfd, message, msg_len, 0, (struct sockaddr*) &_client_addr, &addrlen) < 0) {
		printf("receive data failed!!\n");
		return false;
	}
	return true;
}
