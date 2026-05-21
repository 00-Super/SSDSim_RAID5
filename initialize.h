/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName锟斤拷 initialize.h
Author: Hu Yang		Version: 2.1	Date:2011/12/02
Description: 

History:
<contributor>     <time>        <version>       <desc>                   <e-mail>
Yang Hu	        2009/09/25	      1.0		    Creat SSDsim       yanghu@foxmail.com
                2010/05/01        2.x           Change 
Zhiming Zhu     2011/07/01        2.0           Change               812839842@qq.com
Shuangwu Zhang  2011/11/01        2.1           Change               820876427@qq.com
Chao Ren        2011/07/01        2.0           Change               529517386@qq.com
Hao Luo         2011/01/01        2.0           Change               luohao135680@gmail.com
*****************************************************************************************************************************/
#ifndef INITIALIZE_H
#define INITIALIZE_H 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
//#include "avlTree.h"


typedef struct _HASH_NODE_
{
	struct _HASH_NODE_ *next;
}HASH_NODE;

#define STRIPENUM 4

#define SECTOR 512
#define BUFSIZE 200

#define DYNAMIC_ALLOCATION 0
#define STATIC_ALLOCATION 1

#define INTERLEAVE 0
#define TWO_PLANE 1

#define NORMAL    2
#define INTERLEAVE_TWO_PLANE 3
#define COPY_BACK	4

#define AD_RANDOM 1
#define AD_COPYBACK 2
#define AD_TWOPLANE 4
#define AD_INTERLEAVE 8
#define AD_TWOPLANE_READ 16

#define READ 1
#define WRITE 0

#define WL_THRESHOLD 1.1
#define BRT 38000

/*********************************all states of each objects************************************************
*一锟铰讹拷锟斤拷锟斤拷channel锟侥匡拷锟叫ｏ拷锟斤拷锟斤拷锟街凤拷锟斤拷洌拷锟斤拷荽锟斤拷洌拷锟斤拷洌拷锟斤拷锟斤拷锟阶刺?
*锟斤拷锟斤拷chip锟侥匡拷锟叫ｏ拷写忙锟斤拷锟斤拷忙锟斤拷锟斤拷锟斤拷锟街凤拷锟斤拷洌拷锟斤拷荽锟斤拷洌拷锟斤拷锟矫︼拷锟絚opyback忙锟斤拷锟斤拷锟斤拷锟斤拷状态
*锟斤拷锟叫讹拷写锟斤拷锟斤拷锟斤拷sub锟斤拷锟侥等达拷锟斤拷锟斤拷锟斤拷锟斤拷锟街凤拷锟斤拷洌拷锟斤拷锟斤拷锟斤拷锟斤拷荽锟斤拷洌达拷锟斤拷锟斤拷址锟斤拷锟戒，写锟斤拷锟捷达拷锟戒，写锟斤拷锟戒，锟斤拷傻锟阶刺?
************************************************************************************************************/

#define CHANNEL_IDLE 000
#define CHANNEL_C_A_TRANSFER 3
#define CHANNEL_GC 4           
#define CHANNEL_DATA_TRANSFER 7
#define CHANNEL_TRANSFER 8
#define CHANNEL_UNKNOWN 9

#define CHIP_IDLE 100
#define CHIP_WRITE_BUSY 101
#define CHIP_READ_BUSY 102
#define CHIP_C_A_TRANSFER 103
#define CHIP_DATA_TRANSFER 107
#define CHIP_WAIT 108
#define CHIP_ERASE_BUSY 109
#define CHIP_COPYBACK_BUSY 110
#define UNKNOWN 111

#define SR_WAIT 200                 
#define SR_R_C_A_TRANSFER 201
#define SR_R_READ 202
#define SR_R_DATA_TRANSFER 203
#define SR_W_C_A_TRANSFER 204
#define SR_W_DATA_TRANSFER 205
#define SR_W_TRANSFER 206
#define SR_COMPLETE 299

#define CPU_IDLE 700
#define CPU_BUSY 701

#define REQUEST_IN 300         //锟斤拷一锟斤拷锟斤拷锟襟到达拷锟绞憋拷锟?
#define OUTPUT 301             //锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞憋拷锟?
#define GC_WAIT 400
#define GC_ERASE_C_A 401
#define GC_COPY_BACK 402
#define GC_COMPLETE 403
#define GC_INTERRUPT 0
#define GC_UNINTERRUPT 1
#define GC_FAST_UNINTERRUPT 1700
#define GC_FAST_UNINTERRUPT_EMERGENCY 1701
#define GC_FAST_UNINTERRUPT_IDLE 1702
#define DR_STATE_RECIEVE 1800
#define DR_STATE_OUTPUT 1801
#define DR_STATE_NULL 1802

#define CHANNEL(lsn) (lsn&0x0000)>>16      
#define chip(lsn) (lsn&0x0000)>>16 
#define die(lsn) (lsn&0x0000)>>16 
#define PLANE(lsn) (lsn&0x0000)>>16 
#define BLOKC(lsn) (lsn&0x0000)>>16 
#define PAGE(lsn) (lsn&0x0000)>>16 
#define SUBPAGE(lsn) (lsn&0x0000)>>16  

#define PG_SUB 0xffffffff			

/*****************************************
*锟斤拷锟斤拷锟斤拷锟阶刺拷锟斤拷锟?
*Status 锟角猴拷锟斤拷锟斤拷锟酵ｏ拷锟斤拷值锟角猴拷锟斤拷锟斤拷锟阶刺拷锟斤拷锟?
******************************************/
#define TRUE		1
#define FALSE		0
#define SUCCESS		1
#define FAILURE		0
#define ERROR		-1
#define INFEASIBLE	-2
#define OVERFLOW	-3

//**************************
#define SUCCESS_LSB 1
#define SUCCESS_MSB 2
#define SUCCESS_CSB 3
#define TARGET_LSB 0
#define TARGET_CSB 1
#define TARGET_MSB 2
//**************************

typedef int Status;     

