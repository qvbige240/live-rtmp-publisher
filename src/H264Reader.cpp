#include <iostream>
#include "H264Reader.h"

#include <stdio.h>
#include <sys/stat.h>


static inline size_t find_start_code(const char *buf)
{
	if (buf[0] != 0x00 || buf[1] != 0x00 ) {
		return 0;
	}

#if (defined H264_AVCC) && (!defined SUBNALU_TO_AVCC_FORMAT)
	if (buf[2] == 0x01) {
		return 3;
	} else if (buf[2] == 0x00 && buf[3] == 0x01) {
		return 4;
	}
#else
	if (buf[2] == 0x00 && buf[3] == 0x01) {
		return 4;
	}
#endif

	return 0;
}

H264Reader::H264Reader(const char* file): mData(NULL) {
	
	//size_t offset = 0, size = 1 << 20;
	int file_size = 0;
	FILE* fp = 0;

	fp = fopen(file, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		//size = (offset + size) <= file_size ? size : file_size;

		mData = calloc(1, file_size + 1);
		if (mData == NULL)
			std::cout << "malloc failed" << std::endl;

		mLength = fread(mData, 1, file_size, fp);
		fclose(fp);

		memset(mMetadata, 0, sizeof(mMetadata));

		mPos = find_start_code((const char *)mData);
		if (mPos == 0)
			std::cout << "read metadata error!" << std::endl;
	}
	else
	{
		std::cout << "open file" << std::endl;
	}
}

H264Reader::~H264Reader() {

}

typedef struct h264_nalu {
	size_t	nalu_len;
	char	nalu_type;
	char*	nalu_data;
} h264_nalu_t;

static int read_nalu(const char *buffer, size_t size, size_t offset, h264_nalu_t *nalu)
{
	size_t start = 0;
	if (offset < size) {
		size_t pos = 0;
		while (offset + pos + 3 < size) {
			start = find_start_code(buffer + offset + pos);
			if (start)
				break;

			pos++;

			//if(buffer[offset + pos] == 0x00 &&
			//	buffer[offset + pos + 1] == 0x00 &&
			//	(buffer[offset + pos + 2] == 0x01 ||
			//	buffer[offset + pos + 2] == 0x00 && buffer[offset + pos + 3] == 0x01)) {
			//		break;
			//}

 			//if(buffer[offset + pos++] == 0x00 &&
 			//	buffer[offset + pos++] == 0x00 &&
 			//	(buffer[offset + pos] == 0x01 || 
				//(buffer[offset + pos++] == 0x00 && buffer[offset + pos++] == 0x01))) {
 			//		break;
 			//}
		}

		if(offset + pos + start == size){
			nalu->nalu_len = pos + start;
		} else {
			nalu->nalu_len = pos;
		}

		nalu->nalu_data = (char*)(buffer + offset);
		//pNalu->forbidden_bit = pNalu->buf[0] & 0x80;
		//pNalu->nal_reference_idc = pNalu->buf[0] & 0x60; // 2 bit
		nalu->nalu_type = (nalu->nalu_data[0]) & 0x1f;// 5 bit

		return (nalu->nalu_len + start);

		//start = find_start_code(buffer + offset);
		//if(start != 0) {
		//	size_t pos = start;
		//	while (offset + pos < size) {
		//		if(buffer[offset + pos++] == 0x00 &&
		//			buffer[offset + pos++] == 0x00 &&
		//			buffer[offset + pos++] == 0x00 &&
		//			buffer[offset + pos++] == 0x01) {
		//				break;
		//		}
		//	}

		//	if(offset + pos == size){
		//		nalu->nalu_len = pos - start;
		//	} else {
		//		nalu->nalu_len = (pos - 4) - start;
		//	}

		//	nalu->nalu_data = buffer + offset + start;
		//	//pNalu->forbidden_bit = pNalu->buf[0] & 0x80;
		//	//pNalu->nal_reference_idc = pNalu->buf[0] & 0x60; // 2 bit
		//	nalu->nalu_type = (nalu->nalu_data[0]) & 0x1f;// 5 bit

		//	return (nalu->nalu_len + start);
		//}
	}

	return 0;
}


#ifdef SUBNALU_TO_AVCC_FORMAT
static inline size_t find_sub_code(char *buf)
{
	if (buf[0] != 0x00 || buf[1] != 0x00) {
		return 0;
	}

	if (buf[2] == 0x01) {
		return 3;
	}

	return 0;
}

