#include <iostream>
#include <cstring>
#include <limits.h>
using namespace std;

#define ERROR 1
#define CORRECT 0
//priority - ключ типа int
//char* - значение
struct item {
    int priority;
    char* value;
};

struct Priority_queue{
    int size, capacity; //размер на данный момент и максимально возможный размер
    item* ITEM; //массив из item-ов
};

//инициализация первого элемента
int initialize(Priority_queue& queue) {
    queue.size = 0;
    queue.capacity = 2;
    try {
        queue.ITEM =new item[queue.capacity];
    }catch(const std::bad_alloc& e){
        return ERROR;
    }
    
    return CORRECT;
}

//расширение очереди до предыдущего размера*2
void resize(Priority_queue& queue) {
    int newCapacity=queue.capacity*2;
    item* newItem=nullptr;
    try {
        newItem = new item[newCapacity];
    } catch (const std::bad_alloc& e) {
        return;
    }

    for (int i = 0; i < queue.size; ++i) {
        newItem[i] = queue.ITEM[i];
    }

    delete[] queue.ITEM;
    queue.ITEM = newItem;
    queue.capacity = newCapacity;

}
//добавление значения типа char * (строка в стиле C) по ключу типа int в приоритетную очередь, с копированием строки в контекст структуры
void enqueue(Priority_queue&queue, item element) {
    if (queue.size == queue.capacity) {
        resize(queue);
    }
    item newElement;
    newElement.priority=element.priority;
    newElement.value=strdup(element.value);
    queue.ITEM[queue.size] = newElement;
    queue.size++;
}

//поиск значения по наиболее приоритетному ключу
char* peek(Priority_queue& queue) {
    if (queue.size == 0) return NULL;
    int highest_priority_index = 0;

    for (int i = 1; i < queue.size; ++i) {
        if (queue.ITEM[i].priority > queue.ITEM[highest_priority_index].priority) {
            highest_priority_index = i;
        }else if (queue.ITEM[i].priority == queue.ITEM[highest_priority_index].priority) {
            if (strcmp(queue.ITEM[i].value, queue.ITEM[highest_priority_index].value) < 0) {
                highest_priority_index = i;
            }
        }
    }
    return queue.ITEM[highest_priority_index].value;
}

//удаление значения по наиболее приоритетному ключу
void dequeue(Priority_queue& queue) {
    if (queue.size == 0) return;

    int highest_priority_index = 0;
    for (int i = 1; i < queue.size; ++i) {
        if (queue.ITEM[i].priority > queue.ITEM[highest_priority_index].priority) {
            highest_priority_index = i;
        } else if (queue.ITEM[i].priority == queue.ITEM[highest_priority_index].priority) {
            if (strcmp(queue.ITEM[i].value, queue.ITEM[highest_priority_index].value) < 0) {
                highest_priority_index = i;
            }
        }
    }

    free(queue.ITEM[highest_priority_index].value);

    for (int i = highest_priority_index; i < queue.size - 1; ++i) {
        queue.ITEM[i] = queue.ITEM[i + 1];
    }
    queue.size--;
}

void cleanup(Priority_queue& queue) {
    for (int i = 0; i < queue.size; i++) {
        free(queue.ITEM[i].value);
    }
    free(queue.ITEM);
}
bool check_if_priority_queue(Priority_queue&queue){
    if (queue.size==0){
        return true;
    }
    for (int i=0;i<queue.size-1;i++){
        if (queue.ITEM[i].priority<queue.ITEM[i+1].priority){
            continue;
        }else if (queue.ITEM[i].priority==queue.ITEM[i+1].priority){
            if (strcmp(queue.ITEM[i].value, queue.ITEM[i + 1].value) <= 0) {
                continue;
            } else {
                return false;
            }
        }else{
            return false;
        }
    }
    return true;
}
//слияние двух приоритетных очередей в вызывающий объект приоритетной очереди, с поддержкой fluent API

class merge_fluent_API{
    private:
        Priority_queue queue;
    public:
        //конструктор
        merge_fluent_API(){initialize(queue);}
        //деструктор
        ~merge_fluent_API(){cleanup(queue);}

        merge_fluent_API& enqueue(item element){
            ::enqueue(queue,element);
            return *this;
        }
        merge_fluent_API& dequeue(){
            ::dequeue(queue);
            return *this;
        }

        char * peek(){
            return ::peek(queue);
        }
        

        merge_fluent_API&merge(merge_fluent_API& other){
            if (check_if_priority_queue(queue)&&check_if_priority_queue(other.queue)){
                int newSize=queue.size+other.queue.size;
                if (newSize>queue.capacity){
                    while(newSize>queue.capacity){
                        resize(queue);
                    }
                }
                for (int i=0;i<other.queue.size;i++){
                    item newElement;
                    newElement.priority=other.queue.ITEM[i].priority;
                    newElement.value=strdup(other.queue.ITEM[i].value);
                    queue.ITEM[queue.size]=newElement;
                    queue.size++;

                }
            }
            return *this;
        }

};
int main(){
    merge_fluent_API queue1,queue2;

    item a;
    a.priority = 1;
    a.value = strdup("butterfly");
    
    item b;
    b.priority = 2;
    b.value = strdup("ladybug");
    
    item c;
    c.priority = 3;
    c.value = strdup("cockroach");

    queue1.enqueue(a).enqueue(b).enqueue(c);

    std::cout << "Peek: " << queue1.peek() << std::endl;
    queue1.dequeue();
    std::cout << "Peek after dequeue: " << queue1.peek() << std::endl;
    
    item a2;
    a2.priority = 1;
    a2.value = strdup("dragonfly");
    
    item b2;
    b2.priority = 2;
    b2.value = strdup("fly");
    
    item c2;
    c2.priority = 3;
    c2.value = strdup("help");

    queue2.enqueue(a2).enqueue(b2).enqueue(c2);

    queue1.merge(queue2);
    std::cout << "Peek after merge: " << queue1.peek() << std::endl;
    return CORRECT;
}