struct ac_time_characteristics{
	int tPROG;     //program time
	int tDBSY;     //bummy busy time for two-plane program
	int tBERS;     //block erase time
	int tCLS;      //CLE setup time
	int tCLH;      //CLE hold time
	int tCS;       //CE setup time
	int tCH;       //CE hold time
	int tWP;       //WE pulse width
	int tALS;      //ALE setup time
	int tALH;      //ALE hold time
	int tDS;       //data setup time
	int tDH;       //data hold time
	int tWC;       //write cycle time
	int tWH;       //WE high hold time
	int tADL;      //address to data loading time
	int tR;        //data transfer from cell to register
	int tAR;       //ALE to RE delay
	int tCLR;      //CLE to RE delay
	int tRR;       //ready to RE low
	int tRP;       //RE pulse width
	int tWB;       //WE high to busy
	int tRC;       //read cycle time
	int tREA;      //RE access time
	int tCEA;      //CE access time
	int tRHZ;      //RE high to output hi-z
	int tCHZ;      //CE high to output hi-z
	int tRHOH;     //RE high to output hold
	int tRLOH;     //RE low to output hold
	int tCOH;      //CE high to output hold
	int tREH;      //RE high to output time
	int tIR;       //output hi-z to RE low
	int tRHW;      //RE high to WE low
	int tWHR;      //WE high to RE low
	int tRST;      //device resetting time
	int tPROG_L;   //program time for LSB page
	int tPROG_C;   //program time for CSB page
	int tPROG_M;   //program time for MSB page
	int t_R_L;   //READ time for LSB page
	int t_R_L1;
	int t_R_L2;
	int t_R_L3;
	int t_R_C;   //READ time for CSB page
	int t_R_C1;
	int t_R_C2;
	int t_R_C3;
	int t_R_M;   //READ time for MSB page
	int t_R_M1;
	int t_R_M2;
	int t_R_M3;
};

struct StripeReq{//struct StripeReq *req;//数据位或者校验位对应的子请求
	unsigned int lpn;
	unsigned int lpnCount;
	unsigned int state;
	unsigned int sub_size;
	unsigned int full_page;
	struct request *req;//所属的大请求
};

struct Stripe{
	unsigned int check;
	unsigned  int checkChange;
	unsigned int all;
	unsigned int now;
	unsigned int checkLpn;
	unsigned int checkStart;
	unsigned long long allStripe;
	unsigned long long nowStripe;
	struct StripeReq *req;//数据位对应的子请求
};	

enum{
	INVAIL_DRAID = -1,
	VAIL_DRAID = 0,
	CHECK_RAID = 1,
	WILLCHANGE_RAID = 2,
};

struct RaidInfo{  // trip2page,好像只有数据位
	unsigned int* lpn;
	unsigned int changeState;
	int* check;
	unsigned char allChange;
	unsigned char nowChange;
	unsigned char using;
	unsigned char readFlag;
	int PChannel;//校验位channel
	struct local* location;//校验位location
	int *changeQueuePos;
};

struct channelInfo {
	unsigned int numPE; //channel的PE次数  PE次数是整个channel的磨损情况
	unsigned int numRead; //读的次数 读或者写的次数是一个时间窗口，一段时间内它的读多写多去决定
	unsigned int numWrite; //写的次数
};

struct ssd_info{
    unsigned long long firstTime_;
	double ssd_energy;                   //SSD锟斤拷锟杰耗ｏ拷锟斤拷时锟斤拷锟叫酒拷锟斤拷暮锟斤拷锟?,锟杰猴拷锟斤拷锟斤拷
	int64_t current_time;                //锟斤拷录系统时锟斤拷
	int64_t next_request_time;
	unsigned int real_time_subreq;       //锟斤拷录实时锟斤拷写锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟饺拷锟教拷锟斤拷锟绞憋拷锟絚hannel锟斤拷锟饺碉拷锟斤拷锟?
	int flag;
	int active_flag;                     //锟斤拷录锟斤拷锟斤拷写锟角凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟揭拷锟绞憋拷锟斤拷锟角帮拷平锟?,0锟斤拷示没锟斤拷锟斤拷锟斤拷锟斤拷1锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷前锟狡斤拷时锟斤拷
	unsigned int page;

	unsigned int token;                  //锟节讹拷态锟斤拷锟斤拷锟叫ｏ拷为锟斤拷止每锟轿凤拷锟斤拷锟节碉拷一锟斤拷channel锟斤拷要维锟斤拷一锟斤拷锟斤拷锟狡ｏ拷每锟轿达拷锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫匡拷始锟斤拷锟斤拷
	unsigned int gc_request;             //锟斤拷录锟斤拷SSD锟叫ｏ拷锟斤拷前时锟斤拷锟叫讹拷锟斤拷gc锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷

	unsigned int write_request_count;    //锟斤拷录写锟斤拷锟斤拷锟侥达拷锟斤拷
	unsigned int read_request_count;     //锟斤拷录锟斤拷锟斤拷锟斤拷锟侥达拷锟斤拷
	unsigned long long write_avg;                   //锟斤拷录锟斤拷锟节硷拷锟斤拷写锟斤拷锟斤拷平锟斤拷锟斤拷应时锟斤拷锟绞憋拷锟?
	unsigned long long read_avg;                    //锟斤拷录锟斤拷锟节硷拷锟斤拷锟斤拷锟斤拷锟狡斤拷锟斤拷锟接κ憋拷锟斤拷时锟斤拷
	//================================================
	int time_day;
	unsigned int performance_mode;
	unsigned int max_queue_depth;
	unsigned int write_request_count_l;
	unsigned int newest_write_request_count;
	unsigned int newest_read_request_count;
	int64_t newest_write_avg;
	int64_t write_avg_l;
	int64_t newest_write_avg_l;
	int64_t newest_read_avg;
	unsigned long completed_request_count;
	unsigned long total_request_num;
	unsigned int moved_page_count;
	unsigned int idle_fast_gc_count;
	unsigned int last_write_lat;
	unsigned int last_ten_write_lat[10];
	unsigned int write_lat_anchor;

	unsigned int newest_write_request_completed_with_same_type_pages;
	unsigned int newest_req_with_msb;
	unsigned int newest_req_with_csb;
	unsigned int newest_req_with_lsb;

	unsigned int newest_write_request_num_l;
	unsigned int newest_write_request_completed_with_same_type_pages_l;
	unsigned int newest_req_with_msb_l;
	unsigned int newest_req_with_csb_l;
	unsigned int newest_req_with_lsb_l;
	
	unsigned int newest_lsb_request_a;
	unsigned int newest_msb_request_a;
	unsigned int newest_csb_request_a;
	int speed_up;
	//================================================

