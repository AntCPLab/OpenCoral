// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * mq.h
 *
 */

#ifndef NETWORK_INC_MQ_H_
#define NETWORK_INC_MQ_H_


#include<iostream>
#include<cstdlib>
#define default_value 500
using namespace std;

template< class T > class Queue
{
    public:
        Queue(int = default_value);//default constructor
        ~Queue()//destructor
          {delete [] values;}
        bool Enqueue( T );
        T Dequeue();
        bool Empty();
        bool isFull();
    private:
        int size;
        T *values;
        int front;
        int back;
};

template< class T > Queue<T>::Queue(int x):
    size(x),//ctor
    values(new T[size]),
    front(0),
    back(0)
      { /*empty*/  }

template< class T > bool Queue<T>::isFull()
{
//	printf("mq:: isFull() - back=%d, size=%d, from=%d\n", back, size, front);
    if((back + 1) %  size == front )
        return 1;
    else
        return 0;
}

template< class T > bool Queue<T>::Enqueue(T x)
{
    bool b = 0;
   if(!isFull())
   {
       values[back] = x;
       back = (back + 1) % size;
       b = 1;
   }
  return b;
}

template< class T > bool Queue<T>::Empty()
{
    if( back  == front )//is empty
        return 1;
    else
    return 0; //is not empty
}

template< class T > T Queue<T>::Dequeue()
{
    T val;
    if(!Empty())
    {
        val = values[front];
        front = ( front + 1 ) % size;
    }
    else
    {
        cerr << "Queue is Empty : ";
    }
return val;

}


#endif /* NETWORK_INC_MQ_H_ */
