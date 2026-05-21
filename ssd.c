/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName锟斤拷 ssd.c
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
#include "ssd.h"
#include "hash.h"
//#include "layer_weight.h"
int index1 = 0, index2 = 0, index3 = 0, RRcount = 0;
float aveber=0;

void init_valid(struct ssd_info *ssd){
		unsigned int i, j, k, l, m, n;	
		unsigned int lpn = 0;
		unsigned int lsn = 100000;
		unsigned int ppn, full_page;
		int sub_size = 8;
		//init valid pages
		for (i = 0; i < ssd->parameter->channel_number; i++)
		{
			for (j = 0; j < ssd->parameter->chip_num / ssd->parameter->channel_number; j++)
			{
				for (k = 0; k < ssd->parameter->die_chip; k++)
				{
					for (l = 0; l < ssd->parameter->plane_die; l++)
					{
						for (m = 0; m < ssd->parameter->block_plane; m++)
						{
							for (n = 0; n < 0.30 * ssd->parameter->page_block; n++)
							{
	
								lpn = lsn / ssd->parameter->subpage_page;
								ppn = find_ppn(ssd, i, j, k, l, m, n);
								//modify state
								ssd->dram->map->map_entry[lpn].pn = ppn;
								ssd->dram->map->map_entry[lpn].state = set_entry_state(ssd, lsn, sub_size);   //0001
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].lpn = lpn;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].valid_state = ssd->dram->map->map_entry[lpn].state;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].free_state = ((~ssd->dram->map->map_entry[lpn].state) & full_page);
								//--
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_page++;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_page_num--;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_page--;
								if(ppn % 3 == 0)
									ssd->free_lsb_count--;
								if(ppn % 3 == 2)
									ssd->free_msb_count--;
								else 
									ssd->free_csb_count--;
								lsn++;
							}
						}
	
					}
				}
			}
		}
	

}

void raid_init(struct ssd_info *ssd){
	unsigned long long pageNum = ((ssd->parameter->page_block)*(ssd->parameter->block_plane)*(ssd->parameter->plane_die)*(ssd->parameter->die_chip)*(ssd->parameter->chip_channel[0])*(ssd->parameter->channel_number));
	unsigned long long i;
	unsigned long long StripeNum = pageNum / STRIPENUM;
	unsigned long long chipNum = ssd->parameter->chip_channel[0] * ssd->parameter->channel_number;
	
	ssd->page2Trip = (unsigned long long*)malloc(pageNum * sizeof(unsigned long long));
	alloc_assert(ssd->page2Trip, "ssd->page2Trip");
	for(i = 0; i < pageNum; i++)
		ssd->page2Trip[i] = 0;

	ssd->stripe.req = (struct StripeReq*)malloc(sizeof(struct StripeReq) * STRIPENUM);
	alloc_assert(ssd->stripe.req, "ssd->stripe.req");
	memset(ssd->stripe.req, 0, sizeof(struct StripeReq) * STRIPENUM);
	ssd->stripe.all = STRIPENUM;
	ssd->stripe.now = 0;
	ssd->stripe.check = 0;
	ssd->stripe.checkLpn = ((ssd->parameter->subpage_page*ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*(ssd->parameter->chip_num)));
	ssd->stripe.checkLpn /= ssd->parameter->subpage_page;
	ssd->stripe.checkLpn *= (1-ssd->parameter->overprovide);
	ssd->stripe.checkLpn = ssd->stripe.checkLpn * (STRIPENUM - 1) / STRIPENUM;
	ssd->stripe.checkLpn += 1;
	ssd->stripe.checkStart = ssd->stripe.checkLpn;
	ssd->stripe.nowStripe = 1;
	ssd->stripe.allStripe = pageNum;
	
	ssd->raidReq = malloc(sizeof(struct request));
	alloc_assert(ssd->raidReq, "ssd->raidReq");
	memset(ssd->raidReq, 0, sizeof(struct request));
	ssd->raidReq->subs = NULL;
	
	ssd->trip2Page = (struct RaidInfo*)malloc(ssd->stripe.allStripe * sizeof(struct RaidInfo));
	alloc_assert(ssd->trip2Page, "ssd->trip2Page");
	memset(ssd->trip2Page, 0, ssd->stripe.allStripe * sizeof(struct RaidInfo));
	for(i = 0; i < ssd->stripe.allStripe; i++){
		ssd->trip2Page[i].lpn = malloc(STRIPENUM * sizeof(unsigned int));
		alloc_assert(ssd->trip2Page[i].lpn, "ssd->trip2Page[i].lpn");
		memset(ssd->trip2Page[i].lpn, 0, STRIPENUM * sizeof(unsigned int));

		ssd->trip2Page[i].check = malloc(STRIPENUM * sizeof(int));
		alloc_assert(ssd->trip2Page[i].check, "ssd->trip2Page[i].check");
		memset(ssd->trip2Page[i].check, 0, STRIPENUM * sizeof(int));
		//ssd->trip2Page[i].PChannel = -1;
		ssd->trip2Page[i].nowChange = 0;
		ssd->trip2Page[i].allChange = 0;
		ssd->trip2Page[i].using = 0;
		ssd->trip2Page[i].location = NULL;
		ssd->trip2Page[i].readFlag = 0;
		ssd->trip2Page[i].changeQueuePos = malloc(STRIPENUM * sizeof(int));
		alloc_assert(ssd->trip2Page[i].changeQueuePos, "ssd->trip2Page[i].check");
		memset(ssd->trip2Page[i].changeQueuePos, 0, STRIPENUM * sizeof(int));
	}

	//ssd->preSubWriteLen = 1;
    ssd->preSubWriteLen = 3;
	ssd->preSubWriteNowPos = 0;
	ssd->preSubWriteLpn = malloc(ssd->preSubWriteLen * sizeof(unsigned long long));
	alloc_assert(ssd->preSubWriteLpn, "ssd->preSubWriteLpn");
	memset(ssd->preSubWriteLpn, 0, ssd->preSubWriteLen * sizeof(unsigned long long));
	
	ssd->chipWrite = malloc(sizeof(unsigned long long) * chipNum);
	alloc_assert(ssd->chipWrite, "ssd->chipWrite");
	for(i = 0; i < ssd->parameter->chip_channel[0] * ssd->parameter->channel_number; ++i){
		ssd->chipWrite[i] = 0;
	}
	ssd->channelWorkload = malloc(sizeof(unsigned long long) * ssd->parameter->channel_number);
	alloc_assert(ssd->channelWorkload, "channelWorkload");
	for(i = 0; i < ssd->parameter->channel_number; ++i){
		ssd->channelWorkload[i] = 0;
	}
	ssd->dataPlace = malloc(sizeof(unsigned long long) * ssd->parameter->channel_number);
	alloc_assert(ssd->dataPlace, "channelWorkload");
	for(i = 0; i < ssd->parameter->channel_number; ++i){
		ssd->dataPlace[i] = 0;
	}

	ssd->chipGc = malloc(sizeof(unsigned long long) * chipNum);
	alloc_assert(ssd->chipGc, "ssd->chipGc");
	for(i = 0; i < ssd->parameter->chip_channel[0] * ssd->parameter->channel_number; ++i){
		ssd->chipGc[i] = 0;
	}

	ssd->preRequestArriveTimeIndexMAX = 16;
	ssd->preRequestArriveTimeIndex = 0;
	ssd->preRequestArriveTime = malloc(sizeof(unsigned long long) * ssd->preRequestArriveTimeIndexMAX);
	alloc_assert(ssd->preRequestArriveTime, "ssd->preRequestArriveTime");
	memset(ssd->preRequestArriveTime, 0, sizeof(unsigned long long) * ssd->preRequestArriveTimeIndexMAX);
	
}

void compute_num(struct ssd_info *ssd){
	int channel = 0;
	int chip = 0;
	if(ssd->completed_request_count > ssd->total_request_num /2){
		int page_plane=0,page_die=0,page_chip=0;
		long long count = 0;
		long long i = 0, j =0;
		page_plane=ssd->parameter->page_block*ssd->parameter->block_plane;
		page_die=page_plane*ssd->parameter->plane_die;
		page_chip=page_die*ssd->parameter->die_chip;

		long long start = (channel * ssd->parameter->chip_channel[0] + chip);
		long long end = start + 1;
		start *= page_chip;
		end *= page_chip;

		for(i = 0; i < ssd->stripe.allStripe; i++){
			if(i >= ssd->stripe.nowStripe){
				break;
			}
			for(j = 0; j < STRIPENUM; ++j){
				int lpn = ssd->trip2Page[i].lpn[j];
				long long ppn = ssd->dram->map->map_entry[lpn].pn;
				if(ppn >= start && ppn <= end){
					count++;
					break;
				}				
			}
			
		}
		long long bufer_count = 0;
		struct buffer_group *buffer_node = ssd->dram->buffer->buffer_head;
		while(buffer_node){
			if(ssd->dram->map->map_entry[buffer_node->group].state != 0){
				long long ppn = ssd->dram->map->map_entry[buffer_node->group].pn;
				if(ppn >= start && ppn <= end){
					bufer_count++;
				}
			}
			buffer_node = buffer_node->LRU_link_next;
		}
		printf("count is %lld bufer_count is %lld\n", count, bufer_count);
		exit(0);
	}
}


void add_valid_date(struct ssd_info *ssd){
	int lpn = 1;
	unsigned int mask = ~(0xffffffff<<(ssd->parameter->subpage_page));
	unsigned int full_page;
	if(ssd->parameter->subpage_page == 32){
		full_page = 0xffffffff;
	}
	else{
		full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
	}
	ssd->allPage = ((ssd->parameter->page_block)*(ssd->parameter->block_plane)*(ssd->parameter->plane_die)*(ssd->parameter->die_chip)*(ssd->parameter->chip_channel[0])*(ssd->parameter->channel_number));
	printf("rate %f\n", (double)ssd->allUsedPage / ssd->allPage);
	while(ssd->allUsedPage < ssd->allPage * 0.7){
		if(ssd->dram->map->map_entry[lpn].state==0){
			ssd->stripe.req[ssd->stripe.now].lpn = lpn;
			ssd->stripe.req[ssd->stripe.now].state = full_page;				
			ssd->stripe.req[ssd->stripe.now].req = ssd->raidReq;
			ssd->stripe.req[ssd->stripe.now].sub_size = size(full_page);
			ssd->stripe.req[ssd->stripe.now].full_page = full_page;
			if(++ssd->stripe.now == (ssd->stripe.all - 1)){
				//printf("add!!!!\n");
				unsigned int i, j;
				for(i = 0, j = 0; i < ssd->stripe.all; ++i){
					if(i == ssd->stripe.check){
						raid_pre_read(ssd, ssd->stripe.checkLpn * ssd->parameter->subpage_page, ssd->stripe.checkLpn, mask, full_page);

						//ssd->trip2Page[ssd->stripe.nowStripe].lpn[i] = ssd->stripe.checkLpn;
						//ssd->trip2Page[ssd->stripe.nowStripe].check[i] = CHECK_RAID;
						
						//++ssd->stripe.checkLpn;
						//continue;
					}else{
						j %= (ssd->stripe.all - 1);
						if(ssd->stripe.req[j].req->operation != 0)
							printf("%d %d\n", j, ssd->stripe.req[j].req->operation);
						raid_pre_read(ssd, ssd->stripe.req[j].lpn * ssd->parameter->subpage_page, ssd->stripe.req[j].lpn, \
							ssd->stripe.req[j].sub_size, ssd->stripe.req[j].full_page);
						
						ssd->trip2Page[ssd->stripe.nowStripe].lpn[j] = ssd->stripe.req[j].lpn;
						ssd->trip2Page[ssd->stripe.nowStripe].check[j] = VAIL_DRAID;
						ssd->page2Trip[ssd->stripe.req[j].lpn] = ssd->stripe.nowStripe;
						ssd->stripe.req[j].state = 0;
						++j;
					}						
				}
						
				ssd->trip2Page[ssd->stripe.nowStripe].allChange = 0;
				ssd->trip2Page[ssd->stripe.nowStripe].nowChange = 0;
				ssd->trip2Page[ssd->stripe.nowStripe].using = 1;
				ssd->trip2Page[ssd->stripe.nowStripe].PChannel = ssd->trip2Page[ssd->stripe.nowStripe].location->channel;
				
				ssd->stripe.nowStripe++;
						
				ssd->stripe.now = 0;
				if(++ssd->stripe.checkChange / 2 == 1 && ssd->stripe.checkChange % 2 == 0){
					if(++ssd->stripe.check >= (ssd->stripe.all))
						ssd->stripe.check = 0;
					ssd->stripe.checkChange = 0;
				}
			}	
		}
		++lpn;
	}

}
/********************************************************************************************************************************
 1，main函数中initiation()函数用来初始化ssd,；
 2，make_aged()函数使SSD成为aged，aged的ssd相当于使用过一段时间的ssd，里面有失效页，
    non_aged的ssd是新的ssd，无失效页，失效页的比例可以在初始化参数中设置；
 3，pre_process_page()函数提前扫一遍读请求，把读请求的lpn<--->ppn映射关系事先建立好，
   写请求的lpn<--->ppn映射关系在写的时候再建立，预处理trace防止读请求是读不到数据；
 4，simulate()是核心处理函数，trace文件从读进来到处理完成都由这个函数来完成；
 5，statistic_output()函数将ssd结构中的信息输出到输出文件，输出的是统计数据和平均数据，输出文件较小，trace_output文件则很大很详细；
 6，free_all_node()函数释放整个main函数中申请的节点
*********************************************************************************************************************************/
int  main(int argc, char* argv[])
{
	unsigned  int i,j,k;
	struct ssd_info *ssd;
	/*add by winks*/
	unsigned long mapSize = 0;
	/*end add*/

	#ifdef DEBUG
	printf("enter main\n"); 
	#endif

	ssd=(struct ssd_info*)malloc(sizeof(struct ssd_info));
	alloc_assert(ssd,"ssd");
	memset(ssd,0, sizeof(struct ssd_info));
	
	//***************************************************
	int sTIMES, speed_up;
	printf("Read parameters to the main function.\n");
	sscanf(argv[1], "%d", &speed_up);
	sscanf(argv[2], "%s", (char*)(ssd->tracefilename));
	printf("Running trace file: %s.\n", ssd->tracefilename);
	
	//*****************************************************
	printf("sizeof u int is %d\n", sizeof(unsigned int));
	ssd=initiation(ssd);
	printf("Chip_channel: %d, %d\n", ssd->parameter->chip_channel[0],ssd->parameter->chip_num);

	raid_init(ssd);//初始化raid
	if(ssd->parameter->allocation_scheme==0 && ssd->parameter->dynamic_allocation == 2)
		pre_process_page_raid(ssd);//执行这个raid预处理
	else
		pre_process_page(ssd);
	get_data_distribute(ssd);

	printf("free_lsb: %ld, free_csb: %ld, free_msb: %ld\n", ssd->free_lsb_count, ssd->free_csb_count, ssd->free_msb_count);
	printf("Total request num: %ld.\n", ssd->total_request_num);
	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		for (j=0;j<ssd->parameter->die_chip;j++)
		{
			for (k=0;k<ssd->parameter->plane_die;k++)
			{
				printf("%d,0,%d,%d:  %5d\n",i,j,k,ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);
			}
		}
	}
	printf("----------------------------------------\n");
	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		for(int w = 0; w < ssd->parameter->chip_channel[0]; ++w)
			for (j=0;j<ssd->parameter->die_chip;j++)
			{
				for (k=0;k<ssd->parameter->plane_die;k++)
				{
					printf("%d,%d,%d,%d:  %5d\n",i,w,j,k,ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);
				}
			}
	}
	fprintf(ssd->outputfile,"\t\t\t\t\t\t\t\t\tOUTPUT\n");
	fprintf(ssd->outputfile,"****************** TRACE INFO ******************\n");

	if(speed_up<=0){
		printf("Parameter ERROR.\n");
		return 0;
		}
	printf("RUN %d TIMES with %dx SPEED.\n", sTIMES,speed_up);
	if(ssd->parameter->fast_gc==1){
		printf("Fast GC is activated, %.2f:%.2f and %.2f:%.2f.\n",ssd->parameter->fast_gc_thr_1,ssd->parameter->fast_gc_filter_1,ssd->parameter->fast_gc_thr_2,ssd->parameter->fast_gc_filter_2);
		}
	printf("dr_switch: %d, dr_cycle: %d\n",ssd->parameter->dr_switch,ssd->parameter->dr_cycle);
	if(ssd->parameter->dr_switch==1){
		printf("Data Reorganization is activated, dr cycle is %d days.\n", ssd->parameter->dr_cycle);
		}
	printf("GC_hard threshold: %.2f.\n", ssd->parameter->gc_hard_threshold);
	ssd->speed_up = speed_up;
	ssd->cpu_current_state = CPU_IDLE;
	ssd->cpu_next_state_predict_time = -1;

	ssd=simulate(ssd);//模拟SSD运行

	statistic_output(ssd);  
	free_all_node(ssd);

	printf("\n");
	printf("the simulation is completed!\n");
	
	return 0;
}

