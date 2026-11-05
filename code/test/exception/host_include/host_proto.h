/* trap.c 在调用点之后才定义这两个函数。
 * 带 weak 的前置声明会作用到本翻译单元后面的定义，测试里的强符号可以替换它们。 */
struct trapframe;
void exception_handler(struct trapframe *tf) __attribute__((weak));
void interrupt_handler(struct trapframe *tf) __attribute__((weak));
