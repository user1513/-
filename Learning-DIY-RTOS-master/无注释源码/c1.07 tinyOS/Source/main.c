#define	NVIC_INT_CTRL			0xE000Ed04
#define	NVIC_PENDSVSET			0x10000000
#define	NVIC_SYSPRI2			0xE000ED22
#define	NVIC_PENDSV_PRI			0x000000FF

#define	MEM32(addr)				*(volatile unsigned long *)(addr)
#define	MEM8(addr)				*(volatile unsigned char *)(addr)
	
void triggerPendSVC (void)
{
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

typedef struct _BlockType_t
{
	unsigned long * stackPtr;
}BlockType_t;

BlockType_t * blockPtr;

void delay (int count)
{
	while (--count > 0);
}

int flag;

unsigned long stackBuffer[1024];
BlockType_t block;

int main ()
{
	blockPtr = &block;
	for (;;)
	{
		flag = 0;
		delay(100);
		flag = 1;
		delay(100);
		
		// 这里做了较小的调整，与视频中不太一致
		// 如果按视频中所写，全速运行稍长时间会导致switch里内存写越办
		block.stackPtr = &stackBuffer[1024];
		triggerPendSVC();
	}
}
