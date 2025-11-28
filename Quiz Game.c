#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_QUESTIONS 50
#define MAX_OPTIONS 4
#define MAX_NAME 50
#define QUESTIONS_FILE "questions.txt"
#define SCORES_FILE "scores.txt"
#define QUIZ_QUESTIONS 5

// Console handle
HANDLE hConsole;
CONSOLE_SCREEN_BUFFER_INFO csbi;

// Color theme identifiers
enum Theme { THEME_NEON_BLUE = 1, THEME_GREEN_HACKER, THEME_CYBERPUNK, THEME_PASTEL };

// Color attributes (Windows)
WORD colorMenu = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // cyan-ish
WORD colorQuestion = FOREGROUND_BLUE | FOREGROUND_INTENSITY; // bright blue
WORD colorOption = FOREGROUND_GREEN | FOREGROUND_INTENSITY; // bright green
WORD colorTimer = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // bright yellow
WORD colorCorrect = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD colorWrong = FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD colorDivider = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
WORD colorInfo = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // white

// Default settings
int clearScreenBeforeQuestion = 1;
int clearScreenAfterAnswer = 1;

// Difficulty settings
typedef struct {
    char name[16];
    int timeLimit;
    int scorePerQ;
    float scoreMultiplier;
} Difficulty;

Difficulty difficulties[] = {
    {"Easy", 15, 10, 1.0f},
    {"Normal", 10, 10, 1.0f},
    {"Hard", 6, 10, 1.5f}
};
int currentDifficultyIndex = 1; // Normal

// Structs
typedef struct {
    char question[200];
    char options[MAX_OPTIONS][100];
    char correct; // 'A'..'D'
} Question;

typedef struct {
    char name[MAX_NAME];
    int score;
} Player;

// Utility: set console color
void setColor(WORD attr) {
    SetConsoleTextAttribute(hConsole, attr);
}

// Utility: clear stray input
void clearInputBuffer() {
    while (_kbhit()) _getch();
}

// Utility: console width
int consoleWidth() {
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi.dwSize.X;
}

// Utility: print centered line
void centerPrint(const char *s) {
    int width = consoleWidth();
    int len = (int)strlen(s);
    int pad = (width - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; ++i) putchar(' ');
    printf("%s\n", s);
}

// Loading animation
void displayLoading(const char *msg) {
    char buf[256];
    for (int i = 0; i < 8; ++i) {
        if (i % 2 == 0) system("cls");
        setColor(colorMenu);
        int pad = (consoleWidth() - (int)strlen(msg) - 4) / 2;
        if (pad < 0) pad = 0;
        for (int j = 0; j < pad; ++j) putchar(' ');
        int dots = i % 4;
        sprintf(buf, "%s", msg);
        printf("%s", buf);
        for (int d = 0; d < dots; ++d) putchar('.');
        fflush(stdout);
        Sleep(200);
    }
    system("cls");
}

// Simple beep wrapper
void playBeep(int freq, int duration_ms) {
    Beep(freq, duration_ms);
}

