#pragma once

typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode_t;

typedef struct List {
    int size;
    ListNode_t* first;
} List_t;

List_t* make_list();
void free_list(List_t* list);
void* list_elem_at(List_t* list, int index);
void list_append(List_t* list, void* elem);
void list_pop(List_t* list);
