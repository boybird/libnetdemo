#include <ev.h>
#include <stdio.h>

// 定时器回调函数
static void timer_cb(EV_P_ ev_timer *w, int revents) {
    printf("Timer triggered at %f seconds\n", ev_time());
}

int main() {
    // 初始化 libev 事件循环
    struct ev_loop *loop = EV_DEFAULT;

    // 设置定时器，1秒后触发，每1秒重复
    ev_timer timer;
    ev_timer_init(&timer, timer_cb, 1.0, 1.0);
    ev_timer_start(loop, &timer);

    printf("Starting event loop...\n");

    // 启动事件循环
    ev_run(loop, 0);

    // 清理（通常不会到达这里）
    ev_timer_stop(loop, &timer);
    ev_loop_destroy(loop);

    return 0;
}