char timedAnswerWithEffects(int totalSeconds) {
    clearInputBuffer();
    char answer = 'X';
    int warned5 = 0;

    setColor(colorInfo);
    printf("\nYour answer (A/B/C/D). Press the letter key (no Enter required).\n");

    for (int sec = totalSeconds; sec >= 0; --sec) {
        // Determine color & intensity (fade + color change)
        // Fade: bright for >66%, normal for 33-66%, dim for <=33%
        float fraction = (float)sec / (float)(totalSeconds > 0 ? totalSeconds : 1);
        WORD baseColor = colorTimer & ~(FOREGROUND_INTENSITY); // remove intensity bit to get base
        // decide color: green (>7ish), yellow (6..3), red (<=2)
        WORD timerAttr = 0;
        if (sec > 7) {
            timerAttr = FOREGROUND_GREEN | FOREGROUND_INTENSITY; // bright green
        } else if (sec >= 3) {
            timerAttr = FOREGROUND_RED | FOREGROUND_GREEN | (fraction > 0.66f ? FOREGROUND_INTENSITY : 0); // yellowish
        } else {
            timerAttr = FOREGROUND_RED | (fraction > 0.5f ? FOREGROUND_INTENSITY : 0); // red (bright then normal)
        }

        // For "fade", if fraction < 0.33, remove intensity to look dim
        if (fraction < 0.33f) timerAttr &= ~FOREGROUND_INTENSITY;

        // If last second, we'll blink: alternate intensity on small sub-intervals
        if (sec == 1) {
            // Show once immediately
            setColor(timerAttr);
            printf("\rTime left: %2d sec   ", sec);
            fflush(stdout);

            // beep every second from 3 -> 1, so here at 1 we beep (handled below)
            if (!warned5 && sec <= 5 && sec == 5) { playBeep(1000, 150); warned5 = 1; } // not really needed here

            // Blink for 1 second in 100ms steps, check input
            int blinkSteps = 10; // 10 * 100ms = 1s
            for (int step = 0; step < blinkSteps; ++step) {
                // alternate intensity every 2 steps (200ms)
                if ((step / 2) % 2 == 0)
                    setColor(timerAttr | FOREGROUND_INTENSITY);
                else
                    setColor(timerAttr & ~FOREGROUND_INTENSITY);

                printf("\rTime left: %2d sec   ", sec);
                fflush(stdout);

                // check keypress
                for (int k = 0; k < 10; ++k) { // 10x10ms = 100ms
                    if (_kbhit()) {
                        int ch = _getch();
                        if (ch == 0 || ch == 224) { _getch(); continue; }
                        if (ch >= 'a' && ch <= 'd') ch -= 32;
                        if (ch >= 'A' && ch <= 'D') { printf("\n"); return (char)ch; }
                        else { setColor(colorWrong); printf("\nInvalid. Press A-D.\n"); setColor(timerAttr); }
                    }
                    Sleep(10);
                }
            }

            // beep behavior: for 1 sec we beep (beep every second from 3->1)
            playBeep(1000, 120);
            continue;
        }

        // For normal seconds (not blinking)
        setColor(timerAttr);
        printf("\rTime left: %2d sec   ", sec);
        fflush(stdout);

        // Beep at 5 seconds once
        if (!warned5 && sec == 5) {
            playBeep(1000, 160);
            warned5 = 1;
        }
        // Beep for seconds 3,2,1 (we beep on entering the second)
        if (sec <= 3 && sec >= 1) {
            playBeep(1000, 100);
        }

        // Now wait 1 second while frequently checking keyboard (100 x 10ms)
        for (int i = 0; i < 100; ++i) {
            if (_kbhit()) {
                int ch = _getch();
                if (ch == 0 || ch == 224) { _getch(); continue; } // special key
                if (ch >= 'a' && ch <= 'd') ch -= 32;
                if (ch >= 'A' && ch <= 'D') { printf("\n"); return (char)ch; }
                else { setColor(colorWrong); printf("\nInvalid. Press A-D.\n"); setColor(timerAttr); }
            }
            Sleep(10);
        }
    }

    // TIMEOUT reached (sec < 0)
    // Double-beep and flashing "TIME'S UP!"
    for (int flash = 0; flash < 6; ++flash) {
        if (flash % 2 == 0) {
            setColor(colorWrong | FOREGROUND_INTENSITY);
            printf("\r*** TIME'S UP! ***           ");
        } else {
            setColor(colorWrong & ~FOREGROUND_INTENSITY);
            printf("\r                       ");
        }
        fflush(stdout);
        if (flash == 0) { playBeep(500, 180); }
        else if (flash == 1) { playBeep(900, 120); }
        Sleep(200);
    }
    playBeep(900, 200);
    playBeep(900, 150);

    printf("\n");
    return 'X';
}

int loadQuestions(Question questions[]) {
    FILE *fp = fopen(QUESTIONS_FILE, "r");
    if (!fp) {
        setColor(colorWrong);
        printf("Could not open questions file: %s\n", QUESTIONS_FILE);
        setColor(15);
        return 0;
    }
    int count = 0;
    while (fgets(questions[count].question, sizeof(questions[count].question), fp)) {
        questions[count].question[strcspn(questions[count].question, "\n")] = 0;
        for (int i = 0; i < MAX_OPTIONS; ++i) {
            if (!fgets(questions[count].options[i], sizeof(questions[count].options[i]), fp)) questions[count].options[i][0] = 0;
            else questions[count].options[i][strcspn(questions[count].options[i], "\n")] = 0;
        }
        char c;
        if (fscanf(fp, " %c\n", &c) == 1) questions[count].correct = c;
        else questions[count].correct = 'A';
        count++;
        if (count >= MAX_QUESTIONS) break;
    }
    fclose(fp);
    return count;
}