/******************simulate() *********************************************************************
*simulate()是核心处理函数，主要实现的功能包括
*1, 从trace文件中获取一条请求，挂到ssd->request
*2，根据ssd是否有dram分别处理读出来的请求，把这些请求处理成为读写子请求，挂到ssd->channel或者ssd上
*3，按照事件的先后来处理这些读写子请求。
*4，输出每条请求的子请求都处理完后的相关信息到outputfile文件中
**************************************************************************************************/
struct ssd_info *simulate(struct ssd_info *ssd)
{
	int flag=1,flag1=0;
	double output_step=0;
	unsigned int a=0,b=0;

	printf("\n");
	printf("begin simulating.......................\n");
	printf("\n");
	printf("\n");
	printf("   ^o^    OK, please wait a moment, and enjoy music and coffee   ^o^    \n");

	ssd->tracefile = fopen(ssd->tracefilename,"r");
	if(ssd->tracefile == NULL)
	{  
		printf("the trace file can't open\n");
		return NULL;
	}

	fprintf(ssd->outputfile,"      arrive           lsn     size ope     begin time    response time    process time\n");	
	fflush(ssd->outputfile);

	while(flag!=100)      
	{
		flag=get_requests(ssd);
		if (flag == 0) {
			flag = 100;
			printf("end pre\n");
		}	
		if(flag == 1)
		{
			if (ssd->parameter->dram_capacity!=0)
			{
				buffer_management(ssd);  
				distribute(ssd);
			} 
			else
			{
				no_buffer_distribute(ssd);
			}		
		}

		if(ssd->cpu_current_state == CPU_IDLE || (ssd->cpu_next_state == CPU_IDLE && ssd->cpu_next_state_predict_time >= ssd->current_time)){
			ssd->cpu_current_state = CPU_IDLE;
			process(ssd);
		}

		trace_output(ssd);
		if (flag == 0) {
			flag = 100;
			printf("end last\n");
		}
			
	}

	fclose(ssd->tracefile);
	return ssd;
}


int update_time(struct ssd_info *ssd)  
{  
	unsigned long long nearest_event_time;

	nearest_event_time=find_nearest_event(ssd);
	if (nearest_event_time==MAX_INT64)
	{
		ssd->current_time += 1;    
		return 0;
	}
	else
	{
		if(ssd->current_time == nearest_event_time)
			ssd->current_time += 1;  
		else
			ssd->current_time = nearest_event_time;
		
		return 1;
	}	
}

unsigned long long get_req_num(struct ssd_info *ssd){
	unsigned long long count = 0;
	struct sub_request *sub = ssd->raidReq->subs;
	for(int j=0;j<ssd->parameter->channel_number;j++){
		sub = ssd->channel_head[j].subs_r_head;
		while(sub){
			++count;
			sub = sub->next_node;
		}
		sub = ssd->channel_head[j].subs_w_head;
		while(sub){
			++count;
			sub = sub->next_node;
		}
	}
	sub = ssd->subs_w_head;
	while(sub){
		++count;
		sub = sub->next_node;
	}
	
	return count;

}

unsigned long long get_req_num_read(struct ssd_info *ssd){
	unsigned long long count = 0;
	struct sub_request *sub = ssd->raidReq->subs;
	for(int j=0;j<ssd->parameter->channel_number;j++){
		sub = ssd->channel_head[j].subs_r_head;
		while(sub){
			++count;
			sub = sub->next_node;
		}
	}
	
	return count;

}


unsigned int get_ppn_base_abs(struct ssd_info *ssd, int channel, int chip, int die, int plane, int block, int page){
	unsigned int page_plane=0,page_die=0,page_chip=0,page_channel=0;
	page_plane=ssd->parameter->page_block*ssd->parameter->block_plane; //number of page per plane
	page_die=page_plane*ssd->parameter->plane_die;                     //number of page per die
	page_chip=page_die*ssd->parameter->die_chip;                       //number of page per chip
	page_channel=page_chip*ssd->parameter->chip_channel[0];            //number of page per channel

	return page + ssd->parameter->page_block * block + page_plane * plane +\
			page_die * die + page_chip * chip + page_channel * channel;
}

struct sub_request *get_sub_request_for_recovery(struct ssd_info *ssd, int channel, int chip, int die, int plane, int block, int page){
	struct sub_request* sub = (struct sub_request*)malloc(sizeof(struct sub_request));                        /*锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷慕峁?*/
	alloc_assert(sub,"sub_request");
	memset(sub,0, sizeof(struct sub_request));
	struct local *location = (struct local*)malloc(sizeof(struct local));
	alloc_assert(sub,"struct local");
	memset(location,0, sizeof(struct local));

	unsigned int mask=0;
	if(ssd->parameter->subpage_page == 32){
		mask = 0xffffffff;
	}else{
		mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	}
	
	location->channel = channel;
	location->chip = chip;
	location->die = die;
	location->plane = plane;
	location->block = block;
	location->page = page;
	
	sub->location = location;
	sub->lpn = ssd->stripe.checkLpn + 1;
	sub->size = size(mask);
	sub->ppn = get_ppn_base_abs(ssd, channel, chip, die, plane, block, page);
	sub->state=mask;
	sub->raidNUM = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].raidID;

	sub->next_subs = ssd->raidReq->subs;
	ssd->raidReq->subs = sub;
	
	return sub;
}


void create_recovery_sub_read(struct ssd_info *ssd, int channel, int chip, int die, int plane, int block, int page){
	struct sub_request* sub = get_sub_request_for_recovery(ssd, channel, chip, die, plane, block, page);
		
	sub->begin_time = ssd->current_time;
	sub->current_state = SR_WAIT;
	sub->current_time=MAX_INT64;
	sub->next_state = SR_R_C_A_TRANSFER;
	sub->next_state_predict_time=MAX_INT64;
	
	sub->operation = READ;

	struct channel_info *p_ch = &ssd->channel_head[channel];
	if (p_ch->subs_r_tail!=NULL){
		p_ch->subs_r_tail->next_node=sub;
		p_ch->subs_r_tail=sub;
	}else{
		p_ch->subs_r_head=sub;
		p_ch->subs_r_tail=sub;
	}
	
	
}

struct sub_request* create_recovery_sub_write(struct ssd_info *ssd, int channel, int chip, int die, int plane, int block, int page){
	unsigned int mask=0; 
	struct sub_request* sub = get_sub_request_for_recovery(ssd, channel, chip, die, plane, block, page);
	
	if(ssd->parameter->subpage_page == 32){
		mask = 0xffffffff;
	}else{
		mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	}
	sub->operation = WRITE;
	sub->current_state=SR_WAIT;
	sub->current_time=ssd->current_time;
	sub->begin_time = ssd->current_time;

	struct channel_info *p_ch = &ssd->channel_head[channel];
	if (p_ch->subs_w_tail!=NULL){
		p_ch->subs_w_tail->next_node=sub;
		p_ch->subs_w_tail=sub;
	}else{
		p_ch->subs_w_head=sub;
		p_ch->subs_w_tail=sub;
	}
	return sub;
}

void create_recovery_sub(struct ssd_info *ssd, int channel){
	for(int chipIndex = 0; chipIndex < ssd->parameter->chip_channel[0]; ++chipIndex){
		for(int dieIndex = 0; dieIndex < ssd->parameter->die_chip; ++dieIndex){
			for(int planeIndex = 0; planeIndex < ssd->parameter->plane_die; ++planeIndex){
				for(int blockIndex = 0; blockIndex < ssd->parameter->block_plane; ++blockIndex){
					for(int pageIndex = 0; pageIndex < ssd->parameter->page_block; ++pageIndex){
						long long raid = ssd->channel_head[channel].chip_head[chipIndex].die_head[dieIndex].plane_head[planeIndex].blk_head[blockIndex].page_head[pageIndex].raidID;
						if(raid != -1 && ssd->trip2Page[raid].using){
							int valid = ssd->channel_head[channel].chip_head[chipIndex].die_head[dieIndex].plane_head[planeIndex].blk_head[blockIndex].page_head[pageIndex].valid_state;
							ssd->needRecoveryAll++;
							if(valid == 0){
								ssd->needRecoveryInvalid++;
							}
							for(int i = 0; i < 3;i ++){
								int ppn = 0;
								if(ssd->trip2Page[raid].check[i] == VAIL_DRAID){
									ppn = ssd->dram->map->map_entry[ssd->trip2Page[raid].lpn[i]].pn;
								}else{
									abort();
								}
								struct local *location = find_location(ssd, ppn);
								alloc_assert(location, "create_recovery_sub");
								if(location->channel != channel){
									create_recovery_sub_read(ssd, location->channel, location->chip, location->die, location->plane, location->block, location->page);
								}else{
									create_recovery_sub_write(ssd, location->channel, location->chip, location->die, location->plane, location->block, location->page);
								}
								free(location);
							}
							struct local *location = ssd->trip2Page[raid].location;
							if(location->channel != channel){
								create_recovery_sub_read(ssd, location->channel, location->chip, location->die, location->plane, location->block, location->page);
							}else{
								create_recovery_sub_write(ssd, location->channel, location->chip, location->die, location->plane, location->block, location->page);
							}						
						}
					}
				}
			}
		}
	}
}


int get_recovery_max(struct ssd_info *ssd){
	int ret = 0;
	int pre = 0;
	long long allValid = 0;
	for(int channelIndex = 0; channelIndex < ssd->parameter->channel_number; channelIndex++){
		int now = 0;
		for(int chipIndex = 0; chipIndex < ssd->parameter->chip_channel[0]; ++chipIndex){
			for(int dieIndex = 0; dieIndex < ssd->parameter->die_chip; ++dieIndex){
				for(int planeIndex = 0; planeIndex < ssd->parameter->plane_die; ++planeIndex){
					for(int blockIndex = 0; blockIndex < ssd->parameter->block_plane; ++blockIndex){
						for(int pageIndex = 0; pageIndex < ssd->parameter->page_block; ++pageIndex){
							long long raid = ssd->channel_head[channelIndex].chip_head[chipIndex].die_head[dieIndex].plane_head[planeIndex].blk_head[blockIndex].page_head[pageIndex].raidID;
							if(raid != -1 && ssd->trip2Page[raid].using){
													
							}
							if(ssd->channel_head[channelIndex].chip_head[chipIndex].die_head[dieIndex].plane_head[planeIndex].blk_head[blockIndex].page_head[pageIndex].valid_state){
								allValid++;	
								now++;	
							}
						}
					}
				}
			}
		}

		printf("%d valid %d\n", channelIndex, now);
		
	}
	printf("all valid %lld\n", allValid);
	return ret;
}


struct ssd_info *simulate_for_recovery(struct ssd_info *ssd){
	while(1){
		if(!update_time(ssd)){
			if(get_req_num(ssd) == 0){
				int i = 0;
				for(; i < ssd->parameter->channel_number; ++i){
					if(ssd->channel_head[i].gc_command){
						break;
					}
				}
				if( i == ssd->parameter->channel_number){
					break;
				}
				
			}
		}
		if(ssd->cpu_current_state == CPU_IDLE || (ssd->cpu_next_state == CPU_IDLE && ssd->cpu_next_state_predict_time >= ssd->current_time)){
			ssd->cpu_current_state = CPU_IDLE;
			process(ssd);
		}
	}


	
	unsigned long long all = 0, entryCount = 0, exitCount = 0;

	long long i = 0;
	get_recovery_max(ssd);
	create_recovery_sub(ssd, 0);
	ssd->recoveryTime = ssd->current_time;
	//return  ssd;
	printf("shouled change %lld\n", get_req_num(ssd));
	//return ssd;
	while(get_req_num(ssd) > 0)      
	{
		update_time(ssd);
		if(++i % 1000 == 0){
			printf("i is %lld all %lld time %lld  read %lld\n", i,  get_req_num(ssd),ssd->current_time, get_req_num_read(ssd));
		}
		
		//if(ssd->cpu_current_state == CPU_IDLE || (ssd->cpu_next_state == CPU_IDLE && ssd->cpu_next_state_predict_time >= ssd->current_time)){
			ssd->cpu_current_state = CPU_IDLE;
			process(ssd);
		//}
		
		trace_output(ssd);
		
	}

	//fclose(ssd->tracefile);
	printf("all %lld entry %lld exit %lld\n", all, entryCount, exitCount);
	printf("%lld end clear.......................\n", ssd->current_time - ssd->recoveryTime);
	return ssd;
}


struct ssd_info *simulate_multiple(struct ssd_info *ssd, int sTIMES)
{
	int flag=1,flag1=0;
	double output_step=0;
	unsigned int a=0,b=0;
	//errno_t err;

