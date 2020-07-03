#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/hugetlb.h>
#include <asm/pgtable.h>

#define BUFSIZE  1024

static char *buf;
static unsigned int len;
static unsigned long vaddr;
static struct task_struct * task;

/***********************
 *  * file_operations->open
 *   * 无操作
 *    ***********************/

static int test_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}
 
static unsigned long get_pfn(void)
{
	struct mm_struct *mm;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long rv = -1;
		
	if(!task)
		return rv;
	
	mm = task->mm;
	if(!mm){
		printk(KERN_INFO "the mm is NULL\n");
		return rv;
	}
	pgd = pgd_offset(mm, vaddr);
	printk(KERN_INFO "the pgd is %lx\n",pgd_val(*pgd));
	
	if(pgd_bad(*pgd)){
		printk(KERN_INFO "the pgd is bad\n");
		return rv;
	}
	pud = pud_offset(pgd, vaddr);
	printk(KERN_INFO "the pud is %lx\n",pud_val(*pud));
	
	if(pud_bad(*pud)){
		printk(KERN_INFO "the pud is bad\n");
		return rv;
	}
	if(pud_large(*pud)){
		unsigned long pfn_offset = (vaddr & ~PUD_MASK) >> PAGE_SHIFT;
		printk(KERN_INFO "this is a huge page\n");
		return pud_pfn(*pud) + pfn_offset;
	}
	pmd = pmd_offset(pud, vaddr);
	printk(KERN_INFO "the pmd is %lx\n",pmd_val(*pmd));
	
	if(pmd_bad(*pmd)){
		printk(KERN_INFO "the pmd is bad\n");
		return rv;
	}
	
	if(pmd_large(*pmd)){
		unsigned long pfn_offset = (vaddr & ~PMD_MASK) >> PAGE_SHIFT;	
		printk(KERN_INFO "this is a huge page\n");
		return pmd_pfn(*pmd) + pfn_offset;
	}
		
	pte = pte_offset_map(pmd, vaddr);	
	printk(KERN_INFO "the pte is %lx\n",pte_val(*pte));
	if(!pte_present(*pte)){
		printk(KERN_INFO "the page is not in memory\n");
		pte_unmap(pte);
		return rv;
	}else{
		pte_unmap(pte);
		return pte_pfn(*pte);
	}

} 
/************************
 *传递到用户空间似乎有点bug，传递的字符串经常少一段。
 ************************/

static ssize_t vaddr_pfn_read(struct file *file,
				char __user *buffer,size_t count, loff_t *f_pos) 
{
	int size;
	unsigned long pfn;
	if(*f_pos > 0)
		return 0;
 
	printk("---start read---\n");
	printk("the pid is >>>>> %d\n",task->pid);
 	printk("the virtual address is : %lx\n",vaddr);
	
	pfn = get_pfn();
	
	memset(buf,0,BUFSIZE+1);
	if(pfn == -1){
		if((size = sprintf(buf,"bad address\n")) < 0){
			printk(KERN_INFO "error in sprintf 1\n");		
			return -EFAULT;
		}
	}else{
		if((size = sprintf(buf,"pfn = 0x%lx\n",pfn)) < 0){
			printk(KERN_INFO "error in sprintf 2\n");
			return -EFAULT;
		}
	}
	//printk(KERN_INFO "size = %d\n",size);
	if(copy_to_user(buffer, buf, size+1))
		return -EFAULT;
	
	*f_pos = *f_pos + len;
	return len;
}


/************************
 *  * file_operations->write
 *   * 可以在adb工具进入机器的pro目录，
 *    * 执行adb shell && cd proc && echo 12345 > tets_rw，即可把12345写入节点test_rw
 *     ************************/

static struct task_struct * get_task(char *pid_str)
{	
	pid_t nr;

	if(kstrtoint(pid_str, 10, &nr) != 0){
		printk(KERN_INFO "kstrtoint failed\n");
		return NULL;
	}
	printk(KERN_INFO "pid is :%d\n",nr);	

	task = pid_task(find_get_pid(nr), PIDTYPE_PID);
	if(!task) 
		printk(KERN_INFO "the progress does not exist\n");

	return task;	
}

static unsigned long get_vaddr(char *vaddr_str)
{
	if(*vaddr_str == '0' && *(vaddr_str + 1) == 'x')
		if(kstrtoul(vaddr_str+2, 16, &vaddr) != 0){
			printk(KERN_INFO "kstrtou64 failed\n");
			return 0;
		}
	printk(KERN_INFO "vaddr is :%lx\n",vaddr);
	return vaddr;	
}

static ssize_t vaddr_pfn_write(struct file *file, const char __user *buffer,
							size_t count, loff_t *f_pos) 
{	
	if(count <= 0)
		return -EFAULT;
	printk("---start write---\n");
	
	len = count > BUFSIZE ? BUFSIZE :count;

	buf = (char *)kmalloc(BUFSIZE+1, GFP_KERNEL);
	if(buf == NULL)
	{
		printk("test_proc_create kmalloc fail!\n");
		return -EFAULT;
	}

	memset(buf, 0, BUFSIZE+1);

	if(copy_from_user(buf,buffer,len))
		return -EFAULT;
	printk("vaddr_pfn_write writing : %s\n",buf);
	get_task(strsep(&buf," "));
	get_vaddr(buf);
	return len;
}

static struct file_operations test_fops = {
	.owner	= THIS_MODULE,
	.open	= test_proc_open,
	.read	= vaddr_pfn_read,
	.write	= vaddr_pfn_write,
};

static int __init vaddr_pfn_init(void)
{
	struct proc_dir_entry *file;

	file = proc_create("vaddr_pfn",0644,NULL, &test_fops);
	if(!file)
		return -ENOMEM;
	printk("vaddr_pfn init success!\n");
	return 0;
}

static void __exit vaddr_pfn_exit(void)
{
	remove_proc_entry("vaddr_pfn", NULL);
	printk("vaddr_pfn_exit\n");
}

module_init(vaddr_pfn_init);
module_exit(vaddr_pfn_exit);

MODULE_AUTHOR("Jianghu Liu");
MODULE_DESCRIPTION("Input a progress' pid and virtual address, Output pfn");
MODULE_LICENSE("GPL");
