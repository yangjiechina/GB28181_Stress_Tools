//
//  h264_parser.c
//  H264_Parser
//
//  Created by Ternence on 2019/3/22.
//  Copyright © 2019 Ternence. All rights reserved.
//

#include "h264_parser.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
	NALU_PRIPORITY_DISPOSABLE = 0,
	NALU_PRIORITY_LOW = 1,
	NALU_PRIORITY_HIGH = 2,
	NALU_PRIORITY_HIGHTEST = 3, a
}NaluPriority;

typedef struct {
	int             startcodeprefix_len;        //! 4 for parameter sets and first slice in picture, 3 for everything else (suggest)
	unsigned        len;                        //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU
	int             max_size;                   //! Nalu Unit Buffer size
	int             forbidden_bit;              //! should be always FALSE
	int             nal_reference_idc;          //! NALU_PRIPORITY_xxxx
	int             nal_unit_type;              //! NALU_TYPE_xxxx
	char*           buf;                        //! contains the first byte followed by the EBSP
}NALU_t;

FILE *h264bitstream = NULL;                     //! the bit stream file

int info2 = 0, info3 = 0;

static int FindStartCode2(unsigned char *Buf) {
	if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1) return 0;    //0x00 0001 ？
	else return 1;
}

static int FindStartCode3(unsigned char *Buf) {
	if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1) return 0;     //0x00 000001?
	else return 1;
}