	int simulate_times = 0;
	//int sTIMES = 10;
	printf("\n");
	printf("begin simulating.......................\n");
	printf("\n");
	printf("\n");
	printf("   ^o^    OK, please wait a moment, and enjoy music and coffee   ^o^    \n");
	if(ssd->parameter->turbo_mode == 2 || ssd->parameter->turbo_mode == 0){
		ssd->parameter->lsb_first_allocation=0;
		ssd->parameter->dr_switch=0;
		ssd->parameter->fast_gc=0;
		}
	else{
		ssd->parameter->lsb_first_allocation=1;
		//ssd->parameter->dr_switch=ssd->parameter->dr_switch;
		//ssd->parameter->fast_gc=1;
		}
	/************************IMPORTANT FACTOR************************************/
	//ssd->parameter->turbo_mode_factor = 100;
	fprintf(ssd->statisticfile, "requests            time       gc_count          r_lat          w_lat          w_lsb          w_csb          w_msb           f_gc       mov_page      free_lsb     r_req_count    w_req_count    same_pages       req_lsb       req_csb       req_msb       w_req_count_l    same_pages_l       req_lsb_l       req_csb_l       req_msb_l\n");
	while(simulate_times < sTIMES){
		//*********************************************************
		/*
		if(simulate_times<7){
			ssd->parameter->turbo_mode=0;
			ssd->parameter->dr_switch=0;
			ssd->parameter->fast_gc=0;
			}
		else{
			ssd->parameter->turbo_mode=1;
			ssd->parameter->dr_switch=1;
			ssd->parameter->fast_gc=1;
			}
		*/
		//*********************************************************
		ssd->tracefile = fopen(ssd->tracefilename,"r");
		if(ssd->tracefile == NULL){  
			printf("the trace file can't open\n");
			return NULL;
			}
		fseek(ssd->tracefile,0,SEEK_SET);
		printf("simulating %d times.\n", simulate_times);
		//printf("file point: %ld\n", ftell(ssd->tracefile));
		fprintf(ssd->outputfile,"      arrive           lsn     size ope     begin time    response time    process time\n");	
		fflush(ssd->outputfile);
		ssd->basic_time = ssd->current_time;
		flag = 1;
		//int trace_count = 0;
		//while(flag!=100 && trace_count < 4500000){
		while(flag!=100){
			flag=get_requests(ssd);

			//trace_count++;
			if(flag == 1){   
				//printf("once\n");
				if (ssd->parameter->dram_capacity!=0){
					buffer_management(ssd);  
					distribute(ssd); 
					} 
				else{
					no_buffer_distribute(ssd);
					}		
				}
			process(ssd);
			trace_output(ssd);
			//if(trace_count%100000 == 0){
			//	printf("trace_num: %d\n",trace_count);
			//	}
			if(flag == 0 && ssd->request_queue == NULL)
				flag = 100;
			if(ssd->completed_request_count > ((int)(ssd->total_request_num/10000))*10000*(simulate_times+1)){
				printf("It should be terminated.\n");
				flag = 100;
				}
			}
			
		fclose(ssd->tracefile);
		//statistic_output_easy(ssd, simulate_times);
		//ssd->newest_read_avg = 0;
		//ssd->newest_write_avg = 0;
		//ssd->newest_read_request_count = 0;
		//ssd->newest_write_request_count = 0;
		//ssd->newest_write_lsb_count = 0;
		//ssd->newest_write_msb_count = 0;
		simulate_times++;
		
		unsigned int channel;
		//if((ssd->parameter->dr_switch==1)&&(simulate_times)%(ssd->parameter->dr_cycle)==0){
		/*
		int this_day;
		this_day = (int)(ssd->current_time/1000000000/3600/24);
		if((ssd->parameter->dr_switch==1)&&(this_day>ssd->time_day)&&(this_day%ssd->parameter->dr_cycle==0)){
			ssd->time_day = this_day;
			for(channel=0;channel<ssd->parameter->channel_number;channel++){
				dr_for_channel(ssd, channel);
				}
			}
			*/
		}
	return ssd;
}

unsigned int  XOR_process(struct ssd_info * ssd, int size){
	unsigned addTime =  19000 / ssd->parameter->subpage_page * size;
	if(ssd->cpu_current_state == CPU_IDLE){
		ssd->cpu_current_state = CPU_BUSY;
		ssd->cpu_next_state = CPU_IDLE;
		ssd->cpu_next_state_predict_time = addTime + ssd->current_time;
	}else {
		ssd->cpu_next_state_predict_time +=  addTime;
	}
	return addTime;
}


/********    get_request    ******************************************************
*	1.get requests that arrived already
*	2.add those request node to ssd->reuqest_queue
*	return	0: reach the end of the trace
*			-1: no request has been added
*			1: add one request to list
*SSD模拟器有三种驱动方式:时钟驱动(精确，太慢) 事件驱动(本程序采用) trace驱动()，
*两种方式推进事件：channel/chip状态改变、trace文件请求达到。
*channel/chip状态改变和trace文件请求到达是散布在时间轴上的点，
*每次从当前状态到达下一个状态都要到达最近的一个状态，每到达一个点执行一次process
********************************************************************************/
int get_requests(struct ssd_info *ssd)  
{  
	char buffer[200];
	unsigned int lsn=0;
	int device,  size, ope,large_lsn, i = 0,j=0;
	int priority;
	struct request *request1;
	int flag = 1;
	long filepoint; 
	int64_t time_t = 0;
	int64_t nearest_event_time;    

	#ifdef DEBUG
	printf("enter get_requests,  current time:%lld\n",ssd->current_time);
	#endif

	if(feof(ssd->tracefile)) {
		return 0;
	}

	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile); 
	sscanf(buffer,"%lld %d %d %d %d %d",&time_t,&device,&lsn,&size,&ope,&priority);
	
	//printf("%lld\n", time_t);
    //=======================================
    priority = 1;   //强锟斤拷锟借定锟斤拷锟饺硷拷
    if(ssd->firstTime_ == 0){
        ssd->firstTime_ = time_t;
    }
    if(time_t > 0){
        time_t -= ssd->firstTime_;
        time_t = ssd->basic_time + time_t;
    }else{
        time_t = ssd->current_time;
    }
	//=======================================
	if ((device<0)&&(lsn<0)&&(size<0)&&(ope<0))
	{
		return 100;
	}
	if(size==0){
		size = 1;
		}
	if (lsn<ssd->min_lsn) 
		ssd->min_lsn=lsn;
	if (lsn>ssd->max_lsn)
		ssd->max_lsn=lsn;
	/******************************************************************************************************
	*锟较诧拷锟侥硷拷系统锟斤拷锟酵革拷SSD锟斤拷锟轿何讹拷写锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷郑锟絃SN锟斤拷size锟斤拷 LSN锟斤拷锟竭硷拷锟斤拷锟斤拷锟脚ｏ拷锟斤拷锟斤拷锟侥硷拷系统锟斤拷锟皆ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥达拷
	*锟斤拷锟秸硷拷锟斤拷一锟斤拷锟斤拷锟皆碉拷锟斤拷锟斤拷锟秸间。锟斤拷锟界，锟斤拷锟斤拷锟斤拷260锟斤拷6锟斤拷锟斤拷示锟斤拷锟斤拷锟斤拷要锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷为260锟斤拷锟竭硷拷锟斤拷锟斤拷锟斤拷始锟斤拷锟杰癸拷6锟斤拷锟斤拷锟斤拷锟斤拷
	*large_lsn: channel锟斤拷锟斤拷锟叫讹拷锟劫革拷subpage锟斤拷锟斤拷锟斤拷锟劫革拷sector锟斤拷overprovide系锟斤拷锟斤拷SSD锟叫诧拷锟斤拷锟斤拷锟斤拷锟叫的空间都锟斤拷锟皆革拷锟矫伙拷使锟矫ｏ拷
	*锟斤拷锟斤拷32G锟斤拷SSD锟斤拷锟斤拷锟斤拷10%锟侥空间保锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫ｏ拷锟斤拷锟皆筹拷锟斤拷1-provide
	***********************************************************************************************************/
	if(ssd->parameter->allocation_scheme==0 && ssd->parameter->dynamic_allocation == 2)
        //计算large_lsn考虑了overprovide25%
		large_lsn=(int)((ssd->parameter->subpage_page*ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*(ssd->parameter->chip_num))*(1-ssd->parameter->overprovide)*0.75);
	else
		large_lsn=(int)((ssd->parameter->subpage_page*ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*ssd->parameter->chip_num)*(1-ssd->parameter->overprovide));
	large_lsn = ssd->stripe.checkStart - 1;
	large_lsn *= ssd->parameter->subpage_page;
	lsn = lsn%large_lsn;

	nearest_event_time=find_nearest_event(ssd);
	if (nearest_event_time==MAX_INT64)
	{
		ssd->current_time=time_t;
	}
	else
	{   
		if(nearest_event_time<time_t)
		{
			/*******************************************************************************
			*锟截癸拷锟斤拷锟斤拷锟斤拷锟矫伙拷邪锟絫ime_t锟斤拷锟斤拷ssd->current_time锟斤拷锟斤拷trace锟侥硷拷锟窖讹拷锟斤拷一锟斤拷锟斤拷录锟截癸拷
			*filepoint锟斤拷录锟斤拷执锟斤拷fgets之前锟斤拷锟侥硷拷指锟斤拷位锟矫ｏ拷锟截癸拷锟斤拷锟侥硷拷头+filepoint锟斤拷
			*int fseek(FILE *stream, long offset, int fromwhere);锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷指锟斤拷stream锟斤拷位锟矫★拷
			*锟斤拷锟街达拷谐晒锟斤拷锟絪tream锟斤拷指锟斤拷锟斤拷fromwhere锟斤拷偏锟斤拷锟斤拷始位锟矫ｏ拷锟侥硷拷头0锟斤拷锟斤拷前位锟斤拷1锟斤拷锟侥硷拷尾2锟斤拷为锟斤拷准锟斤拷
			*偏锟斤拷offset锟斤拷指锟斤拷偏锟斤拷锟斤拷锟斤拷锟斤拷锟街节碉拷位锟矫★拷锟斤拷锟街达拷锟绞э拷锟?(锟斤拷锟斤拷offset锟斤拷锟斤拷锟侥硷拷锟斤拷锟斤拷锟斤拷小)锟斤拷锟津不改憋拷stream指锟斤拷锟轿伙拷谩锟?
			*锟侥憋拷锟侥硷拷只锟杰诧拷锟斤拷锟侥硷拷头0锟侥讹拷位锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷锟叫达拷锟侥硷拷锟斤拷式锟斤拷"r":锟斤拷只锟斤拷锟斤拷式锟斤拷锟侥憋拷锟侥硷拷	
			**********************************************************************************/
			fseek(ssd->tracefile,filepoint,0); 
			if(ssd->current_time<=nearest_event_time)
				ssd->current_time=nearest_event_time;
			return -1;
		}
		else
		{
			if (ssd->request_queue_length>=ssd->parameter->queue_length)
			{
				fseek(ssd->tracefile,filepoint,0);
				ssd->current_time=nearest_event_time;
				return -1;
			} 
			else
			{
				ssd->current_time=time_t;
			}
		}
	}

	if(time_t < 0)
	{
		printf("error!\n");
		while(1){}
	}

	if(feof(ssd->tracefile))
	{

		return 0;
	}

	request1 = (struct request*)malloc(sizeof(struct request));
	alloc_assert(request1,"request");
	memset(request1,0, sizeof(struct request));

	request1->completeFlag = 0;
	request1->all = 0;
	request1->now = 0;
	request1->time = time_t;
	request1->lsn = lsn;
	request1->size = size;
	request1->operation = ope;	
	request1->priority = priority;
	request1->begin_time = time_t;
	request1->response_time = 0;	
	request1->energy_consumption = 0;	
	request1->next_node = NULL;
	request1->distri_flag = 0;              // indicate whether this request has been distributed already
	request1->subs = NULL;
	request1->need_distr_flag = NULL;
	request1->complete_lsn_count=0;         //record the count of lsn served by buffer
	request1->MergeFlag = 0;
	filepoint = ftell(ssd->tracefile);		// set the file point
	ssd->counttmp1++;

	ssd->preRequestArriveTime[ssd->preRequestArriveTimeIndex] = time_t;
	ssd->preRequestArriveTimeIndex = (ssd->preRequestArriveTimeIndex + 1) % ssd->preRequestArriveTimeIndexMAX;
	//request1->operation = WRITE;
	
	if(request1->operation == 1 && lsn == 3686940 ) {
		printf("read!!!!\n");
	}
	//挂载request1到SSD下的request链表的尾部
	if(ssd->request_queue == NULL)          //The queue is empty
	{
		ssd->request_queue = request1;
		ssd->request_tail = request1;
		ssd->request_queue_length++;
	}
	else
	{			
		(ssd->request_tail)->next_node = request1;	
		ssd->request_tail = request1;			
		ssd->request_queue_length++;
	}

	if(ssd->request_queue_length > ssd->max_queue_depth){
		ssd->max_queue_depth = ssd->request_queue_length;
		}

	if (request1->operation==1)             //锟斤拷锟斤拷平锟斤拷锟斤拷锟斤拷锟叫? 1为锟斤拷 0为写
	{
		ssd->ave_read_size=(ssd->ave_read_size*ssd->read_request_count+request1->size)/(ssd->read_request_count+1);
	} 
	else
	{
		ssd->ave_write_size=(ssd->ave_write_size*ssd->write_request_count+request1->size)/(ssd->write_request_count+1);
	}

	
	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile);    //寻锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷牡锟斤拷锟绞憋拷锟?
	sscanf(buffer,"%lld %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	ssd->next_request_time=time_t;
	fseek(ssd->tracefile,filepoint,0);
	
	return 1;
}

/*unsigned char read_buffer(struct ssd_info *ssd, struct request *req){
	unsigned int lsn,lpn,last_lpn,first_lpn,state,full_page, mask;
	unsigned int sub_size=0;
	unsigned int sub_state=0;
	unsigned char ret = 0;
	
	lsn=req->lsn;
	lpn=req->lsn/ssd->parameter->subpage_page;
	last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;
	first_lpn=req->lsn/ssd->parameter->subpage_page;
	struct sub_request *sub=NULL;
	if(ssd->parameter->subpage_page == 32){
		mask = 0xffffffff;
	}else{
		mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	}
	if(req->operation==READ)        
	{	
		
		if((lpn * ssd->parameter->subpage_page + ssd->parameter->subpage_page) > (req->lsn+req->size)){
			sub = creat_sub_request(ssd,lpn,size(mask),mask,req,req->operation,0);
			sub->readXorFlag = 1;
			ret = 1;
		}	
		if(lpn != last_lpn){
			if((last_lpn * ssd->parameter->subpage_page + ssd->parameter->subpage_page) > (req->lsn+req->size)){
				sub = creat_sub_request(ssd,last_lpn,size(mask),mask,req,req->operation,0);
				sub->readXorFlag = 1;
				ret = 1;
			}
		}
		

		
	}
	return ret;
}*/

