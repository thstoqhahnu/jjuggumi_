#include "jjuggumi.h"
#include "canvas.h"
#include "keyin.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define DIR_UP		0
#define DIR_DOWN	1
#define DIR_LEFT	2
#define DIR_RIGHT	3

void mugunghwa_init(void);
void move_manual(key_t key);
void move_random_players();
void move_tail(int i, int nx, int ny);
int check_passed();
void clear_phrase();
void print_phrase(int end);
void draw_yh(char ch);
void init_moved_player();
void clear_player(int p);

// �� �÷��̾� ��ġ, �̵� �ֱ�
int p_row[PLAYER_MAX], p_col[PLAYER_MAX], period[PLAYER_MAX];

// ����� player
bool passed_player[PLAYER_MAX] = { false };

// ���� ���� ���� ������ player
bool moved_player[PLAYER_MAX];

// Ż���� player
bool eliminated_player[PLAYER_MAX];

// ������ ��ġ
int yh_col = 1;
int yh_rows[3];

// ���� ���� �ִ� ������
bool yh_watching = false;

char phrase[] = "����ȭ�����Ǿ����ϴ�";
int phrase_len = 10;
// phrase ǥ�� �ֱ�, "����ȭ����"�� ���� ������, "�Ǿ����ϴ�"�� ���� ������
int phrase_interval[] = { 300, 400, 500, 600, 700, 400, 300, 200, 100, 50 };

// �� �������� ������ �� x, y�� delta
int d_row[4] = { -1, 1, 0, 0 };
int d_col[4] = { 0, 0, -1, 1 };

// �ʱ�ȭ
void mugunghwa_init(void) {
	// �� �ʱ�ȭ
	map_init(n_player + 4, 40);

	for (int i = 0; i < n_player; i++) {
		// �� player�� ������ ���� ������� ��ġ
		p_row[i] = 2 + i;
		p_col[i] = N_COL - 2;

		// �̵� �ֱ�� random
		period[i] = randint(50, 200);

		back_buf[p_row[i]][p_col[i]] = '0' + i;  // (0 .. n_player-1)
	}

	// ���� �׸���.
	draw_yh('#');

	tick = 0;
}

// ���� �׸���.
void draw_yh(char ch) {
	// ������ ��� �׸���.
	int yh_row = (N_ROW - 3) / 2;
	for (int i = 0; i < 3; i++) {
		back_buf[yh_row + i][1] = ch;
		yh_rows[i] = yh_row + i;
	}
}

// ������� �������� ó��
void move_manual(key_t key) {
	// Ż�� �Ǵ� ��������� ó������ ����
	if (!player[0] || passed_player[0]) {

		return;
	}

	int dir;// ������ ����(0 ~ 3)
	switch (key) {
	case K_UP: dir = DIR_UP; break;
	case K_DOWN: dir = DIR_DOWN; break;
	case K_LEFT: dir = DIR_LEFT; break;
	case K_RIGHT: dir = DIR_RIGHT; break;
	default: return;
	}

	// �������� ���� �ڸ�
	int n_row, n_col;
	n_row = p_row[0] + d_row[dir];
	n_col = p_col[0] + d_col[dir];

	// �ٸ� �÷��̾�� ��ġ�� ��� �������� ó������ ����
	if (!placable(n_row, n_col)) {

		return;
	}

	move_tail(0, n_row, n_col);

	// �������� �־����� ����
	// ���� ���� ���� ���������� Ȯ���ϱ� ���� ���
	if (yh_watching) {
		moved_player[0] = true;
	}
}