	unsigned int min_lsn;
	unsigned int max_lsn;
	unsigned long read_count;
	unsigned long program_count;
	unsigned long erase_count;
	unsigned long direct_erase_count;
	unsigned long copy_back_count;
	unsigned long m_plane_read_count;
	unsigned long m_plane_prog_count;
	unsigned long interleave_count;
	unsigned long interleave_read_count;
	unsigned long inter_mplane_count;
	unsigned long inter_mplane_prog_count;
	unsigned long interleave_erase_count;
	unsigned long mplane_erase_conut;
	unsigned long interleave_mplane_erase_count;
	unsigned long gc_copy_back;
	unsigned long write_flash_count;     //实锟绞诧拷锟斤拷锟侥讹拷flash锟斤拷写锟斤拷锟斤拷
	unsigned long sub_request_success;
	unsigned long sub_request_all;
	//=============================================================
	unsigned long write_lsb_count;
	unsigned long write_msb_count;
	unsigned long write_csb_count;
	unsigned long newest_write_lsb_count;
	unsigned long newest_write_msb_count;
	unsigned long newest_write_csb_count;
	unsigned long fast_gc_count;
	int64_t basic_time;
	unsigned long free_lsb_count;
	unsigned long free_csb_count;
	unsigned long free_msb_count;
	//=============================================================
	
	unsigned long waste_page_count;      //锟斤拷录锟斤拷为锟竭硷拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷碌锟揭筹拷朔锟?
	float ave_read_size;
	float ave_write_size;
	unsigned int request_queue_length;
	unsigned int update_read_count;      //锟斤拷录锟斤拷为锟斤拷锟铰诧拷锟斤拷锟斤拷锟铰的讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟?

	char parameterfilename[30];
	char tracefilename[30];
	char outputfilename[30];
	char statisticfilename[30];
	char statisticfilename2[30];

	FILE * raidOutfile;
	FILE * gcCreateRequest;
	FILE * gcIntervalFile;
	FILE * outputfile;
	FILE * tracefile;
	FILE * statisticfile;
	FILE * statisticfile2;

    struct parameter_value *parameter;   //SSD锟斤拷锟斤拷锟斤拷锟斤拷
	struct dram_info *dram;
	struct request *request_queue;       //dynamic request queue
	struct request *request_tail;	     // the tail of the request queue
	struct sub_request *subs_w_head;     //锟斤拷锟斤拷锟斤拷全锟斤拷态锟斤拷锟斤拷时锟斤拷锟斤拷锟斤拷锟角诧拷知锟斤拷应锟矫癸拷锟斤拷锟侥革拷channel锟较ｏ拷锟斤拷锟斤拷锟饺癸拷锟斤拷ssd锟较ｏ拷锟饺斤拷锟斤拷process锟斤拷锟斤拷时锟脚挂碉拷锟斤拷应锟斤拷channel锟侥讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
	struct sub_request *subs_w_tail;
	struct event_node *event;            //锟铰硷拷锟斤拷锟叫ｏ拷每锟斤拷锟斤拷一锟斤拷锟铰碉拷锟铰硷拷锟斤拷锟斤拷锟斤拷时锟斤拷顺锟斤拷拥锟斤拷锟斤拷锟斤拷锟叫ｏ拷锟斤拷simulate锟斤拷锟斤拷锟斤拷螅锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷锟阶碉拷时锟戒，确锟斤拷时锟斤拷
	struct channel_info *channel_head;   //指锟斤拷channel锟结构锟斤拷锟斤拷锟斤拷锟斤拷椎锟街?

	struct block_erase_count* block_head;//锟斤拷锟斤拷锟斤拷指锟斤拷锟斤拷锟斤拷锟斤拷锟?
	unsigned int page_move_count;        //page move in gc
	unsigned int wl_page_move_count;     //page move in wl
	unsigned int rr_page_move_count;
	float mapping_time;
	float avg_erase;                     //锟斤拷录锟斤拷锟斤拷锟斤拷锟斤拷平锟斤拷值
	unsigned int max_erase;              //锟斤拷录锟斤拷前锟斤拷锟斤拷锟斤拷锟街?
	int wl_hotdata_flag;                 //锟斤拷谴锟绞憋拷欠锟斤拷锟絯l锟叫碉拷hotdata
	int wl_colddata_flag;                 //锟斤拷谴锟绞憋拷欠锟斤拷锟絯l锟叫碉拷colddata
	int wl_hotdata_count;                //wl锟斤拷锟斤拷锟斤拷锟斤拷械锟絟ot read data锟侥硷拷锟斤拷
	int wl_colddata_count;               //wl锟斤拷锟斤拷锟斤拷锟斤拷械锟絚old read data锟侥硷拷锟斤拷
	int find_a_empty_block;              //flag锟斤拷录锟角凤拷要锟斤拷一锟斤拷empty锟侥匡拷
	unsigned int wl_count;
	unsigned int wl_delta;
	//struct bitmap *bit;                  //位锟斤拷锟斤拷锟斤拷录锟斤拷锟斤拷锟叫╋拷锟斤拷欠锟斤拷薷墓锟?
	struct block_update_table* block_update;//锟斤拷录锟斤拷锟角凤拷updated
	struct block_update_table* block_update_current;//锟斤拷录锟斤拷前锟斤拷位锟斤拷
	float table_count;                      //锟斤拷录锟斤拷锟斤拷flag为1锟侥革拷锟斤拷f count
	float total_erase_count;              //锟斤拷录锟斤拷前锟斤拷erase锟斤拷锟斤拷,e count
	int bet_num;                          //f index
	int BET[65535];                      //Block Erase Table, one array indicates one block status
	//int UBT[32767];
	unsigned int wl_request;
	//int PRT[4718591];                     //Page Read Table,i.e. record top20 blocks' page
	struct wl_table* wl_block_a;          //proposed scheme's block a
	struct wl_table* wl_block_b;          //proposed scheme's block b
	int wl_current_block;                 //锟斤拷录锟斤拷前锟斤拷锟节斤拷锟斤拷wl锟侥匡拷
	int wl_flag;                          //0锟斤拷示锟斤拷锟斤拷锟斤拷gc锟斤拷锟斤拷锟斤拷1锟斤拷示wl锟斤拷锟斤拷
	int selected_block_flag;//1锟斤拷示锟剿匡拷锟斤拷hotdata占锟斤拷锟斤拷锟斤拷锟?90%锟斤拷锟侥块，0锟斤拷示锟剿匡拷锟斤拷hotdata占锟斤拷锟斤拷锟斤拷10%锟斤拷锟侥匡拷,2锟斤拷示锟剿匡拷锟絟otdata锟斤拷锟斤拷10%-90%之锟斤拷
	struct wl_table* current_selected_block_a;//锟斤拷录锟斤拷前锟斤拷原table锟斤拷匹锟斤拷锟絫able位锟斤拷a
	struct wl_table* current_selected_block_b;//锟斤拷录锟斤拷前锟斤拷原table锟斤拷匹锟斤拷锟絫able位锟斤拷b
	int wl_match_flag;               //1锟斤拷示锟斤拷wl锟侥匡拷锟斤拷锟斤拷锟矫伙拷斜锟狡ワ拷洌?0为锟斤拷始值

