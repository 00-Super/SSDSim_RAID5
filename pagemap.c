/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName�� pagemap.h
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

#define _CRTDBG_MAP_ALLOC
 
#include "pagemap.h"
#include "ssd.h"
//#include "layer_weight.h"
#include <limits.h>


/************************************************
*����,�����ļ�ʧ��ʱ�������open �ļ��� error��
*************************************************/
void file_assert(int error,char *s)
{
	if(error == 0) return;
	printf("open %s error\n",s);
	getchar();
	exit(-1);
}

/*****************************************************
*����,�������ڴ�ռ�ʧ��ʱ�������malloc ������ error��
******************************************************/
void alloc_assert(void *p,char *s)//����
{
	if(p!=NULL) return;
	printf("malloc %s error\n",s);
	getchar();
	exit(-1);
}

/*********************************************************************************
*����
*A��������time_t��device��lsn��size��ope��<0ʱ�������trace error:.....��
*B��������time_t��device��lsn��size��ope��=0ʱ�������probable read a blank line��
**********************************************************************************/
void trace_assert(int64_t time_t,int device,unsigned int lsn,int size,int ope)//����
{
	if(time_t <0 || device < 0 || lsn < 0 || size < 0 || ope < 0)
	{
		printf("trace error:%lld %d %d %d %d\n",time_t,device,lsn,size,ope);
		getchar();
		exit(-1);
	}
	if(time_t == 0 && device == 0 && lsn == 0 && size == 0 && ope == 0)
	{
		printf("probable read a blank line\n");
		getchar();
	}
}


/************************************************************************************
*�����Ĺ����Ǹ�������ҳ��ppn���Ҹ�����ҳ���ڵ�channel��chip��die��plane��block��page
*�õ���channel��chip��die��plane��block��page���ڽṹlocation�в���Ϊ����ֵ
*************************************************************************************/
struct local *find_location(struct ssd_info *ssd,unsigned int ppn)
{
	struct local *location=NULL;
	unsigned int i=0;
	int pn,ppn_value=ppn;
	int page_plane=0,page_die=0,page_chip=0,page_channel=0;

	pn = ppn;

#ifdef DEBUG
	printf("enter find_location\n");
#endif

	location=(struct local *)malloc(sizeof(struct local));
    alloc_assert(location,"location");
	memset(location,0, sizeof(struct local));

	page_plane=ssd->parameter->page_block*ssd->parameter->block_plane; //number of page per plane
	page_die=page_plane*ssd->parameter->plane_die;                     //number of page per die
	page_chip=page_die*ssd->parameter->die_chip;                       //number of page per chip
	page_channel=page_chip*ssd->parameter->chip_channel[0];            //number of page per channel
	
	/*******************************************************************************
	*page_channel��һ��channel��page����Ŀ�� ppn/page_channel�͵õ������ĸ�channel��
	*��ͬ���İ취���Եõ�chip��die��plane��block��page
	********************************************************************************/
	location->channel = ppn/page_channel;
	location->chip = (ppn%page_channel)/page_chip;
	location->die = ((ppn%page_channel)%page_chip)/page_die;
	location->plane = (((ppn%page_channel)%page_chip)%page_die)/page_plane;
	location->block = ((((ppn%page_channel)%page_chip)%page_die)%page_plane)/ssd->parameter->page_block;
	location->page = (((((ppn%page_channel)%page_chip)%page_die)%page_plane)%ssd->parameter->page_block)%ssd->parameter->page_block;

	return location;
}


/*****************************************************************************
 * 根据传入的channel，chip，die，plane，block，page，找到物理页号ppn
 * *****************************************************************************/
unsigned int find_ppn(struct ssd_info * ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int block,unsigned int page)
{
	unsigned int ppn=0;
	unsigned int i=0;
	int page_plane=0,page_die=0,page_chip=0;
	int page_channel[100];

#ifdef DEBUG
	printf("enter find_psn,channel:%d, chip:%d, die:%d, plane:%d, block:%d, page:%d\n",channel,chip,die,plane,block,page);
#endif
    
	/*********************************************
	*plane, die, chip, channel中的page数量
	**********************************************/
	page_plane=ssd->parameter->page_block*ssd->parameter->block_plane;
	page_die=page_plane*ssd->parameter->plane_die;
	page_chip=page_die*ssd->parameter->die_chip;
	while(i<ssd->parameter->channel_number)
	{
		page_channel[i]=ssd->parameter->chip_channel[i]*page_chip;
		i++;
	}

    /****************************************************************************
	计算出物理页号ppn
	*****************************************************************************/
	i=0;
	while(i<channel)
	{
		ppn=ppn+page_channel[i];
		i++;
	}
	ppn=ppn+page_chip*chip+page_die*die+page_plane*plane+block*ssd->parameter->page_block+page;
	
	return ppn;
}

/********************************
*获得一个读子请求的状态
*********************************/
int set_entry_state(struct ssd_info *ssd,unsigned int lsn,unsigned int size)
{
	int temp,state,move;
	if(size == 32){
		temp = 0xffffffff;
		}
	else{
		temp=~(0xffffffff<<size);
		}
	move=lsn%ssd->parameter->subpage_page;
	state=temp<<move;

	return state;
}

/**************************************************
读请求与处理（raid中不适用这个）
***************************************************/
struct ssd_info *pre_process_page(struct ssd_info *ssd)
{
	int fl=0;
	unsigned int device,lsn,size,ope,lpn,full_page;
	unsigned int largest_lsn,sub_size,ppn,add_size=0;
	unsigned int i=0,j,k;
	int map_entry_new,map_entry_old,modify;
	int flag=0;
	char buffer_request[200];
	struct local *location;
	int64_t time;

	printf("\n");
	printf("begin pre_process_page.................\n");

	ssd->tracefile=fopen(ssd->tracefilename,"r");
	if(ssd->tracefile == NULL )      /*��trace�ļ����ж�ȡ����*/
	{
		printf("the trace file can't open\n");
		return NULL;
	}
	if(ssd->parameter->subpage_page == 32){
		full_page = 0xffffffff;
		}
	else{
		full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
		}

	if(ssd->parameter->allocation_scheme==0 && ssd->parameter->dynamic_allocation == 2)
		largest_lsn=(unsigned int )(((ssd->parameter->chip_num - 1)*ssd->parameter->die_chip*ssd->parameter->plane_die*ssd->parameter->block_plane*ssd->parameter->page_block*ssd->parameter->subpage_page)*(1-ssd->parameter->overprovide));
	else 
		largest_lsn=(unsigned int )((ssd->parameter->chip_num*ssd->parameter->die_chip*ssd->parameter->plane_die*ssd->parameter->block_plane*ssd->parameter->page_block*ssd->parameter->subpage_page)*(1-ssd->parameter->overprovide));

	while(fgets(buffer_request,200,ssd->tracefile))
	{
		sscanf(buffer_request,"%lld %d %d %d %d",&time,&device,&lsn,&size,&ope);
		fl++;
		trace_assert(time,device,lsn,size,ope);                         /*���ԣ���������time��device��lsn��size��ope���Ϸ�ʱ�ͻᴦ��*/
		ssd->total_request_num++;
		add_size=0;                                                     /*add_size����������Ѿ�Ԥ�����Ĵ�С*/

		if(ope==1)                                                      /*����ֻ�Ƕ������Ԥ��������Ҫ��ǰ����Ӧλ�õ���Ϣ������Ӧ�޸�*/
		{
			while(add_size<size)
			{				
				lsn=lsn%largest_lsn;                                    /*��ֹ��õ�lsn������lsn����*/		
				sub_size=ssd->parameter->subpage_page-(lsn%ssd->parameter->subpage_page);		
				if(add_size+sub_size>=size)                             /*ֻ�е�һ������Ĵ�СС��һ��page�Ĵ�Сʱ�����Ǵ���һ����������һ��pageʱ������������*/
				{		
					sub_size=size-add_size;		
					add_size+=sub_size;		
				}

				if((sub_size>ssd->parameter->subpage_page)||(add_size>size))/*��Ԥ����һ���Ӵ�Сʱ�������С����һ��page�����Ѿ������Ĵ�С����size�ͱ���*/		
				{		
					printf("pre_process sub_size:%d\n",sub_size);		
				}

                /*******************************************************************************************************
				*�����߼�������lsn������߼�ҳ��lpn
				*�ж����dram��ӳ���map����lpnλ�õ�״̬
				*A�����״̬==0����ʾ��ǰû��д����������Ҫֱ�ӽ�ub_size��С����ҳд��ȥд��ȥ
				*B�����״̬>0����ʾ����ǰ��д��������Ҫ��һ���Ƚ�״̬����Ϊ��д��״̬��������ǰ��״̬���ص��������ĵط�
				********************************************************************************************************/
				lpn=lsn/ssd->parameter->subpage_page;
				if(ssd->dram->map->map_entry[lpn].state==0)                 /*״̬Ϊ0�����*/
				{
					/**************************************************************
					*�������get_ppn_for_pre_process�������ppn���ٵõ�location
					*�޸�ssd����ز�����dram��ӳ���map���Լ�location�µ�page��״̬
					***************************************************************/
					ppn=get_ppn_for_pre_process(ssd,lsn);                  
					location=find_location(ssd,ppn);
					ssd->program_count++;	
					ssd->channel_head[location->channel].program_count++;
					ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
					ssd->dram->map->map_entry[lpn].pn=ppn;	
					ssd->dram->map->map_entry[lpn].state=set_entry_state(ssd,lsn,sub_size);   //0001
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=lpn;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=ssd->dram->map->map_entry[lpn].state;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~ssd->dram->map->map_entry[lpn].state)&full_page);
					
					free(location);
					location=NULL;
				}//if(ssd->dram->map->map_entry[lpn].state==0)
				else if(ssd->dram->map->map_entry[lpn].state>0)           /*״̬��Ϊ0�����*/
				{
					map_entry_new=set_entry_state(ssd,lsn,sub_size);      /*�õ��µ�״̬������ԭ����״̬���ĵ�һ��״̬*/
					map_entry_old=ssd->dram->map->map_entry[lpn].state;
                    modify=map_entry_new|map_entry_old;
					ppn=ssd->dram->map->map_entry[lpn].pn;
					location=find_location(ssd,ppn);

					ssd->program_count++;	
					ssd->channel_head[location->channel].program_count++;
					ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
					ssd->dram->map->map_entry[lsn/ssd->parameter->subpage_page].state=modify; 
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=modify;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~modify)&full_page);
					
