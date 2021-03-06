
#ifndef GISKOOK_ENDIANNESS_H_H
#define GISKOOK_ENDIANNESS_H_H

static union{
	char c[4];
	unsigned long l;
}endian_test={{'l','?','?','b'}};
#define ENDIANNESS ((char)endian_test.l)
#define ISBIGENDIAN (ENDIANNESS=='b')

#if defined(__linux__)
#define swap16(x) \
	({\
	 unsigned short _x=(x);\
	 ((unsigned short)(\
		 (((unsigned short)(_x)&(unsigned short)0x00ffU)<<8)|\
		 (((unsigned short)(_x)&(unsigned short)0xff00U)>>8) ));\
	 })

#define swap32(x) \
	({\
	 unsigned int _x=(x);\
	 ((unsigned int)(\
		 (((unsigned int)(_x)&(unsigned int)0x000000ffUL)<<24)|\
		 (((unsigned int)(_x)&(unsigned int)0x0000ff00UL)<<8)|\
		 (((unsigned int)(_x)&(unsigned int)0x00ff0000UL)>>8)|\
		 (((unsigned int)(_x)&(unsigned int)0xff000000UL)>>24)));\
	 })
#elif defined(_WIN32)
#define swap16(x) \
	((unsigned short)(\
	(((unsigned short)(x)&(unsigned short)0x00ffU)<<8)|\
	(((unsigned short)(x)&(unsigned short)0xff00U)>>8)));
#define swap32(x) \
	((unsigned int)(\
	(((unsigned int)(x)&(unsigned int)0x000000ffUL)<<24)|\
	(((unsigned int)(x)&(unsigned int)0x0000ff00UL)<<8)|\
	(((unsigned int)(x)&(unsigned int)0x00ff0000UL)>>8)|\
	(((unsigned int)(x)&(unsigned int)0xff000000UL)>>24)));
#endif

#endif
