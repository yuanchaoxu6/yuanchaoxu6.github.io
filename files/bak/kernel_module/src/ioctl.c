//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2018
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng, Yu-Chia Liu
//
//   Description:
//     Core of Kernel Module for Processor Container
//
////////////////////////////////////////////////////////////////////////


#include "memory_container.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/kthread.h>


// thread list
//todo
struct threadnode {
		int pid;
		struct threadnode *next;
};
typedef struct threadnode * tlist;

tlist *tlist_create(void *new_data)
{
		struct threadnode *new_node;

		tlist *new_list = (tlist *)kmalloc(sizeof (tlist),GFP_KERNEL);
		*new_list = (struct threadnode *)kmalloc(sizeof (struct threadnode),GFP_KERNEL);

		new_node = *new_list;
		new_node->pid = -1;
		new_node->next = NULL;
		return new_list;
}

void tlist_free(tlist *list)
{
		if (list == NULL) return;
		printk(KERN_ERR "current thread: %d, tlist free\n", current->pid);
		struct threadnode *curr = *list;
		struct threadnode *next;
		while (curr != NULL) {
				next = curr->next;
				kfree(curr);
				curr = next;
		}
		kfree(list);
}

void tlist_push(tlist *list, int data, int num)
{
		struct threadnode *head;
		struct threadnode *new_node;
		if (list == NULL || *list == NULL) {
				printk(KERN_ERR "current thread: %d, tlist in null\n", current->pid);
		}
		head = *list;
		// Head is empty node
		if (num == 0)
				head->pid = data;
		// Head is not empty, add new node to front
		else {
				new_node = kmalloc(sizeof (struct threadnode),GFP_KERNEL);
				new_node->pid = data;
				new_node->next = head;
				*list = new_node;
		}
}

bool tlist_search(tlist *list, int id)
{
		struct threadnode *curr = *list;
		while (curr != NULL) {
				if (curr->pid == id) return true;
				curr = curr->next;
		}
		return false;
}

bool tlist_pop(tlist *list, int id)
{
		struct threadnode *head = *list;
		if (list == NULL || *list == NULL)
		{
				printk(KERN_ERR "current thread: %d, do not have any thread\n", current->pid);
				return false;
		}
		if (head->pid == id)
		{
				*list = head->next;
				kfree(head);
				return true;
		}
		else
		{
				struct threadnode *curr = *list;
				struct threadnode *now = *list;
				while (curr->next != NULL) {
						if (curr->next->pid == id) break;
						curr = curr->next;
				}
				if (curr->next == NULL) {
						printk(KERN_ERR "current thread: %d, do not have %d thread\n", current->pid, id);
						return false;
				}
				else {
						now = curr->next;
						curr->next = now->next;
						kfree(now);
						return true;
				}
		}
		return false;
}

void tlist_print(tlist *list)
{
		struct threadnode *curr = *list;
		while (curr != NULL) {
				printk(KERN_ERR "current thread: %d, thraedlist pid:%d\n", current->pid, curr->pid);
				curr = curr->next;
		}
}


// memory object id list
struct memorynode {
		bool state;
		char * address;
		__u64 pages;
		__u64 ID;
		struct memorynode *next;
};
typedef struct memorynode * mlist;

mlist *mlist_create(void * data)
{
		struct memorynode *new_node;

		mlist *new_list = (mlist *)kmalloc(sizeof (mlist),GFP_KERNEL);
		*new_list = (struct memorynode *)kmalloc(sizeof (struct memorynode),GFP_KERNEL);

		new_node = *new_list;
		new_node->address = NULL;
		new_node->ID = -1;
		new_node->next = NULL;
		return new_list;
}

void mlist_free(mlist *list)
{
		printk(KERN_ERR "current thread: %d, mlist have  free\n", current->pid);
		if (list == NULL) return;
		printk(KERN_ERR "current thread: %d, mlist free\n", current->pid);
		struct memorynode *curr = *list;
		struct memorynode *next;
		while (curr != NULL) {
				next = curr->next;
				memset(curr->address, 0, curr->pages);
				kfree(curr->address);
				kfree(curr);
				curr = next;
		}
		kfree(list);
}