/**********************************************************************************************************************************************
*首先buffer是个写buffer，就是为写请求服务的，因为读flash的时间tR为20us，写flash的时间tprog为200us，所以为写服务更能节省时间
*  读操作：如果命中了buffer，从buffer读，不占用channel的I/O总线，没有命中buffer，从flash读，占用channel的I/O总线，但是不进buffer了
*  写操作：首先request分成sub_request子请求，如果是动态分配，sub_request挂到ssd->sub_request上，因为不知道要先挂到哪个channel的sub_request上
*           如果是静态分配则sub_request挂到channel的sub_request链上,同时不管动态分配还是静态分配sub_request都要挂到request的sub_request链上
*           因为每处理完一个request，都要在traceoutput文件中输出关于这个request的信息。处理完一个sub_request,就将其从channel的sub_request链
*           或ssd的sub_request链上摘除，但是在traceoutput文件输出一条后再清空request的sub_request链。
*           sub_request命中buffer则在buffer里面写就行了，并且将该sub_page提到buffer链头(LRU)，若没有命中且buffer满，则先将buffer链尾的sub_request
*           写入flash(这会产生一个sub_request写请求，挂到这个请求request的sub_request链上，同时视动态分配还是静态分配挂到channel或ssd的
*           sub_request链上),在将要写的sub_page写入buffer链头
***********************************************************************************************************************************************/
struct ssd_info *buffer_management(struct ssd_info *ssd)
{   
	unsigned int j,lsn,lpn,last_lpn,first_lpn,index,complete_flag=0, state,full_page;
	unsigned int flag=0,need_distb_flag,lsn_flag,flag1=1,active_region_flag=0;           
	struct request *new_request;
	struct buffer_group *buffer_node,key;
	unsigned int mask=0,offset1=0,offset2=0;

	#ifdef DEBUG
	printf("enter buffer_management,  current time:%lld\n",ssd->current_time);
	#endif
	ssd->dram->current_time=ssd->current_time;
	full_page=~(0xffffffff<<ssd->parameter->subpage_page);
	
	new_request=ssd->request_tail;
	lsn=new_request->lsn;
	lpn=new_request->lsn/ssd->parameter->subpage_page;
	last_lpn=(new_request->lsn+new_request->size-1)/ssd->parameter->subpage_page;
	first_lpn=new_request->lsn/ssd->parameter->subpage_page;

	new_request->need_distr_flag=(unsigned int*)malloc(sizeof(unsigned int)*((last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32+1));
	alloc_assert(new_request->need_distr_flag,"new_request->need_distr_flag");
	memset(new_request->need_distr_flag, 0, sizeof(unsigned int)*((last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32+1));
	
	if(new_request->operation==READ) 
	{		
		while(lpn<=last_lpn)      		
		{
			unsigned char readBufHit = 0;
			unsigned int readBitMAp = 0;
			unsigned int readHitMAp = 0;
			need_distb_flag=full_page;   
			key.group=lpn;
			//buffer_node= (struct buffer_group*)avlTreeFind(ssd->dram->buffer, (TREE_NODE *)&key);		// buffer node 
			
			buffer_node= (struct buffer_group*)hash_find(ssd->dram->buffer, (HASH_NODE *)&key);
			while((buffer_node!=NULL)&&(lsn<(lpn+1)*ssd->parameter->subpage_page)&&(lsn<=(new_request->lsn+new_request->size-1)))             			
			{      
				
				lsn_flag=full_page;
				mask=1 << (lsn%ssd->parameter->subpage_page);
				readBitMAp |= mask;
				if(mask> (1 <<31))
				{
					printf("the subpage number is larger than 32!add some cases");
					getchar(); 		   
				}
				else if((buffer_node->stored & mask)==mask)
				{
					flag=1;
					lsn_flag=lsn_flag&(~mask);
				}

				if(flag==1)				
				{	if(ssd->dram->buffer->buffer_head!=buffer_node)
					{		
						if(ssd->dram->buffer->buffer_tail==buffer_node)								
						{			
							buffer_node->LRU_link_pre->LRU_link_next=NULL;					
							ssd->dram->buffer->buffer_tail=buffer_node->LRU_link_pre;							
						}				
						else								
						{				
							buffer_node->LRU_link_pre->LRU_link_next=buffer_node->LRU_link_next;				
							buffer_node->LRU_link_next->LRU_link_pre=buffer_node->LRU_link_pre;								
						}								
						buffer_node->LRU_link_next=ssd->dram->buffer->buffer_head;
						ssd->dram->buffer->buffer_head->LRU_link_pre=buffer_node;
						buffer_node->LRU_link_pre=NULL;			
						ssd->dram->buffer->buffer_head=buffer_node;													
					}
					
					
					readHitMAp |= mask;
					readBufHit = 1;
					
					
					//ssd->dram->buffer->read_hit++;					
					new_request->complete_lsn_count++;											
				}		
				else if(flag==0){
						//ssd->dram->buffer->read_miss_hit++;
				}
				need_distb_flag=need_distb_flag&lsn_flag;
				
				flag=0;		
				lsn++;
		
			}	
			while((buffer_node == NULL)&&(lsn<(lpn+1)*ssd->parameter->subpage_page)&&(lsn<=(new_request->lsn+new_request->size-1))){
				mask=1 << (lsn%ssd->parameter->subpage_page);
				readBitMAp |= mask;
				lsn++;
				//ssd->dram->buffer->read_miss_hit++;
			}
			if(need_distb_flag){
				new_request->all++;
				new_request->now++;
				struct sub_request *sub = creat_sub_request(ssd,lpn,size(need_distb_flag), need_distb_flag,new_request,new_request->operation,0, ssd->page2Trip[lpn]);
			}
			
			
			if(readBufHit == 1){	
				ssd->dram->buffer->read_hit++;
				readBufHit = 0;
			}else{
				ssd->dram->buffer->read_miss_hit++;
			}
			
			index=(lpn-first_lpn)/(32/ssd->parameter->subpage_page); 			
			new_request->need_distr_flag[index]=new_request->need_distr_flag[index]|(need_distb_flag<<(((lpn-first_lpn)%(32/ssd->parameter->subpage_page))*ssd->parameter->subpage_page));	
			lpn++;
			
		}
	}  
	else if(new_request->operation==WRITE)
	{
		while(lpn<=last_lpn)           	
		{	
			need_distb_flag=full_page;
			mask=~(0xffffffff<<(ssd->parameter->subpage_page));
			state=mask;

			if(lpn==first_lpn)
			{
				offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-new_request->lsn);
				state=state&(0xffffffff<<offset1);
			}
			if(lpn==last_lpn)
			{
				offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(new_request->lsn+new_request->size));
				state=state&(~(0xffffffff<<offset2));
			}
			
			ssd=insert2buffer(ssd, lpn, state,NULL,new_request);
			lpn++;
		}
	}
	complete_flag = 1;
	for(j=0;j<=(last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32;j++)
	{
		if(new_request->need_distr_flag[j] != 0)
		{
			complete_flag = 0;
		}
	}
	if((complete_flag == 1) && (new_request->subs == NULL) && (new_request->all == 0))
	{
		if(new_request->operation==WRITE){
			new_request->begin_time=ssd->current_time;
			new_request->response_time=ssd->current_time+1000;
		}
	}

	return ssd;
}

/*****************************
*lpn锟斤拷ppn锟斤拷转锟斤拷锟斤拷
******************************/
unsigned int lpn2ppn(struct ssd_info *ssd,unsigned int lsn)
{
	int lpn, ppn;	
	struct entry *p_map = ssd->dram->map->map_entry;                //锟斤拷取映锟斤拷锟?
#ifdef DEBUG
	printf("enter lpn2ppn,  current time:%lld\n",ssd->current_time);
#endif
	lpn = lsn/ssd->parameter->subpage_page;			//subpage_page指一锟斤拷page锟叫帮拷锟斤拷锟斤拷锟斤拷页锟斤拷锟斤拷锟斤拷锟节诧拷锟斤拷锟侥硷拷锟叫匡拷锟斤拷锟借定
	ppn = (p_map[lpn]).pn;                     //锟竭硷拷页lpn锟斤拷锟斤拷锟斤拷页ppn锟斤拷映锟斤拷锟铰硷拷锟接筹拷锟斤拷锟斤拷
	return ppn;
}

/**********************************************************************************
*读请求分配子请求函数，这里只处理读请求，写请求已经在buffer_management()函数中处理了
*根据请求队列和buffer命中的检查，将每个请求分解成子请求，将子请求队列挂在channel上，
*不同的channel有自己的子请求队列
**********************************************************************************/
struct ssd_info *distribute(struct ssd_info *ssd)
{
	unsigned int start, end, first_lsn,last_lsn,lpn,flag=0,flag_attached=0,full_page;
	unsigned int j, k, sub_size;
	int i=0;
	struct request *req;
	struct sub_request *sub;
	unsigned int* complt;
	return ssd;
	#ifdef DEBUG
	printf("enter distribute,  current time:%lld\n",ssd->current_time);
	#endif
	full_page=~(0xffffffff<<ssd->parameter->subpage_page);

	req = ssd->request_tail;
	if(req->response_time != 0){
		return ssd;
	}
	if (req->operation==WRITE)
	{
		return ssd;
	}

