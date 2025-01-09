### **Features and Differences of `poll` and `epoll`**

#### **1. `poll`**
`poll` is a system call in Unix-like systems for monitoring multiple file descriptors to see if I/O is possible. It's similar to `select` but avoids some of its limitations.

**Features**:
1. **Dynamic Descriptor Monitoring**:
   - Unlike `select`, `poll` does not require a fixed-size array for file descriptors. It uses a `struct pollfd` array, allowing dynamic allocation.
   
2. **Scalable Descriptor Limit**:
   - `poll` removes the file descriptor limit imposed by `select` (which is often restricted to 1024 descriptors).
   
3. **Event Notification**:
   - `poll` supports multiple event types, such as read readiness (`POLLIN`), write readiness (`POLLOUT`), and error conditions (`POLLERR`).

**Usage**:
- Used when a moderate number of file descriptors need to be monitored.
- Less efficient for very large numbers of file descriptors because it scans the entire descriptor list linearly.

**Structure**:
```c
struct pollfd {
    int fd;        // File descriptor to monitor
    short events;  // Events to monitor (e.g., POLLIN, POLLOUT)
    short revents; // Events returned (e.g., POLLIN triggered)
};
```

**Example**:
```cpp
#include <poll.h>
#include <iostream>

pollfd fds[2];
fds[0].fd = server_fd;  // Monitor server socket
fds[0].events = POLLIN; // Monitor for incoming data

int ret = poll(fds, 1, 1000); // Timeout of 1000ms
if (ret > 0 && (fds[0].revents & POLLIN)) {
    std::cout << "Data ready to read!" << std::endl;
}
```

---

#### **2. `epoll`**
`epoll` is a Linux-specific API designed to handle large-scale file descriptor monitoring efficiently. It is an improvement over both `select` and `poll`.

**Features**:
1. **High Scalability**:
   - Optimized for a large number of file descriptors. It uses a tree-based or hash-based data structure, making event lookups O(1) in many cases.
   
2. **Edge and Level Triggering**:
   - Supports **edge-triggered** mode (notify only on new events) and **level-triggered** mode (notify as long as the condition persists).

3. **Persistent Descriptor List**:
   - Once a file descriptor is added to the `epoll` instance, there is no need to pass it again (unlike `poll` or `select`).

4. **Efficient Event Notifications**:
   - Uses an event buffer to return only descriptors with active events, avoiding the need to scan all descriptors.

**Usage**:
- Ideal for high-performance servers needing to monitor thousands of file descriptors.

**Key Functions**:
1. **`epoll_create`**:
   - Creates an `epoll` instance for managing descriptors.
2. **`epoll_ctl`**:
   - Adds, modifies, or removes descriptors in the `epoll` instance.
3. **`epoll_wait`**:
   - Waits for events on the descriptors in the `epoll` instance.

**Example**:
```cpp
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>

int epfd = epoll_create(1); // Create epoll instance
if (epfd < 0) {
    std::cerr << "epoll_create failed!" << std::endl;
    exit(EXIT_FAILURE);
}

struct epoll_event ev, events[10];
ev.events = EPOLLIN;  // Monitor for incoming data
ev.data.fd = server_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev); // Add server_fd to epoll

int nfds = epoll_wait(epfd, events, 10, 1000); // Wait for events
for (int i = 0; i < nfds; ++i) {
    if (events[i].data.fd == server_fd) {
        std::cout << "Incoming connection!" << std::endl;
    }
}
close(epfd);
```

---

### **Comparison**

| Feature               | `poll`                             | `epoll`                              |
|-----------------------|------------------------------------|--------------------------------------|
| **Platform**          | Cross-platform (POSIX)             | Linux-specific                       |
| **Performance**       | Linear time with descriptor count  | O(1) for most operations             |
| **Descriptor Limit**  | No intrinsic limit (system-defined)| No intrinsic limit (system-defined)  |
| **Event Filtering**   | Scans all descriptors              | Reports only descriptors with events |
| **Triggering Modes**  | Level-triggered only               | Level-triggered and edge-triggered   |

---
   - Use poll if you need portability and have a small to moderate number of file descriptors.
   - Use epoll for Linux-only applications that require high scalability and performance for large descriptor sets.
