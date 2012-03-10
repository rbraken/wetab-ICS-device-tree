/*
* wakeup_button.c - Power Button which is pushed on resume from standby.
*
* Copyright (c) 2011 Stefan Seidel
*
* This file is released under the GPLv2 or later.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/earlysuspend.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Seidel <android@stefanseidel.info>");
MODULE_DESCRIPTION("Sets up a virtual input device and sends a power key event during early resume. Needed for some to make Android on x86 wake up properly.");

static struct input_dev *input;

static void wakeup_button_early_suspend(struct early_suspend *h)
{
return;
}

static void wakeup_button_early_resume(struct early_suspend *h)
{
printk("Early resume, push virtual power button!\n");
input_report_key(input, KEY_POWER, 1);
input_sync(input);
input_report_key(input, KEY_POWER, 0);
input_sync(input);
}

static struct early_suspend wakeup_button_early_suspend_handlers = {
.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1, // very late resume
.suspend = wakeup_button_early_suspend,
.resume = wakeup_button_early_resume
};

static int __init wakeup_button_init(void)
{
int error;
printk("Registering Android Wakeup Button.\n");
input = input_allocate_device();
input->name = "Wakeup Button";
input->id.bustype = BUS_USB; // use BUS_USB here so that Android registers this as an external key
input->evbit[0] = BIT_MASK(EV_KEY);
set_bit(KEY_POWER, input->keybit);
error = input_register_device(input);
if (error) {
input_free_device(input);
} else {
register_early_suspend(&wakeup_button_early_suspend_handlers);
}
return error;
}

static void __exit wakeup_button_exit(void)
{
printk("Unregistering Android Wakeup Button.\n");
unregister_early_suspend(&wakeup_button_early_suspend_handlers);
input_unregister_device(input);
return;
}

module_init(wakeup_button_init);
module_exit(wakeup_button_exit);