void saveScore(Player p) {
    FILE *fp = fopen(SCORES_FILE, "a");
    if (!fp) {
        setColor(colorWrong); printf("Could not open scores file.\n"); setColor(15); return;
    }
    fprintf(fp, "%s %d\n", p.name, p.score);
    fclose(fp);
}

void viewHighScores() {
    FILE *fp = fopen(SCORES_FILE, "r");
    if (!fp) { setColor(colorWrong); printf("No high scores yet.\n"); setColor(15); return; }
    Player players[200];
    int count = 0;
    while (fscanf(fp, "%s %d", players[count].name, &players[count].score) == 2) {
        count++; if (count >= 200) break;
    }
    fclose(fp);
    // sort descending (simple)
    for (int i = 0; i < count - 1; ++i) for (int j = i + 1; j < count; ++j)
        if (players[j].score > players[i].score) { Player tmp = players[i]; players[i] = players[j]; players[j] = tmp; }
    setColor(colorMenu);
    printf("\n=== Top 5 High Scores ===\n");
    setColor(colorInfo);
    for (int i = 0; i < count && i < 5; ++i) printf("%d. %s: %d\n", i + 1, players[i].name, players[i].score);
    setColor(15);
}

// Themes
void applyTheme(int themeId) {
    switch (themeId) {
        case THEME_NEON_BLUE:
            colorMenu = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            colorQuestion = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            colorOption = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorTimer = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorCorrect = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorWrong = FOREGROUND_RED | FOREGROUND_INTENSITY;
            colorDivider = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            colorInfo = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case THEME_GREEN_HACKER:
            colorMenu = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorQuestion = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorOption = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorTimer = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorCorrect = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorWrong = FOREGROUND_RED | FOREGROUND_INTENSITY;
            colorDivider = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorInfo = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case THEME_CYBERPUNK:
            colorMenu = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            colorQuestion = FOREGROUND_RED | FOREGROUND_BLUE;
            colorOption = FOREGROUND_RED | FOREGROUND_INTENSITY;
            colorTimer = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorCorrect = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorWrong = FOREGROUND_RED | FOREGROUND_INTENSITY;
            colorDivider = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            colorInfo = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case THEME_PASTEL:
            colorMenu = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            colorQuestion = FOREGROUND_BLUE;
            colorOption = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorTimer = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            colorCorrect = FOREGROUND_GREEN;
            colorWrong = FOREGROUND_RED;
            colorDivider = FOREGROUND_BLUE;
            colorInfo = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        default:
            // neon blue default
            applyTheme(THEME_NEON_BLUE);
            break;
    }
}

// Conduct quiz
int conductQuiz(Question questions[], int totalQuestions) {
    int score = 0;
    int asked[QUIZ_QUESTIONS];
    for (int i = 0; i < QUIZ_QUESTIONS; ++i) asked[i] = -1;
    srand((unsigned int)time(NULL));

    int timeLimit = difficulties[currentDifficultyIndex].timeLimit;
    int scorePerQ = (int)(difficulties[currentDifficultyIndex].scorePerQ * difficulties[currentDifficultyIndex].scoreMultiplier);

    for (int q = 0; q < QUIZ_QUESTIONS; ++q) {
        // pick random unused question
        int idx;
        do { idx = rand() % totalQuestions; int used = 0; for (int z=0; z<QUIZ_QUESTIONS; ++z) if (asked[z]==idx) { used=1; break; } if(!used) break; } while(1);
        asked[q] = idx;

        if (clearScreenBeforeQuestion) system("cls");

        setColor(colorDivider); printf("------------------------------------------------------------\n");
        setColor(colorQuestion); centerPrint(questions[idx].question);

        setColor(colorOption);
        for (int o = 0; o < MAX_OPTIONS; ++o) centerPrint(questions[idx].options[o]);

        // timed answer with effects
        char ans = timedAnswerWithEffects(timeLimit);

        if (ans == questions[idx].correct) {
            setColor(colorCorrect); playBeep(1200, 120); printf("Correct! +%d points\n", scorePerQ); score += scorePerQ;
        } else if (ans == 'X') {
            setColor(colorWrong); playBeep(600, 160); printf("No answer. Correct: %c\n", questions[idx].correct);
        } else {
            setColor(colorWrong); playBeep(600, 160); printf("Wrong. Correct: %c\n", questions[idx].correct);
        }

        setColor(colorInfo); printf("Score so far: %d\n", score);

        if (clearScreenAfterAnswer) {
            setColor(colorInfo); printf("Press any key to continue...");
            clearInputBuffer();
            while (!_kbhit()) Sleep(50);
            _getch();
            system("cls");
        } else {
            Sleep(600);
        }
    }
    return score;
}

