/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"

static const char *ep;

const char *cJSON_GetErrorPtr(void) { return ep; }
/*
cJSON_strcasecmp 字符串比较compare
返回值 0:
情况1:形参s1和s2均为空
情况2:形参s1不为空,遍历比较时,tolower(s1)和tolower(s2)完全相等
返回值 1:形参s2为空
返回值: = tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
情况1: > 0 
情况2: < 0
*/
static int cJSON_strcasecmp(const char *s1, const char *s2)
{
	if (!s1)
		return (s1 == s2) ? 0 : 1;
	if (!s2)
		return 1;
	for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
		if (*s1 == 0)
			return 0;
	return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

/*定义函数型指针cJSON_malloc,指向系统函数malloc*/
static void *(*cJSON_malloc)(size_t sz) = malloc;
/*定义函数型指针cJSON_free,指向系统函数free*/
static void (*cJSON_free)(void *ptr) = free;

/*
cJSON_strdup 字符串拷贝函数
返回值=NULL:代表malloc失败
返回值非NULL:代表分配成功 

strdup()在内部调用了malloc()为变量分配内存，
不需要使用返回的字符串时，
需要用free()释放相应的内存空间，
否则会造成内存泄漏。
*/
static char *cJSON_strdup(const char *str)
{
	size_t len;
	char *copy;
	//strlen(char *)碰到'\0'就返回'\0'以前的字符数。
	len = strlen(str) + 1;
	if (!(copy = (char *)cJSON_malloc(len)))
		return 0;
	memcpy(copy, str, len);
	return copy;
}

/*
cJSON_InitHooks 初始化钩子函数
作用:
给cJSON_malloc函数指针重新赋值
给cJSON_free函数指针重新赋值
*/
void cJSON_InitHooks(cJSON_Hooks *hooks)
{

	if (!hooks)
	{ /* Reset hooks */
		cJSON_malloc = malloc;
		cJSON_free = free;
		return;
	}

	cJSON_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
	cJSON_free = (hooks->free_fn) ? hooks->free_fn : free;
}

/* Internal constructor. */
static cJSON *cJSON_New_Item(void)
{
	cJSON *node = (cJSON *)cJSON_malloc(sizeof(cJSON));
	if (node)//给结构体整体赋为0
		memset(node, 0, sizeof(cJSON));
	return node;
}
//存在疑问,type的第九位和第八位有什么用处
//01000000000
//1<<9 1<<8 1<<7 1<<6 1<<5 1<<4 1<<3 1<<2 1<<1 1<<0
//512  256  128  64   32   16   8    4    2    1
/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *c)
{
	cJSON *next;
	while (c)
	{
		next = c->next;
		//代表c->type不存在上述结构
		if (!(c->type & cJSON_IsReference) && c->child)
			cJSON_Delete(c->child);
		if (!(c->type & cJSON_IsReference) && c->valuestring)
			cJSON_free(c->valuestring);
		if (!(c->type & cJSON_StringIsConst) && c->string)
			cJSON_free(c->string);
		cJSON_free(c);
		c = next;
	}
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(cJSON *item, const char *num)
{
	double n = 0, sign = 1, scale = 0;
	//分量表			//符号量表
	int subscale = 0, signsubscale = 1;

	if (*num == '-')
		sign = -1, num++; /* Has sign? */
	if (*num == '0')
		num++; /* is zero */
	if (*num >= '1' && *num <= '9')
		do
			n = (n * 10.0) + (*num++ - '0');
		while (*num >= '0' && *num <= '9'); /* Number? */
	
	//经过上面这步,可以将整数部分全部获取
	if (*num == '.' && num[1] >= '0' && num[1] <= '9')
	{
		num++;
		do
			n = (n * 10.0) + (*num++ - '0'), scale--;//scale记录小数的位数
		while (*num >= '0' && *num <= '9');
	}								/* Fractional part? */
	if (*num == 'e' || *num == 'E') /* Exponent? */
	{
		num++;
		if (*num == '+')
			num++;
		else if (*num == '-')
			signsubscale = -1, num++; /* With sign? */
		while (*num >= '0' && *num <= '9')
			subscale = (subscale * 10) + (*num++ - '0'); /* Number? */
	}

	n = sign * n * pow(10.0, (scale + subscale * signsubscale)); /* number = +/- number.fraction * 10^+/- exponent */

	item->valuedouble = n;
	item->valueint = (int)n;
	item->type = cJSON_Number;
	return num;
}


//该函数的作用是返回>=x的最小的2的N次方数。
static int pow2gt(int x)
{					
	--x;			
	x |= x >> 1;	
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

typedef struct
{
	char *buffer;
	int length;
	int offset;			//偏移量
} printbuffer;

//使用buffer时当offset+needed值大于length时存在数组越界
//当出现越界会重新分配一个pow2gt()大的buffer存放数据
//ensure只负责数组是否安全不负责offset的增加,增加由update函数辅助
static char *ensure(printbuffer *p, int needed)
{
	char *newbuffer;
	int newsize;
	if (!p || !p->buffer)
		return 0;
	needed += p->offset;		//needed加上原有偏移量
	if (needed <= p->length)	//原有偏移量加上输入的needed的小于数组总长度
		return p->buffer + p->offset;//代表数组没有越界

	newsize = pow2gt(needed);
	newbuffer = (char *)cJSON_malloc(newsize);
	if (!newbuffer)
	{
		cJSON_free(p->buffer);
		p->length = 0, p->buffer = 0;
		return 0;
	}
	if (newbuffer)
		memcpy(newbuffer, p->buffer, p->length);
	cJSON_free(p->buffer);
	p->length = newsize;
	p->buffer = newbuffer;
	return newbuffer + p->offset;
}

//更新(偏移量+偏移量位置到\0的长度)
static int update(printbuffer *p)
{
	char *str;
	if (!p || !p->buffer)
		return 0;
	str = p->buffer + p->offset;
	return p->offset + strlen(str);
}
//疑问 1.为什么要给int分配21个字节
//疑问 2.为什么要给double分配64个字节
/* Render the number nicely from the given item into a string. */
static char *print_number(cJSON *item, printbuffer *p)
{
	char *str = 0;
	double d = item->valuedouble;
	if (d == 0)
	{
		if (p)
			str = ensure(p, 2);
		else
			str = (char *)cJSON_malloc(2); /* special case for 0. */
		if (str)
			strcpy(str, "0");
	}
	//判断valuedouble是不是一个整数
	else if (fabs(((double)item->valueint) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
	{
		if (p)
			str = ensure(p, 21);
		else
			str = (char *)cJSON_malloc(21); /* 2^64+1 can be represented in 21 chars. */
		if (str)
			sprintf(str, "%d", item->valueint);
	}
	else
	{
		if (p)
			str = ensure(p, 64);
		else
			str = (char *)cJSON_malloc(64); /* This is a nice tradeoff. */
		if (str)
		{
			//判断valuedouble的值是否存在小数,在上面判断整数的if语句中限制了valuedouble的大小,使得大于int的数值不会进入到整数中
			if (fabs(floor(d) - d) <= DBL_EPSILON && fabs(d) < 1.0e60)
				sprintf(str, "%.0f", d);
			else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9)
				sprintf(str, "%e", d);
			else
				sprintf(str, "%f", d);
		}
	}
	return str;
}

//解析2个字节的内容(16进制转换成10进制)
static unsigned parse_hex4(const char *str)
{
	unsigned h = 0;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	return h;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
static const char *parse_string(cJSON *item, const char *str)
{
	const char *ptr = str + 1;
	char *ptr2;
	char *out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"')
	{
		ep = str;
		return 0;
	} /* not a string! */

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\')//
			ptr++; /* Skip escaped quotes. 跳过转义的引号*/

	out = (char *)cJSON_malloc(len + 1); /* This is how long we need for the string, roughly. */
	if (!out)
		return 0;

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr)
	{
		if (*ptr != '\\')
			*ptr2++ = *ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
			case 'b':
				*ptr2++ = '\b';//--->/b:回退：向后退一格
				break;
			case 'f':
				*ptr2++ = '\f';//--->/f:换页
				break;
			case 'n':
				*ptr2++ = '\n';//--->/n:换行，光标到下行行首
				break;
			case 'r':
				*ptr2++ = '\r';//--->/r:回车，光标到本行行首
				break;
			case 't':
				*ptr2++ = '\t';//--->/t:水平制表
				break;
			case 'u': /* transcode utf16 to utf8. */
				uc = parse_hex4(ptr + 1);
				ptr += 4; /* get the unicode char. */
				//低位的数据先读出,可能是一个无效的数据
				if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
					break; /* check for invalid.	*/
				//utf-16高字节代码对
				if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/
				{
					//判断是否缺少低字节
					if (ptr[1] != '\\' || ptr[2] != 'u')
						break; /* missing second-half of surrogate.	*/
					uc2 = parse_hex4(ptr + 3);
					ptr += 6;
					//判断新得到的两个字节是否处于低字节代码对
					if (uc2 < 0xDC00 || uc2 > 0xDFFF)
						break; /* invalid second-half of surrogate.	*/

					//utf-16 --->>>  unicode
					uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
				}

				len = 4;
				if (uc < 0x80)
					len = 1;
				else if (uc < 0x800)
					len = 2;
				else if (uc < 0x10000)
					len = 3;
				ptr2 += len;

				switch (len)
				{
				case 4:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 3:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 2:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 1:
					*--ptr2 = (uc | firstByteMark[len]);
				}
				ptr2 += len;
				break;
			default:
				*ptr2++ = *ptr;
				break;
			}
			ptr++;
		}
	}
	*ptr2 = 0;
	if (*ptr == '\"')
		ptr++;
	item->valuestring = out;
	item->type = cJSON_String;
	return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
//个人理解:print_string_ptr渲染是一些我们不是很敏感的的字符转义成实实在在的文字
static char *print_string_ptr(const char *str, printbuffer *p)
{
	const char *ptr;
	char *ptr2, *out;
	int len = 0, flag = 0;
	unsigned char token;

	for (ptr = str; *ptr; ptr++)//判断字符串内部有没有控制字符,'\"'和'\\'
		flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
	if (!flag)//字符串内部不存在控制字符,'\"'和'\\'则进入if
	{
		len = ptr - str;
		if (p)
			out = ensure(p, len + 3);//+3是为了开头增加'\"'结尾增加'\"'和'\0'
		else
			out = (char *)cJSON_malloc(len + 3);
		if (!out)
			return 0;
		ptr2 = out;
		*ptr2++ = '\"';
		strcpy(ptr2, str);
		ptr2[len] = '\"';
		ptr2[len + 1] = 0;
		return out;//输出字符串
	}

	if (!str)//代表输入str为NULL
	{
		if (p)
			out = ensure(p, 3);
		else
			out = (char *)cJSON_malloc(3);
		if (!out)//分配空间识别
			return 0;
		strcpy(out, "\"\"");
		return out;
	}
	ptr = str;
	//这里计算加2('\\'+字符)和加6('\\' + uxxxx)是按照下面格式分配的
	while ((token = *ptr) && ++len)
	{
		if (strchr("\"\\\b\f\n\r\t", token))
			len++;
		else if (token < 32)
			len += 5;
		ptr++;
	}

	if (p)
		out = ensure(p, len + 3);
	else
		out = (char *)cJSON_malloc(len + 3);
	if (!out)
		return 0;

	ptr2 = out;
	ptr = str;
	*ptr2++ = '\"';
	while (*ptr)
	{
		if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
			*ptr2++ = *ptr++;
		else
		{
			*ptr2++ = '\\';
			switch (token = *ptr++)
			{
			case '\\':
				*ptr2++ = '\\';
				break;
			case '\"':
				*ptr2++ = '\"';
				break;
			case '\b':
				*ptr2++ = 'b';
				break;
			case '\f':
				*ptr2++ = 'f';
				break;
			case '\n':
				*ptr2++ = 'n';
				break;
			case '\r':
				*ptr2++ = 'r';
				break;
			case '\t':
				*ptr2++ = 't';
				break;
			default:
				sprintf(ptr2, "u%04x", token);
				ptr2 += 5;
				break; /* escape and print */
			}
		}
	}
	*ptr2++ = '\"';
	*ptr2++ = 0;
	return out;
}
/* Invoke print_string_ptr (which is useful) on an item. */
static char *print_string(cJSON *item, printbuffer *p) { return print_string_ptr(item->valuestring, p); }

/* Predeclare these prototypes. */
static const char *parse_value(cJSON *item, const char *value);
static char *print_value(cJSON *item, int depth, int fmt, printbuffer *p);
static const char *parse_array(cJSON *item, const char *value);
static char *print_array(cJSON *item, int depth, int fmt, printbuffer *p);
static const char *parse_object(cJSON *item, const char *value);
static char *print_object(cJSON *item, int depth, int fmt, printbuffer *p);

// CR：Carriage Return，对应ASCII中转义字符\r，表示回车
// LF：Linefeed，对应ASCII中转义字符\n，表示换行
// CRLF：Carriage Return & Linefeed，\r\n，表示回车并换行

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
	/**/
	while (in && *in && (unsigned char)*in <= 32)
		in++;
	return in;
}

/* Parse an object - create a new root, and populate. */
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
	const char *end = 0;
	cJSON *c = cJSON_New_Item();
	ep = 0;
	if (!c)
		return 0; /* memory fail */
	//通过skip(value)后的value为空
	end = parse_value(c, skip(value));
	if (!end)
	{
		cJSON_Delete(c);
		return 0;
	} /* parse failure. ep is set. */

	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
	if (require_null_terminated)
	{
		end = skip(end);
		if (*end)
		{
			cJSON_Delete(c);
			ep = end;
			return 0;
		}
	}
	if (return_parse_end)
		*return_parse_end = end;
	return c;
}
/* Default options for cJSON_Parse */
cJSON *cJSON_Parse(const char *value) { return cJSON_ParseWithOpts(value, 0, 0); }