	if(req != NULL)
	{
		
		if(req->distri_flag == 0)
		{
			if(req->complete_lsn_count != ssd->request_tail->size)
			{		
				first_lsn = req->lsn;				
				last_lsn = first_lsn + req->size;
				complt = req->need_distr_flag;
				start = first_lsn - first_lsn % ssd->parameter->subpage_page;
				end = (last_lsn/ssd->parameter->subpage_page + 1) * ssd->parameter->subpage_page;
				i = (end - start - 1)/32;	// ?

				while(i >= 0)
				{	

					for(j=0; j<32/ssd->parameter->subpage_page; j++)
					{	
						k = (complt[((end-start - 1)/32-i)] >>(ssd->parameter->subpage_page*j)) & full_page;	
						ssd->read2++;
						if (k !=0)
						{
							
							lpn = start/ssd->parameter->subpage_page+ ((end-start - 1)/32-i)*32/ssd->parameter->subpage_page + j;
							sub_size=transfer_size(ssd,k,lpn,req);
							
							if (sub_size==0) 
							{
								continue;
							}
							else
							{	
								ssd->read4++;
								sub=creat_sub_request(ssd,lpn,sub_size,0,req,req->operation,0, ssd->page2Trip[lpn]);
								if(sub_size != ssd->parameter->subpage_page)
									sub->readXorFlag = 1;
							}	
						}
					}
					i = i-1;
				}

			}
			else if(req->subs == NULL)
			{
				req->begin_time=ssd->current_time;
				req->response_time=ssd->current_time+1000;   
			}

		}
	}
	return ssd;
}

void delete_req_help_read(struct ssd_info *ssd, struct sub_request *tmp){
	struct sub_request *p = NULL;
	struct sub_request *sub = ssd->channel_head[tmp->location->channel].subs_r_head;
	int i = tmp->location->channel;
	while(sub){
		if(sub == tmp){
			if(sub!=ssd->channel_head[i].subs_r_head){		
				p->next_node=sub->next_node;
				if(sub == ssd->channel_head[i].subs_r_tail){
					ssd->channel_head[i].subs_r_tail = p;
				}
			}			
			else{	
				if (ssd->channel_head[i].subs_r_head!=ssd->channel_head[i].subs_r_tail){
					ssd->channel_head[i].subs_r_head=sub->next_node;
				}else{
					ssd->channel_head[i].subs_r_head=NULL;
					ssd->channel_head[i].subs_r_tail=NULL;
				}							
			}
			return;
		}
		p = sub;
		sub = sub->next_node;
	}

}

void delete_req(struct ssd_info *ssd ,struct request **pre_node, struct request **req){
	struct sub_request *tmp;
	while((*req)->subs!=NULL)
	{
		tmp = (*req)->subs;
		if(tmp->operation == READ){
			delete_req_help_read(ssd , tmp);
		}
		(*req)->subs = tmp->next_subs;
		if (tmp->update!=NULL)
		{
			delete_req_help_read(ssd , tmp->update);
			free(tmp->update->location);
			tmp->update->location=NULL;
			free(tmp->update);
			tmp->update=NULL;
		}
		free(tmp->location);
		tmp->location=NULL;
		free(tmp);
		tmp=NULL;
	}
				
	if( (*pre_node) == NULL)
	{
		if((*req)->next_node == NULL)
		{
			free((*req)->need_distr_flag);
			(*req)->need_distr_flag=NULL;
			free((*req));
			(*req) = NULL;
			ssd->request_queue = NULL;
			ssd->request_tail = NULL;
			ssd->request_queue_length--;
		}
		else
		{
			ssd->request_queue = (*req)->next_node;
			(*pre_node) = (*req);
			(*req) = (*req)->next_node;
			free((*pre_node)->need_distr_flag);
			(*pre_node)->need_distr_flag=NULL;
			free((*pre_node));
			(*pre_node) = NULL;
			ssd->request_queue_length--;
		}
	}
	else
	{
		if((*req)->next_node == NULL)
		{
			(*pre_node)->next_node = NULL;
			free((*req)->need_distr_flag);
			(*req)->need_distr_flag=NULL;
			free((*req));
			(*req) = NULL;
			ssd->request_tail = (*pre_node);	
			ssd->request_queue_length--;
		}
		else
		{
			(*pre_node)->next_node = (*req)->next_node;
			free((*req)->need_distr_flag);
			(*req)->need_distr_flag=NULL;
			free((*req));
			(*req) = (*pre_node)->next_node;
			ssd->request_queue_length--;
		}

	}
}

/**********************************************************************
*trace_output()锟斤拷锟斤拷锟斤拷锟斤拷每一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷缶锟絧rocess()锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
*锟斤拷印锟斤拷锟斤拷锟截碉拷锟斤拷锟叫斤拷锟斤拷锟給utputfile锟侥硷拷锟叫ｏ拷锟斤拷锟斤拷慕锟斤拷锟斤拷要锟斤拷锟斤拷锟叫碉拷时锟斤拷
**********************************************************************/
void trace_output(struct ssd_info* ssd){
	int flag = 1;	
	int64_t start_time, end_time;
	struct request *req, *pre_node;
	struct sub_request *sub, *tmp;

	unsigned int full_page;

#ifdef DEBUG
	printf("enter trace_output,  current time:%lld\n",ssd->current_time);
#endif
	int debug_0918 = 0;
	pre_node=NULL;
	req = ssd->request_queue;
	start_time = 0;
	end_time = 0;

	if(req == NULL)
		return;
	if(ssd->parameter->subpage_page == 32){
		full_page = 0xffffffff;
	}
	else{
		full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
	}
	
	struct sub_request *preCacheNode = NULL;
	req = ssd->raidReq;
	sub = req->subs;
	while(sub){
		//printf("%p %p\n", sub, ssd->raidReq->subs);
		if((sub->current_state == SR_COMPLETE)||((sub->next_state==SR_COMPLETE)&&(sub->next_state_predict_time<=ssd->current_time))){
			//printf("free \n");
			if(!preCacheNode){
				req->subs = sub->next_subs;
				free(sub->location);
				free(sub);
				sub = req->subs;
			}else {
				preCacheNode->next_subs = sub->next_subs;
				free(sub->location);
				free(sub);
				sub = preCacheNode->next_subs;
			}
		}else{	
			preCacheNode = sub;
			sub = sub->next_subs;
		}
	}
	
	req = ssd->request_queue;
	sub = req->subs;
	while(req != NULL)	
	{
		sub = req->subs;
		flag = 1;
		start_time = 0;
		end_time = 0;
		if(req->response_time != 0)
		{
			//printf("Rsponse time != 0?\n");
			fprintf(ssd->outputfile,"%16lld %10d %6d %2d %16lld %16lld %10lld\n",req->time,req->lsn, req->size, req->operation, req->begin_time, req->response_time, req->response_time-req->time);
			fflush(ssd->outputfile);
			ssd->completed_request_count++;
			if(ssd->completed_request_count%10000 == 0){
				printf("completed requests: %ld\n", ssd->completed_request_count);
				//statistic_output_easy(ssd, ssd->completed_request_count);
				ssd->newest_read_avg = 0;
				ssd->newest_write_avg = 0;
				ssd->newest_read_request_count = 0;
				ssd->newest_write_request_count = 0;
				ssd->newest_write_lsb_count = 0;
				ssd->newest_write_csb_count = 0;
				ssd->newest_write_msb_count = 0;
				ssd->newest_write_request_completed_with_same_type_pages = 0;
				//***************************************************************************
				int channel;
				int this_day;
				this_day = (int)(ssd->current_time/1000000000/3600/24);
				/*
				if(this_day>ssd->time_day){
					printf("Another Day begin, %d.\n", this_day);
					}
					*/
				if(this_day>ssd->time_day){
					printf("Day %d begin......\n", this_day);
					ssd->time_day = this_day;
					if((ssd->parameter->dr_switch==1)&&(this_day%ssd->parameter->dr_cycle==0)){
						for(channel=0;channel<ssd->parameter->channel_number;channel++){
							dr_for_channel(ssd, channel);
							}
						}
					/*
					if((ssd->parameter->turbo_mode==2)&&(this_day%7==3)){
						printf("Enter turbo-mode.....\n");
						ssd->parameter->lsb_first_allocation = 1;
						ssd->parameter->fast_gc = 1;
						}
					else if(ssd->parameter->turbo_mode==2){
						//printf("Exist turbo-mode....\n");
						ssd->parameter->lsb_first_allocation = 0;
						ssd->parameter->fast_gc = 0;
						}
						*/
					}
				//***************************************************************************
				}
			
			if(debug_0918){
				printf("completed requests: %ld\n", ssd->completed_request_count);
				}
			
			if(req->response_time-req->begin_time==0)
			{
				printf("the response time is 0?? \n");
				getchar();
			}

			if (req->operation==READ)
			{
				ssd->read_request_count++;
				ssd->read_avg=ssd->read_avg+(req->response_time-req->time);
				//===========================================
				ssd->newest_read_request_count++;
				ssd->newest_read_avg = ssd->newest_read_avg+(end_time-req->time);
				//===========================================
			} 
			else
			{
				ssd->write_request_count++;
				ssd->write_avg=ssd->write_avg+(req->response_time-req->time);
				//===========================================
				ssd->newest_write_request_count++;
				ssd->newest_write_avg = ssd->newest_write_avg+(end_time-req->time);
				ssd->last_write_lat = end_time-req->time;
				//--------------------------------------------
				int new_flag = 1;
				int origin;
				struct sub_request *next_sub_a;
				next_sub_a = req->subs;
				/*origin = next_sub_a->allocated_page_type;
				next_sub_a = next_sub_a->next_subs;
				while(next_sub_a!=NULL){
					if(next_sub_a->allocated_page_type != origin){
						new_flag = 0;
						break;
						}
					next_sub_a = next_sub_a->next_subs;
					}
				if(new_flag==1){
					ssd->newest_write_request_completed_with_same_type_pages++;
					if(origin==1){
						ssd->newest_msb_request_a++;
						}
					else{
						ssd->newest_lsb_request_a++;
						}
					}*/
				//-------------------------------------------

				//===========================================
			}

			if(pre_node == NULL)
			{
				if(req->next_node == NULL)
				{
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free(req);
					req = NULL;
					ssd->request_queue = NULL;
					ssd->request_tail = NULL;
					ssd->request_queue_length--;
				}
				else
				{
					ssd->request_queue = req->next_node;
					pre_node = req;
					req = req->next_node;
					free(pre_node->need_distr_flag);
					pre_node->need_distr_flag=NULL;
					free((void *)pre_node);
					pre_node = NULL;
					ssd->request_queue_length--;
				}
			}
			else
			{
				if(req->next_node == NULL)
				{
					pre_node->next_node = NULL;
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free(req);
					req = NULL;
					ssd->request_tail = pre_node;
					ssd->request_queue_length--;
				}
				else
				{
					pre_node->next_node = req->next_node;
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free((void *)req);
					req = pre_node->next_node;
					ssd->request_queue_length--;
				}
			}
		}
		else
		{
			//printf("Rsponse time = 0!\n");
			flag=0;
			while(sub != NULL)
			{
				if(start_time == 0)
					start_time = sub->begin_time;
				if(start_time > sub->begin_time)
					start_time = sub->begin_time;
				if(end_time < sub->complete_time)
					end_time = sub->complete_time;
				if((sub->current_state == SR_COMPLETE)||((sub->next_state==SR_COMPLETE)&&(sub->next_state_predict_time<=ssd->current_time)))	// if any sub-request is not completed, the request is not completed
				{
					flag = 1;
					sub = sub->next_subs;
				}
				else
				{
					flag=0;
					break;
				}
			}
			/*if((flag == 0) && (req->MergeFlag == 1) && (req->all == 0)){
				flag = 1;
			}
			if((flag == 0) &&(req->all != 0) && (req->now == 0)){
				flag = 1;
			}*/
			if((flag == 0) && (req->subs == NULL)){
				flag = 1;
			}
			if(flag == 1 && req->completeFlag == 0)
			{	
				/*if(req->all != 0 && req->now == 0){
					start_time = req->time;
					end_time = start_time + 1000;
				}else if(req->all == 0 && req->MergeFlag == 1){
					start_time = req->time;
					end_time = start_time + 1000;
				}*/
				if(req->subs == NULL){
					start_time = req->time;
					end_time = start_time + 1000;
				}
					
				req->completeFlag = 1;
				//fprintf(ssd->outputfile,"%10I64u %10u %6u %2u %16I64u %16I64u %10I64u\n",req->time,req->lsn, req->size, req->operation, start_time, end_time, end_time-req->time);
				fprintf(ssd->outputfile,"%16lld %10d %6d %2d %16lld %16lld %10lld",req->time,req->lsn, req->size, req->operation, start_time, end_time, end_time-req->time);
				fprintf(ssd->outputfile," %lu %u %u %u\n",ssd->direct_erase_count + ssd->normal_erase, 0, 0,ssd->moved_page_count);
				fflush(ssd->outputfile);
				ssd->completed_request_count++;
				if(ssd->completed_request_count%10000 == 0){
					printf("completed requests: %ld, max_queue_depth: %d, ", ssd->completed_request_count, ssd->max_queue_depth);
					printf("free_lsb: %ld, free_csb: %ld, free_msb: %ld\n", ssd->free_lsb_count, ssd->free_csb_count, ssd->free_msb_count);
					ssd->max_queue_depth = 0;
					statistic_output_easy(ssd, ssd->completed_request_count);
					ssd->newest_read_avg = 0;
					ssd->newest_write_avg = 0;
					ssd->newest_write_avg_l = 0;
					ssd->newest_read_request_count = 0;
					ssd->newest_write_request_count = 0;
					ssd->newest_write_lsb_count = 0;
					ssd->newest_write_csb_count = 0;
					ssd->newest_write_msb_count = 0;
					ssd->newest_write_request_completed_with_same_type_pages_l = 0;
					ssd->newest_write_request_num_l= 0;
					ssd->newest_req_with_lsb_l = 0;
					ssd->newest_req_with_csb_l = 0;
					ssd->newest_req_with_msb_l = 0;
					ssd->newest_write_request_completed_with_same_type_pages = 0;
					ssd->newest_req_with_lsb = 0;
					ssd->newest_req_with_csb = 0;
					ssd->newest_req_with_msb = 0;
					//***************************************************************************
					int channel;
					int this_day;
					this_day = (int)(ssd->current_time/1000000000/3600/24);
					if(this_day>ssd->time_day){
						printf("Day %d begin......\n", this_day);
						ssd->time_day = this_day;
						if((ssd->parameter->dr_switch==1)&&(this_day%ssd->parameter->dr_cycle==0)){
							for(channel=0;channel<ssd->parameter->channel_number;channel++){
								dr_for_channel(ssd, channel);
								}
							}
						/*
						if((ssd->parameter->turbo_mode==2)&&(this_day%2==1)){
							printf("Enter turbo-mode.....\n");
							ssd->parameter->lsb_first_allocation = 1;
							ssd->parameter->fast_gc = 1;
							}
						else{
							//printf("Exist turbo-mode....\n");
							ssd->parameter->lsb_first_allocation = 0;
							ssd->parameter->fast_gc = 0;
							}
							*/
						}
					//***************************************************************************
					}
				
				if(debug_0918){
					printf("completed requests: %ld\n", ssd->completed_request_count);
					}
				if(end_time-start_time==0)
				{
					printf("the response time is 0?? position 2\n");
					//getchar();
				}
				if (req->operation==READ)
				{
					ssd->read_request_count++;
					ssd->read_avg=ssd->read_avg+(end_time-req->time);
					//=============================================
					ssd->newest_read_request_count++;
					ssd->newest_read_avg = ssd->newest_read_avg+(end_time-req->time);
					//==============================================
				} 
				else
				{
					ssd->write_request_count++;
					ssd->write_avg=ssd->write_avg+(end_time-req->time);
					//=============================================
					ssd->newest_write_request_count++;
					ssd->newest_write_avg = ssd->newest_write_avg+(end_time-req->time);
					ssd->last_write_lat = end_time-req->time;
					ssd->last_ten_write_lat[ssd->write_lat_anchor] = end_time-req->time;
					ssd->write_lat_anchor = (ssd->write_lat_anchor+1)%10;
					
					//--------------------------------------------
					int new_flag = 1;
					int origin, actual_type;
					int num_of_sub = 1;
					/*struct sub_request *next_sub_a;
					next_sub_a = req->subs;
					origin = next_sub_a->allocated_page_type;
					actual_type = next_sub_a->allocated_page_type;
					next_sub_a = next_sub_a->next_subs;
					while(next_sub_a!=NULL){
						num_of_sub++;
						if(next_sub_a->allocated_page_type > actual_type){
							actual_type = next_sub_a->allocated_page_type;
							}
						if(next_sub_a->allocated_page_type != origin){
							new_flag = 0;
							}
						next_sub_a = next_sub_a->next_subs;
						}
					if(num_of_sub>1){
						ssd->write_request_count_l++;
						ssd->newest_write_request_num_l++;
						ssd->newest_write_avg_l = ssd->newest_write_avg_l+(end_time-req->time);
						ssd->write_avg_l = ssd->write_avg_l+(end_time-req->time);
						}
					if(new_flag==1){
						ssd->newest_write_request_completed_with_same_type_pages++;
						if(num_of_sub>1){
							ssd->newest_write_request_completed_with_same_type_pages_l++;
							}
						if(origin==1){
							ssd->newest_msb_request_a++;
							}
						else if(origin==0){
							ssd->newest_lsb_request_a++;
							}
						else{
							ssd->newest_csb_request_a++;
							}
						}
					if(actual_type==TARGET_LSB){
						ssd->newest_req_with_lsb++;
						if(num_of_sub>1){
							ssd->newest_req_with_lsb_l++;
							}
						}
					else if(actual_type==TARGET_CSB){
						ssd->newest_req_with_csb++;
						if(num_of_sub>1){
							ssd->newest_req_with_csb_l++;
							}
						}
					else{
						ssd->newest_req_with_msb++;
						if(num_of_sub>1){
							ssd->newest_req_with_msb_l++;
							}
						}*/
					//-------------------------------------------
					
					//==============================================
				}
				//if(req->now == req->all){
					delete_req(ssd, &pre_node, &req);
				//}
				
			}else if(flag == 1 && req->completeFlag == 1 && req->now == req->all){
				//delete_req(ssd, &pre_node, &req);
			}
			else
			{	
				pre_node = req;
				req = req->next_node;
			}
		}		
	}
}


/*******************************************************************************
*获取数据在不同通道的分布情况，通过统计每个通道上的数据块数量，并将其打印出来。
*******************************************************************************/
void get_data_distribute(struct ssd_info *ssd){
	unsigned long long *chanData = malloc(ssd->parameter->channel_number * sizeof(unsigned long long));
	memset(chanData, 0, ssd->parameter->channel_number * sizeof(unsigned long long));
	int page_num = ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*ssd->parameter->chip_num;
    //遍历所有页，检查这个页有物理位置，就增加相应通道的计数
	for(int i = 0; i < page_num; ++i){
		if(ssd->dram->map->map_entry[i].state != 0){
			struct local *location = find_location(ssd, ssd->dram->map->map_entry[i].pn);
			chanData[location->channel]++;
			free(location);
		}
	}
    //统计每个通道上的数据块数量
	for(int i = 0; i < ssd->parameter->channel_number; ++i){
		printf("%lld\t%lld\n", chanData[i], ssd->dataPlace[i]);
	}
	printf("\n");
	free(chanData);

}

void statistic_output(struct ssd_info *ssd)
{
	unsigned int lpn_count=0,i,j,k,m,p,erase=0,plane_erase=0;
	double gc_energy=0.0;
	extern float aveber ;
#ifdef DEBUG
	printf("enter statistic_output,  current time:%lld\n",ssd->current_time);
#endif

	for(i=0;i<ssd->parameter->channel_number;i++)
	{
		for(j=0;j<ssd->parameter->chip_channel[0];j++)
		{
			for(k=0;k<ssd->parameter->die_chip;k++)
			{
				for(p=0;p<ssd->parameter->plane_die;p++)
				{
					plane_erase=0;
					for(m=0;m<ssd->parameter->block_plane;m++)
					{
						if(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[p].blk_head[m].erase_count>0)
						{
							erase=erase+ssd->channel_head[i].chip_head[j].die_head[k].plane_head[p].blk_head[m].erase_count;
							plane_erase+=ssd->channel_head[i].chip_head[j].die_head[k].plane_head[p].blk_head[m].erase_count;
						}
					}
					fprintf(ssd->outputfile,"the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n",i,j,k,p,plane_erase);
					fprintf(ssd->statisticfile,"the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n",i,j,k,p,plane_erase);
				}
			}
		}
	}
	aveber = (index1 * 0.002013 + index2 * 0.000607) / (index1 + index2);
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"---------------------------statistic data---------------------------\n");	 
	fprintf(ssd->outputfile,"min lsn: %13d\n",ssd->min_lsn);	
	fprintf(ssd->outputfile,"max lsn: %13d\n",ssd->max_lsn);
	fprintf(ssd->outputfile,"read count: %13ld\n",ssd->read_count);	  
	fprintf(ssd->outputfile,"program count: %13ld",ssd->program_count);	
	fprintf(ssd->outputfile,"                        include the flash write count leaded by read requests\n");
	fprintf(ssd->outputfile,"the read operation leaded by un-covered update count: %13d\n",ssd->update_read_count);
	fprintf(ssd->outputfile,"erase count: %13ld\n",ssd->erase_count);
	fprintf(ssd->outputfile,"direct erase count: %13ld\n",ssd->direct_erase_count);
	fprintf(ssd->outputfile,"copy back count: %13ld\n",ssd->copy_back_count);
	fprintf(ssd->outputfile,"multi-plane program count: %13ld\n",ssd->m_plane_prog_count);
	fprintf(ssd->outputfile,"multi-plane read count: %13ld\n",ssd->m_plane_read_count);
	fprintf(ssd->outputfile,"interleave write count: %13ld\n",ssd->interleave_count);
	fprintf(ssd->outputfile,"interleave read count: %13ld\n",ssd->interleave_read_count);
	fprintf(ssd->outputfile,"interleave two plane and one program count: %13ld\n",ssd->inter_mplane_prog_count);
	fprintf(ssd->outputfile,"interleave two plane count: %13ld\n",ssd->inter_mplane_count);
	fprintf(ssd->outputfile,"gc copy back count: %13ld\n",ssd->gc_copy_back);
	fprintf(ssd->outputfile,"write flash count: %13ld\n",ssd->write_flash_count);
	//=================================================================
	fprintf(ssd->outputfile,"write LSB count: %13ld\n",ssd->write_lsb_count);
	fprintf(ssd->outputfile,"write MSB count: %13ld\n",ssd->write_msb_count);
	//=================================================================
	fprintf(ssd->outputfile,"interleave erase count: %13ld\n",ssd->interleave_erase_count);
	fprintf(ssd->outputfile,"multiple plane erase count: %13ld\n",ssd->mplane_erase_conut);
	fprintf(ssd->outputfile,"interleave multiple plane erase count: %13ld\n",ssd->interleave_mplane_erase_count);
	fprintf(ssd->outputfile,"read request count: %13d\n",ssd->read_request_count);
	fprintf(ssd->outputfile,"write request count: %13d\n",ssd->write_request_count);
	fprintf(ssd->outputfile,"read request average size: %13f\n",ssd->ave_read_size);
	fprintf(ssd->outputfile,"write request average size: %13f\n",ssd->ave_write_size);
	fprintf(ssd->outputfile,"read request average response time: %lld\n",ssd->read_avg/ssd->read_request_count);
	fprintf(ssd->outputfile,"write request average response time: %lld\n",ssd->write_avg/ssd->write_request_count);
	fprintf(ssd->outputfile,"buffer read hits: %13ld\n",ssd->dram->buffer->read_hit);
	fprintf(ssd->outputfile,"buffer read miss: %13ld\n",ssd->dram->buffer->read_miss_hit);
	fprintf(ssd->outputfile,"buffer write hits: %13ld\n",ssd->dram->buffer->write_hit);
	fprintf(ssd->outputfile,"buffer write miss: %13ld\n",ssd->dram->buffer->write_miss_hit);
	fprintf(ssd->outputfile,"erase: %13d\n",erase);
	fprintf(ssd->outputfile,"sub_request_all: %13ld, sub_request_success: %13ld\n", ssd->sub_request_all, ssd->sub_request_success);
	fflush(ssd->outputfile);