					free(location);
					location=NULL;
				}//else if(ssd->dram->map->map_entry[lpn].state>0)
				lsn=lsn+sub_size;                                         /*�¸����������ʼλ��*/
				add_size+=sub_size;                                       /*�Ѿ������˵�add_size��С�仯*/
			}//while(add_size<size)
		}//if(ope==1) 
	}	

	printf("\n");
	printf("pre_process is complete!\n");

	fclose(ssd->tracefile);

	for(i=0;i<ssd->parameter->channel_number;i++)
    for(j=0;j<ssd->parameter->die_chip;j++)
	for(k=0;k<ssd->parameter->plane_die;k++)
	{
		fprintf(ssd->outputfile,"chip:%d,die:%d,plane:%d have free page: %d\n",i,j,k,ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);				
		fflush(ssd->outputfile);
	}
	
	return ssd;
}

//确定lpn的ppn,location,并建立映射关系，
void raid_pre_read(struct ssd_info *ssd, unsigned int lsn, unsigned int lpn, unsigned int sub_size, unsigned int full_page){
	unsigned int ppn;
	struct local *location;

	ppn=get_ppn_for_pre_process(ssd,lsn);    //找物理位置
	location=find_location(ssd,ppn);
	ssd->program_count++;	
	ssd->channel_head[location->channel].program_count++;
	ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
	ssd->dram->map->map_entry[lpn].pn=ppn;	
	ssd->dram->map->map_entry[lpn].state=set_entry_state(ssd,lsn,sub_size);
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=lpn;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=ssd->dram->map->map_entry[lpn].state;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~ssd->dram->map->map_entry[lpn].state)&full_page);

	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].raidID = ssd->stripe.nowStripe;
	if(lpn == ssd->stripe.checkLpn){
		ssd->trip2Page[ssd->stripe.nowStripe].location = find_location(ssd,ppn);
		ssd->trip2Page[ssd->stripe.nowStripe].PChannel = location->channel;//校验位所在的channel
		ssd->dram->map->map_entry[lpn].state= full_page;
		ssd->dataPlace[location->channel]++;
	}
	++ssd->allUsedPage;
	free(location);
	location=NULL;
}

// 以条带的形式对页面进行预处理（填充页面）
struct ssd_info *pre_process_page_raid(struct ssd_info *ssd)
{
	int fl=0;
	unsigned int device,lsn,size,ope,lpn,full_page;
	unsigned int largest_lsn,sub_size,ppn,add_size=0;
	unsigned int i=0,j,k;
	int map_entry_new,map_entry_old,modify;
	int flag=0;
	char buffer_request[200];
	struct local *location;
	int64_t time;
	
	unsigned int mask = ~(0xffffffff<<(ssd->parameter->subpage_page));
	printf("\n");
	printf("begin pre_process_page.................\n");

	ssd->tracefile=fopen(ssd->tracefilename,"r");
	if(ssd->tracefile == NULL )
	{
		printf("the trace file can't open\n");
		return NULL;
	}
	if(ssd->parameter->subpage_page == 32){
		full_page = 0xffffffff;
		}
	else{
		full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
		}

	largest_lsn = ssd->stripe.checkStart - 1;
	largest_lsn *= ssd->parameter->subpage_page;
	while(fgets(buffer_request,200,ssd->tracefile))
	{
		sscanf(buffer_request,"%lld %d %d %d %d",&time,&device,&lsn,&size,&ope);
		fl++;
		trace_assert(time,device,lsn,size,ope);
		ssd->total_request_num++;
		add_size=0;

		if(ope==1)         //读类型的请求
		{
			while(add_size<size)
			{				
				lsn=lsn%largest_lsn;
				sub_size=ssd->parameter->subpage_page-(lsn%ssd->parameter->subpage_page);		
				if(add_size+sub_size>=size)
				{		
					sub_size=size-add_size;		
					add_size+=sub_size;		
				}

				if((sub_size>ssd->parameter->subpage_page)||(add_size>size))/*��Ԥ����һ���Ӵ�Сʱ�������С����һ��page�����Ѿ������Ĵ�С����size�ͱ���*/		
				{		
					printf("pre_process sub_size:%d\n",sub_size);		
				}

                /*******************************************************************************************************

				********************************************************************************************************/
				lpn=(lsn/ssd->parameter->subpage_page);
				for(i = 0; i < ssd->stripe.all; ++i){
					if(ssd->stripe.req[i].lpn == lpn && ssd->stripe.req[i].state != 0){
						break;
					}
				}
                //1.之前写入过条带
				if(i != ssd->stripe.all && ssd->stripe.req[i].lpn == lpn && ssd->stripe.req[i].state != 0){
					 ssd->stripe.req[i].state |= set_entry_state(ssd,lsn,sub_size);
				}
                //2.第一次写入
                else if(ssd->dram->map->map_entry[lpn].state==0)
				{/**************************************************************
					*get_ppn_for_pre_process
					*location
					***************************************************************/
					ssd->stripe.req[ssd->stripe.now].lpn = lpn;
                    unsigned int now=ssd->stripe.now;
                    unsigned int nowStripe=ssd->stripe.nowStripe;
					ssd->stripe.req[ssd->stripe.now].state = set_entry_state(ssd,lsn,sub_size);				
					ssd->stripe.req[ssd->stripe.now].req = ssd->raidReq;//标记所属的大请求struct request *raidReq;
					ssd->stripe.req[ssd->stripe.now].sub_size = sub_size;
					ssd->stripe.req[ssd->stripe.now].full_page = full_page;
					if(++ssd->stripe.now == (ssd->stripe.all - 1)){
						unsigned int i, j;
						j = ssd->stripe.now;
						for(i = 0; i < ssd->stripe.all; ++i){
							if(i == ssd->stripe.check ){
                                int check=ssd->stripe.check;
								raid_pre_read(ssd, ssd->stripe.checkLpn * ssd->parameter->subpage_page, ssd->stripe.checkLpn, mask, full_page);
                                //检查正在处理的校验位和数据位不在同一个通道上
								// if(!ssd->trip2Page[ssd->stripe.nowStripe].location || ssd->trip2Page[ssd->stripe.nowStripe].location->channel % STRIPENUM != i){
								// 	printf("parity %p %d %d \n", ssd->trip2Page[ssd->stripe.nowStripe].location, ssd->trip2Page[ssd->stripe.nowStripe].location->channel, i);	
								// 	abort();
								// }
							}
                            else{
								j %= (ssd->stripe.all - 1);
								if(ssd->stripe.req[j].req->operation != 0 || ssd->stripe.req[j].state == 0){
									printf("%d %d\n", j, ssd->stripe.req[j].req->operation);
									abort();
								}
								raid_pre_read(ssd, ssd->stripe.req[j].lpn * ssd->parameter->subpage_page, ssd->stripe.req[j].lpn, \
									ssd->stripe.req[j].sub_size, full_page);
								struct local *location = find_location(ssd, ssd->dram->map->map_entry[ssd->stripe.req[j].lpn].pn);
                                // if(location->channel % STRIPENUM != i){
								// 	printf("data %d %d \n", location->channel, i);	
								// 	abort();
								// }
								free(location);
                                //trip2page[条带号].lpn[0/1/2]:这里的lpn数组只存放数据位的lpn，与校验位无关。
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
						ssd->stripe.nowStripe++;//准备下一个条带
						ssd->raidBaseJ++;
						ssd->stripe.now = 0;
						if(++ssd->stripe.checkChange / 2 == 1 && ssd->stripe.checkChange % 2 == 0){
							if(++ssd->stripe.check >= (ssd->stripe.all))
								ssd->stripe.check = 0;//校验位
							ssd->stripe.checkChange = 0;
						}
						
					}
				}
				else if(ssd->dram->map->map_entry[lpn].state>0)
				{
					map_entry_new=set_entry_state(ssd,lsn,sub_size);
					map_entry_old=ssd->dram->map->map_entry[lpn].state;
                    modify=map_entry_new|map_entry_old;
					ppn=ssd->dram->map->map_entry[lpn].pn;
					location=find_location(ssd,ppn);

					ssd->program_count++;	
					ssd->channel_head[location->channel].program_count++;
					ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
					ssd->dram->map->map_entry[lsn/ssd->parameter->subpage_page].state=modify; 
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=modify;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~modify)&full_page);
					
					free(location);
					location=NULL;
				}
				lsn=lsn+sub_size;
				add_size+=sub_size;
			}
		}
	}	

	printf("\n");
	printf("pre_process is complete!\n");

	fclose(ssd->tracefile);

	for(i=0;i<ssd->parameter->channel_number;i++)
    for(j=0;j<ssd->parameter->die_chip;j++)
	for(k=0;k<ssd->parameter->plane_die;k++)
	{
		fprintf(ssd->outputfile,"chip:%d,die:%d,plane:%d have free page: %d\n",i,j,k,ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);				
		fflush(ssd->outputfile);
	}
	
	return ssd;
}


