#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9a31bb74, "module_layout" },
	{ 0xb7c59293, "cdev_del" },
	{ 0x5cd9dbb5, "kmalloc_caches" },
	{ 0xaa4fb4f8, "cdev_init" },
	{ 0x4c4fef19, "kernel_stack" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xc8980067, "no_llseek" },
	{ 0xf22449ae, "down_interruptible" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xf3b7642d, "nonseekable_open" },
	{ 0xf432dd3d, "__init_waitqueue_head" },
	{ 0x4f8b5ddb, "_copy_to_user" },
	{ 0x64ce4311, "current_task" },
	{ 0x27e1a049, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xebd1da60, "cdev_add" },
	{ 0x1000e51, "schedule" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xd61adcbd, "kmem_cache_alloc_trace" },
	{ 0xcf21d241, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0x5c8b5ce8, "prepare_to_wait" },
	{ 0x71e3cecb, "up" },
	{ 0xfa66f77c, "finish_wait" },
	{ 0x4f6b400b, "_copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "609C60CE4249E3FDE119EF9");
