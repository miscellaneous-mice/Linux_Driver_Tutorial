### **Global Variables**

```
static struct hrtimer my_hrtimer;
u64 start_t;
```

- `my_hrtimer` is the **high-resolution timer object** itself. Unlike `struct timer_list` (older kernel timers), `hrtimer`can schedule timers with **nanosecond precision**.
- `start_t` stores the **start time in jiffies** (kernel ticks). The kernel uses `jiffies` as a monotonic counter that increments with every timer tick (~1–10 ms depending on configuration).
	- global and static variables are zero-initialized by default
	- So `start_t` starts as `0` before `ModuleInit` runs.

---
### **Timer Callback Function**

```
static enum hrtimer_restart test_hrtimer_handler(struct hrtimer *timer) {
	u64 now_t = jiffies;
	printk("start_t - now_t = %u\n", jiffies_to_msecs(now_t - start_t));
	return HRTIMER_NORESTART;
}
```

- `now_t = jiffies` → Reads **current kernel time** in ticks.
- `now_t - start_t` → Computes **time elapsed since the timer was started** in ticks.
- `jiffies_to_msecs(...)` → Converts ticks to **milliseconds**.
- `printk` → Prints the elapsed time to the kernel log.
    - This allows you to measure **how accurate the timer is**.        
- `return HRTIMER_NORESTART` → Ensures the timer runs **only once**.
    - If you wanted a repeating timer, you’d return `HRTIMER_RESTART`.

**Extra detail:**
- This handler runs in **softirq context**, which means **no sleeping** is allowed.
- You cannot use functions like `msleep()` here. Only async-safe kernel APIs are allowed.

---

### **Module Initialization**

```
static int __init ModuleInit(void) {

	printk("Hello, Kernel!\n");
	hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL); // init timef

	my_hrtimer.function = &test_hrtimer_handler; // Set the callback func

	start_t = jiffies; // current timer ticks

	hrtimer_start(&my_hrtimer, ms_to_ktime(100), HRTIMER_MODE_REL); // start the timer for 100 ms then trigger the callback function after expiration

	return 0;
}
```

Step-by-step:
- **`hrtimer_init`**
    - `CLOCK_MONOTONIC`: Timer uses a clock that always moves forward, unaffected by system time changes.
    - `HRTIMER_MODE_REL`: The timer expires **relative to now** (not an absolute timestamp).

-  **Assign the callback**:
    - `my_hrtimer.function = &test_hrtimer_handler;`
    
- **Record start time**:
    - `start_t = jiffies` lets the callback calculate elapsed time.
    
- **Start the timer**:
    - `hrtimer_start(&my_hrtimer, ms_to_ktime(100), HRTIMER_MODE_REL)`
        - Converts 100 ms to `ktime_t` (nanosecond precision).
        - Schedules the callback to run **100 ms later**.
	- When `hrtimer` reaches its expiry, the kernel schedules the callback **to run in softirq context** (not normal process context). This means:
	    - You **cannot sleep or block** (`msleep`, `schedule` are forbidden).
	    - Only async-safe operations are allowed (`printk`, updating variables, queueing work, etc.).
- Returns `0` → Kernel knows module loaded successfully.



**Extra detail:**
- `hrtimer_start` schedules the timer **in the kernel’s high-resolution timer queue**, which is **more precise than standard jiffies timers**.
- The actual callback may not fire **exactly at 100 ms** due to CPU scheduling, but it’s very close (microsecond-level precision possible).
    

---

### **Module Exit**

```
static void __exit ModuleExit(void) {

	hrtimer_cancel(&my_hrtimer); // to handle timer when rmmod is execute while timer is executing

	printk("Goodbye, Kernel\n");
}
```

- `hrtimer_cancel(&my_hrtimer)`
    - Safely cancels the timer if it hasn’t already expired.
    - Returns 1 if a pending callback was active and canceled, 0 otherwise.
- Prints `"Goodbye, Kernel"` to indicate module unload.

**Extra detail:**

- Cancelling ensures you don’t leave **dangling timer callbacks**, which could crash the kernel if they try to access freed memory.
- Even though this module’s timer is **one-shot**, calling cancel is a safe practice.

---

### **Key Points**

- High-resolution timers are **better than `timer_list`** for sub-millisecond precision.
- `jiffies` provides coarse-grained time; it’s used here to **measure elapsed time**.
- Timer callbacks run in **softirq context** → cannot block/sleep.
- Returning `HRTIMER_NORESTART` makes it **one-shot**, while `HRTIMER_RESTART` makes it **periodic**.