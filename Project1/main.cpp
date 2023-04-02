#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>
#include <memory.h>
#include <string.h>

#include "Util.h"

#define FPS_24	24.0f
#define FPS_60	60.0f
#define FPS_144	144.0f

#define LEFTARROW	75
#define RIGHTARROW	77
#define UPARROW		72
#define DOWNARROW	80

#define MAP_SIZE_VERTICAL	17
#define MAP_SIZE_HORIZONTAL	15

#define SNAKE_BODY_LENGTH	(MAP_SIZE_VERTICAL - 2) * (MAP_SIZE_HORIZONTAL - 2)

#define GET_KEY(buffer, key) buffer == key ? true : false

static int input_buffer = 0;	// 키 입력 버퍼 (이 프로그램에선 방향키와 ESC만 입력받음)
static int fps			= 0;	// fps
static int fps_count	= 0;	// 초당 프레임 카운트
static float fps_time	= 0;	// 프레임 계산을 위한 delta time 누적 시간
static float delta_time = 0;	// 이 전 프레임의 작동 시간 (보통 0 ~ 2ms 내외)

static int screen_buffer_index = 0;
static HANDLE screen_buffer[2];

// 0: floor
// 1: wall
// 2: food
// 3: snake head
// 4: snake body
int current_map_buffer[MAP_SIZE_HORIZONTAL][MAP_SIZE_VERTICAL] = {0, };

// n= Move n tile per sec
float snake_speed = 10.0f;
Vector2 snake_pos = Vector2(2, 5);
Vector2 snake_dir = *Vector2_Right;
LinkedList snake_body;

Vector2* food_spawn_tiles;
Vector2 food_pos = Vector2(-1, -1);

bool game = true;

void ScreenInit();
void ScreenFlipping();
void ScreenClear();
void ScreenPrint(int, int, char*);

void SnakeMove(Vector2, Vector2, float);
void DrawScreen();
void MapIntoBuffer();
void FoodIntoBuffer();
void SnakeIntoBuffer();

bool GameOver();

int main()
{
	clock_t frame_start;
	clock_t frame_end;
	
	// init
	ScreenInit();

	for (int i = 0; i < 3; i++)
	{
		snake_pos += *Vector2_Right;
		snake_body.AddFront(snake_pos);
	}

	// game
	while (game)
	{
		frame_start = clock();
		// render per frame
		if (fps_time >= (float)(1.0f / FPS_144))
		{
			DrawScreen();

			fps_time = 0;
			fps_count++;
		}

		// input
		if (_kbhit() && _getch() == 224)
		{
			input_buffer = _getch();

			// 위 화살표와 아래 화살표의 역활이 반대임..
			// 아마 스크린의 시작(0,0)이 좌측하단이 아니라 좌측상단에 있어서 그런것 같음
			if (GET_KEY(input_buffer, UPARROW))
			{
				if (snake_dir != *Vector2_Up)
				{
					snake_dir = *Vector2_Down;
				}
			}
			else if (GET_KEY(input_buffer, DOWNARROW))
			{
				if (snake_dir != *Vector2_Down)
				{
					snake_dir = *Vector2_Up;
				}
			}
			else if (GET_KEY(input_buffer, LEFTARROW))
			{
				if (snake_dir != *Vector2_Right)
				{
					snake_dir = *Vector2_Left;
				}
			}
			else if (GET_KEY(input_buffer, RIGHTARROW))
			{
				if (snake_dir != *Vector2_Left)
				{
					snake_dir = *Vector2_Right;
				}
			}
		}

		// processing
		SnakeMove(snake_body.head->position, snake_dir, snake_speed * delta_time);

		// frame
		frame_end = clock();
		delta_time = (float)(frame_end - frame_start) / CLOCKS_PER_SEC;
		fps_time += delta_time;

		// debug
		if (fps_count >= FPS_144)
		{
			fps_count = 0;
		}
	}

	CloseHandle(screen_buffer[0]);
	CloseHandle(screen_buffer[1]);

	return 0;
}

