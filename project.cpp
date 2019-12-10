#include "project.h"

Total *total; //동적할당 하는 인스턴스인데 전역적으로 사용
int total_num = 0; //디렉토리와 파일 전체 갯수 저장할 변수

int fileType(const struct stat *fileInfo); //파일인지 디렉토리인지 알려주는 함수
// void print_set(set<string> s);

void signalHandler(int signum);

string commands[5];

char past_path[MAX_PATH_LEN + 1]; //최근 경로를 저장

void execute();
void ls(string str); //디폴트 ls는 현재 디렉토리 출력
void cd(string str);
void cp(const char *av1, const char *av2); // av1 경로의 파일을 av2 파일에 복사
void mv(const char *av1,
        const char *av2); // av1 경로의 파일을 av2 디렉토리에 복사
void cat(string str, int decide);
#define MAXLINE 511
void tr_client();

int main() {
    execute();
    return 0;
}

void execute() { // 구현 : 염연웅
    ls(".");     // 처음에 ls 명령어를 수행
    while (1) {
        string temp;
        cout << "command>> ";
        getline(cin, temp);
        int k = 0;
        int index = 0;

        int n = temp.find(" ");
        if (n == string::npos) {
            commands[0] = temp;
        } else {
            while (n != string::npos) {
                commands[index] = temp.substr(k, n - k);
                k = n + 1;
                n = temp.find(" ", k);
                ++index;
                if (n == string::npos) {
                    commands[index] = temp.substr(k, temp.size() - k);
                    break;
                }
            }
        }
        if (commands[0] == "ls" && commands[1] == "") {
            ls(".");
        } else if (commands[0] == "ls" && commands[1] != "") {
            ls(commands[1]);
        } else if (commands[0] == "cd" && commands[1] == "") {
            cout << "Usage: cd [number]." << endl;
        } else if (commands[0] == "cd" && commands[1] != "") {
            cd(commands[1]);
            ls(".");
        } else if (commands[0] == "cp" && commands[1] != "" &&
                   commands[2] != "") {
            string sub1 = commands[1]; //첫 번째 입력 파일
            string sub2 = commands[2]; //두 번째 입력 파일
            if (atoi(sub1.c_str()) == 0 &&
                atoi(sub2.c_str()) == 0) { //반환값이 0이면 단순 문자열
                const char *av1 = sub1.c_str(); //경로 지정
                const char *av2 = sub2.c_str();
                cp(av1, av2);
            } else if (atoi(sub1.c_str()) != 0 &&
                       atoi(sub2.c_str()) !=
                           0) { //반환값이 0이 아니면 정수를 포함한 문자열
                string sub3 = total[atoi(sub1.c_str())]
                                  .getName(); //번호에 대한 파일 경로 받아오기
                string sub4 = total[atoi(sub2.c_str())].getName();
                const char *av1 = sub3.c_str(); //경로 지정
                const char *av2 = sub4.c_str();
                cp(av1, av2);
            }
        } else if (commands[0] == "mv" && commands[1] != "" &&
                   commands[2] != "") {
            string sub1 = commands[1];
            string sub2 = commands[2];
            if (atoi(sub1.c_str()) != 0 && atoi(sub2.c_str()) != 0) {
                string sub3 = total[atoi(sub1.c_str())].getName();
                string sub4 = total[atoi(sub2.c_str())].getName();
                if (total[atoi(sub2.c_str())].getDet() == DIRECTORY) {
                    const char *av1 = sub3.c_str();
                    const char *av2 = sub4.c_str();
                    mv(av1, av2);
                }
            }
        } else if (commands[0] == "cat" && commands[1] != ">") {
            cat(commands[1], 1);
        } else if (commands[0] == "cat" && commands[1] == ">") {
            cat(commands[2], 2);
        } else if (commands[0] == "tr") {
            tr_client();
        } else if (commands[0] == "q" || commands[0] == "Q") {
            break;
        }
        commands[1] = "";
    }
}

