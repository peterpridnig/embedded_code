#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9c635008, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xcd61fef0, "class_destroy" },
	{ 0x3ef03cbe, "device_destroy" },
	{ 0x1f95bd7d, "device_create" },
	{ 0x5b3a92a7, "__class_create" },
	{ 0xedafb01d, "__register_chrdev" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x92997ed8, "_printk" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1C2DBFD28D5C6727D91D0BD");