	int uninterrupt_gc_count;				//debug
	int move_page_debug;					//debug

	unsigned int init_count;
	int rr_request;
	int rr_erase_count;
	int wl_erase_count;
	int normal_erase;
	int same_block_flag;

	/*add for count invaild pageby winks*/
	unsigned long invaild_page_of_all;
	unsigned long invaild_page_of_change;
	unsigned long frequently_count;
	unsigned long Nofrequently_count;
	unsigned long counttmp1;

	unsigned long long *page2Trip;
	struct RaidInfo *trip2Page;
	struct Stripe stripe;
	struct request *raidReq;
	unsigned long long raidWriteCount[2];
	
	unsigned int chipToken;
	unsigned int channelToken;
	
	int cpu_current_state;                  //channel has serveral states, including idle, command/address transfer,data transfer,unknown
	int cpu_next_state;
	int64_t cpu_next_state_predict_time;

	unsigned char *bufferBitMap;
	struct request *readBufReqQue;
	
	unsigned long long read1;
	unsigned long long read2;
	unsigned long long read3;
	unsigned long long read4;
	unsigned long long pageMoveRaid;
	unsigned long long parityChange;

	unsigned long long allPage;
	unsigned long long *chipWrite;
	unsigned long long *chipGc;
	unsigned long long allUsedPage;
	unsigned long long gcInterval128;
	unsigned long long gcInterval256;

	unsigned long long *preRequestArriveTime;
	int preRequestArriveTimeIndex;
	int preRequestArriveTimeIndexMAX;

	unsigned long long recoveryTime;
	unsigned long long raidBaseJ;

	unsigned long long needRecoveryAll;
	unsigned long long needRecoveryInvalid;

	unsigned long long  *channelWorkload;

	unsigned long long *preSubWriteLpn;
	int preSubWriteLen;
	int preSubWriteNowPos;
	int getPpnCount;
	unsigned long long *dataPlace;
	unsigned long long allBlockReq;
};

struct wl_table {
	unsigned int selected_channel;
	unsigned int selected_chip;
	unsigned int selected_die;
	unsigned int selected_plane;
	unsigned int selected_block;
	int selected_block_flag;    //1锟斤拷示锟剿匡拷锟斤拷hotdata占锟斤拷锟斤拷锟斤拷锟?90%锟斤拷锟侥块，0锟斤拷示锟剿匡拷锟斤拷hotdata占锟斤拷锟斤拷锟斤拷10%锟斤拷锟侥匡拷
	unsigned int channel;
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;
	struct wl_table* next;
};


struct block_erase_count {
	unsigned int channel;
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;
	unsigned int erase_count;
	int update_flag;        //1 update recently, 0 not update recently.
	struct block_erase_count* next;
};

struct channel_info{
	int chip;                            //锟斤拷示锟节革拷锟斤拷锟斤拷锟斤拷锟叫讹拷锟劫匡拷锟斤拷
	unsigned long read_count;
	unsigned long program_count;
	unsigned long erase_count;
	unsigned int token;                  //锟节讹拷态锟斤拷锟斤拷锟叫ｏ拷为锟斤拷止每锟轿凤拷锟斤拷锟节碉拷一锟斤拷chip锟斤拷要维锟斤拷一锟斤拷锟斤拷锟狡ｏ拷每锟轿达拷锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫匡拷始锟斤拷锟斤拷

	//***************************************
	unsigned long fast_gc_count;
	//***************************************
	int current_state;                   //channel has serveral states, including idle, command/address transfer,data transfer,unknown
	int next_state;
	int64_t current_time;                //锟斤拷录锟斤拷通锟斤拷锟侥碉拷前时锟斤拷
	int64_t next_state_predict_time;     //the predict time of next state, used to decide the sate at the moment

	struct event_node *event;
	struct sub_request *subs_r_head;     //channel锟较的讹拷锟斤拷锟斤拷锟斤拷锟酵凤拷锟斤拷确锟斤拷锟斤拷诙锟斤拷锟酵凤拷锟斤拷锟斤拷锟斤拷锟?
	struct sub_request *subs_r_tail;     //channel锟较的讹拷锟斤拷锟斤拷锟斤拷锟轿诧拷锟斤拷录咏锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟接碉拷锟斤拷尾
	struct sub_request *subs_w_head;     //channel锟较碉拷写锟斤拷锟斤拷锟斤拷锟酵凤拷锟斤拷确锟斤拷锟斤拷诙锟斤拷锟酵凤拷锟斤拷锟斤拷锟斤拷锟?
	struct sub_request *subs_w_tail;     //channel锟较碉拷写锟斤拷锟斤拷锟斤拷校锟斤拷录咏锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟接碉拷锟斤拷尾
	struct gc_operation *gc_command;     //锟斤拷录锟斤拷要锟斤拷锟斤拷gc锟斤拷位锟斤拷
	struct chip_info *chip_head;
	/*add by winks*/
	struct gc_operation *last_gc_command;     //锟斤拷录锟斤拷要锟斤拷锟斤拷gc锟斤拷位锟斤拷
	int gc_req_nums;
};


struct chip_info{
	unsigned int die_num;               //锟斤拷示一锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷锟劫革拷die
	unsigned int plane_num_die;         //indicate how many planes in a die
	unsigned int block_num_plane;       //indicate how many blocks in a plane
	unsigned int page_num_block;        //indicate how many pages in a block
	unsigned int subpage_num_page;      //indicate how many subpage in a page
	unsigned int ers_limit;             //锟斤拷chip锟斤拷每锟斤拷锟杰癸拷锟斤拷锟斤拷锟斤拷锟侥达拷锟斤拷
	unsigned int token;                 //锟节讹拷态锟斤拷锟斤拷锟叫ｏ拷为锟斤拷止每锟轿凤拷锟斤拷锟节碉拷一锟斤拷die锟斤拷要维锟斤拷一锟斤拷锟斤拷锟狡ｏ拷每锟轿达拷锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫匡拷始锟斤拷锟斤拷
	
	int current_state;                  //channel has serveral states, including idle, command/address transfer,data transfer,unknown
	int next_state;
	int64_t current_time;               //锟斤拷录锟斤拷通锟斤拷锟侥碉拷前时锟斤拷
	int64_t next_state_predict_time;    //the predict time of next state, used to decide the sate at the moment
 
	unsigned long read_count;           //how many read count in the process of workload
	unsigned long program_count;
	unsigned long erase_count;

