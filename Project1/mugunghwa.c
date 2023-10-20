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

// 각 플레이어 위치, 이동 주기
int p_row[PLAYER_MAX], p_col[PLAYER_MAX], period[PLAYER_MAX];

// 통과한 player
bool passed_player[PLAYER_MAX] = { false };

// 영희가 보는 동안 움직인 player
bool moved_player[PLAYER_MAX];

// 탈락한 player
bool eliminated_player[PLAYER_MAX];

// 영희의 위치
int yh_col = 1;
int yh_rows[3];

// 영희가 보고 있는 중인지
bool yh_watching = false;

char phrase[] = "무궁화꽃이피었습니다";
int phrase_len = 10;
// phrase 표시 주기, "무궁화꽃이"는 점점 느리게, "피었습니다"는 점점 빠르게
int phrase_interval[] = { 300, 400, 500, 600, 700, 400, 300, 200, 100, 50 };

// 각 방향으로 움직일 때 x, y값 delta
int d_row[4] = { -1, 1, 0, 0 };
int d_col[4] = { 0, 0, -1, 1 };

// 초기화
void mugunghwa_init(void) {
	// 맵 초기화
	map_init(n_player + 4, 40);

	for (int i = 0; i < n_player; i++) {
		// 각 player는 오른쪽 끝에 순서대로 배치
		p_row[i] = 2 + i;
		p_col[i] = N_COL - 2;

		// 이동 주기는 random
		period[i] = randint(50, 200);

		back_buf[p_row[i]][p_col[i]] = '0' + i;  // (0 .. n_player-1)
	}

	// 영희를 그린다.
	draw_yh('#');

	tick = 0;
}

// 영희를 그린다.
void draw_yh(char ch) {
	// 가능한 가운데 그린다.
	int yh_row = (N_ROW - 3) / 2;
	for (int i = 0; i < 3; i++) {
		back_buf[yh_row + i][1] = ch;
		yh_rows[i] = yh_row + i;
	}
}

// 사용자의 움직임을 처리
void move_manual(key_t key) {
	// 탈락 또는 통과했으면 처리하지 않음
	if (!player[0] || passed_player[0]) {

		return;
	}

	int dir;// 움직일 방향(0 ~ 3)
	switch (key) {
	case K_UP: dir = DIR_UP; break;
	case K_DOWN: dir = DIR_DOWN; break;
	case K_LEFT: dir = DIR_LEFT; break;
	case K_RIGHT: dir = DIR_RIGHT; break;
	default: return;
	}

	// 움직여서 놓일 자리
	int n_row, n_col;
	n_row = p_row[0] + d_row[dir];
	n_col = p_col[0] + d_col[dir];

	// 다른 플레이어와 겹치는 경우 움직임을 처리하지 않음
	if (!placable(n_row, n_col)) {

		return;
	}

	move_tail(0, n_row, n_col);

	// 움직임이 있었는지 저장
	// 영희가 보는 동안 움직였는지 확인하기 위해 사용
	if (yh_watching) {
		moved_player[0] = true;
	}
}

// NPC들의 움직임을 처리
void move_random_players() {
	// player 1 부터는 랜덤으로 움직임(3 방향)
	for (int player_index = 1; player_index < n_player; player_index++) {
		// 탈락 또는 통과했으면 처리하지 않음
		if (!player[player_index] || passed_player[player_index]) {
			continue;
		}

		// NPC는 이동 주기가 되면 움직인다.
		if (tick % period[player_index] == 0) {
			int dir = -1;
			// 왼쪽70%, 위10%, 아래10%, 제자리10% 이므로
			// 임의의 수 1 ~ 10 중 1 ~ 7이면 왼쪽, 8이면 위, 9면 아래, 10이면 움직이지 않도록 한다.
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

			// 영희가 보는 중에는 10%의 확률로 움직인다.
			if (yh_watching) {
				random = randint(1, 10);
				// 임의의 수 1 ~ 10 중 1이 나오면 움직인다.
				if (random != 1) {
					break;
				}
			}

			// 움직여서 다음에 놓일 자리
			int n_row = p_row[player_index] + d_row[dir];
			int n_col = p_col[player_index] + d_col[dir];
			if (!placable(n_row, n_col)) {
				return;
			}

			move_tail(player_index, n_row, n_col);

			// 움직임이 있었는지 저장
			// 영희가 보는 동안 움직였는지 확인하기 위해 사용
			if (yh_watching) {
				moved_player[player_index] = true;
			}
		}
	}
}

// back_buf[][]에 기록
void move_tail(int p, int n_row, int n_col) {
	back_buf[n_row][n_col] = back_buf[p_row[p]][p_col[p]];
	back_buf[p_row[p]][p_col[p]] = ' ';

	// 탈락하지 않은 사용자만 화면에 표시
	if (player[p]) {
		p_row[p] = n_row;
		p_col[p] = n_col;
	}
}

// player를 맵에서 지운다
void clear_player(int p) {
	back_buf[p_row[p]][p_col[p]] = ' ';
}

