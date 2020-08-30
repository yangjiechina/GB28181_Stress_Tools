#pragma once
#include "gb28181_header_maker.h"
#include <memory.h>
#include "bits.h"


/***
 *@remark:   ps头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *           s64Src [in] 时间戳
 *@return:   0 success, others failed
*/
int gb28181_make_ps_header(char *pData, int64_t s64Scr)
{
	int64_t lScrExt = 0; //(s64Scr) % 100;
//    s64Scr = s64Scr * 3600; // / 100; // 90000/fps
	// 这里除以100是由于sdp协议返回的video的频率是90000，帧率是25帧/s，所以每次递增的量是3600,
	// 所以实际你应该根据你自己编码里的时间戳来处理以保证时间戳的增量为3600即可，
	//如果这里不对的话，就可能导致卡顿现象了
	bits_buffer_t  	bitsBuffer;
	bitsBuffer.i_size = PS_HDR_LEN;
	bitsBuffer.i_data = 0;
	bitsBuffer.i_mask = 0x80; // 二进g制：10000000 这里是为了后面对一个字节的每一位进行操作，避免大小端夸字节字序错乱
	bitsBuffer.p_data = (unsigned char *)(pData);
	memset(bitsBuffer.p_data, 0, PS_HDR_LEN);
	bits_write(&bitsBuffer, 32, 0x000001BA);			/*start codes*/
	bits_write(&bitsBuffer, 2, 1);						/*marker bits '01b'*/
	bits_write(&bitsBuffer, 3, (s64Scr >> 30) & 0x07);     /*System clock [32..30] system_clock_reference_base*/
	bits_write(&bitsBuffer, 1, 1);						/*marker bit*/
	bits_write(&bitsBuffer, 15, (s64Scr >> 15) & 0x7FFF);   /*System clock [29..15] system_clock_reference_base*/
	bits_write(&bitsBuffer, 1, 1);						/*marker bit*/
	bits_write(&bitsBuffer, 15, s64Scr & 0x7fff);         /*System clock [29..15] system_clock_reference_base*/
	bits_write(&bitsBuffer, 1, 1);						/*marker bit*/
	bits_write(&bitsBuffer, 9, lScrExt & 0x01ff);		/*System clock [14..0] system_clock_reference_extension*/
	bits_write(&bitsBuffer, 1, 1);						/*marker bit*/
	bits_write(&bitsBuffer, 22, (3750) & 0x3fffff);		/*bit rate(n units of 50 bytes per second.) program_mux_rate*/
	bits_write(&bitsBuffer, 2, 3);						/*marker bits '11'*/
	bits_write(&bitsBuffer, 5, 0x1f);					/*reserved(reserved for future use)*/
	bits_write(&bitsBuffer, 3, 0);						/*stuffing length*/
	return 0;
}


/***
 *@remark:   sys头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *@return:   0 success, others failed
*/
int gb28181_make_sys_header(char *pData, int audioCnt)
{

	bits_buffer_t  	bitsBuffer;
	bitsBuffer.i_size = SYS_HDR_LEN;
	bitsBuffer.i_data = 0;
	bitsBuffer.i_mask = 0x80;
	bitsBuffer.p_data = (unsigned char *)(pData);
	memset(bitsBuffer.p_data, 0, SYS_HDR_LEN);
	/*system header*/
	bits_write(&bitsBuffer, 32, 0x000001BB);	/*start code*/
	bits_write(&bitsBuffer, 16, SYS_HDR_LEN - 6);/*header_length 表示次字节后面的长度，后面的相关头也是次意思*/
	bits_write(&bitsBuffer, 1, 1);            /*marker_bit*/
	bits_write(&bitsBuffer, 22, 3967);		/*rate_bound*/
	bits_write(&bitsBuffer, 1, 1);            /*marker_bit*/
	bits_write(&bitsBuffer, 6, audioCnt);            /*audio_bound*/
	bits_write(&bitsBuffer, 1, 0);            /*fixed_flag */
	bits_write(&bitsBuffer, 1, 1);        	/*CSPS_flag */
	bits_write(&bitsBuffer, 1, 1);        	/*system_audio_lock_flag*/
	bits_write(&bitsBuffer, 1, 1);        	/*system_video_lock_flag*/
	bits_write(&bitsBuffer, 1, 1);        	/*marker_bit*/
	bits_write(&bitsBuffer, 5, 1);        	/*video_bound*/
	bits_write(&bitsBuffer, 1, 0);        	/*dif from mpeg1*/
	bits_write(&bitsBuffer, 7, 0x7F);     	/*reserver*/
	/*video stream bound*/
	bits_write(&bitsBuffer, 8, 0xE0);         /*stream_id*/
	bits_write(&bitsBuffer, 2, 3);        	/*marker_bit */
	bits_write(&bitsBuffer, 1, 1);        	/*PSTD_buffer_bound_scale*/
	bits_write(&bitsBuffer, 13, 2048);     	/*PSTD_buffer_size_bound*/
	/*audio stream bound*/
	bits_write(&bitsBuffer, 8, 0xC0);         /*stream_id*/
	bits_write(&bitsBuffer, 2, 3);        	/*marker_bit */
	bits_write(&bitsBuffer, 1, 0);            /*PSTD_buffer_bound_scale*/
	bits_write(&bitsBuffer, 13, 512);          /*PSTD_buffer_size_bound*/
	return 0;
}