/**************************************
 * ppn
**************************************/
unsigned int get_ppn_for_pre_process(struct ssd_info *ssd,unsigned int lsn)     
{
	unsigned int channel=0,chip=0,die=0,plane=0; 
	unsigned int ppn,lpn;
	unsigned int active_block;
	unsigned int channel_num=0,chip_num=0,die_num=0,plane_num=0;

#ifdef DEBUG
	printf("enter get_psn_for_pre_process\n");
#endif

	channel_num=ssd->parameter->channel_number;//8
	chip_num=ssd->parameter->chip_channel[0];//4
	die_num=ssd->parameter->die_chip;//1
	plane_num=ssd->parameter->plane_die;//1
	lpn=lsn/ssd->parameter->subpage_page;

	if (ssd->parameter->allocation_scheme==0)    //动态分配方式
	{
		if (ssd->parameter->dynamic_allocation==0)
		{
			channel=ssd->token;
			ssd->token=(ssd->token+1)%ssd->parameter->channel_number;
			chip=ssd->channel_head[channel].token;
			ssd->channel_head[channel].token=(chip+1)%ssd->parameter->chip_channel[0];
			die=ssd->channel_head[channel].chip_head[chip].token;
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
		} 
		else if (ssd->parameter->dynamic_allocation==1)
		{
			channel=lpn%ssd->parameter->channel_number;
			chip=ssd->channel_head[channel].token;
			ssd->channel_head[channel].token=(chip+1)%ssd->parameter->chip_channel[0];
			die=ssd->channel_head[channel].chip_head[chip].token;
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
		}
        else if(ssd->parameter->dynamic_allocation == 2){
			channel = ssd->channelToken;
			chip = ssd->chipToken; 
			die=ssd->channel_head[channel].chip_head[chip].token;
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;

			if(++ssd->channelToken >= ssd->parameter->channel_number){
				ssd->channelToken = 0;
				if(++ssd->chipToken >= ssd->parameter->chip_channel[0])
					ssd->chipToken = 0;
			}
			
		}
	} 
	else if (ssd->parameter->allocation_scheme==1)
	{
		switch (ssd->parameter->static_allocation)
		{
			
			case 0:         
			{
				channel=(lpn/(plane_num*die_num*chip_num))%channel_num;
				chip=lpn%chip_num;
				die=(lpn/chip_num)%die_num;
				plane=(lpn/(die_num*chip_num))%plane_num;
				break;
			}
		case 1:
			{
				channel=lpn%channel_num;
				chip=(lpn/channel_num)%chip_num;
				die=(lpn/(chip_num*channel_num))%die_num;
				plane=(lpn/(die_num*chip_num*channel_num))%plane_num;
							
				break;
			}
		case 2:
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*channel_num))%chip_num;
				die=(lpn/(plane_num*chip_num*channel_num))%die_num;
				plane=(lpn/channel_num)%plane_num;
				break;
			}
		case 3:
			{
				channel=lpn%channel_num;
				chip=(lpn/(die_num*channel_num))%chip_num;
				die=(lpn/channel_num)%die_num;
				plane=(lpn/(die_num*chip_num*channel_num))%plane_num;
				break;
			}
		case 4:  
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*die_num*channel_num))%chip_num;
				die=(lpn/(plane_num*channel_num))%die_num;
				plane=(lpn/channel_num)%plane_num;
							
				break;
			}
		case 5:   
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*die_num*channel_num))%chip_num;
				die=(lpn/channel_num)%die_num;
				plane=(lpn/(die_num*channel_num))%plane_num;
							
				break;
			}
		default : return 0;
		}
	}
    
	/******************************************************************************
	*确定了channel,chip,die,plane,active_block之后，
	*通过write_page函数返回ppn
	******************************************************************************/
	if(find_active_block(ssd,channel,chip,die,plane)==FAILURE)
	{
		printf("the read operation is expand the capacity of SSD\n");	
		return 0;
	}
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	if(write_page(ssd,channel,chip,die,plane,active_block,&ppn)==ERROR)
	{
		return 0;
	}

	return ppn;
}

int check_plane(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane){
	unsigned int free_page_num;
	unsigned int cumulate_free_page_num=0;
	unsigned int cumulate_free_lsb_num=0;
	unsigned int i;
	struct plane_info candidate_plane;
	candidate_plane = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane];
	for(i=0;i<ssd->parameter->block_plane;i++){
		if(candidate_plane.blk_head[i].free_lsb_num+candidate_plane.blk_head[i].free_csb_num+candidate_plane.blk_head[i].free_msb_num!=candidate_plane.blk_head[i].free_page_num){
			printf("Meta data Wrong, BLOCK LEVEL!!\n");
			return FALSE;
			}
		cumulate_free_page_num+=candidate_plane.blk_head[i].free_page_num;
		cumulate_free_lsb_num+=candidate_plane.blk_head[i].free_lsb_num;
		}
	if(cumulate_free_page_num!=candidate_plane.free_page){
		printf("Meta data Wrong, PLANE LEVEL!!\n");
		if(cumulate_free_page_num>candidate_plane.free_page){
			printf("The data in plane_info is SMALLER than the blocks.\n");
			}
		else{
			printf("The data in plane_info is GREATER than the blocks.\n");
			}
		return FALSE;
		}
	return TRUE;
}


void invaild_page(struct ssd_info *ssd, struct local *location){
	if(ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state != 0){
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;             /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;              /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].raidID=-1;
	}else{
		printf("%d\n", ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn);
		printf("%lld\n", ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].raidID);
		abort();
	}

}

void judge_erase_node(struct ssd_info *ssd, struct local *location){
	struct direct_erase *direct_erase_node,*new_direct_erase;
    //这个block里全部都是无效页
	if (ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num==ssd->parameter->page_block)    
	{
		int i = 0;
		int count = 0;

		for(i=0;i<ssd->parameter->page_block;i++){
            //如果page.valid_state!=0,说明page有效
			if(ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[i].valid_state!=0){
				++count;	
			}
		}
		if(count){
			printf("create error %d\n",count);
			abort();
		}
		
		new_direct_erase=(struct direct_erase *)malloc(sizeof(struct direct_erase));
        alloc_assert(new_direct_erase,"new_direct_erase");
		memset(new_direct_erase,0, sizeof(struct direct_erase));

		new_direct_erase->block=location->block;
		new_direct_erase->next_node=NULL;
		direct_erase_node=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
		if (direct_erase_node==NULL)
		{
			ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node=new_direct_erase;
		} 
		else
		{
			new_direct_erase->next_node=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
			ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node=new_direct_erase;
		}
	}
}


/***************************************************************************************************
*channel-chip-die-plane--active_block--last_write_page++
*find_ppn ppn
****************************************************************************************************/
struct ssd_info *get_ppn(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,struct sub_request *sub)
{

	int old_ppn=-1;
	unsigned int ppn,lpn,full_page;
	unsigned int active_block;
	unsigned int block;
	unsigned int page,flag=0,flag1=0;
	unsigned int old_state=0,state=0,copy_subpage=0;
	struct local *location;
	struct direct_erase *direct_erase_node,*new_direct_erase;
	struct gc_operation *gc_node;
	struct gc_operation *gc_node_tmp;


	unsigned int i=0,j=0,k=0,l=0,m=0,n=0;
	float debug = 0;

