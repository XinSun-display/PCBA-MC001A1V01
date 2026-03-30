#ifndef __FT5446_H_
#define __FT5446_H_

typedef unsigned char U8;
typedef unsigned short U16;

typedef unsigned char UINT8;
typedef unsigned short UINT16;
enum 
{
	TP_Stu_NoPress = 0,
	TP_Stu_Press = 1,
	TP_Stu_DoPress = 2,
};

typedef struct 
{
    unsigned int x;
   unsigned int y;
}TP_POINT;


#define FT5XXX_CMD_WR               0X70        
#define FT5XXX_CMD_RD               0X71     

#define FT5XXX_OP_DEVIDE_MODE          0x00      
#define FT5XXX_OP_GEST_ID              0x01
#define FT5XXX_OP_TD_STATUS       	   0x02       
#define FT5XXX_OP_TP1_REG              0X03
#define FT5XXX_OP_TP2_REG              0X09  
#define FT5XXX_OP_TP3_REG              0X0F        
#define FT5XXX_OP_TP4_REG              0X15      
#define FT5XXX_OP_TP5_REG              0X1B    
#define FT5XXX_OP_ID_G_THGROUP 		   0x80	
#define FT5XXX_OP_ID_G_THPEAK           0x81
#define FT5XXX_OP_ID_G_THCAL            0x82
#define FT5XXX_OP_ID_G_THWATER          0x83
#define FT5XXX_OP_ID_G_THTEMP           0x84
#define FT5XXX_OP_ID_G_THTDIFF          0x85
#define FT5XXX_OP_ID_G_CTRL             0x86
#define FT5XXX_OP_ID_G_TIMMONITOR       0x87
#define FT5XXX_OP_ID_G_PERIODACTIVE     0x88
#define FT5XXX_OP_ID_G_PERIODMONITOR    0x89
#define FT5XXX_OP_ID_G_L_R_OFFSET       0x92
#define FT5XXX_OP_ID_G_U_D_OFFSET       0x93
#define FT5XXX_OP_ID_G_DISTANCE_LEFT_RIGHT 0x94
#define FT5XXX_OP_ID_G_DISTANCE_UP_DOWN 0x95
#define FT5XXX_OP_ID_G_RADIAN_VALUE     0x96
#define FT5XXX_OP_ID_G_ZOOM_DIS_SQR     0x97
#define FT5XXX_OP_ID_G_MAX_X_HIGH       0x98
#define FT5XXX_OP_ID_G_MAX_X_LOW        0x99
#define FT5XXX_OP_ID_G_MAX_Y_HIGH       0x9A
#define FT5XXX_OP_ID_G_MAX_Y_LOW        0x9B
#define FT5XXX_OP_ID_G_K_X_HIGH         0x9C
#define FT5XXX_OP_ID_G_K_X_LOW          0x9D
#define FT5XXX_OP_ID_G_K_Y_HIGH         0x9E
#define FT5XXX_OP_ID_G_K_Y_LOW          0x9F
#define FT5XXX_OP_ID_G_AUTOCLBMODE      0xA0
#define FT5XXX_OP_ID_G_LIBVERSIONH      0xA1
#define FT5XXX_OP_ID_G_LIBVERSIONL      0xA2
#define FT5XXX_OP_ID_G_CIPHER           0xA3
#define FT5XXX_OP_ID_G_MODE             0xA4
#define FT5XXX_OP_ID_G_PMODE            0xA5
#define FT5XXX_OP_ID_G_FIRMID           0xA6
#define FT5XXX_OP_ID_G_STATE            0xA7
#define FT5XXX_OP_ID_G_ERR              0xA9
#define FT5XXX_OP_ID_G_CLB              0xAA
#define FT5XXX_OP_ID_G_STATIC_TH        0xAB
#define FT5XXX_OP_ID_G_AUTOCLB_STATUS   0xAC	
#define FT5XXX_OP_ID_G_AUTOCLB_TIMER    0xAD
#define FT5XXX_OP_ID_G_DRAW_LINE_TH     0xAE
 
// touch weight and misc
#define FT5XXX_OP_TOUCH1_WEIGHT        0X07
#define FT5XXX_OP_TOUCH1_MISC          0X08
#define FT5XXX_OP_TOUCH2_WEIGHT        0X0D
#define FT5XXX_OP_TOUCH2_MISC          0X0E  
#define FT5XXX_OP_TOUCH3_WEIGHT        0X13
#define FT5XXX_OP_TOUCH3_MISC          0X14  
#define FT5XXX_OP_TOUCH4_WEIGHT        0X19
#define FT5XXX_OP_TOUCH4_MISC          0X1A   
#define FT5XXX_OP_TOUCH5_WEIGHT        0X1F
#define FT5XXX_OP_TOUCH5_MISC          0X20   
 
#define	FT5XXX_OP_ID_G_LIB_VERSION     0xA1      
#define FT5XXX_OP_ID_G_MODE            0xA4      
#define FT5XXX_OP_ID_G_THGROUP         0x80        
#define FT5XXX_OP_ID_G_PERIODACTIVE    0x88       


void FT5446_Init (void);
esp_err_t FT5XXX_Reg(U8 reg, U8 *value, U8 len);

void FT5446_ScanV1(lv_indev_t *indev_drv, lv_indev_data_t *data);
void FT5446_ScanV2(lv_indev_t *indev_drv, lv_indev_data_t *data);

#endif
