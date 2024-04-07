#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <winSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include<array>
using namespace std;

static char** createMatrix(int amount_of_rows = 3, int amount_of_cols = 3) {
    char** matrix = new char* [amount_of_rows];
    for (int i = 0; i < amount_of_rows; ++i)
        matrix[i] = new char[amount_of_cols];

    return matrix;
}


bool isInt(string str) {

    for (int i = 0; i < str.length(); i++)
        if (!isdigit(str[i]))
            return false;

    return true;
}


static void fillMatrix(char** user_matrix, int amount_of_rows = 3, int amount_of_cols = 3) {
    for (int i = 0; i < amount_of_rows; i++)
        for (int j = 0; j < amount_of_cols; j++)
            user_matrix[i][j] = '-';
}

string matrixToString(char** user_matrix, int matrix_size = 3) {
    string string_for_output;

    string_for_output += '\n';
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++)
            string_for_output += user_matrix[i][j];
        string_for_output += '\n';
    }
    return string_for_output;

}

void Handler(SOCKET CurrentSocket, ADDRINFO* CurrentAddress) {
    closesocket(CurrentSocket);
    freeaddrinfo(CurrentAddress);
    WSACleanup();
}

int* stringToArray(string string_of_nums) {
    int* num_array = new int[string_of_nums.length()];
    int j, array_index = 0;
    string sub_string;

    sub_string = string_of_nums[0];
    for (int i = 0; i < string_of_nums.length() - 1; i++) {
        j = i + 1;

        while (string_of_nums[j] != ' ') {

            if (j >= string_of_nums.length())
                break;
            sub_string += string_of_nums[j];
            j++;
        }

        if (isInt(sub_string)) {
            num_array[array_index] = stoi(sub_string);
            array_index++;
        }

        sub_string = "";
        i = j - 1;
    }

    return num_array;
}


string ChangedMatrix(string cifs,char** user_matrix) {
    int* arr = stringToArray(cifs);
    char cross;
    if (arr[2] == 1) {
        cross = '*' ;
    }
    else {
        cross = 'O';
    }
    user_matrix[arr[0]][arr[1]] = cross;
    return matrixToString(user_matrix);

}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;
    SOCKET ClientSocket = INVALID_SOCKET;
    SOCKET ListenSocket = INVALID_SOCKET;

    string sendBuffer;
    char recvBuffer[512];

    int result;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        cout << "WSAStartup failed, result = " << result << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    // AF_INET - IPv4
    // AF_INET6 - IPv6
    hints.ai_family = AF_INET;
    // SOCK_STREAM - like TCP
    // SOCK_DGRAM  - like UDP   �connectionless sockets�
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, "444", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    // Socket creation
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }


    // Connection 
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Binding socket failed" << endl;
        ListenSocket = INVALID_SOCKET;
        Handler(ListenSocket, addrResult);
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << endl;
        Handler(ListenSocket, addrResult);
        return 1;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Accepting socket failed" << endl;
        Handler(ListenSocket, addrResult);
        return 1;
    }

    closesocket(ListenSocket);
    cout << "Someone has connected" << endl;

    char** matrix = createMatrix();
    fillMatrix(matrix);
  

    do {
        ZeroMemory(recvBuffer, 512);

        result = recv(ClientSocket, recvBuffer, 512, 0);
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;
            if (true) {
                cout << "Data is valid." << endl;
                cout << "Altered data: " << recvBuffer << endl;
                sendBuffer = ChangedMatrix(recvBuffer,matrix);
            }
            else {
                cout << "Data isn't valid." << endl;
                sendBuffer = "Data isn't valid.";
            }
            cout << endl;

            result = send(ClientSocket, sendBuffer.c_str(), sendBuffer.length(), 0);
            if (result == SOCKET_ERROR) {
                cout << "Failed to send data back" << endl;
                Handler(ClientSocket, addrResult);
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing..." << endl;
        }
        else {
            cout << "recv failde with error" << endl;
            Handler(ClientSocket, addrResult);
            return 1;
        }
    } while (result > 0);

    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "shutdown client socket failed" << endl;
        Handler(ClientSocket, addrResult);
        return 1;
    }

    Handler(ClientSocket, addrResult);
    return 0;
}