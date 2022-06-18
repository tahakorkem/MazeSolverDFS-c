#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <time.h>

#define DELAY 250000
#define N 50
#define M 50
#define PAD_X 6
#define PAD_Y 4
#define APPLE 24
#define FILE_SIZE 2000

typedef struct Position {
    int r;
    int c;
} Position;

void intro();

void outro(int n);

void gotoxy(int x, int y);

void hideCursor();

void resetConsoleColor();

void redConsole();

void cyanConsole();

void printScore(int score);

void generateApples(int apples[2 * N + 1][2 * M + 1], int n, int m, Position start, Position exit);

void dfs(int maze[2 * N + 1][2 * M + 1], int n, int m, Position pos, int visited[2 * N + 1][2 * M + 1],
         int apples[2 * N + 1][2 * M + 1], Position exit, int *score, int *isReachedExit);

void populate(int maze[2 * N + 1][2 * M + 1], int *n, int *m, Position *start, Position *exit);

int main() {
    srand((int) time(0));

    int i, j;
    int n, m;

    int maze[2 * N + 1][2 * M + 1];
    Position start;
    Position exit;

    // dosyadan okunan veri kullanılarak
    // labirent ve başlangıç çıkış değerleri doldurulur
    populate(maze, &n, &m, &start, &exit);

    int visited[2 * N + 1][2 * M + 1] = {0};
    int apples[2 * N + 1][2 * M + 1] = {0};

    // pozisyonun ilk değeri başlangıç konumudur
    Position pos;
    pos.r = start.r;
    pos.c = start.c;

    int score = 0;

    intro();
    hideCursor();
    resetConsoleColor();

    // labirent üzerine rastgele dağılacak
    // belirli sayıda elmalar üretilir
    generateApples(apples, n, m, start, exit);

    // labirent en başta tamamen yazdırılır
    for (i = 0; i < 2 * n + 1; i++) {
        for (j = 0; j < 2 * m + 1; j++) {
            gotoxy(j, i);
            if (i % 2 == 0 && j % 2 == 0) { // çift - çift
                printf("+");
            } else if (i % 2 == 1 && j % 2 == 1) { // tek - tek
                if (apples[i][j]) {
                    // elmaları yazdır
                    redConsole();
                    printf("O");
                    resetConsoleColor();
                } else if (i == start.r && j == start.c) {
                    // başlangıç noktasını yazdır
                    cyanConsole();
                    printf("b");
                    resetConsoleColor();
                } else if (i == exit.r && j == exit.c) {
                    // bitiş noktasını yazdır
                    cyanConsole();
                    printf("c");
                    resetConsoleColor();
                } else {
                    // boş hücreyi yazdır
                    printf(" ");
                }
            } else if (i % 2 == 0 && j % 2 == 1) { // çift - tek
                // duvar varsa tire yoksa boşluk yazdır
                printf(maze[i][j] == 0 ? "-" : " ");
            } else { // tek - çift
                // duvar varsa dikey çizgi yoksa boşluk yazdır
                printf(maze[i][j] == 0 ? "|" : " ");
            }
        }
        printf("\n");
    }

    cyanConsole();
    printScore(score);
    int isReachedExit = 0;

    usleep(DELAY * 5);

    // gezinti başlasın!!
    dfs(maze, n, m, pos, visited, apples, exit, &score, &isReachedExit);

    outro(n);

    usleep(DELAY * 10);

    return 0;
}

