/** based on EmbeTronicX **/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> //kmalloc()
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/kthread.h> //KERNEL THREADS
#include <linux/wait.h> // Required for the wait queues
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/delay.h>

#pragma region VARIABLES
//GLOBAL VARIABLES - QUEUE
uint32_t read_count;
static struct task_struct *wait_thread;
wait_queue_head_t wait_queue_etx;

dev_t dev;
static struct class *dev_class;
static struct cdev etx_cdev;
int wait_queue_flag;
volatile int etx_value;
struct kobject *kobj_ref;

//list running processes
struct process_struct {
	struct list_head _list;
	struct task_struct *_task;
};
struct list_head m_processListHead;

int nextPID = -999;
const int PIDnoCHANGED = -999;
#pragma endregion

#pragma region PROTOTYPES
/* Function Prototypes */
static int __init rtosp_init(void);
static void __exit rtosp_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len,
			 loff_t *off);

/*************** Sysfs functions **********************/
static ssize_t set_rtosp_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf);
static ssize_t set_rtosp_store(struct kobject *kobj,
			       struct kobj_attribute *attr, const char *buf,
			       size_t count);
static ssize_t get_next_rtosp_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf);
static ssize_t get_next_rtosp_store(struct kobject *kobj,
				    struct kobj_attribute *attr,
				    const char *buf, size_t count);

/*************** aux functions **********************/
void addRunningProcesses(void);
#pragma endregion

#pragma region PERMISSIONS_setAndGet_RTOSP
/* warning! need write-all permission so overriding check */
#undef VERIFY_OCTAL_PERMISSIONS
#define VERIFY_OCTAL_PERMISSIONS(perms) (perms)

struct kobj_attribute set_rtosp_attr =
	__ATTR(set_rtosp, 0666, set_rtosp_show, set_rtosp_store);
struct kobj_attribute get_next_rtosp_attr =
	__ATTR(get_next_rtosp, 0444, get_next_rtosp_show, get_next_rtosp_store);

/* File operation structure */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = etx_read,
	.write = etx_write,
	.open = etx_open,
	.release = etx_release,
};
#pragma endregion

#pragma region QUEUE_FUNCTION
static int wait_function(void *unused)
{
	while (1) {
		struct list_head *iter;
		struct process_struct *objPtr;
		int changed = 0;

		pr_info("WAITING QUEUE - SLEEP MODE ACTIVATED...\n");
		wait_event_interruptible(wait_queue_etx, wait_queue_flag != 0);
		if (wait_queue_flag == 2) {
			pr_info("WAITING QUEUE - EXIT MODE - Event Came From Exit Function\n");
			return 0;
		}

		pr_info("WAITING QUEUE - ACTIVE - Event Came From Read Function - usageCount:%d\n",
			++read_count);

		list_for_each(iter, &m_processListHead) {
			objPtr = list_entry(iter, struct process_struct, _list);
			pr_info("\titeration PID:%d, rtosp:%d\n",
				objPtr->_task->pid,
				objPtr->_task->rtosp); /*DEBUG*/
			if (objPtr->_task->rtosp == 1) {
				changed = 1;
				nextPID = objPtr->_task->pid;
				//  pr_info("\t\t IT IS ONE;\n");  /*DEBUG*/
				list_del(iter);
				break;
			}
		}

		if (changed == 0) {
			// pr_info("\t\t NOT MEXIDO NEXT PID = %d;\n", PIDnoCHANGED);      /*DEBUG*/
			nextPID = PIDnoCHANGED;
		}
		wait_queue_flag = 0;
	}
	return 0;
}
#pragma endregion

#pragma region SHOW_STORE_SET_RTOPS
/* This function will be called when we read the sysfs file */
static ssize_t set_rtosp_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	pr_info("set_rtosp - Read!!!\n");
	return sprintf(buf, "%d", etx_value);
}

/* This function will be called when we write the sysfsfs file */
static ssize_t set_rtosp_store(struct kobject *kobj,
			       struct kobj_attribute *attr, const char *buf,
			       size_t count)
{
	int paramPID = 0;
	int changed = 0;
	struct task_struct *task;

	sscanf(buf, "%d", &paramPID);

	pr_info("SET RTOSP - Write function!\n");
	for_each_process(task) {
		if (task->pid == paramPID) {
			task->rtosp = 1;
			changed = 1;
			break;
		}
	}

	if (changed == 1) {
		pr_info("SET RTOSP - RTOSP var CHANGED ON - pid: %d\n",
			paramPID);
	} else {
		pr_info("SET RTOSP - PID NOT FOUND AND RTOSP var NOT CHANGED - pid: %d\n",
			paramPID);
	}

	return count;
}
#pragma endregion