	unsigned int mask=~(0xffffffff<<(ssd->parameter->subpage_page));
    //正在处理的写子请求是写入新的校验数据的请求，而不是正常的数据写入请求，返回SSD？
	if(sub->lpn == (ssd->stripe.checkLpn + 1)){
		return ssd;
	}
	ssd->channelWorkload[channel]++;//记录当前channel上的workload
	
#ifdef DEBUG
	printf("enter get_ppn,channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);
#endif
	if(ssd->parameter->subpage_page == 32){
		full_page = 0xffffffff;
		}
	else{
		full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
		}
	lpn=sub->lpn;
    //当前处理的子请求是用于更新校验数据的子请求
	if(lpn == ssd->stripe.checkLpn)
		++ssd->parityChange;
	else
		++ssd->getPpnCount;	//数据位的子请求

	if(find_active_block(ssd,channel,chip,die,plane)==FAILURE)                      
	{
		printf("ERROR :there is no free page in channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);	
		return ssd;
	}

	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page>=ssd->parameter->page_block)
	{
		printf("error! the last write page larger than the number of pages per block!!\n");
		while(1){}
	}

	block=active_block;	
	page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page;	
	if(page%3==0){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_lsb=page;
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_lsb_num--;
		ssd->free_lsb_count--;
		ssd->write_lsb_count++;
		ssd->newest_write_lsb_count++;
		//**********************
		sub->allocated_page_type = TARGET_LSB;
		//**********************
		}
	else if(page%3==2){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_msb=page;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_msb_num--;
		ssd->write_msb_count++;
		ssd->free_msb_count--;
		ssd->newest_write_msb_count++;
		//**********************
		sub->allocated_page_type = TARGET_MSB;
		//**********************
		}
	else{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_csb=page;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_csb_num--;
		ssd->write_csb_count++;
		ssd->free_csb_count--;
		ssd->newest_write_csb_count++;
		sub->allocated_page_type = TARGET_CSB;
		}
    //更新校验位
	if(lpn == ssd->stripe.checkLpn){
		if(ssd->trip2Page[sub->raidNUM].location){
            //检查sub所在的raidnum是否和sub的location对应的raidID一致
			if(ssd->channel_head[ssd->trip2Page[sub->raidNUM].location->channel].chip_head[ssd->trip2Page[sub->raidNUM].location->chip].die_head[ssd->trip2Page[sub->raidNUM].location->die].plane_head[ssd->trip2Page[sub->raidNUM].location->plane].blk_head[ssd->trip2Page[sub->raidNUM].location->block].page_head[ssd->trip2Page[sub->raidNUM].location->page].raidID != sub->raidNUM){
				printf("don`t accord %lld %d\n",ssd->channel_head[ssd->trip2Page[sub->raidNUM].location->channel].chip_head[ssd->trip2Page[sub->raidNUM].location->chip].die_head[ssd->trip2Page[sub->raidNUM].location->die].plane_head[ssd->trip2Page[sub->raidNUM].location->plane].blk_head[ssd->trip2Page[sub->raidNUM].location->block].page_head[ssd->trip2Page[sub->raidNUM].location->page].raidID , sub->raidNUM);
				abort();
			}
			invaild_page(ssd, ssd->trip2Page[sub->raidNUM].location);
			judge_erase_node(ssd, ssd->trip2Page[sub->raidNUM].location);
			free(ssd->trip2Page[sub->raidNUM].location);
			ssd->trip2Page[sub->raidNUM].location = NULL;
		}
        //重新建立映射
		ssd->dram->map->map_entry[lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
	}
    //第一次写入
    else if(ssd->dram->map->map_entry[lpn].state==0)                                       /*this is the first logical page*/
	{
		if(ssd->dram->map->map_entry[lpn].pn!=0)
		{
			printf("Error in get_ppn()\n");
		}
		ssd->dram->map->map_entry[lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[lpn].state=sub->state;
	}
    //更新写入数据位
	else
	{
		ppn=ssd->dram->map->map_entry[lpn].pn;
		location=find_location(ssd,ppn);
		if(	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn!=lpn)
		{
			printf("%d, %d\n", lpn, ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn);
			printf("\nError in get_ppn()\n");
		}
		invaild_page(ssd, location);
		judge_erase_node(ssd, location);

		free(location);
		location=NULL;
		ssd->dram->map->map_entry[lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[lpn].state=(ssd->dram->map->map_entry[lpn].state|sub->state);
	}

	sub->ppn = ssd->dram->map->map_entry[lpn].pn;
	sub->location->block=active_block;
	sub->location->page=page;

	ssd->program_count++;
	ssd->channel_head[channel].program_count++;
	ssd->channel_head[channel].chip_head[chip].program_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].lpn=lpn;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].valid_state=sub->state;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].free_state=((~(sub->state))&full_page);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].written_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].raidID = sub->raidNUM;
	ssd->write_flash_count++;
    //如果是更新校验位，再记录location到trip2Page
	if(sub->lpn == ssd->stripe.checkLpn){
		ssd->trip2Page[sub->raidNUM].location = find_location(ssd, sub->ppn);
        ssd->trip2Page[sub->raidNUM].PChannel = sub->location->channel;
        ssd->dram->map->map_entry[lpn].state= full_page;//校验位这个也没意义
	}
	if (ssd->parameter->active_write==0)
	{
		if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*(ssd->parameter->gc_hard_threshold + 0.00)))
		{
			static int gcCount = 0;
			static int64_t pre = -1;
			
			pre = ssd->current_time;
			
			struct gc_operation * last_node = ssd->channel_head[channel].last_gc_command;
			fprintf(ssd->gcCreateRequest,"%d\n", ssd->request_queue_length);	
			gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
			alloc_assert(gc_node,"gc_node");
			memset(gc_node,0, sizeof(struct gc_operation));

			gc_node->next_node=NULL;
			gc_node->chip=chip;
			gc_node->die=die;
			gc_node->plane=plane;
			gc_node->block=0xffffffff;
			gc_node->page=0;
			gc_node->state=GC_WAIT;
			gc_node->priority=GC_UNINTERRUPT;
			gc_node->wl_flag=0;
			/*add by winks*/
			if(last_node != NULL)
				last_node->next_node = gc_node;
			else 
				ssd->channel_head[channel].gc_command=gc_node;
			ssd->channel_head[channel].last_gc_command = gc_node;
			/*end add by winks*/
			ssd->gc_request++;
		}
	} 

	if(sub->lpn == ssd->stripe.checkLpn && sub->pChangeCount){
		struct sub_request *psub = creat_sub_request(ssd, ssd->stripe.checkLpn , size(mask), mask,\
						ssd->raidReq, WRITE, TARGET_LSB, sub->raidNUM);
		psub->pChangeCount = sub->pChangeCount - 1;
	}

	return ssd;
}



 unsigned int get_ppn_for_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)     
{
	unsigned int ppn;
	unsigned int active_block,block,page;

#ifdef DEBUG
	printf("enter get_ppn_for_gc,channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);
#endif

	if(find_active_block(ssd,channel,chip,die,plane)!=SUCCESS)
	{
		printf("\n\n Error int get_ppn_for_gc().\n");
		return 0xffffffff;
	}
	
	/*if(find_active_block_for_gc(ssd,channel,chip,die,plane, 0, 0)!= SUCCESS)                      
	{
		printf("ERROR :there is no free page in channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);	
		return 0xffffffff;
	}*/
    
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page>ssd->parameter->page_block)
	{
		
		if(find_active_block(ssd,channel,chip,die,plane) != SUCCESS)
		{
			printf("\n\n Error int get_ppn_for_gc().\n");
			return 0xffffffff;
		}
		active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
		
	}

	//*********************************************
	/*
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_msb<ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_csb){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_msb+3;
		}
	else if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_csb<ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_lsb){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_csb+3;
		}
	else{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_lsb+3;
		}
		*/
	//*********************************************

	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page++;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;

	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page>ssd->parameter->page_block)
	{
		printf("DDD error! the last write page larger than 64!!\n");
		while(1){}
		
	}

	block=active_block;	
	page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page;	

	if(page%3==0){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_lsb=page;
		ssd->free_lsb_count--;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_lsb_num--;
		//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
		ssd->write_lsb_count++;
		ssd->newest_write_lsb_count++;
		}
	else if(page%3==2){
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_msb=page;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_msb_num--;
		//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
		ssd->write_msb_count++;
		ssd->free_msb_count--;
		ssd->newest_write_msb_count++;
		}
	else{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_csb=page;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_csb_num--;
		ssd->write_csb_count++;
		ssd->free_csb_count--;
		ssd->newest_write_csb_count++;
		}

	ppn=find_ppn(ssd,channel,chip,die,plane,block,page);

	ssd->program_count++;
	ssd->channel_head[channel].program_count++;
	ssd->channel_head[channel].chip_head[chip].program_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].written_count++;
	ssd->write_flash_count++;

	/*add by winks*/
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].write_frequently = 0;
	/*end add by winks*/
	return ppn;

}

/*********************************************************************************************************************
* 计算所有channel中当前处于读取（subs_r_head）和写入（subs_w_head）状态的子请求数量之和，并返回这个总数
*********************************************************************************************************************/
int get_sub_num_channel(struct ssd_info * ssd, int chan){
	struct sub_request *sub = ssd->channel_head[chan].subs_r_head;
	int ret = 0;
	for(chan = 0; chan < ssd->parameter->channel_number; ++chan){
		sub = ssd->channel_head[chan].subs_r_head;
		while(sub){
			++ret;
			sub = sub->next_node;
		}
		sub = ssd->channel_head[chan].subs_w_head;
		while(sub){
			++ret;
			sub = sub->next_node;
		}
	}
	return ret;
}

Status erase_operation(struct ssd_info * ssd,unsigned int channel ,unsigned int chip ,unsigned int die ,unsigned int plane ,unsigned int block)
{
	unsigned int flag, i;
	flag = 0;
	int bet_temp;
	for(i=0;i<ssd->parameter->page_block;i++){
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state!=0){
			flag = 1;
			}
		}
	
	if(flag==1){
		printf("Erasing a block with valid data: %d, %d, %d, %d, %d.\n",channel, chip, die, plane, block);
		return FAILURE;
		}
	fprintf(ssd->raidOutfile,"%d\n", ssd->request_queue_length);
	ssd->chipGc[channel * ssd->parameter->chip_channel[0] + chip]++;
	unsigned int origin_free_page_num, origin_free_lsb_num, origin_free_csb_num, origin_free_msb_num;
	origin_free_page_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num;
	origin_free_lsb_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_lsb_num;
	origin_free_csb_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_csb_num;
	origin_free_msb_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_msb_num;
	/* if(origin_free_lsb_num+origin_free_csb_num+origin_free_msb_num!=origin_free_page_num){
		printf("WRONG METADATA in BLOCK LEVEL.(erase_operation)\n");
		} */
	//printf("ERASE_OPERATION: %d, %d, %d, %d, %d.\n",channel,chip,die,plane,block);
	//unsigned int i=0;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num=ssd->parameter->page_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num=0;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page=-1;
	//*****************************************************
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_lsb=-3;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_csb=-2;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_msb=-1;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_lsb_num=(int)(ssd->parameter->page_block/3);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_csb_num=(int)(ssd->parameter->page_block/3);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_msb_num=(int)(ssd->parameter->page_block/3);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_lsb_num=0;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].fast_erase=FALSE;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].dr_state=DR_STATE_NULL;
	//*****************************************************
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].erase_count++;
	for (i=0;i<ssd->parameter->page_block;i++)
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state=PG_SUB;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state=0;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].lpn=-1;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].raidID = -1;
	}
	ssd->erase_count++;
	ssd->channel_head[channel].erase_count++;			
	ssd->channel_head[channel].chip_head[chip].erase_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page+=ssd->parameter->page_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page-=origin_free_page_num;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_lsb_num+=(ssd->parameter->page_block/3);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_lsb_num-=origin_free_lsb_num;
	ssd->free_lsb_count+=(ssd->parameter->page_block/3);
	ssd->free_csb_count+=(ssd->parameter->page_block/3);
	ssd->free_msb_count+=(ssd->parameter->page_block/3);
	ssd->free_lsb_count-=origin_free_lsb_num;
	ssd->free_csb_count-=origin_free_csb_num;
	ssd->free_msb_count-=origin_free_msb_num;
	
	ssd->channel_head[channel].gc_req_nums = get_sub_num_channel(ssd, channel);
	
	ssd->total_erase_count++;
	
	

	/* 
	if(ssd->UBT[bet_temp] == 1){
				ssd->UBT[bet_temp] = 0;
				ssd->rr_erase_count++;
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].read_count = 0;
				printf("debug_rr: rr_erase_count:%d, erase_count:%d\n", ssd->rr_erase_count, ssd->erase_count);
				//printf("debug::wl_value:%d, total_erase_count:%d, table_count:%d\n", ssd->total_erase_count/ssd->table_count, ssd->total_erase_count, ssd->table_count);
	} */

	return SUCCESS;

}


