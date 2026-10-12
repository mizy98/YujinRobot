#include "stdint.h"

typedef enum {
    DIR0_ADDR_OTP,
    DIR1_ADDR_OTP,
    DEV_CONF,
    ACTIVE_CELL,
    OTP_SPARE15,
    BBVC_POSN1,
    BBVC_POSN2,
    ADC_CONF1,
    ADC_CONF2,
    OV_THRESH,
    UV_THRESH,
    OTUT_THRESH,
    UV_DIABLE1,
    UV_DIABLE2,
    GPIO_CONF1,
    GPIO_CONF2,
    GPIO_CONF3,
    GPIO_CONF4,
    OTP_SPARE14,
    OTP_SPARE13,
    OTP_SPARE12,
    OTP_SPARE11,
    FAULT_MSK1,
    FAULT_MSK2,
    PWR_TRANSIT_CONF,
    COMM_TIMEOUT_CONF,
    TX_HOLD_OFF,
    MAIN_ADC_CAL1,
    MAIN_ADC_CAL2,
    AUX_ADC_CAL1,
    AUX_ADC_CAL2,
    OTP_RSVD1F,
    OTP_RSVD20,

    OTP_PROG_ULOCK1A = 0x300,
    OTP_PROG_ULOCK2B,
    OTP_PROG_ULOCK2C,
    OTP_PROG_ULOCK2D,
    DIR0_ADDR = 0x306,
    DIR1_ADDR,
    COMM_CTRL,
    CONTROL1,
    CONTROL2,
    OTP_PROG_CTRL,
    ADC_CTRL1 = 0x30D,
    ADC_CTRL2,
    ADC_CTRL3,
    REG_INT_RSVD,
    CB_CELL16_CTRL = 0x318,
    CB_CELL15_CTRL,
    CB_CELL14_CTRL,
    CB_CELL13_CTRL,
    CB_CELL12_CTRL,
    CB_CELL11_CTRL,
    CB_CELL10_CTRL,
    CB_CELL9_CTRL,
    CB_CELL8_CTRL,
    CB_CELL7_CTRL,
    CB_CELL6_CTRL,
    CB_CELL5_CTRL,
    CB_CELL4_CTRL,
    CB_CELL3_CTRL,
    CB_CELL2_CTRL,
    CB_CELL1_CTRL,
    VMB_DONE_THRESH,
    MB_TIMER_CTRL,
    VCB_DONE_THRESH,
    OTCB_THRESH,
    OVUV_CTRL,
    OTUT_CTRL,
    BAL_CTRL1,
    BAL_CTRL2,
    BAL_CTRL3,
    FAULT_RST1,
    FAULT_RST2,
    DIAG_COMM_CTRL,
    DIAG_PWR_CTRL,
    DIAG_CBFET_CTRL1,
    DIAG_CBFET_CTRL2,
    DIAG_CBFET_CTRL3,
    DIAG_CBFET_CTRL4,
    DIAG_PROT_CTRL,
    OTP_ECC_DATAIN1 = 0x343,
    OTP_ECC_DATAIN2,
    OTP_ECC_DATAIN3,
    OTP_ECC_DATAIN4,
    OTP_ECC_DATAIN5,
    OTP_ECC_DATAIN6,
    OTP_ECC_DATAIN7,
    OTP_ECC_DATAIN8,
    OTP_ECC_DATAIN9,
    OTP_ECC_TEST,
    SPI_CONF,
    SPI_TX3,
    SPI_TX2,
    SPI_TX1,
    SPI_EXE,
    OTP_PROG_UNLOCK2A,
    OTP_PROG_UNLOCK2B,
    OTP_PROG_UNLOCK2C,
    OTP_PROG_UNLOCK2D,
    DEBUG_CTRL_UNLOCK = 0x700,
    DEBUG_COMM_CTRL1,
    DEBUG_COMM_CTRL2,

    // Read-Only
    PARTID = 0x500,
    DEV_REVID = 0xE00,
    DIE_ID1 = 0x501,
    DIE_ID2,
    DIE_ID3,
    DIE_ID4,
    DIE_ID5,
    DIE_ID6,
    DIE_ID7,
    DIE_ID8,
    DIE_ID9,
    CUST_CRC_RSLT_HI,
    CUST_CRC_RSLT_LO,
    OTP_ECC_DATA_OUT1 = 0x510,
    OTP_ECC_DATA_OUT2,
    OTP_ECC_DATA_OUT3,
    OTP_ECC_DATA_OUT4,
    OTP_ECC_DATA_OUT5,
    OTP_ECC_DATA_OUT6,
    OTP_ECC_DATA_OUT7,
    OTP_ECC_DATA_OUT8,
    OTP_ECC_DATA_OUT9,
    OTP_PROG_STAT,
    OTP_CUST1_STAT,
    OTP_CUST2_STAT,
    SPI_RX3 = 0x520,
    SPI_RX2,
    SPI_RX1,
    DIAG_STAT = 0x526,
    ADC_STAT1,
    ADC_STAT2,
    GPIO_STAT = 0x52A,
    BAL_STAT,
    DEV_STAT,
    FAULT_SUMMARY,
    FAULT_COMM1 = 0x530,
    FAULT_COMM2,
    FAULT_COMM3,
    FAULT_OTP,
    FAULT_SYS,
    FAULT_PROT1 = 0x53A,
    FAULT_PROT2,
    FAULT_OV1,
    FAULT_OV2,
    FAULT_UV1,
    FAULT_UV2,
    FAULT_OT,
    FAULT_UT,
    FAULT_COMP_GPIO = 0x543,
    FAULT_COMP_VCCB1 = 0x545,
    FAULT_COMP_VCCB2,
    FAULT_COMP_VCOW1 = 0x548,
    FAULT_COMP_VCOW2,
    FAULT_COMP_VBOW1 = 0x54B,
    FAULT_COMP_VBOW2,
    FAULT_COMP_CBFET1,
    FAULT_COMP_CBFET2,
    FAULT_COMP_MISC,
    FAULT_PWR1 = 0x552,
    FAULT_PWR2,
    FAULT_PWR3,
    CB_COMPLETE1 = 0x556,
    CB_COMPLETE2,
    BAL_TIME,
    VCELL16_HI = 0x568,
    VCELL16_LO,
    VCELL15_HI,
    VCELL15_LO,
    VCELL14_HI,
    VCELL14_LO,
    VCELL13_HI,
    VCELL13_LO,
    VCELL12_HI,
    VCELL12_LO,
    VCELL11_HI,
    VCELL11_LO,
    VCELL10_HI,
    VCELL10_LO,
    VCELL9_HI,
    VCELL9_LO,
    VCELL8_HI,
    VCELL8_LO,
    VCELL7_HI,
    VCELL7_LO,
    VCELL6_HI,
    VCELL6_LO,
    VCELL5_HI,
    VCELL5_LO,
    VCELL4_HI,
    VCELL4_LO,
    VCELL3_HI,
    VCELL3_LO,
    VCELL2_HI,
    VCELL2_LO,
    VCELL1_HI,
    VCELL1_LO,
    BUSBAR_HI = 0x588,
    BUSBAR_LO,
    TSREF_HI = 0x58C,
    TSREF_LO,
    GPIO1_HI = 0x58E,
    GPIO1_LO,
    GPIO2_HI,
    GPIO2_LO,
    GPIO3_HI,
    GPIO3_LO,
    GPIO4_HI,
    GPIO4_LO,
    GPIO5_HI,
    GPIO5_LO,
    GPIO6_HI,
    GPIO6_LO,
    GPIO7_HI,
    GPIO7_LO,
    GPIO8_HI,
    GPIO8_LO,
    DIETEMP1_HI = 0x5AE,
    DIETEMP1_LO,
    DIETEMP2_HI,
    DIETEMP2_LO,
    AUX_CELL_HI,
    AUX_CELL_LO,
    AUX_GPIO_HI,
    AUX_GPIO_LO,
    AUX_BAT_HI,
    AUX_BAT_LO,
    AUX_REFL_HI,
    AUX_REFL_LO,
    AUX_VBG2_HI,
    AUX_VBG2_LO,

    INVALID_REG = 0xFFFF
} Drvbq79616Register;