	fclose(ssd->outputfile);


	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"---------------------------statistic data---------------------------\n");
	fprintf(ssd->statisticfile,"page_move_count: %d\n",ssd->page_move_count);
	fprintf(ssd->statisticfile,"all need recovery and invalid: %lld  %lld\n",ssd->needRecoveryAll, ssd->needRecoveryInvalid);
	fprintf(ssd->statisticfile,"recover %lld \n", ssd->current_time - ssd->recoveryTime);
	fprintf(ssd->statisticfile,"wl_page_move_count: %d\n",ssd->wl_page_move_count);	
	fprintf(ssd->statisticfile,"rr_page_move_count: %d\n",ssd->rr_page_move_count);	
	fprintf(ssd->statisticfile,"min lsn: %13d\n",ssd->min_lsn);	
	fprintf(ssd->statisticfile,"max lsn: %13d\n",ssd->max_lsn);
	fprintf(ssd->statisticfile,"read count: %13ld\n",ssd->read_count);	  
	fprintf(ssd->statisticfile,"program count: %13ld",ssd->program_count);	  
	fprintf(ssd->statisticfile,"                        include the flash write count leaded by read requests\n");
	fprintf(ssd->statisticfile,"the read operation leaded by un-covered update count: %13d\n",ssd->update_read_count);
	fprintf(ssd->statisticfile,"wl_request: %13d\n",ssd->wl_request);
	fprintf(ssd->statisticfile,"erase count: %13ld\n",ssd->erase_count);	
	fprintf(ssd->statisticfile,"rr_erase count: %13d\n",ssd->rr_erase_count);
	fprintf(ssd->statisticfile,"wl_erase count: %13d\n",ssd->wl_erase_count);	
	fprintf(ssd->statisticfile,"normal_erase count: %13d\n",ssd->normal_erase);
	fprintf(ssd->statisticfile,"direct erase count: %13ld\n",ssd->direct_erase_count);
	fprintf(ssd->statisticfile,"copy back count: %13ld\n",ssd->copy_back_count);
	fprintf(ssd->statisticfile,"multi-plane program count: %13ld\n",ssd->m_plane_prog_count);
	fprintf(ssd->statisticfile,"multi-plane read count: %13ld\n",ssd->m_plane_read_count);
	fprintf(ssd->statisticfile,"interleave count: %13ld\n",ssd->interleave_count);
	fprintf(ssd->statisticfile,"interleave read count: %13ld\n",ssd->interleave_read_count);
	fprintf(ssd->statisticfile,"interleave two plane and one program count: %13ld\n",ssd->inter_mplane_prog_count);
	fprintf(ssd->statisticfile,"interleave two plane count: %13ld\n",ssd->inter_mplane_count);
	fprintf(ssd->statisticfile,"gc copy back count: %13ld\n",ssd->gc_copy_back);
	fprintf(ssd->statisticfile,"write flash count: %13ld\n",ssd->write_flash_count);
	//=================================================================
	fprintf(ssd->statisticfile,"write LSB count: %13ld\n",ssd->write_lsb_count);
	fprintf(ssd->statisticfile,"write CSB count: %13ld\n",ssd->write_csb_count);
	fprintf(ssd->statisticfile,"write MSB count: %13ld\n",ssd->write_msb_count);
	//=================================================================
	fprintf(ssd->statisticfile,"waste page count: %13ld\n",ssd->waste_page_count);
	fprintf(ssd->statisticfile,"interleave erase count: %13ld\n",ssd->interleave_erase_count);
	fprintf(ssd->statisticfile,"multiple plane erase count: %13ld\n",ssd->mplane_erase_conut);
	fprintf(ssd->statisticfile,"interleave multiple plane erase count: %13ld\n",ssd->interleave_mplane_erase_count);
	fprintf(ssd->statisticfile,"read request count: %13d\n",ssd->read_request_count);
	fprintf(ssd->statisticfile,"write request count: %13d\n",ssd->write_request_count);
	fprintf(ssd->statisticfile,"read request average size: %13f\n",ssd->ave_read_size);
	fprintf(ssd->statisticfile,"write request average size: %13f\n",ssd->ave_write_size);
	fprintf(ssd->statisticfile,"read request average response time: %llu\n",ssd->read_avg/((unsigned long long)ssd->read_request_count));
	fprintf(ssd->statisticfile,"write request average response time: %llu\n",ssd->write_avg/((unsigned long long)ssd->write_request_count));
	fprintf(ssd->statisticfile,"I/O response time: %llu\n",ssd->write_avg + ssd->read_avg);
	if(ssd->write_request_count_l==0){
		fprintf(ssd->statisticfile,"large write request average response time: 0\n");
		}
	else{
		fprintf(ssd->statisticfile,"large write request average response time: %llu\n",ssd->write_avg_l/((unsigned long long)ssd->write_request_count_l));
		}
	fprintf(ssd->statisticfile,"buffer read hits: %13ld\n",ssd->dram->buffer->read_hit);
	fprintf(ssd->statisticfile,"buffer read miss: %13ld\n",ssd->dram->buffer->read_miss_hit);
	fprintf(ssd->statisticfile,"buffer write hits: %13ld\n",ssd->dram->buffer->write_hit);
	fprintf(ssd->statisticfile,"buffer write miss: %13ld\n",ssd->dram->buffer->write_miss_hit);
	fprintf(ssd->statisticfile,"buffer write free: %13ld\n",ssd->dram->buffer->write_free);
	fprintf(ssd->statisticfile,"buffer eject: %13ld\n",ssd->dram->buffer->eject);
	fprintf(ssd->statisticfile,"erase: %13d\n",erase);

	fprintf(ssd->statisticfile, "RRcount: %13d\n", RRcount);

	fprintf(ssd->statisticfile,"sub_request_all: %13ld, sub_request_success: %13ld\n", ssd->sub_request_all, ssd->sub_request_success);
	fprintf(ssd->statisticfile, "index1: %13d\n", index1);
	fprintf(ssd->statisticfile, "index2: %13d\n", index2);
	fprintf(ssd->statisticfile, "aveber: %9f\n", aveber);
	fprintf(ssd->statisticfile,"\n\n\n");

	double available_capacity=0;
	
	for (i = 0; i < ssd->parameter->channel_number; i++)
	{	
		int validData = 0;
		for (j = 0; j < ssd->parameter->chip_num / ssd->parameter->channel_number; j++)
		{
			for (k = 0; k < ssd->parameter->die_chip; k++)
			{
				for (m = 0; m < ssd->parameter->plane_die; m++)
				{
					available_capacity += ssd->channel_head[i].chip_head[j].die_head[k].plane_head[m].free_page;
					
				}
			}
		}
		printf("%d\n", validData);
	}
	printf("available capacity:%f\n", available_capacity/(ssd->parameter->channel_number*ssd->parameter->chip_channel[0]*ssd->parameter->die_chip*ssd->parameter->plane_die*ssd->parameter->block_plane*ssd->parameter->page_block)); 
	printf("%lld\t%lld\n", ssd->raidWriteCount[0], ssd->raidWriteCount[1]);
	
	fprintf(ssd->statisticfile,"number of vaild page be changed: %ld\n",ssd->invaild_page_of_change);
	ssd->invaild_page_of_all += ssd->invaild_page_of_change;
	fprintf(ssd->statisticfile,"number of vaild page when gc be created: %ld\n",ssd->invaild_page_of_all);
	fprintf(ssd->statisticfile,"rate is : %lf\n", ((double)(ssd->invaild_page_of_change)) / ((double)(ssd->invaild_page_of_all)));
	fprintf(ssd->statisticfile,"number of frequently write: %ld\n",ssd->frequently_count);
	fprintf(ssd->statisticfile,"number of not frequently write: %ld\n",ssd->Nofrequently_count);
	fprintf(ssd->statisticfile,"when ssdsim finish the number of ssd->request: %d\n",ssd->gc_request);
	fflush(ssd->statisticfile);
	printf("%lld %lld %lld %lld\n",  ssd->read1, ssd->read2, ssd->read3, ssd->read4);
	fprintf(ssd->statisticfile,"parityChange %lld\n", ssd->parityChange);

	fprintf(ssd->statisticfile,"all gcInterval128 %lld\n", ssd->gcInterval128);
	fprintf(ssd->statisticfile,"gcInterval128 %lld\n", ssd->gcInterval128 / 128);
	fprintf(ssd->statisticfile,"all  gcInterval256 %lld\n", ssd->gcInterval256);
	fprintf(ssd->statisticfile,"gcInterval256 %lld\n", ssd->gcInterval256 / 256);
	
	printf("pageMoveRaid %lld\n", ssd->pageMoveRaid);
	printf("parityChange %lld\n", ssd->parityChange);
	fprintf(ssd->statisticfile,"write\n");
	for(i = 0; i < ssd->parameter->chip_channel[0] * ssd->parameter->channel_number; ++i){
		fprintf(ssd->statisticfile,"%lld\t", ssd->chipWrite[i]);
	}
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"gc\n");
	for(i = 0; i < ssd->parameter->chip_channel[0] * ssd->parameter->channel_number; ++i){
		fprintf(ssd->statisticfile,"%lld\t", ssd->chipGc[i]);
	}
	fprintf(ssd->statisticfile,"\nchannelWorkload:\n");
	for(i = 0; i < ssd->parameter->channel_number ; ++i){
		fprintf(ssd->statisticfile, "%lld\t", ssd->channelWorkload[i]);	
	}
	printf("\nget ppn %d\n", ssd->getPpnCount);
	printf("avg block %f\n", ((double)ssd->allBlockReq) / ssd->erase_count);
	fprintf(ssd->statisticfile,"\navg block %f\n", ((double)ssd->allBlockReq) / ssd->erase_count);
	fprintf(ssd->statisticfile,"\n");
	fclose(ssd->statisticfile);
	fclose(ssd->raidOutfile);
	fclose(ssd->gcIntervalFile);
	fclose(ssd->gcCreateRequest);
	get_data_distribute(ssd);
	printf("\n");
}