void ls(string str) { //구현 : 임형준
    char current_dir[MAX_PATH_LEN + 1];
    if (getcwd(current_dir, MAX_PATH_LEN) == NULL) { //현재 디렉토리 임시 저장
        perror("getcwd() error!");
        exit(-1);
    }

    struct stat fileInfo;
    struct passwd *userInfo;
    char cwd[MAX_PATH_LEN + 1];
    char path[MAX_PATH_LEN + 1];
    int temp_num = 0;

    DIR *dirp;
    struct dirent *dirInfo;
    set<string> temp_dir;  //임시로 디렉토리 저장할 set
    set<string> temp_file; //임시로 파일 저장할 set

    if (isdigit(str[0])) { //만약 ls 뒤에 나오는게 숫자라면 total의 숫자와 맞는
                           //것을 찾아 chdir로 디렉토리 변경
        for (int i = 0; i < total_num; i++) {
            int num = atoi(str.c_str());
            if (total[i].getOrder() == num) {
                string name = total[i].getName();
                strcpy(path, name.c_str()); // string to char
                break;
            }
        }
        dirp = opendir(path);
        if (chdir(path) == -1) {
            perror("chdir() error!(1)");
            exit(-1);
        }
    } else { // ls 뒤에 나온게 숫자가 아니라면 그냥 str을 c타입으로 바꿔서
             // 디렉토리
        dirp = opendir(str.c_str());
        if (chdir(str.c_str()) == -1) {
            perror("chdir() error!(2)");
            exit(-1);
        }
    }
    if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
        perror("getcwd() error!");
        exit(-1);
    }
    while ((dirInfo = readdir(dirp)) !=
           NULL) { // while문 돌며 디렉토리 정보 읽는다.
        char *name = dirInfo->d_name;

        struct stat fileInfo2;
        stat(name, &fileInfo2);

        int k = fileType(&fileInfo2);
        if (k == DIRECTORY) {
            temp_dir.insert(name);
        } else if (k == FILENUM) {
            temp_file.insert(name);
        }
        ++temp_num; //디렉토리, 파일 전체 몇 개인지 세는 변수
    }

    int dir_num = temp_dir.size();
    set<string>::iterator iter;
    iter = temp_dir.begin();
    total = new Total[temp_num];

    for (int i = 0; i < dir_num; ++i) {
        if (iter != temp_dir.end()) {
            total[i].insert(i, *iter, DIRECTORY);
            ++iter;
        }
    }
    iter = temp_file.begin();
    for (int i = dir_num; i < temp_num; ++i) {
        if (iter != temp_file.end()) {
            total[i].insert(i, *iter, FILENUM);
            ++iter;
        }
    }
    for (int i = 0; i < temp_num; ++i) {
        total[i].print();
        if ((i % 3 != 2) && (i == (temp_num - 1)))
            cout << endl;
    }

    if (getcwd(past_path, MAX_PATH_LEN) == NULL) {
        perror("getcwd() error!");
        exit(-1);
    }

    if (chdir(current_dir) == -1) {
        perror("chdir() to current directory!");
        exit(-1);
    }
    total_num = temp_num;
}
void cp(const char *av1, const char *av2) { //구현 : 김경배
    FILE *src;
    FILE *dst;
    char ch;

    // 복사할 파일을 연다.
    if ((src = fopen(av1, "r")) == NULL) {
        fprintf(stderr, "%s: Can't open file.\n", av1);
        return;
    }

    // 쓰기할 파일을 연다.
    if ((dst = fopen(av2, "w")) == NULL) {
        fprintf(stderr, "%s: Can't open file.\n", av2);
        return;
    }

    // 복사
    while (!feof(src)) {
        ch = (char)fgetc(src);

        if (ch != EOF) {
            fputc((int)ch, dst);
        }
    }

    fclose(src); //닫기
    fclose(dst);
}
//파일인지 디렉토리인지 구분하는 함수
int fileType(const struct stat *fileInfo) { //구현 : 
    if (S_ISDIR(fileInfo->st_mode)) { //디렉토리이면 DIRECTORY 리턴
        return DIRECTORY;
    } else { //아닐 경우 FILE 리턴
        return FILENUM;
    }
}

