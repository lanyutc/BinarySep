#ifndef _BINARYSEP_H
#define	_BINARYSEP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "intrusive_list.h"

using namespace std;

/**
 *   将变量内容分开保存
 */
template <typename T>
class BinarySep {
public:
	BinarySep() {}
    BinarySep(const T *t, uint32_t heap_size = 0);
    BinarySep(const T &t, uint32_t heap_size = 0);
	BinarySep(const BinarySep &cp);
    ~BinarySep();
	BinarySep& operator=(const BinarySep &cp);

    void hide(const T &t, uint32_t heap_size = 0);
    T get();



private:
	void init_hide_list();
	bool hide_list_empty();
	void check_heap_content(uint32_t heap_size);
	bool is_heap_size() {return false;}
	char* get_content_ptr() {return (char *)&origi;}


#define SEP_UNIT 3 //设置按几个字节来分片
#define PTR_SIZE (sizeof(unsigned long))
private:
	uint32_t size;
	static const uint32_t padding_size = PTR_SIZE - (SEP_UNIT % PTR_SIZE);
	list_head_t hide_head;
	char *heap_origi;
	T origi;

	typedef struct unit_list {
		struct list_head list;
		char hide[SEP_UNIT];
		char padding[padding_size]; //用随机内容填充多余的空间造成混淆
	}unit_list_t;
};

template<> bool  BinarySep<char *>::is_heap_size() {return true;}
template<> bool  BinarySep<const char *>::is_heap_size() {return true;}
template<> char *BinarySep<char *>::get_content_ptr() {return heap_origi;}
template<> char *BinarySep<const char *>::get_content_ptr() {return heap_origi;}


template <typename T>
BinarySep<T>::BinarySep(const T *t, uint32_t heap_size)
{
	size = is_heap_size() ? heap_size : sizeof(T);
	heap_origi = NULL;
	init_hide_list();
	hide(*t, heap_size);
}


template <typename T>
BinarySep<T>::BinarySep(const T &t, uint32_t heap_size)
{
	size = is_heap_size() ? heap_size : sizeof(T);
	heap_origi = NULL;
	init_hide_list();
	hide(t, heap_size);
}

template <typename T>
BinarySep<T>::BinarySep(const BinarySep<T> &cp)
{
	size = cp.size;
	heap_origi = NULL;
	init_hide_list();
	hide(cp.origi, cp.size);
}

template <typename T>
BinarySep<T>::~BinarySep()
{
	list_head_t *l, *p;
	list_for_each_safe(l, p, &hide_head) {
		unit_list_t *unit = list_entry(l, unit_list_t, list);
		if (!unit) continue;
		free(unit);
	}

	if (heap_origi) {
		free(heap_origi);
	}
}

template <typename T>
BinarySep<T>& BinarySep<T>::operator=(const BinarySep<T> &cp) 
{
	if (this == &cp) {
		return *this;
	}

	size = cp.size;

	if (hide_list_empty()) {
		init_hide_list();
	}

	hide(cp.origi, cp.size);
	return *this;
}

template <typename T>
void BinarySep<T>::hide(const T &t, uint32_t heap_size)
{
	origi = t;
	check_heap_content(heap_size);

	list_head_t *l = hide_head.next;
	char *c = get_content_ptr();
	for (uint32_t i = 0; i < size; l = l->next) {
		if (l == &hide_head) break;

		unit_list_t *field = list_entry(l, unit_list_t, list);

		for (uint32_t j = 0; (i < size && j < SEP_UNIT); i++, j++, c++) {
			field->hide[j] = *c ^ *(char *)&hide_head;
		}
	}
}

template <typename T>
T BinarySep<T>::get()
{
	list_head_t *l = hide_head.next;
	char *c = get_content_ptr();
	for (uint32_t i = 0; i < size; l = l->next) {
		if (l == &hide_head) break;

		unit_list_t *field = list_entry(l, unit_list_t, list);

		for (uint32_t j = 0; (i < size && j < SEP_UNIT); i++, j++, c++) {
			if (*c != (field->hide[j] ^ *(char *)&hide_head)) {
				//这里处理内存被修改了
			}
		}
	}
	return heap_origi ? *(T *)&heap_origi : origi;
}

template <typename T>
void BinarySep<T>::init_hide_list()
{
	INIT_LIST_HEAD(&hide_head);
	srand(*(uint32_t *)&hide_head);
	for (uint32_t i = 0; i < size; i += SEP_UNIT) {
		unit_list_t *unit = (unit_list_t *)calloc(1, (sizeof(unit_list_t)));
		for (uint32_t j = 0; j < padding_size; j++) {
			unit->padding[j] = rand();
		}

		list_add(&unit->list, &hide_head);
	}
}


template <typename T>
bool BinarySep<T>::hide_list_empty()
{
	if (list_empty(&hide_head)) return true;

	list_head_t *l = hide_head.next;
	for (uint32_t i = 0; i < size; i += SEP_UNIT, l = l->next) {

		unit_list_t *field = list_entry(l, unit_list_t, list);
		if (!field) return true;
	}
	return false;
}

template <typename T>
void BinarySep<T>::check_heap_content(uint32_t heap_size)
{
	heap_origi = NULL;
}

template <>
void BinarySep<char *>::check_heap_content(uint32_t heap_size)
{
	if (heap_origi && heap_size > size) { //已经存在数据，现有的内存空间不足
		free(heap_origi);
		heap_origi = NULL;
		//TODO 扩大list
	}

	if (!heap_origi) {
		heap_origi = (char *)calloc(1, sizeof(char) * heap_size + 1);
	}

	memcpy(heap_origi, ((char *)origi), heap_size);
	heap_origi[heap_size] = '\0';
	size = heap_size;
}

template <>
void BinarySep<const char *>::check_heap_content(uint32_t heap_size)
{
	if (heap_origi && heap_size > size) { //已经存在数据，现有的内存空间不足
		free(heap_origi);
		heap_origi = NULL;
		//TODO 扩大list
	}

	if (!heap_origi) {
		heap_origi = (char *)calloc(1, sizeof(char) * heap_size + 1);
	}

	memcpy(heap_origi, ((char *)origi), heap_size);
	heap_origi[heap_size] = '\0';
	size = heap_size;
}


#endif