void statistic_output_easy(struct ssd_info *ssd, unsigned long completed_requests_num){
	unsigned int lpn_count=0,i,j,k,m,erase=0,plane_erase=0;
	double gc_energy=0.0;
#ifdef DEBUG
	fprintf(ssd->debugfile,"enter statistic_output,  current time:%lld\n",ssd->current_time);
	//printf("enter statistic_output,  current time:%lld\n",ssd->current_time);
#endif
	unsigned long read_avg_lat, write_avg_lat, write_avg_lat_l;
	if(ssd->newest_read_request_count==0){
		read_avg_lat=0;
		}
	else{
		read_avg_lat=ssd->newest_read_avg/ssd->newest_read_request_count;
		}
	if(ssd->newest_write_request_count==0){
		write_avg_lat=0;
		}
	else{
		write_avg_lat=ssd->newest_write_avg/ssd->newest_write_request_count;
		}
	if(ssd->newest_write_request_num_l==0){
		write_avg_lat_l=0;
		}
	else{
		write_avg_lat_l = ssd->newest_write_avg_l/ssd->newest_write_request_num_l;
		}
	fprintf(ssd->statisticfile, "%ld, %16lld, %13ld, %13ld, %13ld, %13ld, %13ld, %13ld, ", completed_requests_num, ssd->current_time, ssd->erase_count, read_avg_lat, write_avg_lat,ssd->newest_write_lsb_count,ssd->newest_write_csb_count,ssd->newest_write_msb_count);
	fprintf(ssd->statisticfile, "%13ld, %13d, %13ld, %13d, %13d, ", ssd->fast_gc_count, ssd->moved_page_count, ssd->free_lsb_count, ssd->newest_read_request_count, ssd->newest_write_request_count);
	fprintf(ssd->statisticfile, "%13d, %13d, %13d, %13d, ", ssd->newest_write_request_completed_with_same_type_pages, ssd->newest_req_with_lsb, ssd->newest_req_with_csb, ssd->newest_req_with_msb);
	fprintf(ssd->statisticfile, "% 13d, %13d, %13d, %13d, %13d, %13ld\n", ssd->newest_write_request_num_l, ssd->newest_write_request_completed_with_same_type_pages_l, ssd->newest_req_with_lsb_l, ssd->newest_req_with_csb_l, ssd->newest_req_with_msb_l, write_avg_lat_l);
	
	//fprintf(ssd->statisticfile, "%13d, %13d, %13d\n", ssd->newest_write_request_completed_with_same_type_pages, ssd->newest_write_lsb_count, ssd->newest_write_msb_count);
	//fprintf(ssd->statisticfile,"\n\n");
	fflush(ssd->statisticfile);
}


/***********************************************************************************
*锟斤拷锟斤拷每一页锟斤拷状态锟斤拷锟斤拷锟矫恳伙拷锟揭拷锟斤拷锟斤拷锟斤拷锟揭筹拷锟斤拷锟侥匡拷锟揭诧拷锟斤拷锟揭伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟揭拷锟斤拷锟斤拷锟斤拷锟揭筹拷锟揭筹拷锟?
************************************************************************************/
unsigned int size(unsigned int stored)
{
	unsigned int i,total=0,mask=0x80000000;

	#ifdef DEBUG
	printf("enter size\n");
	#endif
	for(i=1;i<=32;i++)
	{
		if(stored & mask) total++;
		stored<<=1;
	}
	#ifdef DEBUG
	    printf("leave size\n");
    #endif
    return total;
}


/*********************************************************
*transfer_size()锟斤拷锟斤拷锟斤拷锟斤拷锟矫撅拷锟角硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷size
*锟斤拷锟斤拷锟叫碉拷锟斤拷锟斤拷锟斤拷锟斤拷first_lpn锟斤拷last_lpn锟斤拷锟斤拷锟斤拷锟截憋拷锟斤拷锟斤拷锟斤拷锟轿拷锟?
*锟斤拷锟斤拷锟斤拷锟斤拷潞锟斤拷锌锟斤拷懿锟斤拷谴锟斤拷锟揭伙拷锟揭筹拷锟斤拷谴锟斤拷锟揭灰筹拷锟揭伙拷锟斤拷郑锟斤拷锟?
*为lsn锟叫匡拷锟杰诧拷锟斤拷一页锟侥碉拷一锟斤拷锟斤拷页锟斤拷
*********************************************************/
unsigned int transfer_size(struct ssd_info *ssd,int need_distribute,unsigned int lpn,struct request *req)
{
	unsigned int first_lpn,last_lpn,state,trans_size;
	unsigned int mask=0,offset1=0,offset2=0;

	first_lpn=req->lsn/ssd->parameter->subpage_page;
	last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;

	mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	state=mask;
	if(lpn==first_lpn)
	{
		offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-req->lsn);
		state=state&(0xffffffff<<offset1);
	}
	if(lpn==last_lpn)
	{
		offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(req->lsn+req->size));
		state=state&(~(0xffffffff<<offset2));
	}

	trans_size=size(state&need_distribute);

	return trans_size;
}


/**********************************************************************************************************  
*int64_t find_nearest_event(struct ssd_info *ssd)       
*寻锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷绲斤拷锟斤拷锟铰革拷状态时锟斤拷,锟斤拷锟饺匡拷锟斤拷锟斤拷锟斤拷锟揭伙拷锟阶刺憋拷洌拷锟斤拷锟斤拷锟斤拷锟斤拷赂锟阶刺憋拷锟叫★拷诘锟斤拷诘锟角笆憋拷洌?
*说锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟介看channel锟斤拷锟竭讹拷应die锟斤拷锟斤拷一状态时锟戒。Int64锟斤拷锟叫凤拷锟斤拷 64 位锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵ｏ拷值锟斤拷锟酵憋拷示值锟斤拷锟斤拷
*-2^63 ( -9,223,372,036,854,775,808)锟斤拷2^63-1(+9,223,372,036,854,775,807 )之锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷娲拷占锟秸? 8 锟街节★拷
*channel,die锟斤拷锟铰硷拷锟斤拷前锟狡斤拷锟侥关硷拷锟斤拷锟截ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞癸拷录锟斤拷锟斤拷锟斤拷锟角帮拷平锟斤拷锟絚hannel锟斤拷die锟街憋拷氐锟絠dle状态锟斤拷die锟叫碉拷
*锟斤拷锟斤拷锟斤拷准锟斤拷锟斤拷锟斤拷
***********************************************************************************************************/
int64_t find_nearest_event(struct ssd_info *ssd) 
{
	unsigned int i,j;
	int64_t time=MAX_INT64;
	int64_t time1=MAX_INT64;
	int64_t time2=MAX_INT64;

	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		if (ssd->channel_head[i].next_state==CHANNEL_IDLE)
			if(time1>ssd->channel_head[i].next_state_predict_time)
				if (ssd->channel_head[i].next_state_predict_time>ssd->current_time)    
					time1=ssd->channel_head[i].next_state_predict_time; //next state time
		for (j=0;j<ssd->parameter->chip_channel[i];j++)
		{
			if ((ssd->channel_head[i].chip_head[j].next_state==CHIP_IDLE)||(ssd->channel_head[i].chip_head[j].next_state==CHIP_DATA_TRANSFER))
				if(time2>ssd->channel_head[i].chip_head[j].next_state_predict_time)
					if (ssd->channel_head[i].chip_head[j].next_state_predict_time>ssd->current_time)    
						time2=ssd->channel_head[i].chip_head[j].next_state_predict_time;	
		}   
	} 

	/*****************************************************************************************************
	 *time为锟斤拷锟斤拷 A.锟斤拷一状态为CHANNEL_IDLE锟斤拷锟斤拷一状态预锟斤拷时锟斤拷锟斤拷锟絪sd锟斤拷前时锟斤拷锟紺HANNEL锟斤拷锟斤拷一状态预锟斤拷时锟斤拷
	 *           B.锟斤拷一状态为CHIP_IDLE锟斤拷锟斤拷一状态预锟斤拷时锟斤拷锟斤拷锟絪sd锟斤拷前时锟斤拷锟紻IE锟斤拷锟斤拷一状态预锟斤拷时锟斤拷
	 *		     C.锟斤拷一状态为CHIP_DATA_TRANSFER锟斤拷锟斤拷一状态预锟斤拷时锟斤拷锟斤拷锟絪sd锟斤拷前时锟斤拷锟紻IE锟斤拷锟斤拷一状态预锟斤拷时锟斤拷
	 *CHIP_DATA_TRANSFER锟斤拷准锟斤拷锟斤拷状态锟斤拷锟斤拷锟斤拷锟窖从斤拷锟绞达拷锟斤拷锟斤拷register锟斤拷锟斤拷一状态锟角达拷register锟斤拷锟斤拷buffer锟叫碉拷锟斤拷小值 
	 *注锟斤拷锟斤拷芏锟矫伙拷锟斤拷锟斤拷锟揭拷锟斤拷time锟斤拷锟斤拷时time锟斤拷锟斤拷0x7fffffffffffffff 锟斤拷
	*****************************************************************************************************/
	time=(time1>time2)?time2:time1;
	if(ssd->cpu_current_state != CPU_IDLE && ssd->cpu_next_state_predict_time <= time)
			time = ssd->cpu_next_state_predict_time;
	return time;
}

/***********************************************
*free_all_node()锟斤拷锟斤拷锟斤拷锟斤拷锟矫撅拷锟斤拷锟酵凤拷锟斤拷锟斤拷锟斤拷锟斤拷慕诘锟?
************************************************/
void free_all_node(struct ssd_info *ssd)
{
	unsigned int i,j,k,l,n;
	struct buffer_group *pt=NULL;
	struct direct_erase * erase_node=NULL;
	unsigned long long StripeNum =  ssd->parameter->page_block * ssd->parameter->block_plane * ssd->parameter->plane_die * ssd->parameter->die_chip;
	unsigned long long chipNum = ssd->parameter->chip_channel[0] * ssd->parameter->channel_number;
	printf("free_all_node\n");
	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		for (j=0;j<ssd->parameter->chip_channel[0];j++)
		{
			for (k=0;k<ssd->parameter->die_chip;k++)
			{
				for (l=0;l<ssd->parameter->plane_die;l++)
				{
					for (n=0;n<ssd->parameter->block_plane;n++)
					{
						free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head);
						ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head=NULL;
					}
					free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head);
					ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head=NULL;
					while(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node!=NULL)
					{
						erase_node=ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node;
						ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node=erase_node->next_node;
						free(erase_node);
						erase_node=NULL;
					}
				}
				
				free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head);
				ssd->channel_head[i].chip_head[j].die_head[k].plane_head=NULL;
			}
			free(ssd->channel_head[i].chip_head[j].die_head);
			ssd->channel_head[i].chip_head[j].die_head=NULL;
		}
		free(ssd->channel_head[i].chip_head);
		ssd->channel_head[i].chip_head=NULL;
	}
	free(ssd->channel_head);
	ssd->channel_head=NULL;
	
	
	//avlTreeDestroy( ssd->dram->buffer);
	hash_destroy(ssd->dram->buffer);
	ssd->dram->buffer=NULL;

	
	for(i = 0; i < StripeNum; i++){
		free(ssd->trip2Page[i].lpn);
		free(ssd->trip2Page[i].check);		
	}
	free(ssd->preRequestArriveTime);
	free(ssd->chipWrite);
	free(ssd->trip2Page);
	free(ssd->page2Trip);
	free(ssd->stripe.req);
	free(ssd->raidReq);
	free(ssd->dram->map->map_entry);
	ssd->dram->map->map_entry=NULL;
	free(ssd->dram->map);
	ssd->dram->map=NULL;
	free(ssd->dram);
	ssd->dram=NULL;
	free(ssd->parameter);
	ssd->parameter=NULL;
	
	free(ssd);
	ssd=NULL;
}


/*****************************************************************************
*make_aged()锟斤拷锟斤拷锟斤拷锟斤拷锟矫撅拷锟斤拷模锟斤拷锟斤拷实锟斤拷锟矫癸拷一锟斤拷时锟斤拷锟絪sd锟斤拷
*锟斤拷么锟斤拷锟絪sd锟斤拷锟斤拷应锟侥诧拷锟斤拷锟斤拷要锟侥变，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞碉拷锟斤拷暇锟斤拷嵌锟絪sd锟叫革拷锟斤拷锟斤拷锟斤拷锟侥革拷值锟斤拷
******************************************************************************/
struct ssd_info *make_aged(struct ssd_info *ssd)
{
	unsigned int i,j,k,l,m,n,ppn;
	int threshould,flag=0;
    
	if (ssd->parameter->aged==1)
	{
		//threshold锟斤拷示一锟斤拷plane锟斤拷锟叫讹拷锟斤拷页锟斤拷要锟斤拷前锟斤拷为失效
		threshould=(int)(ssd->parameter->block_plane*ssd->parameter->page_block*ssd->parameter->aged_ratio);  
		for (i=0;i<ssd->parameter->channel_number;i++)
			for (j=0;j<ssd->parameter->chip_channel[i];j++)
				for (k=0;k<ssd->parameter->die_chip;k++)
					for (l=0;l<ssd->parameter->plane_die;l++)
					{  
						flag=0;
						for (m=0;m<ssd->parameter->block_plane;m++)
						{  
							if (flag>=threshould)
							{
								break;
							}
							for (n=0;n<(ssd->parameter->page_block*ssd->parameter->aged_ratio+1);n++)
							{  
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].valid_state=0;        //锟斤拷示某一页失效锟斤拷同时锟斤拷锟絭alid锟斤拷free状态锟斤拷为0
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].free_state=0;         //锟斤拷示某一页失效锟斤拷同时锟斤拷锟絭alid锟斤拷free状态锟斤拷为0
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].lpn=0;  //锟斤拷valid_state free_state lpn锟斤拷锟斤拷为0锟斤拷示页失效锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷锟筋都锟斤拷猓拷锟斤拷锟絣pn=0锟斤拷锟斤拷锟斤拷锟斤拷效页
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_page_num--;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].invalid_page_num++;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_page++;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_page--;
								flag++;
								if(n%3==0){
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_lsb=n;
									ssd->free_lsb_count--;
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_lsb_num--;
									//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
									ssd->write_lsb_count++;
									ssd->newest_write_lsb_count++;
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_lsb_num--;
									}
								else if(n%3==2){
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_msb=n;
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_msb_num--;
									//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
									ssd->write_msb_count++;
									ssd->free_msb_count--;
									ssd->newest_write_msb_count++;
									}
								else{
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_csb=n;
									ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_csb_num--;
									ssd->write_csb_count++;
									ssd->free_csb_count--;
									ssd->newest_write_csb_count++;
									}
								ppn=find_ppn(ssd,i,j,k,l,m,n);
							
							}
						} 
					}	 
	}  
	else
	{
		return ssd;
	}

	return ssd;
}

