class priority_queue
{
private:
	char * value;
	int key;
public:
	virtual priority_queue (const char * v="",const int k=0) : value(v.value), key(k.key);
    virtual ~priority_queue() noexcept = default;

public:

    virtual char const *remove_max() = 0;

    virtual char const *obtain_max() = 0;

    virtual priority_queue *merge(
        priority_queue *with) = 0;

};
