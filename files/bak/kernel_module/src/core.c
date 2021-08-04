//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2016
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
//     Core of Kernel Module for Memory Container
//
////////////////////////////////////////////////////////////////////////

// Project 2: Yuanchao Xu, yxu47; Yuhang Lin, ylin34

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

struct threadnode {
		int pid;
		struct threadnode *next;
};
typedef struct threadnode * tlist;

// memory object id list
struct memorynode {
		bool state;
		void * address;
		int pages, ID;
		struct memorynode *next;
};
typedef struct memorynode * mlist;

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


// static int numberofcontainer = 0;
// static clist *containerlist = NULL;
// static DEFINE_MUTEX(lockforall);
// EXPORT_SYMBOL(lockforall);
//
//
extern struct miscdevice memory_container_dev;
extern struct mutex lockforall;
extern void clist_free(clist *list);
extern void mlist_free(mlist *list);
extern void tlist_free(tlist *list);
extern clist *containerlist;
// extern

int memory_container_init(void)
{
		int ret;

		if ((ret = misc_register(&memory_container_dev)))
		{
				printk(KERN_ERR "Unable to register \"memory_container\" misc device\n");
				return ret;
		}

		printk(KERN_ERR "\"memory_container\" misc device installed\n");
		printk(KERN_ERR "\"memory_container\" version 0.1\n");
		return ret;
}


void memory_container_exit(void)
{
		mutex_lock(&lockforall);
		clist_free(containerlist);
		mutex_unlock(&lockforall);
		mutex_destroy(&lockforall);
		misc_deregister(&memory_container_dev);
}