// NPC���� �������� ó��
void move_random_players() {
	// player 1 ���ʹ� �������� ������(3 ����)
	for (int player_index = 1; player_index < n_player; player_index++) {
		// Ż�� �Ǵ� ��������� ó������ ����
		if (!player[player_index] || passed_player[player_index]) {
			continue;
		}

		// NPC�� �̵� �ֱⰡ �Ǹ� �����δ�.
		if (tick % period[player_index] == 0) {
			int dir = -1;
			// ����70%, ��10%, �Ʒ�10%, ���ڸ�10% �̹Ƿ�
			// ������ �� 1 ~ 10 �� 1 ~ 7�̸� ����, 8�̸� ��, 9�� �Ʒ�, 10�̸� �������� �ʵ��� �Ѵ�.
			int random = randint(1, 10);
			if (random >= 1 && random <= 7) {
				dir = DIR_LEFT;
			}
			else if (random == 8) {
				dir = DIR_UP;
			}
			else if (random == 9) {
				dir = DIR_DOWN;
			}
			else {
				continue;
			}

			// ���� ���� �߿��� 10%�� Ȯ���� �����δ�.
			if (yh_watching) {
				random = randint(1, 10);
				// ������ �� 1 ~ 10 �� 1�� ������ �����δ�.
				if (random != 1) {
					break;
				}
			}

			// �������� ������ ���� �ڸ�
			int n_row = p_row[player_index] + d_row[dir];
			int n_col = p_col[player_index] + d_col[dir];
			if (!placable(n_row, n_col)) {
				return;
			}

			move_tail(player_index, n_row, n_col);

			// �������� �־����� ����
			// ���� ���� ���� ���������� Ȯ���ϱ� ���� ���
			if (yh_watching) {
				moved_player[player_index] = true;
			}
		}
	}
}

// back_buf[][]�� ���
void move_tail(int p, int n_row, int n_col) {
	back_buf[n_row][n_col] = back_buf[p_row[p]][p_col[p]];
	back_buf[p_row[p]][p_col[p]] = ' ';

	// Ż������ ���� ����ڸ� ȭ�鿡 ǥ��
	if (player[p]) {
		p_row[p] = n_row;
		p_col[p] = n_col;
	}
}

// player�� �ʿ��� �����
void clear_player(int p) {
	back_buf[p_row[p]][p_col[p]] = ' ';
}

// ����� player���� ã�´�.
int check_passed() {
	int n_passed = 0;
	for (int player_index = 0; player_index < n_player; player_index++) {
		// player�� column ��ġ�� 1�̸�, �� ���� ��(������ ���� �Ʒ�)�� ���������� ���
		if (p_col[player_index] == 1) {
			passed_player[player_index] = true;
			n_passed++;
			continue;
		}

		// player�� ������ �ٷ� �տ� ���������� ���
		if (p_col[player_index] == yh_col + 1) {
			for (int i = 0; i < 3; i++) {
				if (p_row[player_index] == yh_rows[i]) {
					passed_player[player_index] = true;
					n_passed++;
					break;
				}
			}
		}
	}

	return n_passed;
}

// phrase�� ��� �����.
void clear_phrase() {
	goto_cell(N_ROW, 0);
	// �ѱ��� �� ���� ���� 2 + ���� 1, �׷��Ƿ� �� ���ڴ� 3
	for (int i = 0; i < phrase_len * 3; i++) {
		printf(" ");
	}
}

// end ��ġ���� phrase�� ǥ��
void print_phrase(int end) {
	goto_cell(N_ROW, 0);
	for (int i = 0; i <= end; i++) {
		// �ѱ� �� ���ڴ� �Ʒ��� ���� ���
		printf("%c%c ", phrase[i * 2], phrase[i * 2 + 1]);
	}
}

void init_moved_player() {
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		moved_player[i] = false;
	}
}

void init_eliminated_player() {
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		eliminated_player[i] = false;
	}
}

// Ż���� player�� ã�´�.
void check_eliminated() {
	for (int i = 0; i < PLAYER_MAX; i++) {
		// Ż������ �ʾ����鼭 ������ ����ڸ� ������� ã�´�.
		if (moved_player[i]) {
			int c_row = p_row[i];
			int c_col = p_col[i];
			bool covered = false;

			for (int j = 0; j < n_player; j++)
			{
				// ���� row�� ���ʿ� �ٸ� player�� ������ Ż�� �ƴ�
				if (p_row[j] == c_row && p_col[j] < c_col) {
					covered = true;
					break;
				}
			}

			if (!covered) {
				eliminated_player[i] = true;
			}
		}
	}
}