	//***************************************
	unsigned long fast_gc_count;
	//***************************************

    struct ac_time_characteristics ac_timing;  
	struct die_info *die_head;
};


struct die_info{

	unsigned int token;                 //锟节讹拷态锟斤拷锟斤拷锟叫ｏ拷为锟斤拷止每锟轿凤拷锟斤拷锟节碉拷一锟斤拷plane锟斤拷要维锟斤拷一锟斤拷锟斤拷锟狡ｏ拷每锟轿达拷锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫匡拷始锟斤拷锟斤拷
	struct plane_info *plane_head;
	
};


struct plane_info{
	int add_reg_ppn;                    //read锟斤拷write时锟窖碉拷址锟斤拷锟酵碉拷锟矫憋拷锟斤拷锟斤拷锟矫憋拷锟斤拷锟斤拷锟斤拷锟斤拷址锟侥达拷锟斤拷锟斤拷die锟斤拷busy锟斤拷为idle时锟斤拷锟斤拷锟斤拷锟街? //锟叫匡拷锟斤拷锟斤拷为一锟皆讹拷锟接筹拷洌拷锟揭伙拷锟斤拷锟斤拷锟斤拷锟绞憋拷锟斤拷卸锟斤拷锟斤拷同锟斤拷lpn锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷ppn锟斤拷锟斤拷锟斤拷  
	unsigned int free_page;             //锟斤拷plane锟斤拷锟叫讹拷锟斤拷free page
	unsigned int ers_invalid;           //锟斤拷录锟斤拷plane锟叫诧拷锟斤拷失效锟侥匡拷锟斤拷
	unsigned int active_block;          //if a die has a active block, 锟斤拷锟斤拷锟绞撅拷锟斤拷锟斤拷锟斤拷锟斤拷
	//********************************
	unsigned int free_lsb_num;
	unsigned int active_lsb_block;
	unsigned int active_csb_block;
	unsigned int active_msb_block;
	//********************************
	int can_erase_block;                //锟斤拷录锟斤拷一锟斤拷plane锟斤拷准锟斤拷锟斤拷gc锟斤拷锟斤拷锟叫憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥匡拷,-1锟斤拷示锟斤拷没锟斤拷锟揭碉拷锟斤拷锟绞的匡拷
	struct direct_erase *erase_node;    //锟斤拷锟斤拷锟斤拷录锟斤拷锟斤拷直锟斤拷删锟斤拷锟侥匡拷锟?,锟节伙拷取锟铰碉拷ppn时锟斤拷每锟斤拷锟斤拷锟斤拷invalid_page_num==64时锟斤拷锟斤拷锟斤拷锟斤拷锟接碉拷锟斤拷锟街革拷锟斤拷希锟斤拷锟紾C锟斤拷锟斤拷时直锟斤拷删锟斤拷
	//*****************************************
	struct direct_erase *fast_erase_node;
	//*****************************************
	struct blk_info *blk_head;
	int64_t invaild_time;
	unsigned int gc_count;
};


struct blk_info{
	unsigned int erase_count;     //P/E次数
	unsigned int free_page_num;
	unsigned int invalid_page_num;
	int last_write_page;
	struct page_info *page_head;
	//=====================================================
	unsigned int free_lsb_num;
	unsigned int free_msb_num;
	unsigned int free_csb_num;
	unsigned int invalid_lsb_num;
	int last_write_lsb;
	int last_write_msb;
	int last_write_csb;
	int fast_erase;
	unsigned int fast_gc_count;
	unsigned int dr_state;
	unsigned int read_counts;
	unsigned int read_count;
	//=====================================================
	/*add by winks*/
	unsigned long changed_count;
	unsigned char write_frequently; // 0 is no, 1 is yes;

};


struct page_info{                      //lpn锟斤拷录锟斤拷锟斤拷锟斤拷页锟芥储锟斤拷锟竭硷拷页锟斤拷锟斤拷锟斤拷锟竭硷拷页锟斤拷效时锟斤拷valid_state锟斤拷锟斤拷0锟斤拷free_state锟斤拷锟斤拷0锟斤拷
	int valid_state;                   //indicate the page is valid or invalid
	unsigned int free_state;                    //each bit indicates the subpage is free or occupted. 1 indicates that the bit is free and 0 indicates that the bit is used
	unsigned int lpn;                 
	unsigned int written_count;        //锟斤拷录锟斤拷页锟斤拷写锟侥达拷锟斤拷
	unsigned int dadicate;             //0 least, 1 mid, 2 max
	unsigned int read_count;
	long long raidID;
};


struct dram_info{
	unsigned int dram_capacity;     
	int64_t current_time;

	struct dram_parameter *dram_paramters;      
	struct map_info *map;
	//struct buffer_info *buffer; 
	
	struct buffer_info_Hash *buffer; 
};


/*********************************************************************************************
*buffer锟叫碉拷锟斤拷写锟截碉拷页锟侥癸拷锟斤拷锟斤拷锟斤拷:锟斤拷buffer_info锟斤拷维锟斤拷一锟斤拷锟斤拷锟斤拷:written锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷卸锟斤拷祝锟斤拷锟轿诧拷锟?
*每锟斤拷buffer management锟叫ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷锟絞roup要锟狡碉拷LRU锟侥讹拷锟阶ｏ拷同时锟斤拷锟斤拷锟絞roup锟斤拷锟角凤拷锟斤拷锟斤拷
*写锟截碉拷lsn锟斤拷锟叫的伙拷锟斤拷锟斤拷要锟斤拷锟斤拷锟絞roup同时锟狡讹拷锟斤拷written锟斤拷锟叫的讹拷尾锟斤拷锟斤拷锟斤拷锟斤拷械锟斤拷锟斤拷锟斤拷图锟斤拷俚姆锟斤拷锟?
*锟斤拷锟斤拷:锟斤拷锟斤拷要通锟斤拷删锟斤拷锟斤拷写锟截碉拷lsn为锟铰碉拷写锟斤拷锟斤拷锟节筹拷锟秸硷拷时锟斤拷锟斤拷written锟斤拷锟斤拷锟斤拷锟揭筹拷锟斤拷锟斤拷删锟斤拷锟斤拷lsn锟斤拷
*锟斤拷锟斤拷要锟斤拷锟斤拷锟铰碉拷写锟斤拷lsn时锟斤拷锟揭碉拷锟斤拷锟斤拷写锟截碉拷页锟斤拷锟斤拷锟斤拷锟絞roup锟接碉拷指锟斤拷written_insert锟斤拷指锟斤拷锟斤拷written
*锟节碉拷前锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷维锟斤拷一锟斤拷指锟诫，锟斤拷buffer锟斤拷LRU锟斤拷锟斤拷锟斤拷指锟斤拷锟斤拷锟较碉拷一锟斤拷写锟斤拷锟剿碉拷页锟斤拷锟铰达拷要锟斤拷写锟斤拷时锟斤拷
*只锟斤拷锟斤拷锟斤拷锟街革拷锟斤拷锟剿碉拷前一锟斤拷group写锟截硷拷锟缴★拷
**********************************************************************************************/
typedef struct buffer_group{
	HASH_NODE node;                     //锟斤拷锟节碉拷慕峁挂伙拷锟揭拷锟斤拷锟斤拷没锟斤拷远锟斤拷锟结构锟斤拷锟斤拷前锟芥，注锟斤拷!
	struct buffer_group *LRU_link_next;	// next node in LRU list
	struct buffer_group *LRU_link_pre;	// previous node in LRU list

	unsigned int group;                 //the first data logic sector number of a group stored in buffer 
	unsigned int stored;                //indicate the sector is stored in buffer or not. 1 indicates the sector is stored and 0 indicate the sector isn't stored.EX.  00110011 indicates the first, second, fifth, sixth sector is stored in buffer.
	unsigned int dirty_clean;           //it is flag of the data has been modified, one bit indicates one subpage. EX. 0001 indicates the first subpage is dirty
	int flag;			                //indicates if this node is the last 20% of the LRU list	
	unsigned int read_count;
}buf_node;