/* Render a cJSON item/entity/structure to text. */
char *cJSON_Print(cJSON *item) { return print_value(item, 0, 1, 0); }
char *cJSON_PrintUnformatted(cJSON *item) { return print_value(item, 0, 0, 0); }
//先创建一个新的buffer返回buffer
char *cJSON_PrintBuffered(cJSON *item, int prebuffer, int fmt)
{
	printbuffer p;
	p.buffer = (char *)cJSON_malloc(prebuffer);
	p.length = prebuffer;
	p.offset = 0;
	return print_value(item, 0, fmt, &p);
	return p.buffer;
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(cJSON *item, const char *value)
{
	if (!value)
		return 0; /* Fail on null. */
	if (!strncmp(value, "null", 4))
	{
		item->type = cJSON_NULL;
		return value + 4;
	}
	if (!strncmp(value, "false", 5))
	{
		item->type = cJSON_False;
		return value + 5;
	}
	if (!strncmp(value, "true", 4))
	{
		item->type = cJSON_True;
		item->valueint = 1;
		return value + 4;
	}
	if (*value == '\"')
	{
		return parse_string(item, value);
	}
	if (*value == '-' || (*value >= '0' && *value <= '9'))
	{
		return parse_number(item, value);
	}
	if (*value == '[')
	{
		return parse_array(item, value);
	}
	if (*value == '{')
	{
		return parse_object(item, value);
	}

	ep = value;
	return 0; /* failure. */
}

/* Render a value to text. */
static char *print_value(cJSON *item, int depth, int fmt, printbuffer *p)
{
	char *out = 0;
	if (!item)
		return 0;
	if (p)
	{
		switch ((item->type) & 255)
		{
		case cJSON_NULL:
		{
			out = ensure(p, 5);
			if (out)
				strcpy(out, "null");
			break;
		}
		case cJSON_False:
		{
			out = ensure(p, 6);
			if (out)
				strcpy(out, "false");
			break;
		}
		case cJSON_True:
		{
			out = ensure(p, 5);
			if (out)
				strcpy(out, "true");
			break;
		}
		case cJSON_Number:
			out = print_number(item, p);
			break;
		case cJSON_String:
			out = print_string(item, p);
			break;
		case cJSON_Array:
			out = print_array(item, depth, fmt, p);
			break;
		case cJSON_Object:
			out = print_object(item, depth, fmt, p);
			break;
		}
	}
	else
	{
		switch ((item->type) & 255)
		{
		case cJSON_NULL:
			out = cJSON_strdup("null");
			break;
		case cJSON_False:
			out = cJSON_strdup("false");
			break;
		case cJSON_True:
			out = cJSON_strdup("true");
			break;
		case cJSON_Number:
			out = print_number(item, 0);
			break;
		case cJSON_String:
			out = print_string(item, 0);
			break;
		case cJSON_Array:
			out = print_array(item, depth, fmt, 0);
			break;
		case cJSON_Object:
			out = print_object(item, depth, fmt, 0);
			break;
		}
	}
	return out;
}

/* Build an array from input text. */
static const char *parse_array(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '[')
	{
		ep = value;
		return 0;
	} /* not an array! */

	item->type = cJSON_Array;
	value = skip(value + 1);
	if (*value == ']')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0;								   /* memory fail */
	value = skip(parse_value(child, skip(value))); /* skip any spacing, get the value. */
	if (!value)
		return 0;

	while (*value == ',')
	{
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_value(child, skip(value + 1)));
		if (!value)
			return 0; /* memory fail */
	}

	if (*value == ']')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Render an array to text */
