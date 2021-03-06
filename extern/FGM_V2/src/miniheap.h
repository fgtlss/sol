
#ifndef _FULL_MIN_HEAP_H_
#define _FULL_MIN_HEAP_H_
#include<math.h>
#include<stdio.h>
#include "random.h"
#include "linear.h"
#include "shash.h"
//#include<iostream.h>
//using namespace std;

class half_tree
{
public:
	half_tree(int n);
    ~half_tree()
	{
		delete []half_array;
	}
	int insert(int elem_pos, weight elem);
	int insert_elem(int elem_pos, weight elem);
	int get_add_elem();
	int get_change_elem();

	weight get_elem(int elem_pos);
    int add_update(int elem_pos, double elem_val);
	int add_update(stdext::hash_map<size_t,double>   element_hash_map, int probn);

protected:
private:
	weight *half_array;
	int median_pos;
	double median_val;
	weight median_elem;
	int n_size;
	int current_size;
};
weight half_tree::get_elem(int elem_pos)
{
	return half_array[elem_pos];
}
half_tree::half_tree(int n)
{
	if(n<=0)
		return;
	n_size = n;
    median_pos = 10;
	half_array=new weight[n];
	for (int i=0; i<n_size; i++)
	{
		half_array[i].index1 = 0;
		half_array[i].index2 = 0;
		half_array[i].value = 0;
	}
	median_val = 0.0;
	median_elem = half_array[median_pos];
	current_size = 0;
}

int half_tree::add_update(stdext::hash_map<size_t,double>   element_hash_map, int probn)
{
	
	weight w_min;
	int select_idx;
	double w_value;
	weight median_temp;
	for (int i=0; i<current_size; i++ )
	{
		w_min = half_array[i];
		if (w_min.index2==-1)
		{
			select_idx = w_min.index1;
		}else
		{
			select_idx = (w_min.index1+1)*probn - w_min.index1*(w_min.index1+1)/2 + w_min.index2;
		}
		w_value = element_hash_map[select_idx];
		//if(abs(abs(w_value)-abs(w_min.value))>1e-6)
		{
			add_update(i,w_value);
			continue;
		}
		//if(abs(w_value)>abs(median_temp.value)&&i<median_pos)
		//{
		//	add_update(i,w_value);
		//	continue;
		//}
		//if(abs(w_value)<abs(median_temp.value)&&i>median_pos)
		//{
		//	add_update(i,w_value);
		//	continue;
		//}
	}
	return 0;
}



int half_tree::insert(int elem_pos, weight elem)
{
	half_array[current_size] = elem;
	
	
	current_size++;
    weight elem_temp;
	if (current_size==n_size)
	{
		for(int i=0; i<n_size-1; i++)
		{
			for(int j = i+1; j<n_size; j++)
			{
				if(abs(half_array[j].value)>abs(half_array[i].value))
				{
					elem_temp = half_array[j];
					half_array[j] = half_array[i];
					half_array[i] = elem_temp;
				}
			}
		}
	}
	return 0;
}
int half_tree::insert_elem(int elem_pos, weight elem)
{
	median_elem = half_array[median_pos];
	if (abs(elem.value)>abs(median_elem.value))
	{
		//right side, no need to change with elem_pos<median_pos
		half_array[median_pos] =  elem;
		half_array[elem_pos] = median_elem;
	}
	else
	{
		half_array[elem_pos] =  elem;
	}
	return 0;
}

int half_tree::get_add_elem()
{
	int key_elem = random() %(n_size);
	return key_elem;
}

int half_tree::add_update(int elem_pos, double elem_val)
{
	median_elem = half_array[median_pos];
	
	if (abs(elem_val)>abs(median_elem.value))
	{
		//right side, no need to change with elem_pos<median_pos
		if (elem_pos>median_pos)
		{
			half_array[elem_pos].value = elem_val;
		}else
		{
			half_array[elem_pos].value = elem_val;
			half_array[median_pos] =  half_array[elem_pos];
			half_array[elem_pos] = median_elem;
		}

	}
	else
	{
		if (elem_pos < median_pos)
		{
			half_array[elem_pos].value = elem_val;
		}else
		{
			half_array[elem_pos].value = elem_val;
			half_array[median_pos] =  half_array[elem_pos];
			half_array[elem_pos] = median_elem;
		}

	}
	return 0;
}

int half_tree::get_change_elem()
{
	//make sure median_pos-1>0
	int key_elem = random() %(median_pos-1);
	return key_elem;
}

