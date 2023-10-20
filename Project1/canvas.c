#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "jjuggumi.h"
#include "canvas.h"

#define DIALOG_DURATION_SEC		4

void print_status(void);
void set_cursor_invisible();

// (zero-base) row��, col���� Ŀ�� �̵�
void goto_cell(int row, int col) {
	COORD pos = { col,row };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// cursor�� ����
void set_cursor_invisible() {
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.bVisible = 0;
	cursorInfo.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// row��, col���� ch ���
void print_cell(char ch, int row, int col) {
	goto_cell(row, col);
	printf("%c", ch);
}

void map_init(int n_row, int n_col) {
	// �� ���۸��� ������ ����
	for (int i = 0; i < ROW_MAX; i++) {

		for (int j = 0; j < COL_MAX; j++) {
			back_buf[i][j] = front_buf[i][j] = ' ';
		}
	}

	N_ROW = n_row;
	N_COL = n_col;
	for (int i = 0; i < N_ROW; i++) {
		back_buf[i][0] = back_buf[i][N_COL - 1] = '*';

		for (int j = 1; j < N_COL - 1; j++) {
			back_buf[i][j] = (i == 0 || i == N_ROW - 1) ? '*' : ' ';
		}
	}
}

// back_buf[row][col]�� �̵��� �� �ִ� �ڸ����� Ȯ���ϴ� �Լ�
bool placable(int row, int col) {
	if (row < 0 || row >= N_ROW ||
		col < 0 || col >= N_COL ||
		back_buf[row][col] != ' ') {
		return false;
	}
	return true;
}

// ��ܿ� ����, �ϴܿ��� ���� ���¸� ���
void display(void) {
	draw();
	goto_cell(N_ROW + 4, 0);  // �߰��� ǥ���� ������ ������ �ʰ� ����â ������ �� ������ ���
	print_status();
}

void draw(void) {
	for (int row = 0; row < N_ROW; row++) {

		for (int col = 0; col < N_COL; col++) {

			if (front_buf[row][col] != back_buf[row][col]) {
				front_buf[row][col] = back_buf[row][col];
				print_cell(front_buf[row][col], row, col);
			}
		}
	}
	set_cursor_invisible();
}

void print_status(void) {
	printf("no. of players left: %d\n", n_alive);

	for (int p = 0; p < n_player; p++) {
		printf("player %2d: %5s\n", p, player[p] ? "alive" : "DEAD");
	}
}

void draw_box(int row, int col, int width, int height) {
	clear(row, col, width, height);

	for (int i = col; i < col + width; i++) {
		back_buf[row][i] = '*';
	}

	for (int i = row + 1; i < row + height - 1; i++) {
		back_buf[i][col] = '*';

		for (int j = col + 1; j < col + width - 2; j++) {
			back_buf[i][j] = ' ';
		}
		back_buf[i][col + width - 1] = '*';
	}

	for (int i = col; i < col + width; i++) {
		back_buf[row + height - 1][i] = '*';
	}
}

void clear(int row, int col, int width, int height) {
	for (int i = row; i < row + height; i++) {

		for (int j = col; j < col + width; j++) {
			back_buf[i][j] = ' ';
		}
	}
}

void dialog(char message[]) {
	// dialog ǥ�� �� back_buf�� ����
	char temp_buf[ROW_MAX][COL_MAX];

	for (int row = 0; row < ROW_MAX; row++) {

		for (int col = 0; col < COL_MAX; col++) {
			temp_buf[row][col] = back_buf[row][col];
		}
	}

	int message_len = strlen(message);
	draw_box(1, 1, message_len + 6, 3);

	int duration = 4;
	while (duration > 0) {
		back_buf[2][3] = '0' + duration;

		for (int col = 0; col < message_len; col++) {
			back_buf[2][col + 5] = message[col];
		}

		draw();
		Sleep(1000);

		duration--;
	}

	// dialog ǥ�� �� back_buf�� �ǵ���
	for (int row = 0; row < ROW_MAX; row++) {

		for (int col = 0; col < COL_MAX; col++) {
			back_buf[row][col] = temp_buf[row][col];
		}
	}
}