#pragma pack(push, 1)

typedef struct {
    uint8_t hb_en : 1;
    uint8_t ftone_en : 1;
    uint8_t nfault_en : 1;
    uint8_t two_stop_en : 1;
    uint8_t fcomm_en : 1;
    uint8_t multidrop_en : 1;
    uint8_t no_adj_cb : 1;
    uint8_t rsvd : 1;
} DevConf;

typedef struct {
    uint8_t num_cell : 4;
    uint8_t rsvd : 4;
} ActiveCell;

typedef struct {
    uint8_t ov_thr : 6;
    uint8_t rsvd : 2;
} OvThresh;

typedef struct {
    uint8_t uv_thr : 6;
    uint8_t rsvd : 2;
} UvThresh;

typedef struct {
    uint8_t cell[8];
} UvDisable;

typedef struct {
    uint8_t ovuv_mode : 2;
    uint8_t ovuv_go : 1;
    uint8_t ovuv_lock : 4;
    uint8_t vcbdone_thr_lock : 1;
} OvUvCtrl;

typedef struct {
    uint8_t ot_thresh : 5;
    uint8_t ut_thresh : 3;
} OtUtThresh;

typedef struct {
    uint8_t otut_mode : 2;
    uint8_t otut_go : 1;
    uint8_t otut_lock : 4;
    uint8_t otcbdone_thr_lock : 1;
} OtUtCtrl;