struct dram_parameter{
	float active_current;
	float sleep_current;
	float voltage;
	int clock_time;
};


struct map_info{
	struct entry *map_entry;            //锟斤拷锟斤拷锟斤拷映锟斤拷锟斤拷峁癸拷锟街革拷锟?,each entry indicate a mapping information
	struct buffer_info *attach_info;	// info about attach map
};


struct controller_info{
	unsigned int frequency;             //锟斤拷示锟矫匡拷锟斤拷锟斤拷锟侥癸拷锟斤拷频锟斤拷
	int64_t clock_time;                 //锟斤拷示一锟斤拷时锟斤拷锟斤拷锟节碉拷时锟斤拷
	float power;                        //锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷位时锟斤拷锟斤拷芎锟?
};


struct request{
	int64_t time;                      //锟斤拷锟襟到达拷锟绞憋拷洌拷锟轿晃猽s,锟斤拷锟斤拷锟酵拷锟斤拷锟较帮拷卟锟揭伙拷锟斤拷锟酵拷锟斤拷锟斤拷锟絤s为锟斤拷位锟斤拷锟斤拷锟斤拷锟斤拷要锟叫革拷锟斤拷位锟戒换锟斤拷锟斤拷
	unsigned int lsn;                  //锟斤拷锟斤拷锟斤拷锟绞硷拷锟街凤拷锟斤拷呒锟斤拷锟街?
	unsigned int size;                 //锟斤拷锟斤拷拇锟叫★拷锟斤拷榷锟斤拷俑锟斤拷锟斤拷锟?
	unsigned int operation;            //锟斤拷锟斤拷锟斤拷锟斤拷啵?1为锟斤拷锟斤拷0为写

	/*************************/
	unsigned int priority;				//锟斤拷锟斤拷锟叫讹拷SSD锟角凤拷锟斤拷锟?
	/*************************/

	unsigned int* need_distr_flag;
	unsigned int complete_lsn_count;   //record the count of lsn served by buffer

	int distri_flag;		           // indicate whether this request has been distributed already

	int64_t begin_time;
	int64_t response_time;
	double energy_consumption;         //锟斤拷录锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷模锟斤拷锟轿晃猽J

	struct sub_request *subs;          //锟斤拷锟接碉拷锟斤拷锟节革拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
	struct request *next_node;         //指锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷峁癸拷锟?
	unsigned int all;
	unsigned int now;
	unsigned char completeFlag;
	unsigned char MergeFlag;

	unsigned int parityCount;
};


struct sub_request{
	unsigned int lpn;                  //锟斤拷锟斤拷锟绞撅拷锟斤拷锟斤拷锟斤拷锟斤拷锟竭硷拷页锟斤拷
	unsigned int ppn;                  //锟斤拷锟斤拷锟角革拷锟斤拷锟斤拷锟斤拷页锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絤ulti_chip_page_mapping锟叫ｏ拷锟斤拷锟斤拷锟斤拷页锟斤拷锟斤拷时锟斤拷锟杰撅拷知锟斤拷psn锟斤拷值锟斤拷锟斤拷锟斤拷时锟斤拷psn锟斤拷值锟斤拷page_map_read,page_map_write锟斤拷FTL锟斤拷撞愫拷锟斤拷锟斤拷锟斤拷锟? 
	unsigned int operation;            //锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷停锟斤拷锟斤拷硕锟?1 写0锟斤拷锟斤拷锟叫诧拷锟斤拷锟斤拷two plane锟饺诧拷锟斤拷 
	int size;

	unsigned int current_state;        //锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷状态锟斤拷锟斤拷锟疥定锟斤拷sub request
	int64_t current_time;
	unsigned int next_state;
	int64_t next_state_predict_time;
	 unsigned int state;              //使锟斤拷state锟斤拷锟斤拷锟轿伙拷锟绞撅拷锟斤拷锟斤拷锟斤拷锟斤拷欠锟斤拷锟揭伙拷远锟接筹拷锟斤拷系锟叫碉拷一锟斤拷锟斤拷锟角的伙拷锟斤拷锟斤拷要锟斤拷锟斤拷buffer锟叫★拷1锟斤拷示锟斤拷一锟皆多，0锟斤拷示锟斤拷锟斤拷写锟斤拷buffer
	                                  //锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟皆憋拷锟絣sn锟斤拷size锟酵匡拷锟皆分憋拷锟斤拷锟揭筹拷锟阶刺?;锟斤拷锟斤拷写锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟皆憋拷锟斤拷蟛糠锟叫达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絙uffer写锟截诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷页锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟揭拷锟斤拷锟轿拷指贸锟皆?

	int64_t begin_time;               //锟斤拷锟斤拷锟斤拷始时锟斤拷
	int64_t complete_time;            //锟斤拷录锟斤拷锟斤拷锟斤拷锟斤拷拇锟斤拷锟绞憋拷锟?,锟斤拷锟斤拷锟斤拷写锟斤拷锟斤拷叨锟斤拷锟斤拷锟斤拷莸锟绞憋拷锟?