size_t multi_nalu_process(char *dst, char *src, size_t len)
{
	char *d = dst;
	size_t dst_len = 0;
	char *p = src;
	size_t pos = 0, start = 0;
	size_t offset = 0;
	while (offset < len)
	{
		while (pos + offset < len)
		{
			start = find_sub_code(p + offset + pos);
			if (start) break;
			pos++;
		}

		printf("pos: %ld\n", pos);
		d[dst_len + 2] = pos >> 8 & 0xff;
		d[dst_len + 3] = pos & 0xff;
		dst_len += 4;
		memcpy(d + dst_len, p + offset, pos);
		dst_len += pos;
		offset += (pos + start);
		pos = 0;
	}

	return dst_len;
}
#endif //SUBNALU_TO_AVCC_FORMAT

#if 0
size_t split_nalu_process(char *dst, char *src, size_t len)
{
	char *d = dst;
	size_t dst_len = 0;
	char *p = src;
	size_t offset = 0;
    size_t remain = len;

	size_t total = len;
	size_t nal_len = total / 5 + 1;
	while (remain > 0)
	{
		if (remain <= nal_len)
			nal_len = remain;
		//size_t nal_len = 1024 * 21;
		printf("nal_len: %ld\n", nal_len);
		d[dst_len+2] = nal_len >> 8 & 0xff;
		d[dst_len+3] = nal_len & 0xff;
		dst_len += 4;
		memcpy(d+dst_len, p+offset, nal_len);
		dst_len += nal_len;
		offset += nal_len;
		remain -= nal_len;
	}
	return dst_len;
}
#endif

std::pair<int, char*> H264Reader::readnal()
{
	int len = 0;
	h264_nalu_t nalu;
	
	len = read_nalu((const char*)mData, mLength, mPos, &nalu);
	if (len != 0) {
		mPos += len;
		//printf("## nalu type(%d) len(%ld)\n", nalu.nalu_type, nalu.nalu_len);
		//return std::make_pair(nalu.nalu_len, nalu.nalu_data);

#ifdef SUBNALU_TO_AVCC_FORMAT
		memset(mBuffer, 0x00, sizeof(mBuffer));
		size_t total = multi_nalu_process(mBuffer, nalu.nalu_data, nalu.nalu_len);

		printf("## nalu type(%d) len(%ld) retset-len(%ld)\n", nalu.nalu_type, nalu.nalu_len, total);
		return std::make_pair(total, mBuffer);
#else
		printf("## nalu type(%d) len(%ld)\n", nalu.nalu_type, nalu.nalu_len);
		return std::make_pair(nalu.nalu_len, nalu.nalu_data);
#endif
	}

	std::cout << "===== read readnal error or end, mPos" << mPos << std::endl;

	std::cout << "===== restart =====" << std::endl;
	mPos = find_start_code((const char*)mData);

	len = read_nalu((const char*)mData, mLength, mPos, &nalu);
	if (len != 0) {
		mPos += len;
		printf("## nalu type(%d) len(%ld)\n", nalu.nalu_type, nalu.nalu_len);
		return std::make_pair(nalu.nalu_len, nalu.nalu_data);
	}

	std::cout << "===== read readnal error, mPos" << mPos << std::endl;

	return std::make_pair(0, reinterpret_cast<char*>(NULL));
}

std::pair<int, char*> H264Reader::getMetadata() {
	int len, size;

	h264_nalu_t sps, pps;

	len = read_nalu((const char*)mData, mLength, mPos, &sps);
	if (sps.nalu_type == 0x07) {
		mMetadata[2] = (sps.nalu_len >> 8) & 0xff;
		mMetadata[3] = sps.nalu_len & 0xff;
		memcpy(mMetadata+4, sps.nalu_data, sps.nalu_len);
	}

	mPos += len;
	len = read_nalu((const char*)mData, mLength, mPos, &pps);
	if (pps.nalu_type == 0x08) {
		mMetadata[sps.nalu_len + 6] = (pps.nalu_len >> 8) & 0xff;
		mMetadata[sps.nalu_len + 7] = pps.nalu_len & 0xff;
		memcpy(mMetadata+sps.nalu_len+8, pps.nalu_data, pps.nalu_len);

		//pps.nalu_len = 0x04;
		//mMetadata[sps.nalu_len + 6] = 0x00;
		//mMetadata[sps.nalu_len + 7] = 0x04;
		//memcpy(mMetadata+sps.nalu_len+8, pps.nalu_data, 4);
	}
	mPos += len;

	size = sps.nalu_len + pps.nalu_len + 8;
	std::cout << "metadata: " << size << std::endl;

	return std::make_pair(size, mMetadata);
}
