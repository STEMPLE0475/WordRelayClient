#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <format>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;
constexpr int BUFFER_SIZE = 1024;

void recvThread(SOCKET clientSocket)
{
    char buffer[BUFFER_SIZE] = {};
    while (true)
    {
        int len = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (len <= 0) {
            cout << "\n서버 연결 종료" << endl;
            exit(0); // 즉시 종료(실전에서는 동기화 신경 써야 함)
        }
        buffer[len] = '\0';
        string msg(buffer);

        if (msg.substr(0, 7) == "[TIMER]") {
            // 먼저 현재 줄 지우기
            std::cout << "\r" << std::string(100, ' ') << "\r";

            // 타이머 메시지 출력
            std::cout << msg << std::endl;

            // 다시 입력 프롬프트 복구
            std::cout << "입력 > " << std::flush;

            continue;
        }

        // 일반 서버 메시지 처리
        std::cout << "\n[서버] " << msg << std::endl;
        std::cout << "입력 > " << std::flush;
    }
}

int main()
{
    // 1. Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup 실패" << endl;
        return 1;
    }

    // 2. 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "소켓 생성 실패: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 3. 서버 주소 설정
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);

    // 4. 서버 연결 시도
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "서버 연결 실패: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "서버에 정상적으로 연결되었습니다." << endl << endl;

    cout << "/enter    방에 입장합니다" << endl;
    cout << "/roomlist    개설된 방의 정보를 봅니다" << endl;
    cout << "/exit     게임을 종료합니다" << endl << endl;

    cout << "명령어를 입력해주세요...." << endl;

    // 5. 서버 메시지 수신 스레드 시작
    thread t(recvThread, clientSocket);

    // 6. 메인 스레드는 입력만 담당
    string input;
    while (true) {
        //cout << "입력 > ";
        getline(cin, input);
        if (input == "/exit") break;
        send(clientSocket, input.c_str(), input.length(), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    // (실전에서는 스레드 join이나 안전 종료 필요)
    return 0;
}