	struct local *location;           //锟节撅拷态锟斤拷锟斤拷突锟较凤拷锟戒方式锟叫ｏ拷锟斤拷知lpn锟斤拷知锟斤拷锟斤拷lpn锟矫凤拷锟戒到锟角革拷channel锟斤拷chip锟斤拷die锟斤拷plane锟斤拷锟斤拷锟斤拷峁癸拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷玫锟斤拷牡锟街?
	struct sub_request *next_subs;    //指锟斤拷锟斤拷锟斤拷同一锟斤拷request锟斤拷锟斤拷锟斤拷锟斤拷
	struct sub_request *next_node;    //指锟斤拷同一锟斤拷channel锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷峁癸拷锟?
	struct sub_request *update;       //锟斤拷为锟斤拷写锟斤拷锟斤拷锟叫达拷锟节革拷锟铰诧拷锟斤拷锟斤拷锟斤拷为锟节讹拷态锟斤拷锟戒方式锟斤拷锟睫凤拷使锟斤拷copyback锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷原锟斤拷锟斤拷页锟斤拷锟斤拷锟斤拷锟斤拷芙锟斤拷锟叫达拷锟斤拷锟斤拷锟斤拷锟斤拷裕锟斤拷锟斤拷锟斤拷锟铰诧拷锟斤拷锟侥讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街革拷锟斤拷锟?

	//*****************************************
	unsigned int target_page_type;
	unsigned int allocated_page_type;   //0 for lsb page, 1 for msb page;
	//*****************************************

	/*add for flag write freqently*/
	unsigned char write_freqently; // 0: no, 1:yes;
	unsigned char readXorFlag;
	unsigned int readRaidLpn;
	unsigned int readRaidLpnState;

	unsigned int raidNUM;
	unsigned char emergency;
	unsigned int pChangeCount;
	struct request *req;
};


/***********************************************************************
*锟铰硷拷锟节碉拷锟斤拷锟绞憋拷锟斤拷锟斤拷锟斤拷锟斤拷每锟斤拷时锟斤拷锟斤拷锟斤拷锟斤拷歉锟斤拷锟绞憋拷锟斤拷锟斤拷锟斤拷一锟斤拷锟铰硷拷锟斤拷确锟斤拷锟斤拷
************************************************************************/
struct event_node{
	int type;                        //锟斤拷录锟斤拷锟铰硷拷锟斤拷锟斤拷锟酵ｏ拷1锟斤拷示锟斤拷锟斤拷锟斤拷锟酵ｏ拷2锟斤拷示锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷
	int64_t predict_time;            //锟斤拷录锟斤拷锟绞憋拷淇硷拷锟皆わ拷锟绞憋拷洌拷锟街癸拷锟角爸达拷锟斤拷锟斤拷时锟斤拷
	struct event_node *next_node;
	struct event_node *pre_node;
};

struct parameter_value{
	unsigned int chip_num;          //锟斤拷录一锟斤拷SSD锟斤拷锟叫讹拷锟劫革拷锟斤拷锟斤拷
	unsigned int dram_capacity;     //锟斤拷录SSD锟斤拷DRAM capacity
	unsigned int cpu_sdram;         //锟斤拷录片锟斤拷锟叫讹拷锟斤拷

	unsigned int channel_number;    //锟斤拷录SSD锟斤拷锟叫讹拷锟劫革拷通锟斤拷锟斤拷每锟斤拷通锟斤拷锟角碉拷锟斤拷锟斤拷bus
	unsigned int chip_channel[100]; //锟斤拷锟斤拷SSD锟斤拷channel锟斤拷锟斤拷每channel锟较匡拷锟斤拷锟斤拷锟斤拷锟斤拷

	unsigned int die_chip;    
	unsigned int plane_die;
	unsigned int block_plane;
	unsigned int page_block;
	unsigned int subpage_page;

	unsigned int page_capacity;
	unsigned int subpage_capacity;

	/***********************锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷**************************************/
	unsigned int turbo_mode;        //0 for off, 1 for always on, 2 for conditional on
	unsigned int turbo_mode_factor;
	unsigned int turbo_mode_factor_2;
	unsigned int lsb_first_allocation;
	unsigned int fast_gc;			//锟斤拷录锟角凤拷实施锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	float fast_gc_thr_1;
	float fast_gc_filter_1;
	float fast_gc_thr_2;
	float fast_gc_filter_2;
	float fast_gc_filter_idle;
	float dr_filter;
	unsigned int dr_switch;
	unsigned int dr_cycle;
	/*************************************************************/

	unsigned int ers_limit;         //锟斤拷录每锟斤拷锟斤拷刹锟斤拷锟斤拷拇锟斤拷锟?
	int address_mapping;            //锟斤拷录映锟斤拷锟斤拷锟斤拷停锟?1锟斤拷page锟斤拷2锟斤拷block锟斤拷3锟斤拷fast
	int wear_leveling;              // WL锟姐法
	int gc;                         //锟斤拷录gc锟斤拷锟斤拷
	int clean_in_background;        //锟斤拷锟斤拷锟斤拷锟斤拷欠锟斤拷锟角疤拷锟斤拷
	int alloc_pool;                 //allocation pool 锟斤拷小(plane锟斤拷die锟斤拷chip锟斤拷channel),也锟斤拷锟斤拷拥锟斤拷active_block锟侥碉拷位
	float overprovide;
	float gc_threshold;             //锟斤拷锟斤到锟斤拷锟斤拷锟街凳憋拷锟斤拷锟绞糋C锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟叫ｏ拷锟斤拷始GC锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞憋拷卸锟紾C锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟铰碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷锟叫ｏ拷GC锟斤拷锟斤拷锟叫讹拷

	double operating_current;       //NAND FLASH锟侥癸拷锟斤拷锟斤拷锟斤拷锟斤拷位锟斤拷uA
	double supply_voltage;	
	double dram_active_current;     //cpu sdram work current   uA
	double dram_standby_current;    //cpu sdram work current   uA
	double dram_refresh_current;    //cpu sdram work current   uA
	double dram_voltage;            //cpu sdram work voltage  V

	int buffer_management;          //indicates that there are buffer management or not
	int scheduling_algorithm;       //锟斤拷录使锟斤拷锟斤拷锟街碉拷锟斤拷锟姐法锟斤拷1:FCFS
	float quick_radio;
	int related_mapping;

	unsigned int time_step;
	unsigned int small_large_write; //the threshould of large write, large write do not occupt buffer, which is written back to flash directly

