
#include <cstring>
class priority_queue
{
private:
	char * value;
	int key;
public:
	priority_queue (const char * v="", int k=0) : key(k)
	{
		value = new char[strlen(v) + 1];
        strcpy(value, v);
	}
    virtual ~priority_queue() noexcept
	{
		delete[] value;
	}

public:

    virtual char const *remove_max() = 0;

    virtual char const *obtain_max() const = 0;

    virtual priority_queue *merge(
        priority_queue *with) = 0;

};