/**************************************************************************************
*��������Ĺ����Ǵ���INTERLEAVE_TWO_PLANE��INTERLEAVE��TWO_PLANE��NORMAL�µĲ����Ĳ�����
***************************************************************************************/
Status erase_planes(struct ssd_info * ssd, unsigned int channel, unsigned int chip, unsigned int die1, unsigned int plane1,unsigned int command)
{
	unsigned int die=0;
	unsigned int plane=0;
	unsigned int block=0;
	struct direct_erase *direct_erase_node=NULL;
	unsigned int block0=0xffffffff;
	unsigned int block1=0;

	if((ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node==NULL)||               
		((command!=INTERLEAVE_TWO_PLANE)&&(command!=INTERLEAVE)&&(command!=TWO_PLANE)&&(command!=NORMAL)))     /*���û�в�������������command���ԣ����ش���*/           
	{
		return ERROR;
	}

	/************************************************************************************************************
	*������������ʱ������Ҫ���Ͳ����������channel��chip���ڴ��������״̬����CHANNEL_TRANSFER��CHIP_ERASE_BUSY
	*��һ״̬��CHANNEL_IDLE��CHIP_IDLE��
	*************************************************************************************************************/
	block1=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node->block;
	
	ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	

	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;										
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;

	
	
	if(command==INTERLEAVE_TWO_PLANE)                                       /*�߼�����INTERLEAVE_TWO_PLANE�Ĵ���*/
	{
		for(die=0;die<ssd->parameter->die_chip;die++)
		{
			block0=0xffffffff;
			if(die==die1)
			{
				block0=block1;
			}
			for (plane=0;plane<ssd->parameter->plane_die;plane++)
			{
				direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
				if(direct_erase_node!=NULL)
				{
					
					block=direct_erase_node->block; 
					
					if(block0==0xffffffff)
					{
						block0=block;
					}
					else
					{
						if(block!=block0)
						{
							continue;
						}

					}
					ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die,plane,block);     /*��ʵ�Ĳ��������Ĵ���*/
					free(direct_erase_node);                               
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
				}

			}
		}

		ssd->interleave_mplane_erase_count++;                             /*������һ��interleave two plane erase����,���������������ʱ�䣬�Լ���һ��״̬��ʱ��*/
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+18*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tWB;       
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time-9*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tBERS;

	}
	else if(command==INTERLEAVE)                                          /*�߼�����INTERLEAVE�Ĵ���*/
	{
		for(die=0;die<ssd->parameter->die_chip;die++)
		{
			for (plane=0;plane<ssd->parameter->plane_die;plane++)
			{
				if(die==die1)
				{
					plane=plane1;
				}
				direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
				if(direct_erase_node!=NULL)
				{
					block=direct_erase_node->block;
					ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die,plane,block);
					free(direct_erase_node);
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
					break;
				}	
			}
		}
		ssd->interleave_erase_count++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;       
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
	}
	else if(command==TWO_PLANE)                                          /*�߼�����TWO_PLANE�Ĵ���*/
	{

		for(plane=0;plane<ssd->parameter->plane_die;plane++)
		{
			direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node;
			if((direct_erase_node!=NULL))
			{
				block=direct_erase_node->block;
				if(block==block1)
				{
					ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die1,plane,block);
					free(direct_erase_node);
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
				}
			}
		}

		ssd->mplane_erase_conut++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;      
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
	}
	else if(command==NORMAL)                                             /*��ͨ����NORMAL�Ĵ���*/
	{
		
		direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;
		block=direct_erase_node->block;
		/*add by winks*/
		if(ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].blk_head[block].write_frequently) {
			//printf("add  dir %ld\n", ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].blk_head[block].changed_count);
			ssd->invaild_page_of_change +=  ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].blk_head[block].changed_count;
			ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].blk_head[block].changed_count = 0;
		}
		/*end add by winks*/
		ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node=direct_erase_node->next_node;
		free(direct_erase_node);
		direct_erase_node=NULL;
		erase_operation(ssd,channel,chip,die1,plane1,block);

		ssd->direct_erase_count++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+5*ssd->parameter->time_characteristics.tWC;       								
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tWB+ssd->parameter->time_characteristics.tBERS;	

		if(ssd->gcIntervalFile)
			fprintf(ssd->gcIntervalFile,"%lld\t%lld\t%lld\n", ssd->current_time, ssd->channel_head[channel].chip_head[chip].next_state_predict_time, ssd->current_time - ssd->preRequestArriveTime[ssd->preRequestArriveTimeIndex]);
	}
	else
	{
		return ERROR;
	}

	direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;

	if(((direct_erase_node)!=NULL)&&(direct_erase_node->block==block1))
	{
		return FAILURE; 
	}
	else
	{
		return SUCCESS;
	}
}

Status fast_erase_planes(struct ssd_info * ssd, unsigned int channel, unsigned int chip, unsigned int die1, unsigned int plane1,unsigned int command)
{
	unsigned int die=0;
	unsigned int plane=0;
	unsigned int block=0;
	struct direct_erase *direct_erase_node=NULL;
	unsigned int block0=0xffffffff;
	unsigned int block1=-1;
	/*
	if((ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].fast_erase_node==NULL)|| (command!=NORMAL))
	{
		return ERROR;
	}
	*/
	/************************************************************************************************************
	*������������ʱ������Ҫ���Ͳ����������channel��chip���ڴ��������״̬����CHANNEL_TRANSFER��CHIP_ERASE_BUSY
	*��һ״̬��CHANNEL_IDLE��CHIP_IDLE��
	*************************************************************************************************************/
	struct plane_info candidate_plane;
	candidate_plane = ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1];
	while(block < ssd->parameter->block_plane)
		{
		if((candidate_plane.blk_head[block].free_msb_num==ssd->parameter->page_block/2)&&(candidate_plane.blk_head[block].invalid_lsb_num==ssd->parameter->page_block/2)&&(candidate_plane.blk_head[block].fast_erase!=TRUE))
			{
			block1 = block;
			break;
			}
		block++;
		}
	//block1=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].fast_erase_node->block;
	if(block1==-1){
		return FAILURE;
		}
	ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	

	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;										
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;

	
	if(command==NORMAL)                                             /*��ͨ����NORMAL�Ĵ���*/
	{
		//direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].fast_erase_node;
		//block=direct_erase_node->block;
		//ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].fast_erase_node=direct_erase_node->next_node;
		//free(direct_erase_node);
		//direct_erase_node=NULL;
		candidate_plane.blk_head[block1].fast_erase = TRUE;
		erase_operation(ssd,channel,chip,die1,plane1,block1);

		ssd->fast_gc_count++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+5*ssd->parameter->time_characteristics.tWC;       								
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tWB+ssd->parameter->time_characteristics.tBERS;
		return SUCCESS;
	}
	else
	{
		printf("Wrong Commend for fast gc.\n");
		return ERROR;
	}
	/*
	direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].fast_erase_node;

	if(((direct_erase_node)!=NULL)&&(direct_erase_node->block==block1))
	{
		return FAILURE; 
	}
	else
	{
		return SUCCESS;
	}
	*/
}


/*******************************************************************************************************************
*GC������ĳ��plane��free��������ֵ���д�������ĳ��plane������ʱ��GC����ռ�����plane���ڵ�die����Ϊdie��һ��������Ԫ��
*��һ��die��GC���������������ĸ�planeͬʱerase������interleave erase������GC����Ӧ������������ʱֹͣ���ƶ����ݺͲ���
*ʱ���У����Ǽ�϶ʱ�����ֹͣGC���������Է����µ�������󣬵��������������������϶ʱ�䣬����GC������������������
*GC��ֵ��һ������ֵ��һ��Ӳ��ֵ������ֵ��ʾ�������ֵ�󣬿��Կ�ʼ������GC���������ü�Ъʱ�䣬GC���Ա��µ��������жϣ�
*������Ӳ��ֵ��ǿ����ִ��GC�������Ҵ�GC�������ܱ��жϣ�ֱ���ص�Ӳ��ֵ���ϡ�
*������������棬�ҳ����die���е�plane�У���û�п���ֱ��ɾ����block��Ҫ���еĻ�������interleave two plane���ɾ��
*��Щblock�������ж���plane������ֱ��ɾ����block��ͬʱɾ�������еĻ��������ǵ������plane����ɾ��������Ҳ������Ļ���
*ֱ����������gc_parallelism�������н�һ��GC�������ú���Ѱ��ȫ��Ϊinvalid�Ŀ飬ֱ��ɾ�����ҵ���ֱ��ɾ���ķ���1��û����
*������-1��
*********************************************************************************************************************/
int gc_direct_erase(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)     
{
	unsigned int lv_die=0,lv_plane=0;                                                           /*Ϊ����������ʹ�õľֲ����� Local variables*/
	unsigned int interleaver_flag=FALSE,muilt_plane_flag=FALSE;
	unsigned int normal_erase_flag=TRUE;

	struct direct_erase * direct_erase_node1=NULL;
	struct direct_erase * direct_erase_node2=NULL;

	direct_erase_node1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
	if (direct_erase_node1==NULL)
	{
		return FAILURE;
	}
    
	/********************************************************************************************************
	*���ܴ���TWOPLANE�߼�����ʱ��������Ӧ��channel��chip��die��������ͬ��plane�ҵ�����ִ��TWOPLANE������block
	*����muilt_plane_flagΪTRUE
	*********************************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)
	{	
		for(lv_plane=0;lv_plane<ssd->parameter->plane_die;lv_plane++)
		{
			direct_erase_node2=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
			if((lv_plane!=plane)&&(direct_erase_node2!=NULL))
			{
				if((direct_erase_node1->block)==(direct_erase_node2->block))
				{
					muilt_plane_flag=TRUE;
					break;
				}
			}
		}
	}

	/***************************************************************************************
	*���ܴ���INTERLEAVE�߼�����ʱ��������Ӧ��channel��chip�ҵ�����ִ��INTERLEAVE������block
	*����interleaver_flagΪTRUE
	****************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)
	{
		for(lv_die=0;lv_die<ssd->parameter->die_chip;lv_die++)
		{
			if(lv_die!=die)
			{
				for(lv_plane=0;lv_plane<ssd->parameter->plane_die;lv_plane++)
				{
					if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node!=NULL)
					{
						interleaver_flag=TRUE;
						break;
					}
				}
			}
			if(interleaver_flag==TRUE)
			{
				break;
			}
		}
	}
    
	/************************************************************************************************************************
	*A������ȿ���ִ��twoplane������block���п���ִ��interleaver������block����ô��ִ��INTERLEAVE_TWO_PLANE�ĸ߼������������
	*B�����ֻ����ִ��interleaver������block����ô��ִ��INTERLEAVE�߼�����Ĳ�������
	*C�����ֻ����ִ��TWO_PLANE������block����ô��ִ��TWO_PLANE�߼�����Ĳ�������
	*D��û��������Щ�������ô��ֻ�ܹ�ִ����ͨ�Ĳ���������
	*************************************************************************************************************************/
	if ((muilt_plane_flag==TRUE)&&(interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))     
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE_TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	} 
	else if ((interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE)==SUCCESS)
		{
			return SUCCESS;
		}
	}
	else if ((muilt_plane_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	}

	if ((normal_erase_flag==TRUE))                              /*����ÿ��plane���п���ֱ��ɾ����block��ֻ�Ե�ǰplane������ͨ��erase����������ֻ��ִ����ͨ����*/
	{
		if (erase_planes(ssd,channel,chip,die,plane,NORMAL)==SUCCESS)
		{
			return SUCCESS;
		} 
		else
		{
			return FAILURE;                                     /*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ��������*/
		}
	}
	return SUCCESS;
}

