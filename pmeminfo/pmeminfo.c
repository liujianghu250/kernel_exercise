#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/page_types.h>
#include <linux/sched.h>

#define BUFSIZE  1024 

//static char *buf;
//static unsigned int len;

/***********************
 *  * file_operations->open
 *   * 无操作
 *    ***********************/

static int test_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}
 
 
/************************
 *  * file_operations->read
 *   * 可以在adb工具进入机器的pro目录，执行adb shell && cd proc && cat tets_rw，
 *    * 即可读出节点test_rw的内容是12345
 *     ************************/
static unsigned long vmemsize(struct mm_struct *mm)
{
	struct vm_area_struct *vm_area;
	unsigned long vmsize;
	
	if(mm == NULL)
		return 0;
	vm_area = mm->mmap;
	vmsize = 0;
	while(vm_area != NULL){
		vmsize += vm_area->vm_end - vm_area->vm_start;
		vm_area = vm_area->vm_next;
		if(vm_area == mm->mmap)
			break;
	}
	return vmsize >> PAGE_SHIFT;
}

static unsigned long pmemsize(struct mm_struct *mm)
{
	unsigned long pmsize;

	if(mm == NULL)
		return 0;
	pmsize = get_mm_counter(mm,MM_FILEPAGES) + get_mm_counter(mm, MM_SHMEMPAGES) + get_mm_counter(mm, MM_ANONPAGES); 	
	return pmsize;
}

static ssize_t test_proc_read(struct file *file,
				char __user *buffer,size_t count, loff_t *f_pos) 
{
	struct task_struct *p;
	struct mm_struct *mm;
	unsigned long vmsize;
	unsigned long pmsize;
		

	if(*f_pos > 0)
		return 0;
 

	printk("---start read---\n");
	printk("PID\tVMSIZE\tPMSIZE\t[TGID]\n");
	for_each_process(p){
		mm = get_task_mm(p);

		vmsize = vmemsize(mm);
		pmsize = pmemsize(mm);
		printk("%d\t%ld\t%ld", p->pid, vmsize * 4, pmsize * 4);
		if(p->tgid != p->pid){
			printk("\t%d\n",p->tgid);
		}else{
			printk("\n");
		}
	}
		
	/*printk("the	string is >>>>> %s\n", buf);
 
	if(copy_to_user(buffer, buf, len))
		return -EFAULT;
	
	*f_pos = *f_pos + len;*/
	return 0;
}


static struct file_operations pmeminfo_fops = {
	.owner	= THIS_MODULE,
	.open	= test_proc_open,
	.read	= test_proc_read,
};

static int __init pmeminfo_init(void)
{
	struct proc_dir_entry *file;

	file = proc_create("pmeminfo",0444,NULL, &pmeminfo_fops);
	if(!file)
		return -ENOMEM;
	printk("pmeminfo init success!\n");
	return 0;
}

static void __exit pmeminfo_exit(void)
{
	remove_proc_entry("pmeminfo", NULL);
	printk("pmeminfo_exit\n");
}

module_init(pmeminfo_init);
module_exit(pmeminfo_exit);

MODULE_AUTHOR("Jianghu Liu");
MODULE_DESCRIPTION("Print all process' memory infomation.");
MODULE_LICENSE("GPL");
