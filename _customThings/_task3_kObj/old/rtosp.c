#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

static int set_rtosp;
static int get_rtosp;

/*----------------------------------//----------------------------------------*/
/*--------------------------------SET RTOSP-----------------------------------*/
/* "set_rtosp" that is writable by anyone 
When "set_rtosp" is accessed, the field "rtosp" 
in the task structure of the given PID is set to "1" */

static ssize_t set_rtosp_show(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	printk(KERN_INFO "rtosp SET METHOD - SHOW!!!\n");
	// int ret;

	// ret = kstrtoint(buf, 10, &foo);
	// if (ret < 0)
	// 	return ret;

	// return count;
	return 1;
}

static ssize_t set_rtosp_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	printk(KERN_INFO "rtosp SET METHOD - STORE!!!\n");
	// int ret;

	// ret = kstrtoint(buf, 10, &foo);
	// if (ret < 0)
	// 	return ret;

	// return count;
	return 1;
}

//OK
/* "set_rtosp" that is writable by anyone */
static struct kobj_attribute set_rtosp_attribute =
	__ATTR(set_rtosp, 0222, set_rtosp_show, set_rtosp_store);

/*----------------------------------//----------------------------------------*/
/*--------------------------------GET RTOSP-----------------------------------*/
/* "get_next_rtosp" readable by anyone 
When "get_next_rtosp" is accessed, it should return the PID of the next
process in the list of processes that has the value of the field set to 1. Make
sure that the process is still running. */

static ssize_t get_rtosp_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	printk(KERN_INFO "rtosp GET METHOD!!!\n");
	// int var;

	// if (strcmp(attr->attr.name, "baz") == 0)
	// 	var = baz;
	// else
	// 	var = bar;
	// return sprintf(buf, "%d\n", var);
	return 1;
}

static ssize_t get_rtosp_store(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	printk(KERN_INFO "rtosp GET METHOD!!!\n");
	// int var;

	// if (strcmp(attr->attr.name, "baz") == 0)
	// 	var = baz;
	// else
	// 	var = bar;
	// return sprintf(buf, "%d\n", var);
	return 1;
}

//OK
//"get_next_rtosp" readable by anyone - 0444
static struct kobj_attribute get_rtosp_attribute =
	__ATTR(get_rtosp, 0444, get_rtosp_show, get_rtosp_store);

/*----------------------------------//----------------------------------------*/
/*---------------------------------INIT---------------------------------------*/

//OK
static struct attribute *attrs[] = {
	&set_rtosp_attribute.attr,
	&get_rtosp_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

//OK
static struct attribute_group attr_rtosp = {
	.attrs = attrs,
};

//OK
static struct kobject *rtosp_kobj;

/* In the init function of the device driver, a list of running processes should be created.*/
static int __init rtosp_init(void)
{
	int retval;
	/*CREATE LIST RUNNING PROCESSES*/

	rtosp_kobj = kobject_create_and_add("rtosp", kernel_kobj);
	if (!rtosp_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(rtosp_kobj, &attr_rtosp);
	if (retval)
		kobject_put(rtosp_kobj);

	return retval;
}
/*----------------------------------//----------------------------------------*/

/*----------------------------------//----------------------------------------*/
/*---------------------------------EXIT---------------------------------------*/
static void __exit rtosp_exit(void)
{
	kobject_put(rtosp_kobj);
}
/*----------------------------------//----------------------------------------*/

module_init(rtosp_init);
module_exit(rtosp_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Joao Marco");