int gc_direct_fast_erase(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)     
{
	unsigned int lv_die=0,lv_plane=0;                                                           /*Ϊ����������ʹ�õľֲ����� Local variables*/
	unsigned int interleaver_flag=FALSE,muilt_plane_flag=FALSE;
	unsigned int normal_erase_flag=TRUE;

	struct direct_erase * direct_erase_node1=NULL;
	struct direct_erase * direct_erase_node2=NULL;

	/*
	direct_erase_node1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].fast_erase_node;
	if (direct_erase_node1==NULL)
	{
		return FAILURE;
	}
    */
	/********************************************************************************************************
	*���ܴ���TWOPLANE�߼�����ʱ��������Ӧ��channel��chip��die��������ͬ��plane�ҵ�����ִ��TWOPLANE������block
	*����muilt_plane_flagΪTRUE
	*********************************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)
	{	
		printf("Advanced commands are NOT supported.\n");
		return FAILURE;
	}

	/***************************************************************************************
	*���ܴ���INTERLEAVE�߼�����ʱ��������Ӧ��channel��chip�ҵ�����ִ��INTERLEAVE������block
	*����interleaver_flagΪTRUE
	****************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)
	{
		printf("Advanced commands are NOT supported.\n");
		return FAILURE;
	}
    
	/************************************************************************************************************************
	*A������ȿ���ִ��twoplane������block���п���ִ��interleaver������block����ô��ִ��INTERLEAVE_TWO_PLANE�ĸ߼������������
	*B�����ֻ����ִ��interleaver������block����ô��ִ��INTERLEAVE�߼�����Ĳ�������
	*C�����ֻ����ִ��TWO_PLANE������block����ô��ִ��TWO_PLANE�߼�����Ĳ�������
	*D��û��������Щ�������ô��ֻ�ܹ�ִ����ͨ�Ĳ���������
	*************************************************************************************************************************/
	/*
	if ((muilt_plane_flag==TRUE)&&(interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))     
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE_TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	} 
	else if ((interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE)==SUCCESS)
		{
			return SUCCESS;
		}
	}
	else if ((muilt_plane_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	}
	*/

	if ((normal_erase_flag==TRUE))                              /*����ÿ��plane���п���ֱ��ɾ����block��ֻ�Ե�ǰplane������ͨ��erase����������ֻ��ִ����ͨ����*/
	{
		if (fast_erase_planes(ssd,channel,chip,die,plane,NORMAL)==SUCCESS)
		{
			return SUCCESS;
		} 
		else
		{
			return FAILURE;                                     /*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ��������*/
		}
	}
	else{
		printf("Wrong command for fast gc.\n");
		return FAILURE;
		}
	//return SUCCESS;
}

//有效页的迁移，看到和条带相关的代码有一些！
Status move_page(struct ssd_info * ssd, struct local *location, unsigned int * transfer_size)
{
	struct local *new_location=NULL;
	unsigned int free_state=0,valid_state=0;
	unsigned int lpn=0,old_ppn=0,ppn=0;
    /* 检查当前有效页的lpn是否为校验位的lpn,
     * 如果是校验位的lpn,并且与raidID相关的页面不存在位置信息，就置为无效*/
	lpn=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn;
	if(lpn == ssd->stripe.checkLpn){
		int raidID = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].raidID;
		if(!ssd->trip2Page[raidID].location){
			invaild_page(ssd, location);
			return SUCCESS;
		}
	}
	
	valid_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state;
	free_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state;
	old_ppn=find_ppn(ssd,location->channel,location->chip,location->die,location->plane,location->block,location->page);      /*��¼�����Ч�ƶ�ҳ��ppn���Ա�map���߶���ӳ���ϵ�е�ppn������ɾ�������Ӳ���*/
	ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);                /*�ҳ�����ppnһ�����ڷ���gc������plane��,����ʹ��copyback������Ϊgc������ȡppn*/

	new_location=find_location(ssd,ppn);                                                                   /*�����»�õ�ppn��ȡnew_location*/

	if(new_location->block == location->block){
		ssd->same_block_flag = 1;
		ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);                /*�ҳ�����ppnһ�����ڷ���gc������plane��,����ʹ��copyback������Ϊgc������ȡppn*/
		new_location=find_location(ssd,ppn);  
		ssd->same_block_flag = 0;
	}
	if(new_location->block == location->block){
		printf("Data is moved to the same block!!!!!!\n");
		return FAILURE;
	}
	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		printf("Wrong COMMANDS.\n");
		return FAILURE;
		if (ssd->parameter->greed_CB_ad==1)                                                                /*̰����ʹ�ø߼�����*/
		{
			ssd->copy_back_count++;
			ssd->gc_copy_back++;
			while (old_ppn%2!=ppn%2)
			{
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].invalid_page_num++;
				
				free(new_location);
				new_location=NULL;

				ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);    /*�ҳ�����ppnһ�����ڷ���gc������plane�У�����������ż��ַ����,����ʹ��copyback����*/
				ssd->program_count--;
				ssd->write_flash_count--;
				ssd->waste_page_count++;
			}
			if(new_location==NULL)
			{
				new_location=find_location(ssd,ppn);
			}
			
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=free_state;
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=lpn;
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=valid_state;
		} 
		else
		{
			if (old_ppn%2!=ppn%2)
			{
				(*transfer_size) += size(valid_state);
			}
			else
			{

				ssd->copy_back_count++;
				ssd->gc_copy_back++;
			}
		}	
	} 
	else//普通命令
	{
		(* transfer_size)+=size(valid_state);
	}
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=free_state;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=lpn;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=valid_state;
	int raidID = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].raidID;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].raidID = raidID;

	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
	if((location->page)%3==0){
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_lsb_num++;
		}

	if (old_ppn==ssd->dram->map->map_entry[lpn].pn)                                                     /*�޸�ӳ���*/
	{
		ssd->dram->map->map_entry[lpn].pn=ppn;//修改映射关系
	}
    //更新校验位页的位置信息
	if(lpn == ssd->stripe.checkLpn){
		ssd->trip2Page[raidID].location->channel = new_location->channel;
		ssd->trip2Page[raidID].location->chip = new_location->chip;
		ssd->trip2Page[raidID].location->die = new_location->die;
		ssd->trip2Page[raidID].location->plane = new_location->plane;
		ssd->trip2Page[raidID].location->block = new_location->block;
		ssd->trip2Page[raidID].location->page = new_location->page;
		++ssd->pageMoveRaid;
	}
	
	ssd->moved_page_count++;
	
	free(new_location);
	new_location=NULL;

	return SUCCESS;
}

/*******************************************************************************************************************************************
 *目标plane中没有可以直接擦除的block，
 *寻找invalid_page_num最多的块作为目标擦除块（greedy gc）
********************************************************************************************************************************************/
int uninterrupt_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,struct gc_operation *gc_node)       
{
	unsigned int i=0,invalid_page=0;
	unsigned int block,active_block,transfer_size,free_page,page_move_count=0;
	struct local *  location=NULL;
	unsigned int total_invalid_page_num=0;

	if(find_active_block(ssd,channel,chip,die,plane)!=SUCCESS)
	{
		printf("\n\n Error in uninterrupt_gc().\n");
		return ERROR;
	}
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;

	invalid_page=0;
	transfer_size=0;
	block=-1;
	if(gc_node->wl_flag == 2){
		block = gc_node->block;
	}else if(gc_node->wl_flag == 1){
		block = gc_node->block;
		ssd->wl_erase_count++;
	}else{
		// (this normal erase)
		ssd->normal_erase++;//统计擦除次数
        //选plane中仅包含有效页和无效页，并且无效页最多的block
		for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
		{	
			total_invalid_page_num+=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
			//存在空闲页的话就跳过
            if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].free_page_num > 0){
				continue;
				}
			if((active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
			{				
				invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
				block=i;						
			}
		}
	}
	if (block==-1)
	{
		return 1;
	}

	//***********************************************
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].fast_erase = TRUE;
	//***********************************************
	free_page=0;
	/*add by winks*/
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].write_frequently) {
		ssd->invaild_page_of_change +=  ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].changed_count;
		ssd->invaild_page_of_all += (ssd->parameter->page_block - ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num);
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].changed_count = 0;
	}
	/*end add by winks*/

    //检查这个block中的所有page，
	for(i=0;i<ssd->parameter->page_block;i++)
	{		
		if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state&PG_SUB)==0x0000000f)
		{
			free_page++;
		}
		if(free_page!=0)
		{

		}
        //block中的有效页，要进行movepage操作
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state != 0)
		{
			location=(struct local * )malloc(sizeof(struct local ));
			alloc_assert(location,"location");
			memset(location,0, sizeof(struct local));

			location->channel=channel;
			location->chip=chip;
			location->die=die;
			location->plane=plane;
			location->block=block;
			location->page=i;
			move_page(ssd, location, &transfer_size);   //movepage操作                                               /*��ʵ��move_page����*/
			page_move_count++;
			
			if(gc_node->wl_flag == 0){
				ssd->page_move_count++;
			}else if(gc_node->wl_flag == 1){
				ssd->wl_page_move_count++;
			}

			free(location);	
			location=NULL;
		}				
	}
	erase_operation(ssd,channel ,chip , die,plane ,block);	 //实际的擦除操作                                             /*ִ����move_page�����󣬾�����ִ��block�Ĳ�������*/

	ssd->channel_head[channel].current_state=CHANNEL_GC;									
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;			
	
	/***************************************************************
	*高级命令
	***************************************************************/
	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		if (ssd->parameter->greed_CB_ad==1)
		{

			ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG);			
			ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
		} 
	} 
	else
	{//普通命令下：channel，chip，的下一状态预计时间

		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG)+transfer_size*SECTOR*(ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tRC);

		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;

	}
	if(ssd->gcIntervalFile)
		fprintf(ssd->gcIntervalFile,"%lld\t%lld\t%lld\n", ssd->current_time, ssd->channel_head[channel].chip_head[chip].next_state_predict_time, ssd->current_time - ssd->preRequestArriveTime[ssd->preRequestArriveTimeIndex]);
	return 1;
}