	int striping;                   //锟斤拷示锟角凤拷使锟斤拷锟斤拷striping锟斤拷式锟斤拷0锟斤拷示没锟叫ｏ拷1锟斤拷示锟斤拷
	int interleaving;
	int pipelining;
	int threshold_fixed_adjust;
	int threshold_value;
	int active_write;               //锟斤拷示锟角凤拷执锟斤拷锟斤拷锟斤拷写锟斤拷锟斤拷1,yes;0,no
	float gc_hard_threshold;        //锟斤拷通锟斤拷锟斤拷锟斤拷锟矫诧拷锟斤拷锟矫诧拷锟斤拷锟斤拷只锟斤拷锟斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟叫ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街凳憋拷锟紾C锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
	int allocation_scheme;          //锟斤拷录锟斤拷锟戒方式锟斤拷选锟斤拷0锟斤拷示锟斤拷态锟斤拷锟戒，1锟斤拷示锟斤拷态锟斤拷锟斤拷
	int static_allocation;          //锟斤拷录锟斤拷锟斤拷锟街撅拷态锟斤拷锟戒方式锟斤拷锟斤拷ICS09锟斤拷篇锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫撅拷态锟斤拷锟戒方式
	int dynamic_allocation;         //锟斤拷录锟斤拷态锟斤拷锟斤拷姆锟绞?
	int advanced_commands;  
	int ad_priority;                //record the priority between two plane operation and interleave operation
	int ad_priority2;               //record the priority of channel-level, 0 indicates that the priority order of channel-level is highest; 1 indicates the contrary
	int greed_CB_ad;                //0 don't use copyback advanced commands greedily; 1 use copyback advanced commands greedily
	int greed_MPW_ad;               //0 don't use multi-plane write advanced commands greedily; 1 use multi-plane write advanced commands greedily
	int aged;                       //1锟斤拷示锟斤拷要锟斤拷锟斤拷锟絊SD锟斤拷锟絘ged锟斤拷0锟斤拷示锟斤拷要锟斤拷锟斤拷锟絊SD锟斤拷锟斤拷non-aged
	float aged_ratio; 
	int queue_length;               //锟斤拷锟斤拷锟斤拷械某锟斤拷锟斤拷锟斤拷锟?

	struct ac_time_characteristics time_characteristics;
};

/********************************************************
*mapping information,state锟斤拷锟斤拷锟轿伙拷锟绞撅拷欠锟斤拷懈锟斤拷锟接筹拷锟斤拷系
*********************************************************/
struct entry{                       
	unsigned int pn;                //锟斤拷锟斤拷锟脚ｏ拷锟饺匡拷锟皆憋拷示锟斤拷锟斤拷页锟脚ｏ拷也锟斤拷锟皆憋拷示锟斤拷锟斤拷锟斤拷页锟脚ｏ拷也锟斤拷锟皆憋拷示锟斤拷锟斤拷锟斤拷锟?
	unsigned int state;                      //十锟斤拷锟斤拷锟狡憋拷示锟侥伙拷锟斤拷0000-FFFF锟斤拷每位锟斤拷示锟斤拷应锟斤拷锟斤拷页锟角凤拷锟斤拷效锟斤拷页映锟戒）锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟揭筹拷校锟?0锟斤拷1锟斤拷锟斤拷页锟斤拷效锟斤拷2锟斤拷3锟斤拷效锟斤拷锟斤拷锟接︼拷锟斤拷锟?0x0003.
};


struct local{          
	unsigned int channel;
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;
	unsigned int page;
	unsigned int sub_page;
};


struct gc_info{
	int64_t begin_time;            //锟斤拷录一锟斤拷plane什么时锟斤拷始gc锟斤拷锟斤拷锟斤拷
	int copy_back_count;    
	int erase_count;
	int64_t process_time;          //锟斤拷plane锟斤拷锟剿讹拷锟斤拷时锟斤拷锟斤拷gc锟斤拷锟斤拷锟斤拷
	double energy_consumption;     //锟斤拷plane锟斤拷锟剿讹拷锟斤拷锟斤拷锟斤拷锟斤拷gc锟斤拷锟斤拷锟斤拷
};


struct direct_erase{
	unsigned int block;
	struct direct_erase *next_node;
};


/**************************************************************************************
 *锟斤拷锟斤拷锟斤拷一锟斤拷GC锟斤拷锟斤拷时锟斤拷锟斤拷锟斤拷锟斤拷峁癸拷锟斤拷锟斤拷锟接︼拷锟絚hannel锟较ｏ拷锟饺达拷channel锟斤拷锟斤拷时锟斤拷锟斤拷锟斤拷GC锟斤拷锟斤拷锟斤拷锟斤拷
***************************************************************************************/
struct gc_operation{          
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;           //锟矫诧拷锟斤拷只锟节匡拷锟叫断碉拷gc锟斤拷锟斤拷锟斤拷使锟矫ｏ拷gc_interrupt锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷录锟窖斤拷锟揭筹拷锟斤拷锟斤拷目锟斤拷锟斤拷
	unsigned int page;            //锟矫诧拷锟斤拷只锟节匡拷锟叫断碉拷gc锟斤拷锟斤拷锟斤拷使锟矫ｏ拷gc_interrupt锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷录锟窖撅拷锟斤拷傻锟斤拷锟斤拷锟角拷频锟揭筹拷锟?
	unsigned int state;           //锟斤拷录锟斤拷前gc锟斤拷锟斤拷锟阶刺?
	unsigned int priority;        //锟斤拷录锟斤拷gc锟斤拷锟斤拷锟斤拷锟斤拷锟饺硷拷锟斤拷1锟斤拷示锟斤拷锟斤拷锟叫断ｏ拷0锟斤拷示锟斤拷锟叫断ｏ拷锟斤拷锟斤拷值锟斤拷锟斤拷锟斤拷gc锟斤拷锟斤拷
	unsigned int wl_flag;
	struct gc_operation *next_node;
};

/*
*add by ninja
*used for map_pre function
*/
typedef struct Dram_write_map
{
	unsigned int state; 
}Dram_write_map;


struct ssd_info *initiation(struct ssd_info *);
struct parameter_value *load_parameters(char parameter_file[30]);
struct page_info * initialize_page(struct page_info * p_page);
struct blk_info * initialize_block(struct blk_info * p_block,struct parameter_value *parameter);
struct plane_info * initialize_plane(struct plane_info * p_plane,struct parameter_value *parameter );
struct die_info * initialize_die(struct die_info * p_die,struct parameter_value *parameter,long long current_time );
struct chip_info * initialize_chip(struct chip_info * p_chip,struct parameter_value *parameter,long long current_time );
struct ssd_info * initialize_channels(struct ssd_info * ssd );
struct dram_info * initialize_dram(struct ssd_info * ssd);

#endif

