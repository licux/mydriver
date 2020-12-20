#include <linux/module.h>
#include <linux/time.h>

static int num = 10;
// static struct timeval start_time;
static time64_t start_time, end_time;

module_param(num, int, S_IRUGO);

static void say_hello(void){
    int i;
    for(i = 0; i < num; i++){
        pr_info("[%d/%d] Hello!\n", i, num);
    }
}

static int __init hello_init(void){
    //do_gettimeofday(&start_time);
    start_time = ktime_get_seconds();
    pr_info("Loading First!\n");
    say_hello();
    return 0;
}

static void __exit hello_exit(void){
    // struct timeval end_time;
    // do_gettimeofday(&end_time);
    end_time = ktime_get_seconds();
    pr_info("Unloading module after %lld seconds\n", end_time -start_time);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masaki Tsukada");
MODULE_DESCRIPTION("This is a print out Hello World module");