int uninterrupt_fast_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int priority)       
{
	//printf("===>Enter uninterrupt_fast_gc.\n");
	unsigned int i=0,invalid_page=0;
	unsigned int block,block1,active_block,transfer_size,free_page,page_move_count=0;                           /*��¼ʧЧҳ���Ŀ��*/
	struct local *  location=NULL;
	unsigned int total_invalid_page_num=0;
	
	block1 = -1;
	block = 0;
	struct plane_info candidate_plane;
	candidate_plane = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane];
	unsigned int invalid_lsb_num = 0;
	while(block < ssd->parameter->block_plane-1){
		//printf("Enter while, %d\n", block);
		if((candidate_plane.blk_head[block].free_msb_num==ssd->parameter->page_block/2)&&(candidate_plane.blk_head[block].fast_erase!=TRUE)){
			if(candidate_plane.blk_head[block].free_lsb_num==0 && candidate_plane.blk_head[block].invalid_lsb_num>invalid_lsb_num){
				block1 = block;
				invalid_lsb_num = candidate_plane.blk_head[block].invalid_lsb_num;
				}
			}
		block++;
		}
	if (block1==-1)
	{
		//printf("====>No block has invalid LSB page??\n");
		return SUCCESS;
	}
	if(priority==GC_FAST_UNINTERRUPT_EMERGENCY){
		if(candidate_plane.blk_head[block1].invalid_lsb_num < ssd->parameter->page_block/2*ssd->parameter->fast_gc_filter_2){
			return SUCCESS;
			}
		}
	else if(priority==GC_FAST_UNINTERRUPT){
		if(candidate_plane.blk_head[block1].invalid_lsb_num < ssd->parameter->page_block/2*ssd->parameter->fast_gc_filter_1){
			return SUCCESS;
			}
		}
	else if(priority==GC_FAST_UNINTERRUPT_IDLE){
		if(candidate_plane.blk_head[block1].invalid_lsb_num < ssd->parameter->page_block/2*ssd->parameter->fast_gc_filter_idle){
			return SUCCESS;
			}
		//printf("FAST_GC_IDLE on %d, %d, %d, %d, %d.\n", channel, chip, die, plane, block1);
		ssd->idle_fast_gc_count++;
		}
	else{
		printf("GC_FAST ERROR. PARAMETERS WRONG.\n");
		return SUCCESS;
		}
	
	free_page=0;
	struct blk_info candidate_block;
	candidate_block = candidate_plane.blk_head[block1];
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block1].fast_erase = TRUE;
	//printf("%d %d %d %d %d is selected, free_msbs:%d, invalid_lsbs:%d.\n",channel,chip,die,plane,block1, candidate_block.free_msb_num, candidate_block.invalid_lsb_num);
	//printf("begin to move pages.\n");
	//printf("<<<Fast GC Candidate, %d, %d, %d, %d, %d.\n",channel,chip,die,plane,block1);
	//printf("Begin to MOVE PAGES.\n");
	for(i=0;i<ssd->parameter->page_block;i++)		                                                     /*������ÿ��page���������Ч���ݵ�page��Ҫ�ƶ��������ط��洢*/		
	{		
		if(candidate_block.page_head[i].valid_state>0) /*��ҳ����Чҳ����Ҫcopyback����*/		
		{	
			//printf("move page %d.\n",i);
			location=(struct local * )malloc(sizeof(struct local ));
			alloc_assert(location,"location");
			memset(location,0, sizeof(struct local));

			location->channel=channel;
			location->chip=chip;
			location->die=die;
			location->plane=plane;
			location->block=block1;
			location->page=i;
			//printf("state of this block: %d.\n",candidate_block.fast_erase);
			move_page(ssd, location, &transfer_size);                                                   /*��ʵ��move_page����*/
			page_move_count++;

			free(location);	
			location=NULL;
		}				
	}
	//printf("block: %d, %d, %d, %d, %d going to be erased.\n",channel, chip, die, plane, block1);
	erase_operation(ssd,channel ,chip , die, plane ,block1);	                                              /*ִ����move_page�����󣬾�����ִ��block�Ĳ�������*/
	//printf("ERASE OPT: %d, %d, %d, %d, %d\n",channel,chip,die,plane,block1);
	ssd->channel_head[channel].current_state=CHANNEL_GC;									
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;			

	ssd->fast_gc_count++;
	
	/***************************************************************
	*�ڿ�ִ��COPYBACK�߼������벻��ִ��COPYBACK�߼���������������£�
	*channel�¸�״̬ʱ��ļ��㣬�Լ�chip�¸�״̬ʱ��ļ���
	***************************************************************/
	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		if (ssd->parameter->greed_CB_ad==1)
		{

			ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG);			
			ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
		} 
	} 
	else
	{

		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG)+transfer_size*SECTOR*(ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tRC);					
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;

	}
	//printf("===>Exit uninterrupt_fast_gc Successfully.\n");
	return 1;
}

int uninterrupt_dr(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)       
{
	unsigned int i=0,invalid_page=0;
	unsigned int block,active_block,transfer_size,free_page,page_move_count=0;                           /*��¼ʧЧҳ���Ŀ��*/
	struct local *  location=NULL;
	unsigned int total_invalid_page_num=0;
	/*
	if(find_active_block_dr(ssd,channel,chip,die,plane)!=SUCCESS)
	{
		printf("\n\n Error in uninterrupt_dr(). No active block\n");
		return ERROR;
	}
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	*/
	invalid_page=0;
	transfer_size=0;
	block=-1;
	for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
	{	
		total_invalid_page_num+=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page)						
		{				
			invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
			block=i;						
		}
	}
	
	if (block==-1)
	{
		printf("No block is selected.\n");
		return 1;
	}
	//��Ŀ��block��dr_state��Ϊ���
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].dr_state=DR_STATE_OUTPUT;

	
	//printf("Block %d with %d invalid pages is selected.\n", block, ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num);
	free_page=0;
	for(i=0;i<ssd->parameter->page_block;i++)		                                                     /*������ÿ��page���������Ч���ݵ�page��Ҫ�ƶ��������ط��洢*/		
	{		
		if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state&PG_SUB)==0x0000000f)
		{
			free_page++;
		}
		/*
		if(free_page!=0)
		{
			//printf("\ntoo much free page. \t %d\t .%d\t%d\t%d\t%d\t\n",free_page,channel,chip,die,plane);
		}
		*/
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state>0) /*��ҳ����Чҳ����Ҫcopyback����*/		
		{	
			location=(struct local * )malloc(sizeof(struct local ));
			alloc_assert(location,"location");
			memset(location,0, sizeof(struct local));

			location->channel=channel;
			location->chip=chip;
			location->die=die;
			location->plane=plane;
			location->block=block;
			location->page=i;
			if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].dr_state != DR_STATE_OUTPUT){
				printf("The DR_STATE is not settle.\n");
				}
			move_page(ssd, location, &transfer_size);                                                   /*��ʵ��move_page����*/
			page_move_count++;

			free(location);	
			location=NULL;
		}				
	}
	erase_operation(ssd,channel ,chip , die,plane ,block);	                                              /*ִ����move_page�����󣬾�����ִ��block�Ĳ�������*/
	/*
	ssd->channel_head[channel].current_state=CHANNEL_GC;									
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;			
	*/
	/***************************************************************
	*�ڿ�ִ��COPYBACK�߼������벻��ִ��COPYBACK�߼���������������£�
	*channel�¸�״̬ʱ��ļ��㣬�Լ�chip�¸�״̬ʱ��ļ���
	***************************************************************/
	/*
	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		if (ssd->parameter->greed_CB_ad==1)
		{

			ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG);			
			ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
		} 
	} 
	else
	{

		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG)+transfer_size*SECTOR*(ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tRC);					
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;

	}
	*/
	return 1;
}


/*******************************************************************************************************************************************
*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ�������������ڿ����жϵ�gc�����У��ɹ�ɾ��һ���飬����1��û��ɾ��һ���鷵��-1
*����������У����ÿ���Ŀ��channel,die�Ƿ��ǿ��е�
********************************************************************************************************************************************/
int interrupt_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,struct gc_operation *gc_node)        
{
	unsigned int i,block,active_block,transfer_size,invalid_page=0;
	struct local *location;

	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	transfer_size=0;
	if (gc_node->block>=ssd->parameter->block_plane)
	{
		for(i=0;i<ssd->parameter->block_plane;i++)
		{			
			if((active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
			{				
				invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
				block=i;						
			}
		}
		gc_node->block=block;
	}

	if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num!=ssd->parameter->page_block)     /*����Ҫִ��copyback����*/
	{
		for (i=gc_node->page;i<ssd->parameter->page_block;i++)
		{
			if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].page_head[i].valid_state>0)
			{
				location=(struct local * )malloc(sizeof(struct local ));
				alloc_assert(location,"location");
				memset(location,0, sizeof(struct local));

				location->channel=channel;
				location->chip=chip;
				location->die=die;
				location->plane=plane;
				location->block=block;
				location->page=i;
				transfer_size=0;

				move_page( ssd, location, &transfer_size);

				free(location);
				location=NULL;

				gc_node->page=i+1;
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num++;
				ssd->channel_head[channel].current_state=CHANNEL_C_A_TRANSFER;									
				ssd->channel_head[channel].current_time=ssd->current_time;										
				ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[channel].chip_head[chip].current_state=CHIP_COPYBACK_BUSY;								
				ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
				ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;		

				if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
				{					
					ssd->channel_head[channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC;		
					ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
				} 
				else
				{	
					ssd->channel_head[channel].next_state_predict_time=ssd->current_time+(7+transfer_size*SECTOR)*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(7+transfer_size*SECTOR)*ssd->parameter->time_characteristics.tWC;					
					ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
				}
				return 0;    
			}
		}
	}
	else
	{
		erase_operation(ssd,channel ,chip, die,plane,gc_node->block);	

		ssd->channel_head[channel].current_state=CHANNEL_C_A_TRANSFER;									
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;								
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+5*ssd->parameter->time_characteristics.tWC;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;							
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;

		return 1;                                                                      /*��gc������ɣ�����1�����Խ�channel�ϵ�gc����ڵ�ɾ��*/
	}

	printf("there is a problem in interrupt_gc\n");
	return 1;
}

/*************************************************************
*�����Ĺ����ǵ�������һ��gc����ʱ����Ҫ��gc���ϵ�gc_nodeɾ����
**************************************************************/
int delete_gc_node(struct ssd_info *ssd, unsigned int channel,struct gc_operation *gc_node)
{
	struct gc_operation *gc_pre=NULL;

	
	if(gc_node==NULL)                                                                  
	{
		return ERROR;
	}

	if (gc_node==ssd->channel_head[channel].gc_command)
	{
		ssd->channel_head[channel].gc_command=gc_node->next_node;
		/*add by winks*/
		if(gc_node == ssd->channel_head[channel].last_gc_command)
			ssd->channel_head[channel].last_gc_command = NULL;
		/*end add by winks*/
	}
	else
	{
		gc_pre=ssd->channel_head[channel].gc_command;
		while (gc_pre->next_node!=NULL)
		{
			if (gc_pre->next_node==gc_node)
			{
				gc_pre->next_node=gc_node->next_node;
				/*add by winks*/
				if(gc_node == ssd->channel_head[channel].last_gc_command)
					ssd->channel_head[channel].last_gc_command = gc_pre;
				/*end add by winks*/
				break;
			}
			gc_pre=gc_pre->next_node;
		}
	}
	free(gc_node);
	gc_node=NULL;
	ssd->gc_request--;
	return SUCCESS;
}