typedef struct {
    uint8_t addr_wr : 1;
    uint8_t soft_reset : 1;
    uint8_t goto_sleep : 1;
    uint8_t goto_shutdown : 1;
    uint8_t send_slptoact : 1;
    uint8_t send_wake : 1;
    uint8_t send_shutdown : 1;
    uint8_t dir_sel : 1;
} Control1;

typedef struct {
    uint8_t address : 6;
    uint8_t rsvd : 2;
} DirAddr;

typedef struct {
    uint8_t top_stack : 1;
    uint8_t stack_dev : 1;
    uint8_t rsvd : 6;
} CommCtrl;

typedef struct {
    uint8_t lpf_vcell : 3;
    uint8_t lpf_bb : 3;
    uint8_t aux_settle : 2;
} AdcConf1;

typedef struct {
    uint8_t main_mode : 2;
    uint8_t main_go : 1;
    uint8_t lpf_vcell_en : 1;
    uint8_t lpf_bb_en : 1;
    uint8_t rsvd : 3;
} AdcCtrl1;

typedef struct {
    uint8_t aux_cell_sel : 5;
    uint8_t aux_cell_align : 1;
    uint8_t rsvd : 2;
} AdcCtrl2;

typedef struct {
    uint8_t duty : 3;
    uint8_t rsvd : 5;
} BalCtrl1;

typedef struct {
    uint8_t auto_bal : 1;
    uint8_t bal_go : 1;
    uint8_t bal_act : 2;
    uint8_t otcb_en : 1;
    uint8_t fltstop_en : 1;
    uint8_t cb_pause : 1;
    uint8_t rsvd : 1;
} BalCtrl2;

typedef struct {
    uint8_t bal_time_go : 1;
    uint8_t bal_time_sel : 4;
    uint8_t reserved : 3;
} BalCtrl3;

typedef struct {
    uint8_t time : 7;
    uint8_t time_unit : 1;
} BalTime;

typedef struct {
    uint8_t cell1_done : 1;
    uint8_t cell2_done : 1;
    uint8_t cell3_done : 1;
    uint8_t cell4_done : 1;
    uint8_t cell5_done : 1;
    uint8_t cell6_done : 1;
    uint8_t cell7_done : 1;
    uint8_t cell8_done : 1;
} CbComplete;

typedef struct {
    uint8_t cb_done : 1;
    uint8_t mb_done : 1;
    uint8_t abortflt : 1;
    uint8_t cb_run : 1;
    uint8_t mb_run : 1;
    uint8_t cb_inpause : 1;
    uint8_t ot_pause_det : 1;
    uint8_t invalid_cbconf : 1;
} BalStat;

typedef struct {
    uint8_t main_run : 1;
    uint8_t aux_run : 1;
    uint8_t : 1;
    uint8_t ovuv_run : 1;
    uint8_t otut_run : 1;
    uint8_t cust_crc_done : 1;
    uint8_t fact_crc_done : 1;
    uint8_t : 1;
} DevStat;

typedef struct {
    uint8_t time : 5;
    uint8_t rsvd : 3;
} CbCellCtrl;

typedef struct {
    uint8_t cb_thr : 6;
    uint8_t rsvd : 2;
} VcbDoneThresh;

typedef struct {
    uint8_t gpio_0 : 3;
    uint8_t gpio_1 : 3;
    uint8_t spi_en : 1;       // only available for GPIO_CONF1
    uint8_t fault_in_en : 1;  // only available for GPIO_CONF1
} GpioConf;

#pragma pack(pop)