void mlist_push(mlist *list, bool state1, char* address1, __u64 pages1, __u64 ID1)
{
		struct memorynode *head;
		struct memorynode *new_node;
		if (list == NULL || *list == NULL) {
				printk(KERN_ERR "current thread: %d, mlist in null:%d\n", current->pid);
		}
		head = *list;
		// Head is empty node
		if (head->ID  == -1) {
				head->state = state1;
				head->address = address1;
				head->pages = pages1;
				head->ID = ID1;
		}
		// Head is not empty, add new node to front
		else {
				new_node = kmalloc(sizeof (struct memorynode), GFP_KERNEL);
				new_node->state = state1;
				new_node->address = address1;
				new_node->pages = pages1;
				new_node->ID = ID1;
				new_node->next = head;
				*list = new_node;
		}
}

bool mlist_pop(mlist *list, __u64 id)
{
		struct memorynode *curr = *list;
		if (list == NULL || *list == NULL)
		{
				printk(KERN_ERR "current thread: %d, do not have any memoryobj\n", current->pid);
				return false;
		}
		if (curr->ID == id) {
				//*list = curr->next;
				//memset(curr->address, 0, curr->pages);
				//kfree(curr->address);
				//kfree(curr);
				curr->ID = -2;
				return true;
		}
		else
		{
				struct memorynode *now = *list;
				while (curr->next != NULL) {
						if (curr->next->ID == id) break;
						curr = curr->next;
				}
				if (curr->next == NULL)
				{
						printk(KERN_ERR "current thread: %d, do not have %d memobj\n", current->pid, id);
						return false;
				}
				else
				{
						now = curr->next;
						now->ID = -2;
						//curr->next = now->next;
						//memset(now->address, 0, now->pages);
						//kfree(now->address);
						//kfree(now);
						//mlist_print(curr, NULL);
						return true;
				}
		}
		return false;
}

struct memorynode* mlist_search(mlist * list, int id)
{
		struct memorynode *curr = *list;
		while (curr != NULL)
		{
				if (curr->ID == id) return curr;
				curr = curr->next;
		}
		return NULL;
}

void mlist_print(mlist *list, void (*print)(void *))
{
		struct memorynode *curr = *list;
		while (curr != NULL) {
				printk(KERN_ERR "current thread: %d, mlist object ID :%d\n", current->pid, curr->ID);
				curr = curr->next;
		}
}




//container List
struct containernode {
		int cid;
		struct mutex c_lock;
		mlist *memoryobj;
		tlist *threadobj;
		int threadnumber;
		int memorynumber;
		struct containernode *next;
};

typedef struct containernode * clist;

clist *clist_create(void)
{
		struct containernode *new_node;

		clist *new_list = (clist *)kmalloc(sizeof (clist),GFP_KERNEL);
		*new_list = (struct containernode *)kmalloc(sizeof (struct containernode),GFP_KERNEL);

		new_node = *new_list;
		new_node->cid = -1;
		new_node->next = NULL;
		return new_list;
}
//todo
void clist_free(clist *list)
{
		printk(KERN_ERR "current thread: %d, clist free\n", current->pid);
		if (list == NULL) return;
		struct containernode *curr = *list;
		struct containernode *next;
		while (curr != NULL) {
				next = curr->next;
				mlist_free(curr->memoryobj);
				tlist_free(curr->threadobj);
				mutex_destroy(&curr->c_lock);
				kfree(curr);
				curr = next;
		}
		kfree(list);
}

void clist_push(clist *list, int id)
{
		struct containernode *head;
		struct containernode *new_node;
		if (list == NULL || *list == NULL) {
				printk(KERN_ERR "current thread: %d, clist is null\n", current->pid);
		}
		head = *list;
		// Head is empty node
		if (head->cid == -1) {
				mutex_init(&(head->c_lock));
				head->memoryobj = mlist_create(NULL);
				head->threadobj = tlist_create(NULL);
				head->threadnumber = 0;
				head->memorynumber = 0;
				head->cid = id;
		}
		// Head is not empty, add new node to front
		else {
				new_node = kmalloc(sizeof (struct containernode),GFP_KERNEL);
				mutex_init(&(new_node->c_lock));
				new_node->memoryobj = mlist_create(NULL);
				new_node->threadobj = tlist_create(NULL);
				new_node->threadnumber = 0;
				new_node->memorynumber = 0;
				new_node->cid = id;
				new_node->next = head;
				*list = new_node;
		}
}