/***
 *@remark:   psm头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *@return:   0 success, others failed
*/
int gb28181_make_psm_header(char *pData)
{

	bits_buffer_t  	bitsBuffer;
	bitsBuffer.i_size = PSM_HDR_LEN;
	bitsBuffer.i_data = 0;
	bitsBuffer.i_mask = 0x80;
	bitsBuffer.p_data = (unsigned char *)(pData);
	memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);
	bits_write(&bitsBuffer, 24, 0x000001);	/*start code*/
	bits_write(&bitsBuffer, 8, 0xBC);		/*map stream id*/
	bits_write(&bitsBuffer, 16, 18);			/*program stream map length*/
	bits_write(&bitsBuffer, 1, 1);			/*current next indicator */
	bits_write(&bitsBuffer, 2, 3);			/*reserved*/
//    bits_write(&bitsBuffer, 5, 0); 			/*program stream map version*/
	bits_write(&bitsBuffer, 5, 1); 			/*program stream map version*/
	bits_write(&bitsBuffer, 7, 0x7F);		/*reserved */
	bits_write(&bitsBuffer, 1, 1);			/*marker bit */
	bits_write(&bitsBuffer, 16, 0); 			/*programe stream info length*/
	bits_write(&bitsBuffer, 16, 8); 		/*elementary stream map length	is*/
	/*video*/
	bits_write(&bitsBuffer, 8, 0x1B);       /*stream_type*/
	bits_write(&bitsBuffer, 8, 0xE0);		/*elementary_stream_id*/
	bits_write(&bitsBuffer, 16, 0); 		/*elementary_stream_info_length */
	/*audio*/
	bits_write(&bitsBuffer, 8, 0x90);       /*stream_type*/
	bits_write(&bitsBuffer, 8, 0xC0);		/*elementary_stream_id*/
	bits_write(&bitsBuffer, 16, 0); 		/*elementary_stream_info_length is*/
	/*crc (2e b9 0f 3d)*/
	bits_write(&bitsBuffer, 8, 0x45);		/*crc (24~31) bits*/
	bits_write(&bitsBuffer, 8, 0xBD);		/*crc (16~23) bits*/
	bits_write(&bitsBuffer, 8, 0xDC);		/*crc (8~15) bits*/
	bits_write(&bitsBuffer, 8, 0xF4);		/*crc (0~7) bits*/
	return 0;
}