void signalHandler(int signum) { //구현 : 임형준
    if (signum == SIGINT) {
        delete[] total;
        exit(0);
    }
}
void mv(const char *av1, const char *av2) { //구현 : 김경배
    FILE *src;
    FILE *dst;
    char ch;
    char cwd[MAX_PATH_LEN]; //현재 위치 경로
    if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
        perror("getcwd() error!");
        exit(-1);
    }
    string sub =
        commands[1]; // 복사된 파일의 이름도 복사한 파일의 이름과 같게 설정
    string sub2 = total[atoi(sub.c_str())].getName();
    const char *av3 = sub2.c_str();
    // 복사할 파일을 연다.
    if ((src = fopen(av1, "r")) == NULL) {
        fprintf(stderr, "%s: Can't open file.\n", av1);
        return;
    }
    chdir(av2); //디렉토리로 이동
    // 쓰기할 파일을 연다.
    if ((dst = fopen(av3, "w")) == NULL) {
        fprintf(stderr, "%s: Can't open file.\n", "test");
        return;
    }

    // 복사
    while (!feof(src)) {
        ch = (char)fgetc(src);

        if (ch != EOF) {
            fputc((int)ch, dst);
        }
    }
    //닫고 원래 경로로 다시 이동
    fclose(dst);
    chdir(cwd);
    fclose(src);

    //원본 파일 삭제
    unlink(av1);
}
void cd(string str) { //구현 : 이승현
    char cwd[MAX_PATH_LEN];
    char dir[MAX_PATH_LEN];

    if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
        perror("getcwd() error!");
        exit(-1);
    }
    if (strcmp(past_path, cwd)) //최근경로와 현재 경로가 다르면 최근 경로로 이동
    {
        cout << "Recent directory: " << past_path << endl;
        cout << "go to Recent directory" << endl;
        chdir(past_path);
    }
    if (isdigit(str[0])) //숫자를 입력받았는지 확인
    {
        int num = atoi(str.c_str());
        bool find_flag = false; //숫자를 찾았는지 확인
        for (int i = 0; i < total_num; i++) {
            if (total[i].getOrder() == num) {
                find_flag = true;
                string name = total[i].getName();
                strcpy(dir, name.c_str()); // string형을 char형으로 변경
                struct stat fileInfo2;
                stat(dir, &fileInfo2);
                int t = fileType(&fileInfo2);

                if (t == DIRECTORY) //예외1 디렉토리가 아니라면
                {
                    cout << "go to " << dir << endl;
                    if (chdir(dir) == -1) {
                        perror("chdir() error!");
                        exit(-1);
                    }
                    if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
                        perror("getcwd() error!");
                        exit(-1);
                    }
                    cout << "Current directory : " << cwd
                         << endl; // cd 이후 현재 경로
                } else            //예외1 디렉토리가 아닌 경우
                {
                    cout << "Error! It is not Directory!" << endl;
                }
            }
        }
        if (find_flag == false) //예외2 숫자를 못찾은 경우
        {
            cout << "Error! File number does not exist!" << endl;
        }
    } else if (commands[1] == "..") // cd .. 을 예외적으로 허용
    {
        cout << "go to .." << endl;
        if (chdir("..") == -1) {
            perror("chdir() error!");
            exit(-1);
        }
        if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
            perror("getcwd() error!");
            exit(-1);
        }
        cout << "Current directory : " << cwd << endl; // cd 이후 현재 경로 출력
    } else {
        cout << "Usage: cd [number]." << endl;
    }
    if (getcwd(past_path, MAX_PATH_LEN) == NULL) //최근 경로를 저장
    {
        perror("getcwd() error!");
        exit(-1);
    }
}

void cat(string str, int decide) { //구현 : 임형준
    int fd = 0;
    int num = atoi(str.c_str());
    string name;
    pid_t pid = 0;
    int status = 0;

    for (int i = 0; i < total_num; i++) {
        if (total[i].getOrder() == num) {
            name = total[i].getName();
        }
    }

    const char *pathname = name.c_str();

    char buf[MAX_PATH_LEN] = {
        '\0',
    };
    ssize_t rsize = 0;
    ssize_t wsize = 0;
    ssize_t tsize = 0;

    pid = fork();
    if (pid == -1) {
        perror("fork() error!(1)");
        exit(-1);
    } else if (pid == 0) {
        if (decide == 1) {
            fd = open(pathname, O_RDONLY, 0644);
            if (fd == -1) {
                perror("open() error!(1)");
                exit(-1);
            }
            do {
                memset(buf, '\0', MAX_PATH_LEN + 1);
                rsize = read(fd, buf, MAX_PATH_LEN);

                if (rsize == -1) {
                    perror("read() error!(1)");
                    exit(-2);
                }
                printf("%s", buf);
                tsize += rsize;
            } while (rsize > 0);
        } else if (decide == 2) {

            fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror("open() error!(2)");
                exit(-3);
            }
            while (1) {
                memset(buf, '\0', MAX_PATH_LEN);

                string a;
                getline(cin, a);

                for (int i = 0; i <= a.length(); ++i) {
                    if (buf[i] == '\n')
                        break;
                    buf[i] = a[i];
                }

                if (strcmp(buf, "quit") == 0) {
                    break;
                }

                int num = strlen(buf);
                buf[num++] = '\n';
                wsize = write(fd, buf, num);

                if (wsize == -1) {
                    perror("write() error!(1)");
                    exit(-4);
                }
            }
            cout << "Append at " << pathname << endl;
        }
    } else {
        wait(&status);
    }

    close(fd);
}