static char *print_array(cJSON *item, int depth, int fmt, printbuffer *p)
{
	char **entries;
	char *out = 0, *ptr, *ret;
	int len = 5;
	cJSON *child = item->child;
	int numentries = 0, i = 0, fail = 0;
	size_t tmplen = 0;
	//numentries计算同一层内有多少个条数据
	/* How many entries in the array? */
	while (child)
		numentries++, child = child->next;
	/* Explicitly handle numentries==0 */
	if (!numentries)
	{
		if (p)
			out = ensure(p, 3);
		else
			out = (char *)cJSON_malloc(3);
		if (out)
			strcpy(out, "[]");
		return out;
	}

	if (p)
	{
		/* Compose the output array. */
		i = p->offset;
		//ensure只负责数组是否安全不负责offset的增加,增加由update函数辅助
		ptr = ensure(p, 1);
		if (!ptr)
			return 0;
		*ptr = '[';
		p->offset++;
		child = item->child;
		while (child && !fail)
		{
			print_value(child, depth + 1, fmt, p);
			p->offset = update(p);
			if (child->next)
			{
				len = fmt ? 2 : 1;
				ptr = ensure(p, len + 1);
				if (!ptr)
					return 0;
				*ptr++ = ',';
				if (fmt)
					*ptr++ = ' ';
				*ptr = 0;
				p->offset += len;
			}
			child = child->next;
		}
		ptr = ensure(p, 2);
		if (!ptr)
			return 0;
		*ptr++ = ']';
		*ptr = 0;
		out = (p->buffer) + i;
	}
	else
	{
		/* Allocate an array to hold the values for each */

		//创建指针型数组,数组中每一个变量都是指针
		entries = (char **)cJSON_malloc(numentries * sizeof(char *));
		if (!entries)
			return 0;
		memset(entries, 0, numentries * sizeof(char *));
		/* Retrieve all the results: */
		child = item->child;
		while (child && !fail)
		{
			ret = print_value(child, depth + 1, fmt, 0);
			entries[i++] = ret;
			if (ret)
				len += strlen(ret) + 2 + (fmt ? 1 : 0);
			else
				fail = 1;
			child = child->next;
		}

		/* If we didn't fail, try to malloc the output string */
		if (!fail)
			out = (char *)cJSON_malloc(len);
		/* If that fails, we fail. */
		if (!out)
			fail = 1;

		/* Handle failure. */
		if (fail)
		{
			for (i = 0; i < numentries; i++)
				if (entries[i])
					cJSON_free(entries[i]);
			cJSON_free(entries);
			return 0;
		}

		/* Compose the output array. */
		*out = '[';
		ptr = out + 1;
		*ptr = 0;
		for (i = 0; i < numentries; i++)
		{
			tmplen = strlen(entries[i]);
			memcpy(ptr, entries[i], tmplen);
			ptr += tmplen;
			if (i != numentries - 1)
			{
				*ptr++ = ',';
				if (fmt)
					*ptr++ = ' ';
				*ptr = 0;
			}
			cJSON_free(entries[i]);
		}
		cJSON_free(entries);
		*ptr++ = ']';
		*ptr++ = 0;
	}
	return out;
}

