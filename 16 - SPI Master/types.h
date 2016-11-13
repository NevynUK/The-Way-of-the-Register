/*Signed Types*/
typedef   signed char     int8;
#define int8_MAX (127)
#define int8_MIN (-128)

typedef   signed short    int16;
#define int16_MAX (32767)
#define int16_MIN (-32768)

typedef   signed long     int32;
#define int32_MAX (2147483647)
#define int32_MIN (-2147483648uL)


/*Unsigned Types*/
typedef unsigned char     uint8;
#define U8_MAX     (255)

typedef unsigned short    uint16;
#define U16_MAX    (65535u)

typedef unsigned long     uint32;
#define U32_MAX    (4294967295uL)

//typedef enum {FALSE = 0, TRUE = !FALSE} bool;