// Menu & settings
void showMenu(Question questions[], int totalQuestions) {
    Player player;
    while (1) {
        setColor(colorMenu);
        printf("\n"); centerPrint("=== Quiz Game Menu ===");
        setColor(colorInfo);
        printf("\n1. Start Quiz\n2. View High Scores\n3. Settings\n4. Exit\n");
        printf("Enter your choice: ");
        setColor(15);
        int choice = 0;
        if (scanf("%d", &choice) != 1) { clearInputBuffer(); setColor(colorWrong); printf("Invalid input.\n"); continue; }
        clearInputBuffer();

        if (choice == 1) {
            setColor(colorMenu); printf("Enter your name: ");
            setColor(15); scanf("%s", player.name); clearInputBuffer();
            displayLoading("Loading Quiz");
            int pts = conductQuiz(questions, totalQuestions);
            player.score = pts;
            setColor(colorCorrect); printf("\nQuiz finished! Final score: %d\n", player.score);
            saveScore(player);
        } else if (choice == 2) {
            viewHighScores();
        } else if (choice == 3) {
            while (1) {
                setColor(colorMenu); printf("\n=== Settings ===\n");
                setColor(colorInfo);
                printf("1. Choose Theme\n2. Choose Difficulty (current: %s)\n3. Toggle clear-before-question (currently %s)\n4. Toggle clear-after-answer (currently %s)\n5. Back\n",
                       difficulties[currentDifficultyIndex].name,
                       clearScreenBeforeQuestion ? "ON" : "OFF",
                       clearScreenAfterAnswer ? "ON" : "OFF");
                printf("Enter: ");
                setColor(15);
                int s; if (scanf("%d", &s) != 1) { clearInputBuffer(); setColor(colorWrong); printf("Invalid\n"); continue; }
                clearInputBuffer();
                if (s == 1) {
                    setColor(colorMenu); printf("Themes:\n1. Neon Blue\n2. Green Hacker\n3. Cyberpunk\n4. Pastel\nChoose: ");
                    int t; if (scanf("%d", &t) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();
                    applyTheme(t);
                    setColor(colorCorrect); printf("Theme applied.\n");
                } else if (s == 2) {
                    setColor(colorMenu); printf("Difficulty: 1-Easy 2-Normal 3-Hard : ");
                    int d; if (scanf("%d", &d) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();
                    if (d >= 1 && d <= 3) { currentDifficultyIndex = d - 1; setColor(colorCorrect); printf("Difficulty: %s\n", difficulties[currentDifficultyIndex].name); }
                    else { setColor(colorWrong); printf("Invalid.\n"); }
                } else if (s == 3) {
                    clearScreenBeforeQuestion = !clearScreenBeforeQuestion;
                    setColor(colorCorrect); printf("Now: %s\n", clearScreenBeforeQuestion ? "ON" : "OFF");
                } else if (s == 4) {
                    clearScreenAfterAnswer = !clearScreenAfterAnswer;
                    setColor(colorCorrect); printf("Now: %s\n", clearScreenAfterAnswer ? "ON" : "OFF");
                } else if (s == 5) break;
                else { setColor(colorWrong); printf("Invalid.\n"); }
            }
        } else if (choice == 4) {
            setColor(colorMenu); printf("Exiting...\n"); return;
        } else {
            setColor(colorWrong); printf("Invalid choice.\n");
        }
    }
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    applyTheme(THEME_NEON_BLUE);

    Question questions[MAX_QUESTIONS];
    int total = loadQuestions(questions);
    if (total == 0) {
        setColor(colorWrong);
        printf("No questions loaded. Ensure %s exists and is formatted.\n", QUESTIONS_FILE);
        setColor(15);
        return 1;
    }

    system("cls");
    setColor(colorMenu); centerPrint("Welcome to the Quiz!");
    setColor(colorInfo); centerPrint("Press any key to continue...");
    clearInputBuffer();
    while (!_kbhit()) Sleep(50);
    _getch();
    system("cls");

    showMenu(questions, total);

    setColor(15);
    return 0;
}