/* Build an object from the text. */
static const char *parse_object(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '{')
	{
		ep = value;
		return 0;
	} /* not an object! */

	item->type = cJSON_Object;
	value = skip(value + 1);
	if (*value == '}')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0;
	value = skip(parse_string(child, skip(value)));
	if (!value)
		return 0;
	child->string = child->valuestring;
	child->valuestring = 0;
	if (*value != ':')
	{
		ep = value;
		return 0;
	}												   /* fail! */
	value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
	if (!value)
		return 0;

	while (*value == ',')
	{
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value + 1)));
		if (!value)
			return 0;
		child->string = child->valuestring;
		child->valuestring = 0;
		if (*value != ':')
		{
			ep = value;
			return 0;
		}												   /* fail! */
		value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
		if (!value)
			return 0;
	}

	if (*value == '}')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Render an object to text. */
static char *print_object(cJSON *item, int depth, int fmt, printbuffer *p)
{
	char **entries = 0, **names = 0;
	char *out = 0, *ptr, *ret, *str;
	int len = 7, i = 0, j;
	cJSON *child = item->child;
	int numentries = 0, fail = 0;
	size_t tmplen = 0;
	/* Count the number of entries. */
	while (child)
		numentries++, child = child->next;
	/* Explicitly handle empty object case */
	if (!numentries)
	{
		if (p)
			out = ensure(p, fmt ? depth + 4 : 3);
		else
			out = (char *)cJSON_malloc(fmt ? depth + 4 : 3);
		if (!out)
			return 0;
		ptr = out;
		*ptr++ = '{';
		if (fmt)
		{
			*ptr++ = '\n';
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		}
		*ptr++ = '}';
		*ptr++ = 0;
		return out;
	}
	if (p)
	{
		/* Compose the output: */
		i = p->offset;
		len = fmt ? 2 : 1;
		ptr = ensure(p, len + 1);
		if (!ptr)
			return 0;
		*ptr++ = '{';
		if (fmt)
			*ptr++ = '\n';
		*ptr = 0;
		p->offset += len;
		child = item->child;
		depth++;
		while (child)
		{
			if (fmt)
			{
				ptr = ensure(p, depth);
				if (!ptr)
					return 0;
				for (j = 0; j < depth; j++)
					*ptr++ = '\t';
				p->offset += depth;
			}
			print_string_ptr(child->string, p);
			p->offset = update(p);

			len = fmt ? 2 : 1;
			ptr = ensure(p, len);
			if (!ptr)
				return 0;
			*ptr++ = ':';
			if (fmt)
				*ptr++ = '\t';
			p->offset += len;

			print_value(child, depth, fmt, p);
			p->offset = update(p);

			len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
			ptr = ensure(p, len + 1);
			if (!ptr)
				return 0;
			if (child->next)
				*ptr++ = ',';
			if (fmt)
				*ptr++ = '\n';
			*ptr = 0;
			p->offset += len;
			child = child->next;
		}
		ptr = ensure(p, fmt ? (depth + 1) : 2);
		if (!ptr)
			return 0;
		if (fmt)
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		*ptr++ = '}';
		*ptr = 0;
		out = (p->buffer) + i;
	}
	else
	{
		/* Allocate space for the names and the objects */
		entries = (char **)cJSON_malloc(numentries * sizeof(char *));
		if (!entries)
			return 0;
		names = (char **)cJSON_malloc(numentries * sizeof(char *));
		if (!names)
		{
			cJSON_free(entries);
			return 0;
		}
		memset(entries, 0, sizeof(char *) * numentries);
		memset(names, 0, sizeof(char *) * numentries);

		/* Collect all the results into our arrays: */
		child = item->child;
		depth++;
		if (fmt)
			len += depth;
		while (child)
		{
			names[i] = str = print_string_ptr(child->string, 0);
			entries[i++] = ret = print_value(child, depth, fmt, 0);
			if (str && ret)
				len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
			else
				fail = 1;
			child = child->next;
		}

		/* Try to allocate the output string */
		if (!fail)
			out = (char *)cJSON_malloc(len);
		if (!out)
			fail = 1;

		/* Handle failure */
		if (fail)
		{
			for (i = 0; i < numentries; i++)
			{
				if (names[i])
					cJSON_free(names[i]);
				if (entries[i])
					cJSON_free(entries[i]);
			}
			cJSON_free(names);
			cJSON_free(entries);
			return 0;
		}

		/* Compose the output: */
		*out = '{';
		ptr = out + 1;
		if (fmt)
			*ptr++ = '\n';
		*ptr = 0;
		for (i = 0; i < numentries; i++)
		{
			if (fmt)
				for (j = 0; j < depth; j++)
					*ptr++ = '\t';
			tmplen = strlen(names[i]);
			memcpy(ptr, names[i], tmplen);
			ptr += tmplen;
			*ptr++ = ':';
			if (fmt)
				*ptr++ = '\t';
			strcpy(ptr, entries[i]);
			ptr += strlen(entries[i]);
			if (i != numentries - 1)
				*ptr++ = ',';
			if (fmt)
				*ptr++ = '\n';
			*ptr = 0;
			cJSON_free(names[i]);
			cJSON_free(entries[i]);
		}

		cJSON_free(names);
		cJSON_free(entries);
		if (fmt)
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		*ptr++ = '}';
		*ptr++ = 0;
	}
	return out;
}