int GetAnnexbNALU(NALU_t *nalu) {
	int pos = 0;
	int startCodeFound, rewind;
	unsigned char *Buf;


	if ((Buf = (unsigned char*)calloc(nalu->max_size, sizeof(char))) == NULL)
		printf("GetAnnexbNALU: Could not allocate Buf memory\n");

	nalu->startcodeprefix_len = 3;

	if (3 != fread(Buf, 1, 3, h264bitstream)) {
		free(Buf);
		return 0;
	}

	info2 = FindStartCode2(Buf);
	if (info2 != 1) {   //不是0x000001
		if (1 != fread(Buf + 3, 1, 1, h264bitstream)) {
			free(Buf);
			return -1;
		}
		info3 = FindStartCode3(Buf);
		if (info3 != 1) {       //不是0x00 000001?
			free(Buf);
			return -1;
		}
		else {
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}
	else {
		pos = 3;
		nalu->startcodeprefix_len = 3;
	}

	startCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!startCodeFound) {
		if (feof(h264bitstream)) {
			nalu->len = (pos - 1) - nalu->startcodeprefix_len;
			memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
			nalu->forbidden_bit = nalu->buf[0] & 0x80;       //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60;   //2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;     //5 bit
			free(Buf);
			return pos - 1;
		}
		Buf[pos++] = fgetc(h264bitstream);
		info3 = FindStartCode3(&Buf[pos - 4]);
		if (info3 != 1)
			info2 = FindStartCode2(&Buf[pos - 3]);
		startCodeFound = (info2 == 1 || info3 == 1);
	}

	// Here we have found another start code (and read length of startcode bytes more than we should
	// have, hence ,go back in the file)
	rewind = info3 == 1 ? -4 : -3;
	if (0 != fseek(h264bitstream, rewind, SEEK_CUR)) {
		free(Buf);
		printf("GetAnnexbNALU:Cannot fseek in the bit stream file");
	}


	// Here the Start code, the complete NALU, and the next start code is in the Buf
	// The size of Buf is pos , pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len = (pos + rewind) - nalu->startcodeprefix_len;
	memcpy(nalu->buf, Buf, nalu->len+ nalu->startcodeprefix_len);
	nalu->forbidden_bit = nalu->buf[nalu->startcodeprefix_len] & 0x80;       // 1 bit
	nalu->nal_reference_idc = nalu->buf[nalu->startcodeprefix_len] & 0x60;   // 2 bit
	nalu->nal_unit_type = nalu->buf[nalu->startcodeprefix_len] & 0x1f;       // 5 bit
	
	//nalu->len = (pos + rewind) - nalu->startcodeprefix_len;
	//memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
	//nalu->forbidden_bit = nalu->buf[0] & 0x80;       // 1 bit
	//nalu->nal_reference_idc = nalu->buf[0] & 0x60;   // 2 bit
	//nalu->nal_unit_type = nalu->buf[0] & 0x1f;       // 5 bit
	free(Buf);

	return (pos + rewind);
}
int simplest_h264_parser(const char *url,void(*out_nalu)(char * buffer,int size, NaluType type))
//int simplest_h264_parser(const char *url)
{
	NALU_t *n;
	int buffersize = 100000;

	//FILE *myout=fopen("output_log.txt","wb+");
	//C语言中的 stdout 是一个定义在<stdio.h>的宏（macro），它展开到一个 FILE* （“指向 FILE 的指针”）类型的表达式（不一定是常量），这个表达式指向一个与标准输出流（standard output stream）相关连的 FILE 对象。
	FILE *myout = stdout;


	h264bitstream = fopen(url, "rb+");
	if (h264bitstream == NULL) {
		printf("Open file error\n");
		return -1;
	}

	n = (NALU_t *)calloc(1, sizeof(NALU_t));
	if (n == NULL) {
		printf("Alloc NALU Error\n");
		return -1;
	}

	n->max_size = buffersize;
	n->buf = (char *)calloc(buffersize, sizeof(char));
	if (n->buf == NULL) {
		free(n);
		printf("AllocNALU:n->buf");
		return -1;
	}

	int data_offset = 0;
	int nal_num = 0;
	printf("-----+-------- NALU Table ------+---------+\n");
	printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
	printf("-----+---------+--------+-------+---------+\n");

	while (!feof(h264bitstream)) {
		int data_lenth;
		data_lenth = GetAnnexbNALU(n);

		char type_str[20] = { 0 };
		switch (n->nal_unit_type) {
		case NALU_TYPE_SLICE:       sprintf(type_str, "SLICE");      break;
		case NALU_TYPE_DPA:         sprintf(type_str, "DPA");        break;
		case NALU_TYPE_DPB:         sprintf(type_str, "DPB");        break;
		case NALU_TYPE_DPC:         sprintf(type_str, "DPC");        break;
		case NALU_TYPE_IDR:         sprintf(type_str, "IDR");	     break;
		case NALU_TYPE_SEI:         sprintf(type_str, "SEI");        break;
		case NALU_TYPE_SPS:         sprintf(type_str, "SPS");        break;
		case NALU_TYPE_PPS:         sprintf(type_str, "PPS");		 break;
		case NALU_TYPE_AUD:         sprintf(type_str, "AUD");        break;
		case NALU_TYPE_EOSEQ:       sprintf(type_str, "EOSEQ");      break;
		case NALU_TYPE_EOSTREAM:    sprintf(type_str, "EOSTREAM");   break;
		case NALU_TYPE_FILL:        sprintf(type_str, "FILL");       break;
		}

		char idc_str[20] = { 0 };
		switch (n->nal_reference_idc >> 5) {
		case NALU_PRIPORITY_DISPOSABLE: sprintf(idc_str, "DISPOS");     break;
		case NALU_PRIORITY_LOW:         sprintf(idc_str, "LOW");        break;
		case NALU_PRIORITY_HIGH:        sprintf(idc_str, "HIGH");       break;
		case NALU_PRIORITY_HIGHTEST:    sprintf(idc_str, "HIGHTEST");   break;
		}

		fprintf(myout, "%5d| %8d| %7s| %6s| %8d|\n", nal_num, data_offset, idc_str, type_str, n->len);

		if (out_nalu != NULL && n->nal_unit_type != NALU_TYPE_SEI) {
			out_nalu(n->buf,data_lenth,n->nal_unit_type);
		}

		data_offset = data_offset + data_lenth;
		nal_num++;
	}

	//Free
	if (n) {
		if (n->buf) {
			free(n->buf);
			n->buf = NULL;
		}
		free(n);
	}
	return 0;
}





#pragma mark - fread(void * __restrict __ptr, size_t __size, size_t __nitems, FILE * __restrict __stream);
/**
 size_t     fread(void * __restrict __ptr, size_t __size, size_t __nitems, FILE * __restrict __stream);
 从给定流 stream 读取数据到 ptr 所指向的数组中

 @param __ptr 这是指向带有最小尺寸 size*nmemb 字节的内存块的指针。
 @param __size 这是要读取的每个元素的大小，以字节为单位。
 @param __nitems 这是元素的个数，每个元素的大小为 size 字节。
 @param __stream 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输入流
 @return 成功读取的元素总数会以 size_t 对象返回，size_t 对象是一个整型数据类型。如果总数与 nmemb 参数不同，则可能发生了一个错误或者到达了文件末尾。
 */