// 통과한 player들을 찾는다.
int check_passed() {
	int n_passed = 0;
	for (int player_index = 0; player_index < n_player; player_index++) {
		// player의 column 위치가 1이면, 즉 왼쪽 벽(영희의 위나 아래)에 도착했으면 통과
		if (p_col[player_index] == 1) {
			passed_player[player_index] = true;
			n_passed++;
			continue;
		}

		// player가 영희의 바로 앞에 도착했으면 통과
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

// phrase를 모두 지운다.
void clear_phrase() {
	goto_cell(N_ROW, 0);
	// 한글의 한 글자 길이 2 + 공백 1, 그러므로 한 글자당 3
	for (int i = 0; i < phrase_len * 3; i++) {
		printf(" ");
	}
}

// end 위치까지 phrase를 표시
void print_phrase(int end) {
	goto_cell(N_ROW, 0);
	for (int i = 0; i <= end; i++) {
		// 한글 한 글자는 아래와 같이 출력
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

// 탈락한 player를 찾는다.
void check_eliminated() {
	for (int i = 0; i < PLAYER_MAX; i++) {
		// 탈락하지 않았으면서 움직인 사용자를 대상으로 찾는다.
		if (moved_player[i]) {
			int c_row = p_row[i];
			int c_col = p_col[i];
			bool covered = false;

			for (int j = 0; j < n_player; j++)
			{
				// 같은 row의 왼쪽에 다른 player가 있으면 탈락 아님
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

	// 초기화
	mugunghwa_init();
	system("cls");
	display();

	// phrase = "무궁화꽃이피었습니다"
	// 화면에 표시할 문자열의 마지막 index
	// ex) 3이면 "무궁화꽃"까지 표시
	int phrase_index = 0;

	// 최근에 phrase를 표시한 시간
	// 다음 표시 시간을 알기 위해 사용
	int recent_phrase_tick = 0;

	// 영희가 보기 시작한 시간
	int yh_watch_tick = 0;
	while (1) {
		// 영희가 보는 중이 아닐 때
		if (!yh_watching) {
			// phrase 중 다음 문자를 표시할 시간이 되면
			if (tick == recent_phrase_tick + phrase_interval[phrase_index]) {
				// phrase 를 phrase_index까지 표시
				print_phrase(phrase_index);

				// 최근 phrase 표시 시간을 update
				recent_phrase_tick = tick;

				// 표시할 문자의 위치 이동
				phrase_index++;

				// 마지막 문자를 표시했다면
				if (phrase_index == phrase_len) {
					// 카메라가 돌아가고(‘#’->‘@’로 변경)
					draw_yh('@');

					// 영희가 보는 중으로 변경
					yh_watching = true;

					// 영희가 보기 시작한 시간 저장
					yh_watch_tick = tick;

					// 영희가 보는 도중 움직인 player들을 저장하는 배열을 초기화
					init_moved_player();

					// 탈락한 player들을 저장하는 배열을 초기화
					init_eliminated_player();
				}
			}

			// 영희가 보는 중일 때
		}

		else {

			// 탈락한 사용자를 찾는다.
			check_eliminated();

			// 영희가 보는 시간이 끝나면
			if (tick == yh_watch_tick + 3000) {
				// phrase를 모두 지운다.
				clear_phrase();

				// 영희가 보지 않는 상태로 변경
				yh_watching = false;

				// phrase의 처음부터 표시하도록 index 초기화
				phrase_index = 0;

				// 최근 phrase 표시 시간을 초기화
				recent_phrase_tick = tick;

				// 탈락한 사용자가 있는지
				bool exists_eliminated_player = false;
				for (int i = 0; i < PLAYER_MAX; i++)
				{
					if (eliminated_player[i]) {
						exists_eliminated_player = true;
						break;
					}
				}

				// 탈락한 사용자가 있으면 dialog에 message 출력
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
							// 탈락 표시
							player[i] = false;

							// 탈락한 player는 맵에 표시하지 않음
							clear_player(i);

							// 살아남은 player의 수를 줄임
							n_alive--;
						}
					}

					sprintf_s(message + strlen(message), sizeof(message) - strlen(message), " dead!");

					dialog(message);
				}
				// 카메라가돌아가고(‘@’->‘#’로 변경)
				draw_yh('#');
			}
		}

		// player 0만 손으로 움직임(4 방향)
		key_t key = get_key();
		if (key == K_QUIT) {
			break;
		}
		else if (key != K_UNDEFINED) {
			move_manual(key);
		}

		// 0번째 player 외에는 임의로 움직임
		move_random_players();

		display();

		Sleep(10);
		tick += 10;

		// 통과한 player들을 찾는다.
		check_passed();
		// 통과한 player들은 맵에 표시하지 않음
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (passed_player[i]) {
				clear_player(i);
			}
		}

		// 살아남은 player가 하나이면 게임을 종료
		if (n_alive <= 1) {
			break;
		}
	}
}