/* Get Array size/item / object item. */
int cJSON_GetArraySize(cJSON *array)
{
	cJSON *c = array->child;
	int i = 0;
	while (c)
		i++, c = c->next;
	return i;
}
cJSON *cJSON_GetArrayItem(cJSON *array, int item)
{
	cJSON *c = array->child;
	while (c && item > 0)
		item--, c = c->next;
	return c;
}
cJSON *cJSON_GetObjectItem(cJSON *object, const char *string)
{
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
		c = c->next;
	return c;
}

/* Utility for array list handling. */
static void suffix_object(cJSON *prev, cJSON *item)
{
	prev->next = item;
	item->prev = prev;
}
/* Utility for handling references. */
static cJSON *create_reference(cJSON *item)
{
	cJSON *ref = cJSON_New_Item();
	if (!ref)
		return 0;
	memcpy(ref, item, sizeof(cJSON));
	ref->string = 0;
	ref->type |= cJSON_IsReference;
	ref->next = ref->prev = 0;
	return ref;
}

/* Add item to array/object. */
void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
	cJSON *c = array->child;
	if (!item)
		return;
	if (!c)
	{
		array->child = item;
	}
	else
	{
		while (c && c->next)
			c = c->next;
		suffix_object(c, item);
	}
}
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
	if (!item)
		return;
	if (item->string)
		cJSON_free(item->string);
	item->string = cJSON_strdup(string);
	cJSON_AddItemToArray(object, item);
}
void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item)
{
	if (!item)
		return;
	if (!(item->type & cJSON_StringIsConst) && item->string)
		cJSON_free(item->string);
	item->string = (char *)string;
	item->type |= cJSON_StringIsConst;
	cJSON_AddItemToArray(object, item);
}
void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item) { cJSON_AddItemToArray(array, create_reference(item)); }
void cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item) { cJSON_AddItemToObject(object, string, create_reference(item)); }

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which)
{
	cJSON *c = array->child;
	while (c && which > 0)
		c = c->next, which--;
	if (!c)
		return 0;
	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;
	if (c == array->child)
		array->child = c->next;
	c->prev = c->next = 0;
	return c;
}
void cJSON_DeleteItemFromArray(cJSON *array, int which) { cJSON_Delete(cJSON_DetachItemFromArray(array, which)); }
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
	int i = 0;
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
		i++, c = c->next;
	if (c)
		return cJSON_DetachItemFromArray(object, i);
	return 0;
}
void cJSON_DeleteItemFromObject(cJSON *object, const char *string) { cJSON_Delete(cJSON_DetachItemFromObject(object, string)); }

