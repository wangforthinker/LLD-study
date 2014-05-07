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
	{ 0xd2b09ce5, "__kmalloc" },
	{ 0xaa4fb4f8, "cdev_init" },
	{ 0x4c4fef19, "kernel_stack" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xf22449ae, "down_interruptible" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xf432dd3d, "__init_waitqueue_head" },
	{ 0x4f8b5ddb, "_copy_to_user" },
	{ 0x27e1a049, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x68aca4ad, "down" },
	{ 0x5949395a, "__get_page_tail" },
	{ 0xebd1da60, "cdev_add" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xd61adcbd, "kmem_cache_alloc_trace" },
	{ 0xe52947e7, "__phys_addr" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x37a0cba, "kfree" },
	{ 0x71e3cecb, "up" },
	{ 0x4f6b400b, "_copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "81F79C09CDBA20F8669F557");
