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
	{ 0x4fe2c793, "cdev_del" },
	{ 0xcd61fef0, "class_destroy" },
	{ 0x3ef03cbe, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xcf1eada9, "kobject_put" },
	{ 0xf9a482f9, "msleep" },
	{ 0xe867bd1f, "_dev_info" },
	{ 0x1f95bd7d, "device_create" },
	{ 0x5b3a92a7, "__class_create" },
	{ 0xf1baf743, "cdev_add" },
	{ 0xc3a5fb96, "cdev_alloc" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x11089ac7, "_ctype" },
	{ 0x5f754e5a, "memset" },
	{ 0x1e6d26a8, "strstr" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x34d22939, "gpiod_set_raw_value" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0xfe990052, "gpio_free" },
	{ 0xbbcd67e2, "gpiod_direction_output_raw" },
	{ 0xc65e782c, "gpio_to_desc" },
	{ 0x92997ed8, "_printk" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xc358aaf8, "snprintf" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "312B6FC3FF8A1A3C5A1E91D");