int get_old_zwh(struct ssd_info *ssd){
	int cn_id, cp_id, di_id, pl_id;
	printf("Enter get_old_zwh.\n");
	for(cn_id=0;cn_id<ssd->parameter->channel_number;cn_id++){
		//printf("channel %d\n", cn_id);
		for(cp_id=0;cp_id<ssd->parameter->chip_channel[0];cp_id++){
			//printf("chip %d\n", cp_id);
			for(di_id=0;di_id<ssd->parameter->die_chip;di_id++){
				//printf("die %d\n", di_id);
				for(pl_id=0;pl_id<ssd->parameter->plane_die;pl_id++){
					//printf("channel %d, chip %d, die %d, plane %d: ", cn_id, cp_id, di_id, pl_id);
					int active_block, ppn, lpn;
					struct local *location;
					lpn = 0;
					while(ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].free_page > (ssd->parameter->page_block*ssd->parameter->block_plane)*0.3){
						//if(cn_id==0&&cp_id==2&&di_id==0&&pl_id==0){
						//	printf("cummulating....\n");
						//	}
						if(find_active_block(ssd,cn_id,cp_id,di_id,pl_id)==FAILURE)
							{
								printf("Wrong in get_old_zwh, find_active_block\n");	
								return 0;
							}
						active_block=ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].active_block;
						if(write_page(ssd,cn_id,cp_id,di_id,pl_id,active_block,&ppn)==ERROR)
							{
								return 0;
							}
						location=find_location(ssd,ppn);
						ssd->program_count++;
						ssd->channel_head[cn_id].program_count++;
						ssd->channel_head[cn_id].chip_head[cp_id].program_count++;	
						ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].blk_head[active_block].page_head[location->page].lpn=0;
						ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].blk_head[active_block].page_head[location->page].valid_state=0;
						ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].blk_head[active_block].page_head[location->page].free_state=0;
						ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].blk_head[active_block].invalid_page_num++;
						free(location);
						location=NULL;
						}
					//printf("%d\n", ssd->channel_head[cn_id].chip_head[cp_id].die_head[di_id].plane_head[pl_id].free_page);
					}
				}
			}
		}
	printf("Exit get_old_zwh.\n");
}

/*********************************************************************************************
*no_buffer_distribute()针对第一次写入的子请求，创建写子请求，要组建条带
*********************************************************************************************/
void creat_sub_write_request_for_raid(struct ssd_info* ssd, int lpn, unsigned int state, struct request* req, unsigned int mask){
	unsigned int i, j;
	struct request *nowreq = ssd->request_queue;
	struct sub_request *sub, *psub;
	int channel = -1;
	int raidid = 0;
	while(nowreq && channel == -1){
		if(nowreq->operation == WRITE){
			sub	= nowreq->subs;
			while(sub && channel == -1){
				if(sub->lpn == lpn && sub->current_state==SR_WAIT){
					sub->state |= state;
					sub->size = size(sub->state);
					req->all--;
					req->MergeFlag = 1;
					channel = sub->location->channel;
					raidid = sub->raidNUM;

				}
				sub = sub->next_subs;
			}
		}
		nowreq = nowreq->next_node;
	}
	sub	= ssd->raidReq->subs;
	while(sub && channel == -1){
		if(sub->lpn == lpn && sub->current_state==SR_WAIT){
			sub->state |= state;
			sub->size = size(sub->state);
			req->all--;
			req->MergeFlag = 1;
			channel = sub->location->channel;
			raidid = sub->raidNUM;
		}
		sub = sub->next_subs;
	}

	if(channel != -1){
		sub = create_recovery_sub_write(ssd, channel, 0 ,0 ,0 ,0 ,0);
		ssd->raidReq->subs = ssd->raidReq->subs->next_subs;
		sub->next_subs = req->subs;
		req->subs = sub;
		sub->raidNUM = raidid;
		return;
	}
    //当前数据位之前的数据位子请求，看有没有和现在的lpn相同的情况，有就合并写
	for(i = 0 ; i < ssd->stripe.now; ++i){
		if(lpn == ssd->stripe.req[i].lpn){
			ssd->stripe.req[i].state |= state;
			req->all--;
			req->MergeFlag = 1;//合并操作
			ssd->stripe.req[i].lpnCount++;
			return;
		}
	}

	ssd->stripe.req[ssd->stripe.now].lpn = lpn;
	ssd->stripe.req[ssd->stripe.now].state = state;				
	ssd->stripe.req[ssd->stripe.now].req = req;
	if(++ssd->stripe.now == (ssd->stripe.all - 1)){
		unsigned int i, j;
		while(ssd->trip2Page[ssd->stripe.nowStripe].using || ssd->stripe.nowStripe == 0 || ssd->stripe.nowStripe == ssd->stripe.allStripe){
			if(++ssd->stripe.nowStripe == ssd->stripe.allStripe || ssd->stripe.nowStripe == 0)
				ssd->stripe.nowStripe = 1;
		}
		ssd->trip2Page[ssd->stripe.nowStripe].PChannel = -1;
		if(ssd->channelToken % STRIPENUM){//#define STRIPENUM 4
			abort();
		}
		j = ssd->stripe.nowStripe;
		int max = 0;
		for(i = 0; i < ssd->stripe.all; ++i){
			struct sub_request *sub;
			if(i == ssd->stripe.check){
				psub = creat_sub_request(ssd, ssd->stripe.checkLpn , size(mask), mask,\
					ssd->raidReq, WRITE, TARGET_LSB, ssd->stripe.nowStripe);
				if(psub->location->channel % ssd->stripe.all != i){
					printf("sub->location->channel %d %d", sub->location->channel, i);
					abort();
				}
				XOR_process(ssd, 16);
			}else{
				j %= (ssd->stripe.all - 1);
				sub = creat_sub_request(ssd, ssd->stripe.req[j].lpn, size(ssd->stripe.req[j].state), ssd->stripe.req[j].state,\
								req, req->operation, TARGET_LSB, ssd->stripe.nowStripe);
				if(sub->location->channel % ssd->stripe.all != i){
					printf("sub->location->channel %d %d", sub->location->channel, i);
					abort();
				}
				ssd->trip2Page[ssd->stripe.nowStripe].lpn[j] = ssd->stripe.req[j].lpn;
				ssd->trip2Page[ssd->stripe.nowStripe].check[j] = VAIL_DRAID;
				ssd->page2Trip[ssd->stripe.req[j].lpn] = ssd->stripe.nowStripe;
				
				while(ssd->stripe.req[j].lpnCount){
					max += ssd->stripe.req[j].lpnCount;
					ssd->stripe.req[j].lpnCount--;

					sub = create_recovery_sub_write(ssd, sub->location->channel, 0 ,0 ,0 ,0 ,0);
					ssd->raidReq->subs = ssd->raidReq->subs->next_subs;
					
					sub->next_subs = req->subs;
					req->subs = sub;
					sub->state = ssd->stripe.req[j].state;
					sub->size = size(ssd->stripe.req[j].state);
					sub->raidNUM = ssd->stripe.nowStripe;
				}

				ssd->stripe.req[j].state = 0;
				++j;	
			}					
		}
		if(max){
			psub->pChangeCount = max;
		}

		ssd->trip2Page[ssd->stripe.nowStripe].allChange = 0;
		ssd->trip2Page[ssd->stripe.nowStripe].nowChange = 0;
		ssd->trip2Page[ssd->stripe.nowStripe].using = 1;
		ssd->stripe.nowStripe++;//准备下一个条带
		ssd->stripe.now = 0;
		ssd->raidBaseJ++;

		if(++ssd->stripe.checkChange / 2 == 1 && ssd->stripe.checkChange % 2 == 0){
			if(++ssd->stripe.check >= (ssd->stripe.all))
				ssd->stripe.check = 0;
			ssd->stripe.checkChange = 0;
		}

	}

}
//创建并处理奇偶校验的相关操作
void create_parity(struct ssd_info *ssd, struct request *req,long long raidID){
	int changeNUM = 0;
	unsigned int mask=0;
	struct sub_request *sub, *XOR_req;

	if(ssd->parameter->subpage_page == 32){
		mask = 0xffffffff;
	}else{
		mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	}

	if(ssd->trip2Page[raidID].PChannel != -1){
		for(int i = 0; i < ssd->stripe.all - 1; ++i){
			if(ssd->trip2Page[raidID].changeQueuePos[i] != 0){
				++changeNUM;
			}
		}
        //1.rmw读改写
		if(ssd->trip2Page[raidID].location && changeNUM <= (ssd->stripe.all - 1) / 2){
			for (int i = 0; i < ssd->stripe.all - 1; ++i) {
				if (ssd->trip2Page[raidID].changeQueuePos[i] != 0) {
					int lpn = ssd->trip2Page[raidID].lpn[i];
					if(ssd->dram->map->map_entry[lpn].state)
						sub = creat_sub_request(ssd, lpn, size(ssd->dram->map->map_entry[lpn].state), ssd->dram->map->map_entry[lpn].state, \
						req, READ, 0, raidID);
					}
            }
			int lpn = ssd->stripe.checkLpn;
			sub = creat_sub_request(ssd, lpn, size(mask), mask, \
					req, READ, 0, raidID);
		}
        //2.rcw重构写
		else{
			for(int i = 0; i < ssd->stripe.all - 1; ++i){
				if(ssd->trip2Page[raidID].changeQueuePos[i] == 0){
					int lpn = ssd->trip2Page[raidID].lpn[i];
					if(ssd->dram->map->map_entry[lpn].state)
						sub = creat_sub_request(ssd, lpn, size(ssd->dram->map->map_entry[lpn].state), ssd->dram->map->map_entry[lpn].state, \
						req, READ, 0, raidID);
				}
			}
		}
		XOR_req = creat_sub_request(ssd, ssd->stripe.checkLpn , size(mask), mask,\
						req, WRITE, TARGET_LSB, raidID);
		
		XOR_process(ssd, 16);
	}
	
	for(int i = 0; i < ssd->stripe.all - 1; ++i){
		if(ssd->trip2Page[raidID].changeQueuePos[i] != 0){
			ssd->preSubWriteLpn[ssd->trip2Page[raidID].changeQueuePos[i] - 1] = 0;
			ssd->trip2Page[raidID].changeQueuePos[i] = 0;
		}
	}
}

/*
 * 没有缓存的情况下，创建读写子请求，并挂载到相应的位置
 * 读请求：遍历使用creat_sub_request函数创建读子请求，（因为在预处理的时候已经组好了条带）
 * 写请求：遍历
 *          1.第一次写入的话使用creat_sub_write_request_for_raid创建写子请求，
 *          2.更新写入的话使用creat_sub_request创建写子请求，后续使用creat_parity处理奇偶校验的操作
 */
struct ssd_info *no_buffer_distribute(struct ssd_info *ssd)
{
	unsigned int lsn,lpn,last_lpn,first_lpn,complete_flag=0, state;
	unsigned int flag=0,flag1=1,active_region_flag=0; //to indicate the lsn is hit or not
	struct request *req=NULL;
	struct sub_request *sub=NULL,*sub_r=NULL,*update=NULL;
	struct local *loc=NULL;
	struct channel_info *p_ch=NULL;
	int i;
	
	unsigned int mask=0; 
	unsigned int offset1=0, offset2=0;
	unsigned int sub_size=0;
	unsigned int sub_state=0;
	
	if(ssd->cpu_current_state == CPU_BUSY && ssd->cpu_next_state_predict_time > ssd->current_time)
		return ssd;

	ssd->dram->current_time=ssd->current_time;
	req=ssd->request_tail;       
	lsn=req->lsn;
	lpn=req->lsn/ssd->parameter->subpage_page;
	last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;
	first_lpn=req->lsn/ssd->parameter->subpage_page;

	if(req->operation==READ)        
	{		
		while(lpn<=last_lpn) 		
		{
			sub_state=(ssd->dram->map->map_entry[lpn].state&0x7fffffff);
			sub_size=size(sub_state);
            //条带已经组建完成后的创建子请求
			sub=creat_sub_request(ssd,lpn % ssd->stripe.checkLpn,sub_size,sub_state,req,req->operation,0, ssd->page2Trip[lpn]);
			lpn++;
		}
	}
	else if(req->operation==WRITE)
	{
		int target_page_type;
		int random_num;
		random_num = rand()%100;
		if(random_num<ssd->parameter->turbo_mode_factor){
			target_page_type = TARGET_LSB;
			}
		else if(random_num<ssd->parameter->turbo_mode_factor_2){
			target_page_type = TARGET_CSB;
			}
		else{
			target_page_type = TARGET_MSB;
			}
		while(lpn<=last_lpn)     	
		{
			if(ssd->parameter->subpage_page == 32){
				mask = 0xffffffff;
				}
			else{
				mask=~(0xffffffff<<(ssd->parameter->subpage_page));
				}
			state=mask;
			if(lpn==first_lpn)
			{
				offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-req->lsn);
				state=state&(0xffffffff<<offset1);
			}
			if(lpn==last_lpn)
			{
				offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(req->lsn+req->size));
				if(offset2 != 32){
					state=state&(~(0xffffffff<<offset2));
					}
			}
			req->all++;
			int preLpn = lpn;
			lpn = preLpn % ssd->stripe.checkLpn;
            //1.第一次写入：条带尚未组建完成
			if(ssd->dram->map->map_entry[lpn].state == 0 && ssd->parameter->allocation_scheme==0 && (ssd->parameter->dynamic_allocation == 2 || ssd->parameter->dynamic_allocation == 3)){
				creat_sub_write_request_for_raid(ssd, lpn, state, req, mask);
			}else {//2.更新写入：条带组建完成后的更新写
				sub_size=size(state);
				ssd->trip2Page[ssd->page2Trip[lpn]].allChange++;//更新次数
				if(ssd->trip2Page[ssd->page2Trip[lpn]].allChange == 1){
					ssd->trip2Page[ssd->page2Trip[lpn]].changeState = state;
				}else{
					ssd->trip2Page[ssd->page2Trip[lpn]].changeState |= state;
				}

				sub=creat_sub_request(ssd,lpn,sub_size,state,req,req->operation,target_page_type, ssd->page2Trip[lpn]);
                if(lpn != ssd->stripe.checkLpn){
					int raidID = ssd->page2Trip[lpn];
                    ssd->preSubWriteLpn[ssd->preSubWriteNowPos] = lpn+1;
					for(int i = 0; i < ssd->stripe.all - 1; ++i){
						if(ssd->trip2Page[raidID].lpn[i] == lpn){
                            ssd->trip2Page[raidID].changeQueuePos[i] = ssd->preSubWriteNowPos + 1;
							break;
						}
					}
					if(i == (ssd->stripe.all - 1)){
						abort();
					}
					if(++ssd->preSubWriteNowPos == ssd->preSubWriteLen){
						ssd->preSubWriteNowPos = 0;
					}
					if(ssd->preSubWriteLpn[ssd->preSubWriteNowPos]){
                        //创建并处理奇偶校验的相关操作
						create_parity(ssd, req, ssd->page2Trip[ssd->preSubWriteLpn[ssd->preSubWriteNowPos]-1]);
					}

				}
			}
			lpn = preLpn + 1;
		}
	}
	ssd->cpu_current_state = CPU_BUSY;
	ssd->cpu_next_state = CPU_IDLE;
	ssd->cpu_next_state_predict_time = ssd->current_time + 1000;
	return ssd;
}