void populate(int maze[2 * N + 1][2 * M + 1], int *n, int *m, Position *start, Position *exit) {
    FILE *filePointer;
    char buffer[FILE_SIZE];

    filePointer = fopen("maze.txt", "r");

    int i = 0, j;
    while (fgets(buffer, FILE_SIZE, filePointer)) {
        j = 0;
        char c;
        while ((c = buffer[j]) != '\0') {
            if ((i % 2 == 0 && j % 2 == 1) || (i % 2 == 1 && j % 2 == 0)) {
                if (c == '1') {
                    maze[i][j] = 1;
                } else if (c == '0') {
                    maze[i][j] = 0;
                }
            } else if (i % 2 == 1 && j % 2 == 1) {
                // hücrelerden başlangıç ve çıkış içerenler aranır
                if (c == 'b') {
                    start->r = i;
                    start->c = j;
                } else if (c == 'c') {
                    exit->r = i;
                    exit->c = j;
                }
            }
            j++;
        }
        i++;
    }
    *n = (i - 1) / 2;
    *m = (j - 1) / 2;

    fclose(filePointer);
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void colorizeConsole(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetConsoleColor() {
    colorizeConsole(15);
}

void redConsole() {
    colorizeConsole(12);
}

void cyanConsole() {
    colorizeConsole(11);
}

void printScore(int score) {
    gotoxy(0, -PAD_Y / 2);
    printf("score: %3d", score);
}

void eraseTrace(Position pos) {
    usleep(DELAY);
    gotoxy(pos.c, pos.r);
    printf(" ");
}

void leaveTrace(Position pos) {
    usleep(DELAY);
    gotoxy(pos.c, pos.r);
    printf("*");
}

void collideWall(int *score) {
    *score -= 5;
    printScore(*score);
}

void eatApple(int *score) {
    *score += 10;
    printScore(*score);
}

int isInBounds(Position pos, int n, int m) {
    return pos.r > 0 && pos.c > 0 && pos.r < 2 * n && pos.c < 2 * m;
}

int isTraversable(int maze[2 * N + 1][2 * M + 1], int n, int m, Position from, Position to) {
    return isInBounds(to, n, m) && maze[(from.r + to.r) / 2][(from.c + to.c) / 2];
}

Position right(Position pos) {
    Position *rightPos = (Position *) malloc(sizeof(Position));
    rightPos->r = pos.r;
    rightPos->c = pos.c + 2;
    return *rightPos;
}

Position left(Position pos) {
    Position *leftPos = (Position *) malloc(sizeof(Position));
    leftPos->r = pos.r;
    leftPos->c = pos.c - 2;
    return *leftPos;
}

Position up(Position pos) {
    Position *upPos = (Position *) malloc(sizeof(Position));
    upPos->r = pos.r - 2;
    upPos->c = pos.c;
    return *upPos;
}

Position down(Position pos) {
    Position *downPos = (Position *) malloc(sizeof(Position));
    downPos->r = pos.r + 2;
    downPos->c = pos.c;
    return *downPos;
}

void dfs(int maze[2 * N + 1][2 * M + 1], int n, int m, Position pos, int visited[2 * N + 1][2 * M + 1],
         int apples[2 * N + 1][2 * M + 1], Position exit, int *score, int *isReachedExit) {
    if (*isReachedExit) {
        // dolaşma sonucu çıkışa ulaşıldı
        // daha dolaşmaya gerek yok
        return;
    }

    visited[pos.r][pos.c] = 1;

    leaveTrace(pos);

    if (apples[pos.r][pos.c]) {
        eatApple(score);
        apples[pos.r][pos.c] = 0;
    }

    if (pos.r == exit.r && pos.c == exit.c) {
        // çıkışa ulaşıldı
        *isReachedExit = 1;
        return;
    }

    // gidilebilen yönlere dair
    // 4'er bitlik flagler tutar
    // tüm bitler sıfır ise
    // buranın çıkmaz sokak olduğunu ifade eder
    int directionFlags = 0b1111;

    // üst yön kontrolü ve mümkünse gezinti
    Position uPos = up(pos);
    if (!visited[uPos.r][uPos.c] && isTraversable(maze, n, m, pos, uPos)) {
        dfs(maze, n, m, uPos, visited, apples, exit, score, isReachedExit);
        if (!*isReachedExit) {
            eraseTrace(uPos);
        }
    } else {
        directionFlags = directionFlags & 0b1110;
    }

    // sağ yön kontrolü ve mümkünse gezinti
    Position rPos = right(pos);
    if (!visited[rPos.r][rPos.c] && isTraversable(maze, n, m, pos, rPos)) {
        dfs(maze, n, m, rPos, visited, apples, exit, score, isReachedExit);
        if (!*isReachedExit) {
            eraseTrace(rPos);
        }
    } else {
        directionFlags = directionFlags & 0b1101;
    }

    // alt yön kontrolü ve mümkünse gezinti
    Position dPos = down(pos);
    if (!visited[dPos.r][dPos.c] && isTraversable(maze, n, m, pos, dPos)) {
        dfs(maze, n, m, dPos, visited, apples, exit, score, isReachedExit);
        if (!*isReachedExit) {
            eraseTrace(dPos);
        }
    } else {
        directionFlags = directionFlags & 0b1011;
    }

    // sol yön kontrolü ve mümkünse gezinti
    Position lPos = left(pos);
    if (!visited[lPos.r][lPos.c] && isTraversable(maze, n, m, pos, lPos)) {
        dfs(maze, n, m, lPos, visited, apples, exit, score, isReachedExit);
        if (!*isReachedExit) {
            eraseTrace(lPos);
        }
    } else {
        directionFlags = directionFlags & 0b0111;
    }

    // hiçbir yöne gidilemediyse
    // burası çıkmaz sokak demektir
    if (directionFlags == 0b0000) {
        collideWall(score);
    }
}

/*
* The Knuth algorithm implemantasyonu
*/
void generateApples(int apples[2 * N + 1][2 * M + 1], int n, int m, Position start, Position exit) {
    int in = 0, im = 0;

    while (in < n * m && im < APPLE) {
        int rn = n * m - in;
        int rm = APPLE - im;
        int row = 2 * (in / m) + 1;
        int col = 2 * (in % m) + 1;
        if (rand() % rn < rm && !(row == start.r && col == start.c) && !(row == exit.r && col == exit.c)) {
            apples[row][col] = 1;
            im++;
        }
        in++;
    }
}

void gotoxy(int x, int y) {
    HANDLE hConsoleOutput;
    COORD Cursor_Pos = {x - 1 + PAD_X, y - 1 + PAD_Y};
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsoleOutput, Cursor_Pos);
}

void intro() {
    int i;
    gotoxy(10, 5);
    printf("...");
    usleep(DELAY * 5);
    system("cls");

    gotoxy(5, 5);
    char msg[] = "ALGORITMA BASLIYOR...";
    i = 0;
    while (msg[i] != '\0') {
        usleep(DELAY / 2);
        printf("%c", msg[i]);
        i++;
    }

    usleep(DELAY * 10);
    system("cls");
}

void outro(int n) {
    gotoxy(0, 2 * n + PAD_Y / 2);
    printf("ALGORITMA SONA ERDI!");
}
