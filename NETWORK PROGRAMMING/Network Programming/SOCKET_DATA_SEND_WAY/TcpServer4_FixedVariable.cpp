//tcp 고정길이 + 가변길이 전송 서버
#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char* argv[]) {
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	//소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	int fixlen;//고정 길이 데이터, 32bit
	char varbuf[BUFSIZE + 1];//가변 길이 데이터

	while (1) {
		//accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		//접속한 클라이언트 정보
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("TCP accept client: ip=%s, port=%d\n", addr, ntohs(clientaddr.sin_port));

		//클라이언트와 데이터 통신
		while (1) {
			//고정 길이 데이터 받기
			retval = recv(client_sock, (char*)&fixlen, sizeof(int), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("fix recv()");
				break;
			}
			else if (retval == 0) break;

			//가변 길이 데이터 받기
			retval = recv(client_sock, varbuf, fixlen, MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("var recv()");
				break;
			}
			else if (retval == 0) break;

			//받은 데이터 출력
			varbuf[retval] = '\0';
			printf("[TCP|%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), varbuf);
		}

		//소켓 닫기
		closesocket(client_sock);
		printf("TCP close client: ip=%s, port=%d\n", addr, ntohs(clientaddr.sin_port));
	}

	//소켓 닫기
	closesocket(listen_sock);

	//윈속 종료
	WSACleanup();
	return 0;
}