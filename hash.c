#include <stdio.h>
#include <stdlib.h>

#include "hash.h"

#define HASH_CELL_NUM 3

tHash *hash_create(int *freeFunc){
	tHash *pHash = NULL;

	if(!freeFunc)
		return NULL;

	pHash = (tHash *)malloc(sizeof(tHash));
	
	if(pHash != NULL)
	{
		memset((void *)pHash , 0 , sizeof(tHash));
		pHash->free = (void *)freeFunc;
		pHash->nodeArray = NULL;
	}

	return pHash;
}

//
int hash_add(tHash *pHash ,  HASH_NODE *pInsertNode){
	int growthFlag=0 , ret = 0;
	unsigned long long i = 0;
	if(!pHash || !pInsertNode)
		return 0;

	if(pHash->nodeArray == NULL){
		unsigned long long hash_len = ((pHash->max_buffer_sector + HASH_CELL_NUM - 1) / HASH_CELL_NUM);
		pHash->nodeArray = malloc(sizeof(HASH_NODE) * hash_len);
		alloc_assert(pHash->nodeArray, "pHash->nodeArray");		
		for(; i < hash_len; ++i){
			pHash->nodeArray[i] = NULL;
		}
	}

	buf_node* node = (buf_node*)pInsertNode;

	i = node->group % pHash->max_buffer_sector;
	i = i / 3;

	pInsertNode->next = pHash->nodeArray[i];
	pHash->nodeArray[i] = pInsertNode;
	
	pHash->count++;
	return ret;
}


HASH_NODE *hash_find(tHash *pHash, HASH_NODE *pKeyNode){
	if(!pHash || !pHash->count || !pHash->nodeArray)
		return NULL;
	unsigned long long pos = 0;
	buf_node* node = (buf_node*)pKeyNode;
	HASH_NODE *interNode;
	unsigned int target = node->group;
	HASH_NODE *preNode = NULL;
		
	pos = node->group % pHash->max_buffer_sector;
	pos /= 3;
	interNode = pHash->nodeArray[pos];
	
	while(interNode){
		node = (buf_node*)interNode;
		if(node->group == target)
			break;
		preNode = interNode;
		interNode = interNode->next;
	}
	if(interNode){
		if(preNode){
			preNode->next = interNode->next;
			interNode->next = pHash->nodeArray[pos];
			pHash->nodeArray[pos] = interNode;
		}
		return interNode;
	}
	
	return NULL;
}

int hash_del(tHash *pHash ,HASH_NODE *pDelNode){
	int ret = 0;
	
	unsigned long long pos = 0;
	buf_node* node = (buf_node*)pDelNode;
	HASH_NODE *interNode;
	unsigned int target = node->group;
	HASH_NODE *preNode = NULL;
	
	pos = node->group % pHash->max_buffer_sector;
	pos /= 3;
	interNode = pHash->nodeArray[pos];

	while(interNode){
		node = (buf_node*)interNode;
		if(node->group == target)
			break;
		preNode = interNode;
		interNode = interNode->next;
	}
	
	if(interNode){
		if(preNode){
			preNode->next = interNode->next;
		}else{
			pHash->nodeArray[pos] = interNode->next;
		}
		pHash->count--;
	}
	
	return 0;
}

void hash_node_free(tHash *pHash, HASH_NODE *pNode){
	if(!pHash || !pNode)
		return;

	(pHash->free)(pNode);
	return ;
}



int hash_destroy(tHash *pHash){
	HASH_NODE *pNode = NULL;
	if(!pHash)
		return 0;

	unsigned long long hash_len = ((pHash->max_buffer_sector + HASH_CELL_NUM - 1) / HASH_CELL_NUM);
	int i = 0;
	
	for(;i < hash_len; ++i){
		pNode = pHash->nodeArray[i];
		while(pNode){
			HASH_NODE *willFree = pNode;
			pNode = pNode->next;
			free(willFree);
		}
	}

	free(pHash->nodeArray);
	free(pHash);
	
	return 0;
}