/***************************************
*处理channel的每一个gc操作
****************************************/
Status gc_for_channel(struct ssd_info *ssd, unsigned int channel)
{
	int flag_direct_erase=1,flag_gc=1,flag_invoke_gc=1;
	unsigned int chip,die,plane,flag_priority=0;
	unsigned int current_state=0, next_state=0;
	long long next_state_predict_time=0;
	struct gc_operation *gc_node=NULL,*gc_p=NULL;

	/*******************************************************************************************
	*
	********************************************************************************************/
	gc_node=ssd->channel_head[channel].gc_command;
	while (gc_node!=NULL)
	{
		current_state=ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
		next_state=ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
		next_state_predict_time=ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
		if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))
		{
			if (gc_node->priority==GC_UNINTERRUPT)    //优先服务不可中断的GC
			{
				if (ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].free_page < (ssd->parameter->page_block*ssd->parameter->block_plane*(ssd->parameter->gc_hard_threshold))) {
					flag_priority=1;
					break;
				}
			}
		}
		gc_node=gc_node->next_node;
	}
	if (flag_priority!=1)       //没有找到不可中断类型的gc，从队首的gc节点开始执行
	{
		gc_node=ssd->channel_head[channel].gc_command;
		while (gc_node!=NULL)
		{
			current_state=ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
			next_state=ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
			next_state_predict_time=ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
			 /**********************************************
			 *只有chip是空闲的，才可以执行gc操作
			 ***********************************************/
			if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))   
			{
				break;
			}
			gc_node=gc_node->next_node;
		}

	}
	if(gc_node==NULL)
	{
		return FAILURE;
	}

	if(ssd->channel_head[channel].gc_req_nums != -1){
		int add = get_sub_num_channel(ssd, channel) - ssd->channel_head[channel].gc_req_nums;
		ssd->allBlockReq += add;
		ssd->channel_head[channel].gc_req_nums = -1;
	}

    //不可中断的gc，先进行gc_direct_erase，不成功就进行uninterrupt_gc
	chip=gc_node->chip;
	die=gc_node->die;
	plane=gc_node->plane;
	if (gc_node->priority==GC_UNINTERRUPT)
	{
		flag_direct_erase=gc_direct_erase(ssd,channel,chip,die,plane);//直接擦
		if (flag_direct_erase!=SUCCESS)
		{
			flag_gc=uninterrupt_gc(ssd,channel,chip,die,plane,gc_node);  //选择无效页最多的block擦除                       /*��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��*/
			if (flag_gc==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
		}
		else
		{
			delete_gc_node(ssd,channel,gc_node);
		}
		return SUCCESS;
	}
	/*******************************************************************************
	*
	********************************************************************************/
	else if(gc_node->priority==GC_FAST_UNINTERRUPT || gc_node->priority==GC_FAST_UNINTERRUPT_EMERGENCY || gc_node->priority==GC_FAST_UNINTERRUPT_IDLE){
		//printf("===>GC_FAST on %d,%d,%d,%d begin.\n",channel,chip,die,plane);
		flag_direct_erase=gc_direct_fast_erase(ssd,channel,chip,die,plane);
		if (flag_direct_erase!=SUCCESS)
		{

			flag_gc=uninterrupt_fast_gc(ssd,channel,chip,die,plane,gc_node->priority);
			if (flag_gc==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
		}
		else
		{

			delete_gc_node(ssd,channel,gc_node);
		}

		return SUCCESS;
		}
	else        
	{
		flag_invoke_gc=decide_gc_invoke(ssd,channel);                                  /*�ж��Ƿ�����������Ҫchannel���������������Ҫ���channel����ô���gc�����ͱ��ж���*/

		if (flag_invoke_gc==1)
		{
			flag_direct_erase=gc_direct_erase(ssd,channel,chip,die,plane);
			if (flag_direct_erase==-1)
			{
				flag_gc=interrupt_gc(ssd,channel,chip,die,plane,gc_node);             /*��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��*/
				if (flag_gc==1)
				{
					delete_gc_node(ssd,channel,gc_node);
				}
			}
			else if (flag_direct_erase==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
			return SUCCESS;
		} 
		else
		{
			return FAILURE;
		}		
	}
}

Status find_invalid_block(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane){
	unsigned int block, flag;
	for(block=0;block<ssd->parameter->block_plane;block++){
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num>(ssd->parameter->page_block*ssd->parameter->dr_filter)){
			return TRUE;
			}
		}
	return FALSE;
}

Status dr_for_channel(struct ssd_info *ssd, unsigned int channel)
{
	int flag_direct_erase=1,flag_gc=1,flag_invoke_gc=1;
	unsigned int chip,die,plane,flag_priority=0;
	unsigned int current_state=0, next_state=0;
	long long next_state_predict_time=0;
	struct gc_operation *gc_node=NULL,*gc_p=NULL;

	/*******************************************************************************************
	*����ÿһ��gc_node����ȡgc_node���ڵ�chip�ĵ�ǰ״̬���¸�״̬���¸�״̬��Ԥ��ʱ��
	*�����ǰ״̬�ǿ��У������¸�״̬�ǿ��ж��¸�״̬��Ԥ��ʱ��С�ڵ�ǰʱ�䣬�����ǲ����жϵ�gc
	*��ô��flag_priority��Ϊ1������Ϊ0
	********************************************************************************************/
	//======================================================================
	//�����������ղ����������
	ssd->channel_head[channel].gc_command=NULL;
	gc_node=ssd->channel_head[channel].gc_command;
	//��ѯ����plane��������Чҳ��block�����յ�
	unsigned int plane_flag;
	unsigned int counter;
	for(chip=0;chip<ssd->parameter->chip_channel[0];chip++){
		for(die=0;die<ssd->parameter->die_chip;die++){
			for(plane=0;plane<ssd->parameter->plane_die;plane++){
				plane_flag = find_invalid_block(ssd, channel, chip, die, plane);
				counter=0;
				while(plane_flag==TRUE){
					counter++;
					//printf("There is data should be reorganize in %d, %d, %d, %d.\n",channel, chip, die, plane);
					uninterrupt_dr(ssd, channel, chip, die, plane);
					plane_flag = find_invalid_block(ssd, channel, chip, die, plane);
					}
				printf("SUCCESS. Data in %d, %d, %d, %d is reorganized.\n",channel, chip, die, plane);
				}
			}
    	}
	return SUCCESS;
}


/**************************************************
 *flag=1表示整个SSD都是IDLE的情况
 *flag=0表示针对特定的channel，chip，die进行gc请求的操作
 * 跟之前的gc函数基本一样的
***************************************************/
unsigned int gc(struct ssd_info *ssd,unsigned int channel, unsigned int flag)
{
	unsigned int i;
	int flag_direct_erase=1,flag_gc=1,flag_invoke_gc=1;
	unsigned int flag_priority=0, ret;
	struct gc_operation *gc_node=NULL,*gc_p=NULL;
	
    //整个SSD都是空闲的情况
	if (flag==1)                                                                       /*����ssd����IDEL�����*/
	{
		for (i=0;i<ssd->parameter->channel_number;i++)
		{
			flag_priority=0;
			flag_direct_erase=1;
			flag_gc=1;
			flag_invoke_gc=1;
			gc_node=NULL;
			gc_p=NULL;
			if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))
			{
				channel=i;
				if (ssd->channel_head[channel].gc_command!=NULL)
				{
					ret = gc_for_channel(ssd, channel);//对这个channel进行gc
				}
			}
		}
		return ret;

	} 
	else     //只需要针对特定的channel，chip，die进行gc请求的操作
	{
		if ((ssd->parameter->allocation_scheme==1)||((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==1)))
		{
			// 不进入这个地方，因为我们设置的allocation_scheme=0，dynamic_allocation=2
			if ((ssd->channel_head[channel].subs_r_head!=NULL)||(ssd->channel_head[channel].subs_w_head!=NULL))
			{
				return 0;
			}
		}

		ret = gc_for_channel(ssd,channel);//对这个channel进行gc
		return ret;
	}
}



/**********************************************************
*�ж��Ƿ���������Ѫҩchannel������û�з���1�Ϳ��Է���gc����
*����з���0���Ͳ���ִ��gc������gc�������ж�
***********************************************************/
int decide_gc_invoke(struct ssd_info *ssd, unsigned int channel)      
{
	struct sub_request *sub;
	struct local *location;

	if ((ssd->channel_head[channel].subs_r_head==NULL)&&(ssd->channel_head[channel].subs_w_head==NULL))    /*������Ҷ�д�������Ƿ���Ҫռ�����channel�����õĻ�����ִ��GC����*/
	{
		return 1;                                                                        /*��ʾ��ǰʱ�����channelû����������Ҫռ��channel*/
	}
	else
	{
		if (ssd->channel_head[channel].subs_w_head!=NULL)
		{
			return 0;
		}
		else if (ssd->channel_head[channel].subs_r_head!=NULL)
		{
			sub=ssd->channel_head[channel].subs_r_head;
			while (sub!=NULL)
			{
				if (sub->current_state==SR_WAIT)                                         /*����������Ǵ��ڵȴ�״̬���������Ŀ��die����idle������ִ��gc����������0*/
				{
					location=find_location(ssd,sub->ppn);
					if ((ssd->channel_head[location->channel].chip_head[location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[location->channel].chip_head[location->chip].next_state==CHIP_IDLE)&&
						(ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time<=ssd->current_time)))
					{
						free(location);
						location=NULL;
						return 0;
					}
					free(location);
					location=NULL;
				}
				else if (sub->next_state==SR_R_DATA_TRANSFER)
				{
					location=find_location(ssd,sub->ppn);
					if (ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time<=ssd->current_time)
					{
						free(location);
						location=NULL;
						return 0;
					}
					free(location);
					location=NULL;
				}
				sub=sub->next_node;
			}
		}
		return 1;
	}
}

