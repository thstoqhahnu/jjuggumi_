// 2023-2 고급프로그래밍 과제: 쭈꾸미 게임
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include "jjuggumi.h"

#define	DATA_FILE	"jjuggumi.dat"

int jjuggumi_init(void);

// low 이상 high 이하 난수를 발생시키는 함수
int randint(int low, int high) {
	int rnum = rand() % (high - low + 1) + low;
	return rnum;
}

int jjuggumi_init(void) {

	srand((unsigned int)time(NULL));

	/*
	n_player = 5;
	printf("플레이어 수 : %d", n_player);
	*/
	printf("플레이어 수 : ");
	scanf_s("%d", &n_player);

	n_alive = n_player;
	for (int i = 0; i < n_player; i++) {
		player[i] = true;
	}

	return 0;
}

void printLine(const char* line) {
	printf("%s\n", line);
}

void intro() {
	const char* art[] = {
	   "=============================================================================",
	   "=============================================================================",
	   "==   ####                                                                  ==",
	   "==     ##      ####                                                        ==",
	   "==     ##        ##   ##   ##                                              ==",
	   "==#   ###        ##   ##   ##   ######                                     ==",
	   "== #####   #    ###   ##   ##  ##        ######                            ==",
	   "==           #####    ##   ##  ##  ###  ##       ##   ##                   ==",
	   "==                     #####   ##   ##  ##  ###  ##   ##   ##   ##         ==",
	   "==                              #####   ##   ##  ##   ##   ### ###   ######==",
	   "==                                       #####   ##   ##   ## # ##     ##  ==",
	   "==                                                #####    ##   ##     ##  ==",
	   "==                                                         ##   ##     ##  ==",
	   "==                   ######                                          ######==",
	   "==                 ##           #####                                      ==",
	   "==                 ##    ###  ##     ##  ##   ##                           ==",
	   "==                 ##     ##  ## ### ##  ### ###  #######                  ==",
	   "==                   #####    ##     ##  ## # ##  ##                       ==",
	   "==                            ##     ##  ##   ##  #######                  ==",
	   "==                                       ##   ##  ##                       ==",
	   "==                                                #######                  ==",
	   "=============================================================================",
	   "============================================================================="
	};

	for (int i = 0; i < sizeof(art) / sizeof(art[0]); i++) {
		printLine(art[i]);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}
	system("cls");

}

void ending() {

	system("cls");
	printf("===============================================\n");

	if (n_alive == 1) {
		int winner = -1;
		for (int i = 0; i < n_player; i++) {
			if (player[i]) {
				winner = i;
				break;
			}
		}

		printf("Player %d won!\n", winner);
	}
	else {
		printf("우승자를 가리지 못했습니다.\n");
		printf("살아남은 플레이어 : ");

		if (n_alive == 0) {
			printf("없음");
		}
		else {
			bool is_first = true;

			for (int i = 0; i < n_player; i++) {
				if (player[i]) {
					if (!is_first) {
						printf(", ");
					}
					printf("%d", i);
					is_first = false;
				}
			}
		}

		printf("\n");
	}

	printf("===============================================\n");
}

int main(void) {

	intro();
	jjuggumi_init();
	mugunghwa();
	//nightgame();
	//juldarigi();
	//jebi();
	ending();

	return 0;
}