struct containernode * clist_search(clist *list, int id)
{
		struct containernode *curr = *list;
		while (curr != NULL) {
				if (curr->cid == id) return curr;
				curr = curr->next;
		}
		return NULL;
}

void clist_print(clist *list)
{
		struct containernode *curr = *list;
		while (curr != NULL) {
				tlist_print(curr->threadobj);
				printk(KERN_ERR "current thread: %d, container cid:%d\n", current->pid, curr->cid);
				curr = curr->next;
		}
}

DEFINE_MUTEX(lockforall);
EXPORT_SYMBOL(lockforall);
int numberofcontainer = 0;
clist *containerlist = NULL;




int memory_container_mmap(struct file *filp, struct vm_area_struct *vma)
{
		printk(KERN_ERR "current thread: %d, have mmap\n", current->pid);
		struct memory_container_cmd m_cmd;
		mutex_lock(&lockforall);
		// if (copy_from_user(&m_cmd, user_cmd, sizeof(m_cmd)) > 0) {
		// printk(KERN_ERR "mmap: failed to copy from user_cmd");
		// } else {
		// printk(KERN_ERR "mmap Copied container id: %llu", m_cmd.cid);
		// }
		__u64 id = (vma->vm_pgoff);
		struct containernode * node = *containerlist;
		struct memorynode * memnode;
		while (node != NULL)
		{
				bool ret = tlist_search(node->threadobj, current->pid);
				if (ret) break;
				node = node->next;
		}
		if (node == NULL) printk(KERN_ERR "current thread: %d, have mmap, no such thread\n", current->pid);
		struct memorynode *mn = mlist_search((node->memoryobj), id);
		__u64 len = vma->vm_end - vma->vm_start;
		printk(KERN_ERR "current thread: %d, have mmap with the size %llu page size %llu offset %d\n", current->pid, len, PAGE_SHIFT, id);
		char * mem;
		if (mn == NULL)
		{
				mem = kmalloc(len, GFP_KERNEL);
				memset(mem, 0, len);
				mlist_push(node->memoryobj, true, mem, len, id);
				node->memorynumber = node->memorynumber + 1;
		}
		else mem = mn->address;
		__u64 i, pfn;
		__u64 v, ret;
		for (i = 0; i < len; i += PAGE_SIZE) {
				pfn = virt_to_phys((void*) mem + i) >> PAGE_SHIFT;
				ret = remap_pfn_range(vma,vma->vm_start+i, pfn, PAGE_SIZE,vma->vm_page_prot);
				printk(KERN_ERR "current thread: %d, mem: %llu have remap pfn:%llu, vmstart:%llu, ret:%llu\n", current->pid,mem, pfn, vma->vm_start+i,ret);
				if (i == 0) v = ret;
				if (ret < 0)
				{
						printk(KERN_ERR "current thread: %d, have remap error\n", current->pid);
				}
		}
		//mlist_print(node->memoryobj, NULL);
		mutex_unlock(&lockforall);
		printk(KERN_ERR "current thread: %d, have remap return value is %d\n", current->pid, v);

		return v;
}


int memory_container_lock(struct memory_container_cmd __user *user_cmd)
{
		printk(KERN_ERR "current thread: %d, have lock\n", current->pid);
		mutex_lock(&lockforall);
		//printk(KERN_ERR "current thread: %d, have lock, enter\n", current->pid);
		struct containernode * node = *containerlist;
		while (node != NULL)
		{
				bool ret = tlist_search(node->threadobj, current->pid);
				if (ret) break;
				node = node->next;
		}

		if (node == NULL) printk(KERN_ERR "current thread: %d, have lock, no such thread\n", current->pid);
		//printk(KERN_ERR "current thread: %d, have lock, unlock all\n", current->pid);
		mutex_unlock(&lockforall);
		//printk(KERN_ERR "current thread: %d, have lock, lock container %d\n", current->pid, node->cid);
		mutex_lock(&(node->c_lock));
		return 0;
}


int memory_container_unlock(struct memory_container_cmd __user *user_cmd)
{
		printk(KERN_ERR "current thread: %d, have unlock\n", current->pid);
		mutex_lock(&lockforall);
		struct containernode * node = *containerlist;
		while (node != NULL)
		{
				bool ret = tlist_search(node->threadobj, current->pid);
				if (ret) break;
				node = node->next;
		}

		if (node == NULL) printk(KERN_ERR "current thread: %d, have unlock, no such thread\n", current->pid);
		mutex_unlock(&lockforall);
		mutex_unlock(&(node->c_lock));
		return 0;
}


