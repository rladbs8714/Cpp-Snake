#include "Util.h"

const Vector2 Vector2::operator+(const Vector2 ref) const
{
	return Vector2(x + ref.x, y + ref.y);
}

const Vector2 Vector2::operator+=(const Vector2& ref)
{
	return Vector2(x += ref.x, y += ref.y);
}

const bool Vector2::operator==(const Vector2& ref) const
{
	if (x != ref.x) return false;
	if (y != ref.y) return false;

	return true;
}

const bool Vector2::operator!=(const Vector2& ref)
{
	if (x == ref.x && y == ref.y) return false;

	return true;
}

void LinkedList::AddFront(Vector2 pos)
{
	Node* temp = new Node;
	temp->position = pos;

	if (head == null)
	{
		head = temp;
		tail = temp;
	}
	else
	{
		head->previous = temp;
		temp->next = head;
		head = temp;
	}
}

void LinkedList::Clear()
{

}

int LinkedList::GetCount()
{
	int count = 0;
	Node* end = tail;
	while (end != null)
	{
		end = end->next;
		count++;
	}
	return count;
}