void mugunghwa(void) {

	// �ʱ�ȭ
	mugunghwa_init();
	system("cls");
	display();

	// phrase = "����ȭ�����Ǿ����ϴ�"
	// ȭ�鿡 ǥ���� ���ڿ��� ������ index
	// ex) 3�̸� "����ȭ��"���� ǥ��
	int phrase_index = 0;

	// �ֱٿ� phrase�� ǥ���� �ð�
	// ���� ǥ�� �ð��� �˱� ���� ���
	int recent_phrase_tick = 0;

	// ���� ���� ������ �ð�
	int yh_watch_tick = 0;
	while (1) {
		// ���� ���� ���� �ƴ� ��
		if (!yh_watching) {
			// phrase �� ���� ���ڸ� ǥ���� �ð��� �Ǹ�
			if (tick == recent_phrase_tick + phrase_interval[phrase_index]) {
				// phrase �� phrase_index���� ǥ��
				print_phrase(phrase_index);

				// �ֱ� phrase ǥ�� �ð��� update
				recent_phrase_tick = tick;

				// ǥ���� ������ ��ġ �̵�
				phrase_index++;

				// ������ ���ڸ� ǥ���ߴٸ�
				if (phrase_index == phrase_len) {
					// ī�޶� ���ư���(��#��->��@���� ����)
					draw_yh('@');

					// ���� ���� ������ ����
					yh_watching = true;

					// ���� ���� ������ �ð� ����
					yh_watch_tick = tick;

					// ���� ���� ���� ������ player���� �����ϴ� �迭�� �ʱ�ȭ
					init_moved_player();

					// Ż���� player���� �����ϴ� �迭�� �ʱ�ȭ
					init_eliminated_player();
				}
			}

			// ���� ���� ���� ��
		}

		else {

			// Ż���� ����ڸ� ã�´�.
			check_eliminated();

			// ���� ���� �ð��� ������
			if (tick == yh_watch_tick + 3000) {
				// phrase�� ��� �����.
				clear_phrase();

				// ���� ���� �ʴ� ���·� ����
				yh_watching = false;

				// phrase�� ó������ ǥ���ϵ��� index �ʱ�ȭ
				phrase_index = 0;

				// �ֱ� phrase ǥ�� �ð��� �ʱ�ȭ
				recent_phrase_tick = tick;

				// Ż���� ����ڰ� �ִ���
				bool exists_eliminated_player = false;
				for (int i = 0; i < PLAYER_MAX; i++)
				{
					if (eliminated_player[i]) {
						exists_eliminated_player = true;
						break;
					}
				}

				// Ż���� ����ڰ� ������ dialog�� message ���
				if (exists_eliminated_player) {
					char message[100];
					sprintf_s(message, sizeof(message), "player");
					bool is_first = true;
					for (int i = 0; i < PLAYER_MAX; i++)
					{
						if (eliminated_player[i]) {
							if (is_first) {
								sprintf_s(message + strlen(message), sizeof(message) - strlen(message), " %d", i);
								is_first = false;
							}
							else {
								sprintf_s(message + strlen(message), sizeof(message) - strlen(message), ", %d", i);
							}
							// Ż�� ǥ��
							player[i] = false;

							// Ż���� player�� �ʿ� ǥ������ ����
							clear_player(i);

							// ��Ƴ��� player�� ���� ����
							n_alive--;
						}
					}

					sprintf_s(message + strlen(message), sizeof(message) - strlen(message), " dead!");

					dialog(message);
				}
				// ī�޶󰡵��ư���(��@��->��#���� ����)
				draw_yh('#');
			}
		}

		// player 0�� ������ ������(4 ����)
		key_t key = get_key();
		if (key == K_QUIT) {
			break;
		}
		else if (key != K_UNDEFINED) {
			move_manual(key);
		}

		// 0��° player �ܿ��� ���Ƿ� ������
		move_random_players();

		display();

		Sleep(10);
		tick += 10;

		// ����� player���� ã�´�.
		check_passed();
		// ����� player���� �ʿ� ǥ������ ����
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (passed_player[i]) {
				clear_player(i);
			}
		}

		// ��Ƴ��� player�� �ϳ��̸� ������ ����
		if (n_alive <= 1) {
			break;
		}
	}
}