/***
 *@remark:   pes头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData      [in] 填充ps头数据的地址
 *           stream_id  [in] 码流类型
 *           paylaod_len[in] 负载长度
 *           pts        [in] 时间戳
 *           dts        [in]
 *@return:   0 success, others failed
*/
int gb28181_make_pes_header(char *pData, int stream_id, int payload_len, int64_t pts, int64_t dts)
{

	bits_buffer_t  	bitsBuffer;
	bitsBuffer.i_size = PES_HDR_LEN;
	bitsBuffer.i_data = 0;
	bitsBuffer.i_mask = 0x80;
	bitsBuffer.p_data = (unsigned char *)(pData);
	memset(bitsBuffer.p_data, 0, PES_HDR_LEN);

	bits_write(&bitsBuffer, 24, 0x000001);	//*start code*//*
	bits_write(&bitsBuffer, 8, (stream_id));	//*streamID*//*
	bits_write(&bitsBuffer, 16, (payload_len)+13);	//*packet_len*//* //指出pes分组中数据长度和该字节后的长度和
	bits_write(&bitsBuffer, 2, 2);		//*'10'*//*
	bits_write(&bitsBuffer, 2, 0);		//*scrambling_control*//*
	bits_write(&bitsBuffer, 1, 0);		//*priority*//*
	bits_write(&bitsBuffer, 1, 0);		//*data_alignment_indicator*//*
	bits_write(&bitsBuffer, 1, 0);		//*copyright*//*
	bits_write(&bitsBuffer, 1, 0);		//*original_or_copy*//*
	bits_write(&bitsBuffer, 1, 1);		//*PTS_flag*//*
	bits_write(&bitsBuffer, 1, 1);		//*DTS_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*ESCR_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*ES_rate_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*DSM_trick_mode_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*additional_copy_info_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*PES_CRC_flag*//*
	bits_write(&bitsBuffer, 1, 0);		//*PES_extension_flag*//*
	bits_write(&bitsBuffer, 8, 10);		//*header_data_length*//*
	// 指出包含在 PES 分组标题中的可选字段和任何填充字节所占用的总字节数。该字段之前
	//的字节指出了有无可选字段。

	//*PTS,DTS  PTS DTS均为1的情况*//
	bits_write(&bitsBuffer, 4, 3);                    //*'0011'*//*
	bits_write(&bitsBuffer, 3, ((pts) >> 30) & 0x07);     //*PTS[32..30]*//*
	bits_write(&bitsBuffer, 1, 1);
	bits_write(&bitsBuffer, 15, ((pts) >> 15) & 0x7FFF);    //*PTS[29..15]*//*
	bits_write(&bitsBuffer, 1, 1);
	bits_write(&bitsBuffer, 15, (pts) & 0x7FFF);          //*PTS[14..0]*//*
	bits_write(&bitsBuffer, 1, 1);
	bits_write(&bitsBuffer, 4, 1);                    //*'0001'*//*
	bits_write(&bitsBuffer, 3, ((dts) >> 30) & 0x07);     //*DTS[32..30]*//*
	bits_write(&bitsBuffer, 1, 1);
	bits_write(&bitsBuffer, 15, ((dts) >> 15) & 0x7FFF);    //*DTS[29..15]*//*
	bits_write(&bitsBuffer, 1, 1);
	bits_write(&bitsBuffer, 15, (dts) & 0x7FFF);          //*DTS[14..0]*//*
	bits_write(&bitsBuffer, 1, 1);
	return 0;
}


/**
 * RTP头封装
 * @param pData buffer地址
 * @param seqNum 序号
 * @param timestamp 时间戳
 * @param ssrc 标识
 * @return
 */
int gb28181_make_rtp_header(char *pData, int seqNum, int64_t timestamp, int ssrc, int isEnd)
{

	bits_buffer_t  	bitsBuffer;
	bitsBuffer.i_size = RTP_HDR_LEN;
	bitsBuffer.i_data = 0;
	bitsBuffer.i_mask = 0x80;
	bitsBuffer.p_data = (unsigned char *)(pData);
	memset(bitsBuffer.p_data, 0, RTP_HDR_LEN);
	bits_write(&bitsBuffer, 2, 2);	    /*协议版本*/
	bits_write(&bitsBuffer, 1, 0);		/*P*/
	bits_write(&bitsBuffer, 1, 0);		/*X*/
	bits_write(&bitsBuffer, 4, 0);		/*CSRC个数*/
	bits_write(&bitsBuffer, 1, isEnd);			/*一帧是否结束*/
	bits_write(&bitsBuffer, 7, 96); 		/*载荷的数据类型*/
	bits_write(&bitsBuffer, 16, seqNum); 			/*序列号，第几个*/
	bits_write(&bitsBuffer, 32, timestamp);		/*时间戳，第一个 */
	bits_write(&bitsBuffer, 32, ssrc);			/*同步信源(SSRC)标识符*/
	return 0;
}