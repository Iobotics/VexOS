//
//  List.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/06/2012.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
//

#include "VexOS.h"
#include "Error.h"

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

List* List_new() {
    List* list = malloc(sizeof(List));
    list->nodeCount = 0;
    list->firstNode = NULL;
    list->lastNode  = NULL;
    return list;
}

ListNode* List_newNode(void* data) {
    ListNode* node = malloc(sizeof(ListNode));
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    node->list = NULL;
    return node;
}

void List_insertAfter(ListNode* node, ListNode* newNode) {
    ErrorIf(node == NULL,    VEXOS_ARGNULL);
    ErrorIf(newNode == NULL, VEXOS_ARGNULL);
    
    if(node->list == NULL) return;
    if(newNode->list != NULL) {
        List_remove(newNode);
    }
    newNode->prev = node;
    newNode->next = node->next;
    if(node->next == NULL) {
        node->list->lastNode = newNode;
    } else {
        node->next->prev = newNode;
    }
    node->next    = newNode;
    newNode->list = node->list;
    node->list->nodeCount++;
}

void List_insertBefore(ListNode* node, ListNode* newNode) {
    ErrorIf(node == NULL,    VEXOS_ARGNULL);
    ErrorIf(newNode == NULL, VEXOS_ARGNULL);
    
    if(node->list == NULL) return;
    if(newNode->list != NULL) {
        List_remove(newNode);
    }
    newNode->prev = node->prev;
    newNode->next = node;
    if(node->prev == NULL) {
        node->list->firstNode = newNode;
    } else {
        node->prev->next = newNode;
    }
    node->prev    = newNode;
    newNode->list = node->list;
    node->list->nodeCount++;
}

void List_insertFirst(List* list, ListNode* newNode) {
    ErrorIf(list == NULL,    VEXOS_ARGNULL);
    ErrorIf(newNode == NULL, VEXOS_ARGNULL);
    
    if(newNode->list != NULL) {
        List_remove(newNode);
    }
    if(list->firstNode == NULL) {
        list->firstNode = newNode;
        list->lastNode  = newNode;
        newNode->prev   = NULL;
        newNode->next   = NULL;
        newNode->list   = list;
        list->nodeCount++;
    } else {
        List_insertBefore(list->firstNode, newNode);
    }
}

void List_insertLast(List* list, ListNode* newNode) {
    ErrorIf(list == NULL,    VEXOS_ARGNULL);
    ErrorIf(newNode == NULL, VEXOS_ARGNULL);
    
    if(newNode->list != NULL) {
        List_remove(newNode);
    }
    if(list->lastNode == NULL) {
        List_insertFirst(list, newNode);
    } else {
        List_insertAfter(list->lastNode, newNode);
    }
}

// returns the next node in the list //
ListNode* List_remove(ListNode* node) {
    ErrorIf(node == NULL, VEXOS_ARGNULL);
    
    ListNode* ret = node->next;
    if(node->list == NULL) return NULL;
    if(node->prev == NULL) {
        node->list->firstNode = node->next;
    } else {
        node->prev->next = node->next;
    }
    if(node->next == NULL) {
        node->list->lastNode = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    node->list->nodeCount--;
    node->list = NULL;
    node->prev = NULL;
    node->next = NULL;
    return ret;
}

ListNode* List_findNode(List* list, void* data) {
    ErrorIf(list == NULL, VEXOS_ARGNULL);
    
    ListNode* node = list->firstNode;
    while(node != NULL) {
        if(node->data == data) return node;
        node = node->next;
    }
    return NULL;
}

unsigned int List_indexOfData(List* list, void* data) {
    ErrorIf(list == NULL, VEXOS_ARGNULL);
    
    ListNode* node = list->firstNode;
    unsigned int i = 0;
    while(node != NULL) {
        if(node->data == data) return i;
        i++;
        node = node->next;
    }
    return -1;
}

unsigned int List_indexOfNode(ListNode* node) {
    ErrorIf(node == NULL, VEXOS_ARGNULL);
    
    if(node->list == NULL) return -1;
    ListNode* xnode = node->list->firstNode;
    unsigned int i = 0;
    while(xnode != NULL) {
        if(xnode == node) return i;
        i++;
        xnode = xnode->next;
    }
    return -1;
}

ListNode* List_getNodeByIndex(List* list, unsigned int n) {
    ErrorIf(list == NULL, VEXOS_ARGNULL);
    ErrorIf(n < 0, VEXOS_ARGRANGE);
    
    ListNode* node = list->firstNode;
    unsigned int i = 0;
    while(node != NULL) {
        if(i++ == n) break;
        node = node->next;
    }
    return node;
}

void* List_getDataByIndex(List* list, unsigned int n) {
    ErrorIf(list == NULL, VEXOS_ARGNULL);
    ErrorIf(n < 0, VEXOS_ARGRANGE);
    
    ListNode* node = list->firstNode;
    unsigned int i = 0;
    while(node != NULL) {
        if(i++ == n) break;
        node = node->next;
    }
    return (node)? node->data: NULL;
}