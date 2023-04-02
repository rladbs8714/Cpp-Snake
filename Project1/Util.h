#pragma once

#define null 0

class Vector2
{
public:
	float x;
	float y;

	Vector2() : x(0), y(0) {};
	Vector2(float _x, float _y) : x(_x), y(_y) {};

	const Vector2 operator+(const Vector2 ref) const;
	const Vector2 operator+=(const Vector2& ref);

	const bool operator==(const Vector2& ref) const;
	const bool operator!=(const Vector2& ref);
};

struct Node
{
	Vector2 position;
	Node* previous = null;
	Node* next = null;
};

class LinkedList
{
public:
	Node* head;
	Node* tail;

	LinkedList()
	{
		head = null;
		tail = null;
	}

	void AddFront(Vector2 pos);
	void Clear();
	int GetCount();
};

const static Vector2* Vector2_Up		= new Vector2(0, 1);
const static Vector2* Vector2_Down		= new Vector2(0, -1);
const static Vector2* Vector2_Left		= new Vector2(-1, 0);
const static Vector2* Vector2_Right		= new Vector2(1, 0);