int memory_container_delete(struct memory_container_cmd __user *user_cmd)
{
		printk(KERN_ERR "current thread: %d, have delete\n", current->pid);
		mutex_lock(&lockforall);
		struct containernode * node = *containerlist;
		bool res;
		while (node != NULL)
		{
				res = tlist_pop(node->threadobj, current->pid);
				if (res)
				{
						node->threadnumber = node->threadnumber-1;
						if (node->threadnumber == 0) node->threadobj = tlist_create(NULL);
						mutex_unlock(&lockforall);
						return 0;
				}
				node = node->next;
		}
		printk(KERN_ERR "current thread: %d, error container doesn't have this\n", current->pid);
		clist_print(containerlist);
		mutex_unlock(&lockforall);
		return 0;
}


int memory_container_create(struct memory_container_cmd __user *user_cmd)
{
		printk(KERN_ERR "current thread: %d, have create\n", current->pid);
		struct memory_container_cmd m_cmd;
		struct containernode * node;
		mutex_lock(&lockforall);
		if (numberofcontainer == 0) {containerlist = clist_create(); numberofcontainer++;}
		if (copy_from_user(&m_cmd, user_cmd, sizeof(m_cmd)) > 0) {
				printk(KERN_ERR "Create: failed to copy from user_cmd");
		} else {
				printk(KERN_ERR "Create Copied container id: %llu", m_cmd.cid);
		}
		node = clist_search(containerlist, m_cmd.cid);
		if (node == NULL) {
				clist_push(containerlist, m_cmd.cid);
				node = clist_search(containerlist, m_cmd.cid);
		}
		tlist_push(node->threadobj, current->pid, node->threadnumber);
		node->threadnumber = node->threadnumber + 1;
		clist_print(containerlist);
		mutex_unlock(&lockforall);

		return 0;
}


int memory_container_free(struct memory_container_cmd __user *user_cmd)
{
		printk(KERN_ERR "current thread: %d, have free\n", current->pid);
		mutex_lock(&lockforall);
		struct memory_container_cmd m_cmd;
		struct containernode * node = *containerlist;
		if (copy_from_user(&m_cmd, user_cmd, sizeof(m_cmd)) > 0) {
				printk(KERN_ERR "free: failed to copy from user_cmd");
		} else {
				printk(KERN_ERR "free Copied container id: %llu", m_cmd.oid);
		}
		__u64 id = m_cmd.oid;
		while (node != NULL)
		{
				bool ret = tlist_search(node->threadobj, current->pid);
				if (ret) break;
				node = node->next;
		}

		if (node == NULL) printk(KERN_ERR "current thread: %d, have free, no such thread\n", current->pid);
		bool res = mlist_pop(node->memoryobj, id);
		if (res)
		{
				node->memorynumber = node->memorynumber - 1;
				//if (node->memorynumber == 0) node->memoryobj = mlist_create(NULL);
				printk(KERN_ERR "current thread: %d, have free, free success cid:%d oid:%llu\n", current->pid, node->cid, id);
		}
		//mlist_print(node->memoryobj, NULL);
		mutex_unlock(&lockforall);
		return 0;
}


/**
 * control function that receive the command in user space and pass arguments to
 * corresponding functions.
 */
int memory_container_ioctl(struct file *filp, unsigned int cmd,
                           unsigned long arg)
{
		printk(KERN_ERR "current thread: %d, have ioctl\n", current->pid);
		switch (cmd)
		{
		case MCONTAINER_IOCTL_CREATE:
				return memory_container_create((void __user *)arg);
		case MCONTAINER_IOCTL_DELETE:
				return memory_container_delete((void __user *)arg);
		case MCONTAINER_IOCTL_LOCK:
				return memory_container_lock((void __user *)arg);
		case MCONTAINER_IOCTL_UNLOCK:
				return memory_container_unlock((void __user *)arg);
		case MCONTAINER_IOCTL_FREE:
				return memory_container_free((void __user *)arg);
		default:
				return -ENOTTY;
		}
}