/* Replace array/object items with new ones. */
void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem)
{
	cJSON *c = array->child;
	while (c && which > 0)
		c = c->next, which--;
	if (!c)
	{
		cJSON_AddItemToArray(array, newitem);
		return;
	}
	newitem->next = c;
	newitem->prev = c->prev;
	c->prev = newitem;
	if (c == array->child)
		array->child = newitem;
	else
		newitem->prev->next = newitem;
}
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem)
{
	cJSON *c = array->child;
	while (c && which > 0)
		c = c->next, which--;
	if (!c)
		return;
	newitem->next = c->next;
	newitem->prev = c->prev;
	if (newitem->next)
		newitem->next->prev = newitem;
	if (c == array->child)
		array->child = newitem;
	else
		newitem->prev->next = newitem;
	c->next = c->prev = 0;
	cJSON_Delete(c);
}
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem)
{
	int i = 0;
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
		i++, c = c->next;
	if (c)
	{
		newitem->string = cJSON_strdup(string);
		cJSON_ReplaceItemInArray(object, i, newitem);
	}
}

/* Create basic types: */
cJSON *cJSON_CreateNull(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = cJSON_NULL;
	return item;
}
cJSON *cJSON_CreateTrue(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = cJSON_True;
	return item;
}
cJSON *cJSON_CreateFalse(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = cJSON_False;
	return item;
}
cJSON *cJSON_CreateBool(int b)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = b ? cJSON_True : cJSON_False;
	return item;
}
cJSON *cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item();
	if (item)
	{
		item->type = cJSON_Number;
		item->valuedouble = num;
		item->valueint = (int)num;
	}
	return item;
}
cJSON *cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item();
	if (item)
	{
		item->type = cJSON_String;
		item->valuestring = cJSON_strdup(string);
	}
	return item;
}
cJSON *cJSON_CreateArray(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = cJSON_Array;
	return item;
}
cJSON *cJSON_CreateObject(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)
		item->type = cJSON_Object;
	return item;
}

