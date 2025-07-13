#include <ev.h> // libev 的头文件
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTEN_PORT 8080
#define BUFFER_SIZE 1024

// 客户端连接的事件处理回调函数
static void client_read_cb(EV_P_ struct ev_io *w, int revents) {
  char buffer[BUFFER_SIZE];
  ssize_t nread;

  // 读取客户端发送的数据
  nread = recv(w->fd, buffer, sizeof(buffer) - 1, 0);

  if (nread < 0) {
    perror("recv");
    // 如果读取错误或连接被关闭，关闭这个客户端的 socket 并移除事件监听
    ev_io_stop(loop, w); // 停止监听这个 socket 的读事件
    close(w->fd);        // 关闭 socket
    free(w);             // 释放事件结构体
    return;
  } else if (nread == 0) {
    // 连接被客户端关闭
    printf("Client disconnected.\n");
    ev_io_stop(loop, w);
    close(w->fd);
    free(w);
    return;
  } else {
    // 打印接收到的数据（仅用于演示）
    buffer[nread] = '\0';
    printf("Received from client: %s\n", buffer);

    // 构建一个简单的 HTTP 响应
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 17\r\n"
                           "\r\n"
                           "Hello, libev!\r\n";

    // 发送响应给客户端
    if (send(w->fd, response, strlen(response), 0) < 0) {
      perror("send");
    }

    // 在发送完响应后，关闭客户端连接
    // 对于一个简单的 demo，我们不处理长连接（Keep-Alive）
    ev_io_stop(loop, w);
    close(w->fd);
    free(w);
    return;
  }
}

// 新的客户端连接的事件处理回调函数
static void server_accept_cb(EV_P_ struct ev_io *w, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd;

  // 接受新的客户端连接
  client_fd = accept(w->fd, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    perror("accept");
    return;
  }

  printf("Accepted connection from %s:%d (fd: %d)\n",
         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
         client_fd);

  // 为新的客户端连接创建一个 ev_io 事件结构体
  // 使用 malloc 分配内存，因为我们会在事件处理中释放它
  struct ev_io *client_watcher = (struct ev_io *)malloc(sizeof(struct ev_io));
  if (!client_watcher) {
    perror("malloc failed");
    close(client_fd);
    return;
  }

  // 初始化这个 watcher，使其监听客户端的读事件
  // EV_AIO 是异步 I/O 的事件类型
  ev_io_init(client_watcher, client_read_cb, client_fd, EV_READ);

  // 将这个 watcher 添加到 libev 的事件循环中
  ev_io_start(loop, client_watcher);
}

int main() {
  int server_fd;
  struct sockaddr_in server_addr;
  struct ev_loop *loop; // libev 事件循环

  // 1. 创建服务器 socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // 设置 socket 可重用，避免端口被占用时启动失败
  int optval = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    // 这个不致命，可以继续
  }

  // 2. 绑定地址和端口
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
  server_addr.sin_port = htons(LISTEN_PORT); // 指定端口

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 3. 开始监听连接
  if (listen(server_fd, 5) < 0) { // 5 是连接队列的大小
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("HTTP Server started on port %d...\n", LISTEN_PORT);

  // 4. 初始化 libev 事件循环
  loop =
      ev_default_loop(0); // 0 表示使用默认的事件后端 (epoll, kqueue, select 等)

  // 5. 创建并注册服务器 socket 的监听事件
  struct ev_io *server_watcher = (struct ev_io *)malloc(sizeof(struct ev_io));
  if (!server_watcher) {
    perror("malloc failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 初始化 server_watcher，监听 server_fd 的读事件（即新的连接到达）
  // 回调函数是 server_accept_cb
  ev_io_init(server_watcher, server_accept_cb, server_fd, EV_READ);

  // 将 server_watcher 加入事件循环
  ev_io_start(loop, server_watcher);

  // 6. 启动事件循环
  // ev_loop 会一直运行，直到你手动停止所有事件或退出程序
  // 最后一个参数 EVLOOP_AUTO 表示自动选择最优的事件分发机制
  ev_loop(loop, 0);

  // 程序不会执行到这里，除非 loop 被打破 (例如通过 ev_break)

  // 清理（虽然通常不会到达这里）
  ev_io_stop(loop, server_watcher);
  close(server_fd);
  free(server_watcher);
  ev_loop_destroy(loop);

  return 0;
}