#pragma region SHOW_STORE_GET_NEXT_RTOPS
/* This function will be called when we read the sysfs file */
static ssize_t get_next_rtosp_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	nextPID = PIDnoCHANGED;
	pr_info("get_next_rtosp - Read!!!\n");

	wait_queue_flag = 1;
	wake_up_interruptible(&wait_queue_etx);

	while (wait_queue_flag != 0)
		usleep_range(10000, 10001);

	// pr_info("nextPID = %d\n", nextPID);  /*DEBUG*/
	if (nextPID == PIDnoCHANGED) 
		return sprintf(buf, "%s", "NO PID WITH rtosp at 1.");

	return sprintf(buf, "%d", nextPID);
}

/* This function will be called when we write the sysfsfs file */
static ssize_t get_next_rtosp_store(struct kobject *kobj,
				    struct kobj_attribute *attr,
				    const char *buf, size_t count)
{
	int paramPID = 0;

	sscanf(buf, "%d", &paramPID);
	pr_info("get_next_rtosp - Write!!! pid change %d\n", paramPID);
	return count;
}
#pragma endregion

#pragma region DEVICE_FILE_FUNCTIONS
/* This function will be called when we open the Device file */
static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/* This function will be called when we close the Device file */
static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}

/* This function will be called when we read the Device file */
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off)
{
	pr_info("Read function\n");
	return 0;
}

/* This function will be called when we write the Device file */
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off)
{
	pr_info("Write Function\n");
	return len;
}
#pragma endregion

#pragma region MODULE_FUNCTIONS
/* Module Init function */
static int __init rtosp_init(void)
{
	dev = 0;
	read_count = 0;
	wait_queue_flag = 0;
 	etx_value = 0;

	//INSTANTIATE THE LIST
	INIT_LIST_HEAD(&m_processListHead);

	/*Allocating Major number*/
	if ((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0) {
		pr_info("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&etx_cdev, &fops);

	/*Adding character device to the system*/
	if ((cdev_add(&etx_cdev, dev, 1)) < 0) {
		pr_info("Cannot add the device to the system\n");
		goto r_class;
	}

	/*Creating struct class*/
	dev_class = class_create(THIS_MODULE, "etx_class");
	if (dev_class == NULL) {
		pr_info("Cannot create the struct class\n");
		goto r_class;
	}

	/*Creating device*/
	if ((device_create(dev_class, NULL, dev, NULL, "etx_device")) == NULL) {
		pr_info("Cannot create the Device 1\n");
		goto r_device;
	}

	/* Initialize wait queue */
	init_waitqueue_head(&wait_queue_etx);

	//Create the kernel thread with name 'mythread'
	wait_thread = kthread_create(wait_function, NULL, "WaitThread");
	if (wait_thread) {
		pr_info("Thread Created successfully\n");
		wake_up_process(wait_thread);
	} else
		pr_info("Thread creation failed\n");

	/*Creating a directory in /sys/kernel/ */
	kobj_ref = kobject_create_and_add("rtosp", kernel_kobj);

	/*Creating sysfs file for SET_RTOSP*/
	if (sysfs_create_file(kobj_ref, &set_rtosp_attr.attr)) {
		pr_err("Cannot create SET RTOSP file......\n");
		goto r_sysfs_set;
	}

	/*Creating sysfs file for GET_NEXT_RTOSP*/
	if (sysfs_create_file(kobj_ref, &get_next_rtosp_attr.attr)) {
		pr_err("Cannot create GET NEXT RTOSP file......\n");
		goto r_sysfs_get;
	}

	//add running processes to List
	addRunningProcesses();

	/* INIT DONE SUCESSFULY */
	pr_info("RTOSP - Device Driver Insert...Done!!!\n");
	return 0;

r_sysfs_set:
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &set_rtosp_attr.attr);
r_sysfs_get:
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &get_next_rtosp_attr.attr);
r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev, 1);
	cdev_del(&etx_cdev);
	return -1;
}

/* Module exit function */
static void __exit rtosp_exit(void)
{
	wait_queue_flag = 2;
	wake_up_interruptible(&wait_queue_etx);
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &set_rtosp_attr.attr);
	sysfs_remove_file(kernel_kobj, &get_next_rtosp_attr.attr);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("RTOSP - Device Driver Remove...Done!!!\n");
}
#pragma endregion

#pragma region AUXFUNCTIONS
void addRunningProcesses(void)
{
	struct task_struct *taskNow;
	// pr_info("addRunningProcesses! NOW!!!!\n");      /*DEBUG*/

	for_each_process(taskNow) {
		if (taskNow->__state == 0) {
			struct process_struct *procToAdd = kmalloc(
				sizeof(struct process_struct *), GFP_KERNEL);
			procToAdd->_task = taskNow;
			list_add(&procToAdd->_list, &m_processListHead);
			pr_info("Adding pid:%d\n", taskNow->pid);
		}
	}
}
#pragma endregion
module_init(rtosp_init);
module_exit(rtosp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joao Marco RTOSP");
MODULE_VERSION("0.0.0.1");