// tr
int tcp_connect(int af, char *servip, unsigned short port) { //구현 : 염연웅
    struct sockaddr_in servaddr;
    int s;
    // 소켓 생성
    if ((s = socket(af, SOCK_STREAM, 0)) < 0)
        return -1;
    // 채팅 서버의 소켓주소 구조체 servaddr 초기화
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = af;
    inet_pton(AF_INET, servip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    // 연결요청
    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return -1;
    return s;
}

void tr_client() { //구현 : 염연웅
    struct sockaddr_in server;
    struct stat obj;
    int sock;
    char bufmsg[MAXLINE];
    char buf[100], command[5], filename[MAXLINE], *f;
    char temp[20];
    int k, size, status;
    int filehandle;

    cout << "Enter server IP address." << endl;
    char ip_address[16];
    cin >> ip_address;

    cout << "Enter server port number." << endl;
    char port[5];
    cin >> port;

    // error
    if (ip_address == nullptr || port == nullptr) {
        printf("사용법 : tr server_ip port\n");
        return;
    }

    sock = tcp_connect(AF_INET, ip_address, atoi(port));
    if (sock == -1) {
        printf("tcp_connect fail");
        exit(1);
    }

    while (1) {
        printf("\033[1;33m명령어 : get, put, pwd, ls, cd, quit\n");
        printf("\033[1;32mclient> ");
        fgets(bufmsg, MAXLINE, stdin);  //명령어 입력
        fprintf(stderr, "\033[97m");    //글자색을 흰색으로 변경
        if (!strcmp(bufmsg, "get\n")) { // get명령어를 입력받았다면
            printf("다운로드할 파일 : ");
            scanf("%s", filename);       //파일 이름 입력
            fgets(temp, MAXLINE, stdin); //버퍼에 남은 엔터 제거
            strcpy(buf, "get ");
            strcat(buf, filename);
            send(sock, buf, 100, 0); //명령어 전송
            recv(sock, &size, sizeof(int), 0);
            if (!size) { //파일이 없다면
                printf("파일이 없습니다\n");
                continue;
            }
            f = (char *)malloc(sizeof(char));
            recv(sock, f, size, 0);
            while (1) {
                filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if (filehandle == -1) //같은 이름이 있다면 이름 끝에 _1 추가
                    sprintf(filename + strlen(filename), "_1");
                else
                    break;
            }
            write(filehandle, f, size);
            close(filehandle);
            printf("다운로드 완료\n");         //전송이 잘 되었다면
        } else if (!strcmp(bufmsg, "put\n")) { // put명령어를 입력받았다면
            printf("업로드할 파일 : ");
            scanf("%s", filename);       //파일 이름 입력
            fgets(temp, MAXLINE, stdin); //버퍼에 남은 엔터 제거
            filehandle = open(filename, O_RDONLY);
            if (filehandle == -1) { //파일이 없다면
                printf("파일이 없습니다.\n");
                continue;
            }
            strcpy(buf, "put ");
            strcat(buf, filename);
            send(sock, buf, 100, 0);
            stat(filename, &obj);
            size = obj.st_size;
            send(sock, &size, sizeof(int), 0);      //명령어 전송
            sendfile(sock, filehandle, NULL, size); //파일 전송
            recv(sock, &status, sizeof(int), 0);
            if (status) //업로드가 잘 되었다면
                printf("업로드 완료\n");
            else
                printf("업로드 실패\n");
        } else if (!strcmp(bufmsg, "pwd\n")) { // pwd명령어를 입력받았다면
            strcpy(buf, "pwd");
            send(sock, buf, 100, 0);
            recv(sock, buf, 100, 0);
            printf("--The path of the Remote Directory--\n%s", buf);
        } else if (!strcmp(bufmsg, "ls\n")) { // ls명령어를 입력받았다면
            strcpy(buf, "ls");
            send(sock, buf, 100, 0);
            recv(sock, &size, sizeof(int), 0);
            f = (char *)malloc(size);
            recv(sock, f, size, 0);
            filehandle = creat("temp.txt", 0664);
            write(filehandle, f, size);
            close(filehandle);
            printf("--The Remote Directory List--\n");
            system("cat temp.txt"); //현재 디렉토리의 파일 출력
        } else if (!strcmp(bufmsg, "cd\n")) { // cd명령어를 입력받았다면
            strcpy(buf, "cd ");
            printf("이동할 경로 이름 : ");
            scanf("%s", buf + 3);        //위치 입력
            fgets(temp, MAXLINE, stdin); //버퍼에 남은 엔터 제거
            send(sock, buf, 100, 0);     //명령어 전송
            recv(sock, &status, sizeof(int), 0);
            if (status)
                printf("경로 변경 완료\n");
            else
                printf("경로 변경 실패\n");
        } else if (!strcmp(bufmsg, "quit\n")) { // quit명령어를 입력받았다면
            strcpy(buf, "quit");
            send(sock, buf, 100, 0);
            recv(sock, &status, 100, 0);
            if (status) {
                printf("서버를 닫는 중..\n");
                break;
            }
            printf("서버 닫기 실패\n");
        }
    }
}