/* Create Arrays: */
cJSON *cJSON_CreateIntArray(const int *numbers, int count)
{
	int i;
	cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}
cJSON *cJSON_CreateFloatArray(const float *numbers, int count)
{
	int i;
	cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}
cJSON *cJSON_CreateDoubleArray(const double *numbers, int count)
{
	int i;
	cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}
cJSON *cJSON_CreateStringArray(const char **strings, int count)
{
	int i;
	cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateString(strings[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}

/* Duplication */
cJSON *cJSON_Duplicate(cJSON *item, int recurse)
{
	cJSON *newitem, *cptr, *nptr = 0, *newchild;
	/* Bail on bad ptr */
	if (!item)
		return 0;
	/* Create new item */
	newitem = cJSON_New_Item();
	if (!newitem)
		return 0;
	/* Copy over all vars */
	newitem->type = item->type & (~cJSON_IsReference), newitem->valueint = item->valueint, newitem->valuedouble = item->valuedouble;
	if (item->valuestring)
	{
		newitem->valuestring = cJSON_strdup(item->valuestring);
		if (!newitem->valuestring)
		{
			cJSON_Delete(newitem);
			return 0;
		}
	}
	if (item->string)
	{
		newitem->string = cJSON_strdup(item->string);
		if (!newitem->string)
		{
			cJSON_Delete(newitem);
			return 0;
		}
	}
	/* If non-recursive, then we're done! */
	if (!recurse)
		return newitem;
	/* Walk the ->next chain for the child. */
	cptr = item->child;
	while (cptr)
	{
		newchild = cJSON_Duplicate(cptr, 1); /* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild)
		{
			cJSON_Delete(newitem);
			return 0;
		}
		if (nptr)
		{
			nptr->next = newchild, newchild->prev = nptr;
			nptr = newchild;
		} /* If newitem->child already set, then crosswire ->prev and ->next and move on */
		else
		{
			newitem->child = newchild;
			nptr = newchild;
		} /* Set newitem->child and move to it */
		cptr = cptr->next;
	}
	return newitem;
}

void cJSON_Minify(char *json)
{
	char *into = json;
	while (*json)
	{
		if (*json == ' ')
			json++;
		else if (*json == '\t')
			json++; /* Whitespace characters. */
		else if (*json == '\r')
			json++;
		else if (*json == '\n')
			json++;
		else if (*json == '/' && json[1] == '/')
			while (*json && *json != '\n')
				json++; /* double-slash comments, to end of line. */
		else if (*json == '/' && json[1] == '*')
		{
			while (*json && !(*json == '*' && json[1] == '/'))
				json++;
			json += 2;
		} /* multiline comments. */
		else if (*json == '\"')
		{
			*into++ = *json++;
			while (*json && *json != '\"')
			{
				if (*json == '\\')
					*into++ = *json++;
				*into++ = *json++;
			}
			*into++ = *json++;
		} /* string literals, which are \" sensitive. */
		else
			*into++ = *json++; /* All other characters. */
	}
	*into = 0; /* and null-terminate. */
}