HASH_NODE *hash_find_parity(struct ssd_info *ssd, tHash *pHash, HASH_NODE *pKeyNode,unsigned int raidID){
	if(!pHash || !pHash->count || !pHash->nodeArray)
		return NULL;
	unsigned long long pos = 0;
	buf_node* node = (buf_node*)pKeyNode;
	HASH_NODE *interNode;
	unsigned int target = ssd->stripe.checkLpn;
	HASH_NODE *preNode = NULL;
		
	pos = raidID % pHash->max_buffer_sector;
	pos /= 3;
	interNode = pHash->nodeArray[pos];
	
	while(interNode){
		node = (buf_node*)interNode;
		if(node->group == target && node->raidID == raidID)
			break;
 		preNode = interNode;
		interNode = interNode->next;
	}
	if(interNode){
		if(preNode){
			preNode->next = interNode->next;
			interNode->next = pHash->nodeArray[pos];
			pHash->nodeArray[pos] = interNode;
		}
		return interNode;
	}
	
	return NULL;
}

int hash_del_parity(struct ssd_info *ssd, tHash *pHash ,HASH_NODE *pDelNode,unsigned int raidID){
	int ret = 0;
	
	unsigned long long pos = 0;
	buf_node* node = (buf_node*)pDelNode;
	HASH_NODE *interNode;
	unsigned int target = ssd->stripe.checkLpn;
	HASH_NODE *preNode = NULL;
	
	pos = node->raidID % pHash->max_buffer_sector;
	pos /= 3;
	interNode = pHash->nodeArray[pos];

	while(interNode){
		node = (buf_node*)interNode;
		if(node->group == target && node->raidID == raidID)
			break;
		preNode = interNode;
		interNode = interNode->next;
	}
	
	if(interNode){
		if(preNode){
			preNode->next = interNode->next;
		}else{
			pHash->nodeArray[pos] = interNode->next;
		}
		pHash->count--;
	}else {
        // 🚨 终极防线：一旦找不到，说明它要么没被加进来，要么 raidID 被改了！
        printf("\n[FATAL ERROR] 试图删除的节点不在它应该在的哈希桶里！\n");
        printf("异常节点的当前 raidID: %u\n", node->raidID);
        printf("传进来的目标 raidID: %u\n", raidID);
        // 如果你的环境支持 assert.h，直接 assert(0);
        // 不支持的话制造一个崩溃，把堆栈停在这里！
        int *crash = NULL; 
        *crash = 1; 
    }
	
	return 0;
}

int hash_add_parity(tHash *pHash ,  HASH_NODE *pInsertNode,unsigned int raidID){
	int growthFlag=0 , ret = 0;
	unsigned long long i = 0;
	if(!pHash || !pInsertNode)
		return 0;

	if(pHash->nodeArray == NULL){
		unsigned long long hash_len = ((pHash->max_buffer_sector + HASH_CELL_NUM - 1) / HASH_CELL_NUM);
		pHash->nodeArray = malloc(sizeof(HASH_NODE) * hash_len);
		alloc_assert(pHash->nodeArray, "pHash->nodeArray");		
		for(; i < hash_len; ++i){
			pHash->nodeArray[i] = NULL;
		}
	}

	buf_node* node = (buf_node*)pInsertNode;

	i = node->raidID % pHash->max_buffer_sector;
	i = i / 3;

	// === 防御性编程：检查节点是否已经在链表中 ===
    HASH_NODE *tmp = pHash->nodeArray[i];
    while(tmp) {
        if(tmp == pInsertNode) {
            // 如果触发这个报错，说明外部业务逻辑有 Bug，删错了或者重复加了！
            printf("FATAL ERROR: 尝试向哈希表中插入一个已经存在的节点！\n");
            return -1; // 或者直接 assert(0); 让程序崩在这里，方便调堆栈
        }
        tmp = tmp->next;
    }

	pInsertNode->next = pHash->nodeArray[i];
	pHash->nodeArray[i] = pInsertNode;
	
	pHash->count++;
	return ret;
}