#pragma mark - feof(FILE *);
 /**
  测试给定流 stream 的文件结束标识符。

  @param FILE 这是指向 FILE 对象的指针，该 FILE 对象标识了流。
  @return 当设置了与流关联的文件结束标识符时，该函数返回一个非零值，否则返回零。
  */

#pragma mark - void *memcpy(void *dst, const void *src, size_t n)
  /**
   void *memcpy(void *dst, const void *src, size_t n)
   从存储区 str2 复制 n 个字符到存储区 str1。

   @param dst 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针
   @param src 指向要复制的数据源，类型强制转换为 void* 指针
   @param n 要被复制的字节数
   @return 该函数返回一个指向目标存储区 str1 的指针。
   */

#pragma mark - int     fgetc(FILE *);
   /**
	int     fgetc(FILE *);
	从指定的流 stream 获取下一个字符（一个无符号字符），并把位置标识符往前移动。

	@param FILE 这是指向 FILE 对象的指针，该 FILE 对象标识了要在上面执行操作的流
	@return 该函数以无符号 char 强制转换为 int 的形式返回读取的字符，如果到达文件末尾或发生读错误，则返回 EOF。
	*/

#pragma mark - int     fseek(FILE *, long, int);
	/**
	 int     fseek(FILE *, long, int);
	 设置流 stream 的文件位置为给定的偏移 offset，参数 offset 意味着从给定的 whence 位置查找的字节数。

	 @param FILE 这是指向 FILE 对象的指针，该 FILE 对象标识了流。
	 @param long 这是相对 whence 的偏移量，以字节为单位
	 @param int 这是表示开始添加偏移 offset 的位置。它一般指定为下列常量之一：
	 SEEK_SET    文件的开头
	 SEEK_CUR    文件指针的当前位置
	 SEEK_END    文件的末尾
	 @return 如果成功，则该函数返回零，否则返回非零值。
	 */

#pragma mark - FILE    *fopen(const char * __restrict __filename, const char * __restrict __mode) __DARWIN_ALIAS_STARTING(__MAC_10_6, __IPHONE_2_0, __DARWIN_ALIAS(fopen));
	 /**
	  *fopen(const char * __restrict __filename, const char * __restrict __mode)
	  使用给定的模式 mode 打开 filename 所指向的文件。

	  @param __filename 这是 C 字符串，包含了要打开的文件名称
	  @param __mode 这是 C 字符串，包含了文件访问模式，模式如下
				  r      以只读方式打开文件，该文件必须存在。
				  r+     以可读写方式打开文件，该文件必须存在。
				  rb+    读写打开一个二进制文件，允许读数据。
				  rw+    读写打开一个文本文件，允许读和写。
				  w      打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
				  w+     打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
				  a      以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留。（EOF符保留）
				  a+     以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。 （原来的EOF符不保留）
				  wb     只写打开或新建一个二进制文件；只允许写数据。
				  wb+    读写打开或建立一个二进制文件，允许读和写。
				  ab+    读写打开一个二进制文件，允许读或在文件末追加数据。
				  at+    打开一个叫string的文件，a表示append,就是说写入处理的时候是接着原来文件已有内容写入，不是从头写入覆盖掉，t表示打开文件的类型是文本文件，+号表示对文件既可以读也可以写。
				  上述的形态字符串都可以再加一个b字符，如rb、w+b或ab+等组合，加入b 字符用来告诉函数库以二进制模式打开文件。如果不加b，表示默认加了t，即rt,wt,其中t表示以文本模式打开文件。由fopen()所建立的新文件会具有S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH(0666)权限，此文件权限也会参考umask 值。
				  有些C编译系统可能不完全提供所有这些功能，有的C版本不用"r+","w+","a+",而用"rw","wr","ar"等，读者注意所用系统的规定。
	  @return 该函数返回一个 FILE 指针。否则返回 NULL，且设置全局变量 errno 来标识错误。
	  */






#pragma mark - void    *calloc(size_t __count, size_t __size) __result_use_check __alloc_size(1,2);
	  /**
	   void    *calloc(size_t __count, size_t __size) __result_use_check __alloc_size(1,2);
	   分配所需的内存空间，并返回一个指向它的指针。malloc 和 calloc 之间的不同点是，malloc 不会设置内存为零，而 calloc 会设置分配的内存为零。

	   @param __count 要被分配的元素个数。
	   @param __size 元素的大小
	   @return 该函数返回一个指针，指向已分配的内存。如果请求失败，则返回 NULL。
	   */