template <class T>
class MinHeap  
{
public:
	T* heapArray;
	int CurrentSize;
	int MaxSize;

public:
	MinHeap(){};
	MinHeap(const int n);
	virtual ~MinHeap()
	{delete []heapArray;};
	void BuildHeap();
	void Allcate(int n);
	bool isLeaf(int pos) const;
	int leftchild(int pos) const;
	int rightchild(int pos) const;
	// Return parent position
	int parent(int pos) const;
	// 删除给定下标的元素
	bool Remove(int pos, T& node);
	bool Remove(int pos);
	void SetValue(int i, T elem);
	void SiftDown(int left);
	//从position向上开始调整，使序列成为堆
	void SiftUp(int position); 
	bool Insert(T& newNode);
	T& DeleteMin();
	T Min() {return heapArray[0];}
	T GetElement(int i)
	{
		return  heapArray[i];;
	}
	int Size() const {return CurrentSize;}
	bool empty()
	{
		if(CurrentSize>0)
			return false;
		else
			return true;
	}
};


template<class T>
void MinHeap<T>::SetValue(int i, T elem)
{
	Remove(i);
	Insert(elem);
}

template<class T>
MinHeap<T>::MinHeap(const int n)
{
	if(n<=0)
		return;
	CurrentSize=0;
	MaxSize=n;
	heapArray=new T[MaxSize];
	BuildHeap();
}

template<class T>
void MinHeap<T>::Allcate(int n)
{
	if(n<=0)
		return;
	CurrentSize=0;
	MaxSize=n;
	heapArray=new T[MaxSize];
	BuildHeap();
}

template<class T>
void MinHeap<T>::BuildHeap()
{
	for (int i=CurrentSize/2-1; i>=0; i--) 
		SiftDown(i); 
}

template<class T>
bool MinHeap<T>::isLeaf(int pos) const
{
	return (pos>=CurrentSize/2)&&(pos<CurrentSize);
}

template<class T>
int MinHeap<T>::leftchild(int pos) const
{
	return 2*pos+1;      //返回左孩子位置
}

template<class T>
int MinHeap<T>::rightchild(int pos) const
{
	return 2*pos+2;      //返回右孩子位置
}

template<class T>
int MinHeap<T>::parent(int pos) const // 返回父节点位置
{
	return (pos-1)/2;
}

template<class T>
void MinHeap<T>::SiftDown(int left)
{
	//准备
	int i=left;       //标识父结点
	int j=2*i+1;      //标识关键值较小的子结点  
	T temp=heapArray[i];    //保存父结点
	//过筛
	while(j<CurrentSize)
	{
		if((j<CurrentSize-1)&&(heapArray[j]>heapArray[j+1]))
			j++;      //j指向右子结点
		if(temp>heapArray[j])
		{
			heapArray[i]=heapArray[j];
			i=j;
			j=2*j+1;
		}
		else break;
	}
	heapArray[i]=temp;
}

template<class T>
void MinHeap<T>::SiftUp(int position) 
{//从position向上开始调整，使序列成为堆
	int temppos=position;
	T temp=heapArray[temppos];
	while((temppos>0)&&(heapArray[parent(temppos)]>temp))
	{
		heapArray[temppos]=heapArray[parent(temppos)];
		temppos=parent(temppos);
	}
	heapArray[temppos]=temp;
}

template<class T>
bool MinHeap<T>::Insert( T& newNode)
{
	if(CurrentSize==MaxSize)
		return false;
	heapArray[CurrentSize] = newNode;
	SiftUp(CurrentSize);
	CurrentSize++;
	return true;
}

template<class T>
T& MinHeap<T>::DeleteMin()
{
	if(CurrentSize==0)
	{
		printf("Can't Delete\n");

	}
	else
	{
		T temp=heapArray[0];     //取堆顶元素
		heapArray[0]=heapArray[CurrentSize-1]; //堆末元素上升至堆顶
		CurrentSize--;
		if(CurrentSize>1)
			SiftDown(0);      //从堆顶开始筛选
		return temp;
	}
}

template<class T>
bool MinHeap<T>::Remove(int pos, T& node)
{// 删除给定下标的元素
	if((pos<0)||(pos>=CurrentSize))
		return false;
	T temp=heapArray[pos];
	heapArray[pos]=heapArray[--CurrentSize]; //指定元素置于最后
	SiftUp(pos);        //上升筛
	SiftDown(pos);        //向下筛 
	node=temp;
	return true;
}

template<class T>
bool MinHeap<T>::Remove(int pos)
{// 删除给定下标的元素
	if((pos<0)||(pos>=CurrentSize))
		return false;
	T temp=heapArray[pos];
	heapArray[pos]=heapArray[--CurrentSize]; //指定元素置于最后
	SiftUp(pos);        //上升筛
	SiftDown(pos);        //向下筛 
	return true;
}





#endif