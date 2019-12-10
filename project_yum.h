#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pwd.h>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

#define MAX_PATH_LEN 1024
#define DIRECTORY 11
#define FILE 12

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
enum ForeColour {
    Default = 0x0008,
    Black = 0x0000,
    Blue = 0x0001,
    Red = 0x0004,
    White = 0x0007,
};
#else
enum ForeColour {
    Default = 0,
    Black = 30,
    Blue = 34,
    Red = 31,
    White = 37,
};
#endif

class Total { //숫자와 이름 저장하는 클래스
    int order;
    string name;
    int det;

  public:
    void insert(int a, string b, int c) {
        order = a;
        name = b;
        det = c;
    }
    int getOrder() { return order; }
    string getName() { return name; }
    int getDet() { return det; }

    void print() { //디렉토리면 파란색, 파일이면 빨간색으로 출력
        if (getDet() == DIRECTORY) {
#ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, ForeColour::Blue | 0x0008);
#else
            ostringstream oss;
            oss << "\033[1;" << ForeColour::Blue << "m";
            cout << oss.str();
#endif
            cout << order << " " << name << endl;
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, 0x0008);
#else
            cout << "\033[0m";
#endif
        } else if (getDet() == FILE) {
#ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, ForeColour::Red | 0x0008);
#else
            ostringstream oss;
            oss << "\033[1;" << ForeColour::Red << "m";
            cout << oss.str();
#endif
            cout << order << " " << name << endl;
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, 0x0008);
#else
            cout << "\033[0m";
#endif
        }
    }
};