void ScreenInit()
{
	CONSOLE_CURSOR_INFO cci = { 0, };

	// 화면 버퍼 2개를 만든다.
	screen_buffer[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	screen_buffer[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	// 커서를 숨긴다.
	cci.dwSize = 1;
	cci.bVisible = FALSE;
	SetConsoleCursorInfo(screen_buffer[0], &cci);
	SetConsoleCursorInfo(screen_buffer[1], &cci);
}

void ScreenFlipping()
{
	SetConsoleActiveScreenBuffer(screen_buffer[screen_buffer_index]);
	screen_buffer_index == 0 ? 1 : 0;
}

void ScreenClear()
{
	COORD coord = { 0, 0 };
	DWORD dw;
	FillConsoleOutputCharacter(screen_buffer[screen_buffer_index], ' ', MAP_SIZE_VERTICAL * MAP_SIZE_HORIZONTAL, coord, &dw);
}

void ScreenPrint(int x, int y, const char* str)
{
	DWORD dw;
	COORD cursor_pos = { x * 2, y };
	SetConsoleCursorPosition(screen_buffer[screen_buffer_index], cursor_pos);
	WriteFile(screen_buffer[screen_buffer_index], str, strlen(str), &dw, NULL);
}

// speed = speed * delta_time
void SnakeMove(const Vector2 now_pos, const Vector2 dir, const float speed)
{
	static float total_speed_num = 0;
	total_speed_num += speed;
	if (total_speed_num >= 1.0f)
	{
		Node* body = snake_body.head->next;

		// game over logic start
		while (body != null)
		{
			if ((now_pos + dir) == body->position)
			{
				game = false;
				return;
			}
			body = body->next;
		}

		if ((now_pos + dir).x == 0 || (now_pos + dir).x == MAP_SIZE_VERTICAL - 1 ||
			(now_pos + dir).y == 0 || (now_pos + dir).y == MAP_SIZE_HORIZONTAL - 1)
		{
			game = false;
			return;
		}
		// game over logic end

		if ((now_pos + dir) == food_pos)
		{
			snake_body.AddFront(food_pos);
			food_pos = Vector2(-1, -1);
		}

		body = snake_body.tail;
		while (body != null)
		{
			if (body->previous != null)
			{
				body->position = body->previous->position;
			}
			body = body->previous;
		}
		snake_body.head->position = Vector2(now_pos.x + dir.x, now_pos.y + dir.y);
		total_speed_num = 0;
	}
}

void DrawScreen()
{
	ScreenClear();

	MapIntoBuffer();
	SnakeIntoBuffer();
	FoodIntoBuffer();

	for (int x = 0; x < MAP_SIZE_HORIZONTAL; x++)
	{
		for (int y = 0; y < MAP_SIZE_VERTICAL; y++)
		{
			switch (current_map_buffer[x][y])
			{
				case 0:
					ScreenPrint(y, x, "  ");
					break;
				case 1:
					ScreenPrint(y, x, "▤");
					break;
				case 2:
					ScreenPrint(y, x, "♥");
					break;
				case 3:
					ScreenPrint(y, x, "■");
					break;
				case 4:
					ScreenPrint(y, x, "□");
					break;

				default:
					break;
			}
		}
	}
	
	ScreenFlipping();
}

void MapIntoBuffer()
{
	for (int x = 0; x < MAP_SIZE_HORIZONTAL; x++)
	{
		for (int y = 0; y < MAP_SIZE_VERTICAL; y++)
		{
			if (x == 0 || x == MAP_SIZE_HORIZONTAL - 1 || y == 0 || y == MAP_SIZE_VERTICAL - 1)
			{
				current_map_buffer[x][y] = 1;
			}
			else
			{
				current_map_buffer[x][y] = 0;
			}
		}
	}
}

void FoodIntoBuffer()
{
	if ((int)food_pos.x == -1 || (int)food_pos.y == -1)
	{
		srand((unsigned int)time(null));

		int food_random_index = 0;
		int food_spawn_tile_index = 0;
		const int food_spawn_tile_count = MAP_SIZE_VERTICAL * MAP_SIZE_HORIZONTAL - (MAP_SIZE_VERTICAL * 2) - ((MAP_SIZE_HORIZONTAL - 2) * 2) - snake_body.GetCount() - 1;
		food_spawn_tiles = new Vector2[food_spawn_tile_count];
		for (int x = 0; x < MAP_SIZE_HORIZONTAL; x++)
		{
			for (int y = 0; y < MAP_SIZE_VERTICAL; y++)
			{
				if (current_map_buffer[x][y] == 0)
				{
					food_spawn_tiles[food_spawn_tile_index++] = Vector2((float)y, (float)x);
				}
			}
		}

		food_random_index = rand();
		food_pos = food_spawn_tiles[food_random_index % food_spawn_tile_count];

		delete[] food_spawn_tiles;
	}
	else 
	{
		current_map_buffer[(int)food_pos.y][(int)food_pos.x] = 2;
	}
}

void SnakeIntoBuffer()
{
	const Node* body = snake_body.head;
	current_map_buffer[(int)body->position.y][(int)body->position.x] = 3;
	body = body->next;
	while (body != null)
	{
		current_map_buffer[(int)body->position.y][(int)body->position.x] = 4;
		body = body->next;
	}
}

bool GameOver()
{


	return true;
}