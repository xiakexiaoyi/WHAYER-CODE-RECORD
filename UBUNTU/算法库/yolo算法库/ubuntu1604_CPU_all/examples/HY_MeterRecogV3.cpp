#include <iostream>
#include "HY_MeterRecogV3.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
extern "C"
{
#endif

#include "detector.h"

#ifdef __cplusplus
}
#endif

#ifndef PI
#define PI               3.14159265358979323846
#endif

#define GrayStep(x)			(( (x) + 3 ) & (~3) )
typedef struct 
{
	network net;
	float lThresh;
}MRParam;
int resize_image(HYLPMRV3_IMAGES im, HYLPMRV3_IMAGES resized)
{
	int res=0;
    int w = resized->lWidth;
    int h = resized->lHeight;
    unsigned char* data = (unsigned char *)im->pixelArray.chunky.pPixel;
    /*HYMRV3_IMAGES resized = { 0 };
    resized.lHeight = h;
    resized.lWidth = w;
    resized.pixelArray.chunky.lLineBytes = GrayStep(3 * resized.lWidth);
    resized.pixelArray.chunky.pPixel = (unsigned char *)calloc(resized.lHeight*resized.pixelArray.chunky.lLineBytes, sizeof(unsigned char));*/
    unsigned char* dataresized = (unsigned char *)resized->pixelArray.chunky.pPixel;

    HYMRV3_IMAGES part = { 0 };
    part.lHeight = im->lHeight;
    part.lWidth = w;
    part.pixelArray.chunky.lLineBytes = GrayStep(3 * part.lWidth);
    part.pixelArray.chunky.pPixel = (unsigned char *)calloc(part.lHeight*part.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
    if(part.pixelArray.chunky.pPixel==NULL)
	{
		printf("calloc fall!!!\n");
		res=-1;
		return res;
	}
	unsigned char* datapart = (unsigned char *)part.pixelArray.chunky.pPixel;

    int r, c, k;
    float w_scale = (float)(im->lWidth - 1) / (w - 1);
    float h_scale = (float)(im->lHeight - 1) / (h - 1);
    for (k = 0; k < 3; ++k) {//channel=3
        for (r = 0; r < im->lHeight; ++r) {
            for (c = 0; c < w; ++c) {
                float val = 0;
                if (c == w - 1 || im->lWidth == 1) {
                    val = (float)data[r*im->pixelArray.chunky.lLineBytes + (im->lWidth - 1) * 3 + k];
                }
                else {
                    float sx = c*w_scale;
                    int ix = (int)sx;
                    float dx = sx - ix;
                    val= (1 - dx) * (float)data[r*im->pixelArray.chunky.lLineBytes+ix*3+k] + dx * (float)data[r*im->pixelArray.chunky.lLineBytes + (ix+1) * 3 + k];

                }
                datapart[r*part.pixelArray.chunky.lLineBytes+c*3+k]= (unsigned char)val;
            }
        }
    }
    for (k = 0; k < 3; ++k) {//channel=3
        for (r = 0; r < h; ++r) {
            float sy = r*h_scale;
            int iy = (int)sy;
            float dy = sy - iy;
            for (c = 0; c < w; ++c) {
                float val = (1 - dy)*datapart[iy*part.pixelArray.chunky.lLineBytes + c * 3 + k];
                dataresized[r*resized->pixelArray.chunky.lLineBytes + c * 3 + k] = val;
            }
            if (r == h - 1 || im->lHeight == 1) continue;

            for (c = 0; c < w; ++c) {
                float val = dy*datapart[(iy + 1)*part.pixelArray.chunky.lLineBytes + c * 3 + k];
                dataresized[r*resized->pixelArray.chunky.lLineBytes + c * 3 + k] = dataresized[r*resized->pixelArray.chunky.lLineBytes + c * 3 + k] + (unsigned char)val;
            }
        }
    }
	if(part.pixelArray.chunky.pPixel)
	{
		free(part.pixelArray.chunky.pPixel);
		part.pixelArray.chunky.pPixel=NULL;
	}
    return res;
}
int polartrans_ls(HYLPMRV3_IMAGES src, HYLPMRV3_IMAGES dst, MV3Para Meterdescrip, MV3PT *SF)
{
    int res=0;
    HYMRV3_IMAGES IndexImage = { 0 };
    unsigned char* data = (unsigned char *)src->pixelArray.chunky.pPixel;
    int wrect = PI * 2 * Meterdescrip.RadiusOut;
    int wrectactual = PI * 2 * Meterdescrip.RadiusOut*(abs(Meterdescrip.AngleStart - Meterdescrip.AngleEnd)) / 360;
    int hrect = Meterdescrip.RadiusOut - Meterdescrip.RadiusIn;//pOutPattern->RadiusIn内径

    //if (src->lPixelArrayFormat == HYAMR_IMAGE_GRAY)
    //{
    //	IndexImage.lHeight = hrect;
    //	IndexImage.lWidth = wrectactual;
    //	IndexImage.pixelArray.chunky.lLineBytes = GrayStep(IndexImage.lWidth);
    //	IndexImage.pixelArray.chunky.pPixel = (unsigned char *)calloc(IndexImage.lHeight*IndexImage.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
    //	unsigned char* indexdata = (unsigned char *)IndexImage.pixelArray.chunky.pPixel;
    //	for (int j = 0; j < hrect; j++)
    //	{
    //		for (int i = 0; i < wrectactual; i++)
    //			//for (int i = 0; i < wrect; i++)
    //		{
    //			double theta = PI*2.0 / wrect*(i + 1);//逆时针
    //			double rho = Meterdescrip.RadiusOut - j - 1;
    //			if (Meterdescrip.AngleStart > Meterdescrip.AngleEnd)//顺时针
    //				theta = -1 * theta;//调整为顺时针
    //			theta = theta + 1.0*Meterdescrip.AngleStart / 360 * PI*2.0;//调整开始角度为pOutPattern->AngleStart
    //		    //极坐标转换 x = rcos（θ），y = rsin（θ），图片的笛卡尔坐标系y轴是逆序的
    //			int position_x = Meterdescrip.Col + rho*std::cos(theta) + 0.5;
    //			int position_y = Meterdescrip.Row - rho * (float)std::sin(theta) + 0.5;
    //			//if (position_x == 2041 && position_y == 2685)//刻度点定位
    //			//	printf("x=%d y=%d\n",i,j);
    //			indexdata[j*IndexImage.pixelArray.chunky.lLineBytes + i] = data[position_y*src->pixelArray.chunky.lLineBytes + position_x];

    //		}
    //	}
    //}
    if (src->lPixelArrayFormat == HYAMR_IMAGE_BGR)
    {

        IndexImage.lHeight = hrect;
        IndexImage.lWidth = wrectactual;
        IndexImage.pixelArray.chunky.lLineBytes = GrayStep(3 * IndexImage.lWidth);
        IndexImage.pixelArray.chunky.pPixel = (unsigned char *)calloc(IndexImage.lHeight*IndexImage.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
        if(IndexImage.pixelArray.chunky.pPixel==NULL)
		{
			printf("calloc fall!!!\n");
			res=-1;
			return res;
		}
		unsigned char* indexdata= (unsigned char *)IndexImage.pixelArray.chunky.pPixel;
        for (int j = 0; j < hrect; j++)
        {
            for (int i = 0; i < wrectactual; i++)
            //for (int i = 0; i < wrect; i++)
            {
                double theta = PI*2.0 / wrect*(i + 1);//逆时针
                double rho = Meterdescrip.RadiusOut - j - 1;
                if (Meterdescrip.AngleStart > Meterdescrip.AngleEnd)//顺时针
                    theta = -1 * theta;//调整为顺时针
                theta = theta + 1.0*Meterdescrip.AngleStart / 360 * PI*2.0;//调整开始角度为pOutPattern->AngleStart
                //极坐标转换 x = rcos（θ），y = rsin（θ），图片的笛卡尔坐标系y轴是逆序的
                //int position_x = Meterdescrip.Col + rho*std::cos(theta) + 0.5;
                //int position_y = Meterdescrip.Row - rho * (float)std::sin(theta) + 0.5;

                int position_x = Meterdescrip.Col + rho*cos(theta) + 0.5;
                int position_y = Meterdescrip.Row - rho * (float)sin(theta) + 0.5;
                for (int k = 0; k < Meterdescrip.lPtNum; k++)
                {
                    if (Meterdescrip.ptPosList[k].x == position_x &&Meterdescrip.ptPosList[k].y == position_y)
                    {
                        SF->ptPosList[k].x = 1.0*i/wrectactual*dst->lWidth;
                        SF->ptPosList[k].y = 1.0*j/hrect*dst->lHeight;
                        SF->dPtValList[k] = Meterdescrip.dPtValList[k];
                    }
                }
                //if (position_x == 2041 && position_y == 2685)//刻度点定位
                //	printf("x=%d y=%d\n",i,j);
                indexdata[j*IndexImage.pixelArray.chunky.lLineBytes + i * 3 + 0] = data[position_y*src->pixelArray.chunky.lLineBytes + position_x * 3 + 0];
                indexdata[j*IndexImage.pixelArray.chunky.lLineBytes + i * 3 + 1] = data[position_y*src->pixelArray.chunky.lLineBytes + position_x * 3 + 1];
                indexdata[j*IndexImage.pixelArray.chunky.lLineBytes + i * 3 + 2] = data[position_y*src->pixelArray.chunky.lLineBytes + position_x * 3 + 2];

            }
        }
		if(0!=resize_image(&IndexImage, dst))
		{
			res=-1;
			if(IndexImage.pixelArray.chunky.pPixel)
			{
				free(IndexImage.pixelArray.chunky.pPixel);
				IndexImage.pixelArray.chunky.pPixel=NULL;
			}
			return res;
		}
        //resize_image(&IndexImage, dst);
        SF->lPtNum = Meterdescrip.lPtNum;
    }
    //free(IndexImage.pixelArray.chunky.pPixel);
	if(IndexImage.pixelArray.chunky.pPixel)
	{
		free(IndexImage.pixelArray.chunky.pPixel);
		IndexImage.pixelArray.chunky.pPixel=NULL;
	}
    return res;
}
int yoloinit(void **handle,char *cfgfile, char *weightfile,float Thresh)
{
	int res=0;
	MRParam *tmpHandle = NULL;
	network net = { 0 };
	tmpHandle = (MRParam*)calloc(1,sizeof(MRParam));
	if(tmpHandle==NULL)
	{
		res=-1;
		goto EXT;
	}
	tmpHandle->lThresh = Thresh;
	tmpHandle->net = net;
	tmpHandle->net = COM_parse_network_cfg(cfgfile);
	if(tmpHandle->net.errorcode !=0)
	{
		res=tmpHandle->net.errorcode;
		goto EXT;
	}
	if(weightfile){
        COM_load_weights(&(tmpHandle->net), weightfile);
		if(tmpHandle->net.errorcode !=0)
		{
			res=tmpHandle->net.errorcode;
			goto EXT;
		}
    }
    COM_set_batch_network(&(tmpHandle->net), 1);
EXT:
	*handle = (void*)tmpHandle;
	return res;
}
int yolouninit(void *handle)
{
	MRParam *netParam = (MRParam*)handle;
	if (netParam)
	{
		COM_free_network(netParam->net);
		free(netParam);
		netParam = NULL;
	}		
	return 0;
}
int detec(const void *handle,unsigned char* imagdata, int height, int width, int channels, int widstep,int *numDST, float **boxout)
{
	int res=0;
	MRParam *pSRParam = (MRParam*)handle;
	image out = COM_make_image(width, height, channels);
	int i, j, k, count = 0;
    if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}
	for (k = 0; k < channels; ++k){
		for (i = 0; i <height; ++i){
			for (j = 0; j < width; ++j){
				out.data[count++] = imagdata[i*widstep + j*channels + k] / 255.;
			}
		}
	}
	if(0!=COM_YoloDetector(pSRParam->net,out,pSRParam->lThresh,numDST,boxout))
	{
		res=-1;
		goto EXT;
		
	}
EXT:
	COM_free_image(out);
	return res;
}

int MeterReadRecogV3(const void *handle,const HYLPMRV3_IMAGES src,  MV3Para Meterdescrip,HYMR_POINTERRESULT *MeterResult)
{
	int res=0;	  
    MV3PT pPtPos = { 0 };

    unsigned char *IndexData = NULL;
    unsigned char *Index416Data = NULL;

    float thresh = 0.1;
    int Indexnum;
    int Indexboxnum = 1000;

    int tmpgreater = 0;
    int tmpless = 0;
    int fingernum = 0;
    int Maxindex = 0;
    float MaxProb = 0.0;
    MV3POINT sf6finger = { 0 };
    int sf6Distance[100] = { 0 };
    int dMinDist = 0xFFFF;
    double MeterValue = 0;

	float **Indexboxout=NULL;
	HYMRV3_IMAGES IndexImage416 = { 0 };
	HYMRV3_IMAGES IndexImage = { 0 };
	IndexImage.lHeight = 64;
	IndexImage.lWidth = 416;
	IndexImage.pixelArray.chunky.lLineBytes = GrayStep(3*IndexImage.lWidth);
	IndexImage.pixelArray.chunky.pPixel = (unsigned char *)calloc(IndexImage.lHeight*IndexImage.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
	if(IndexImage.pixelArray.chunky.pPixel==NULL)
	{
		res=-1;
		goto EXT;
	}
	IndexImage.lPixelArrayFormat = HYAMR_IMAGE_BGR;
    //MV3PT pPtPos = { 0 };//
	if(!src)
	{
		res=-1;
		goto EXT;
	}
	if (&IndexImage == NULL)
	{
		res = -1;
		goto EXT;
	}
    if(0!=polartrans_ls(src, &IndexImage, Meterdescrip, &pPtPos))
    {
        printf("polartrans_ls fialed\n");
		res=-1;
		goto EXT;
	}

	//HYMRV3_IMAGES IndexImage416 = { 0 };
	IndexImage416.lPixelArrayFormat = HYAMR_IMAGE_BGR;
	IndexImage416.lWidth = 416;
	IndexImage416.lHeight = 416;
	IndexImage416.pixelArray.chunky.lLineBytes = GrayStep(3 * IndexImage416.lWidth);
	IndexImage416.pixelArray.chunky.pPixel = (unsigned char *)calloc(IndexImage416.lHeight*IndexImage416.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
	if(IndexImage416.pixelArray.chunky.pPixel==NULL)
	{
		res=-1;
		goto EXT;
	}
    //unsigned char *IndexData = (unsigned char*)IndexImage.pixelArray.chunky.pPixel;//
    //unsigned char *Index416Data = (unsigned char*)IndexImage416.pixelArray.chunky.pPixel;//
    IndexData = (unsigned char*)IndexImage.pixelArray.chunky.pPixel;
    Index416Data = (unsigned char*)IndexImage416.pixelArray.chunky.pPixel;
	for (int j = 0; j<IndexImage416.lHeight; j++)
	{
		for (int i = 0; i<3 * IndexImage416.lWidth; i++)
		{
			if (j<IndexImage.lHeight)
				Index416Data[i + j*IndexImage416.pixelArray.chunky.lLineBytes] = IndexData[i + j*IndexImage.pixelArray.chunky.lLineBytes];
			else
				Index416Data[i + j*IndexImage416.pixelArray.chunky.lLineBytes] = 255;
		}
	}
    //float thresh = 0.1;//
    //int Indexnum;//
    //int Indexboxnum = 1000;//
	//float **Indexboxout=NULL;
	Indexboxout = (float **)calloc(Indexboxnum, sizeof(float *));
	if(Indexboxout==NULL)
	{
		res=-1;
		goto EXT;
	}
	for (int i = 0; i<Indexboxnum; i++){
		Indexboxout[i] = (float *)calloc(6, sizeof(float));
		if(Indexboxout[i]==NULL)
		{
			res=-1;
			goto EXT;
		}
		Indexboxout[i][5]=-100;
	}
	//char *cfgfile = Meterdescrip.cfgfile;
	//char *weightfile = Meterdescrip.weightfile;
	//time = clock();

	/*void *handle=NULL;
	if(0!=init(&handle,cfgfile,weightfile,thresh))
	{
		printf("error in init.\n");
		uninit(handle);
		res=-1;
		goto EXT;
	}*/
	if(0!=detec(handle,Index416Data,IndexImage416.lHeight, IndexImage416.lWidth, 3, (int)IndexImage416.pixelArray.chunky.lLineBytes,&Indexnum, Indexboxout))
	{
		printf("error in detec.\n");
		//uninit(handle);
		res=-1;
		goto EXT;
	}
	//uninit(handle);
	
	//printf("sf6 %f seconds\n", sec(clock() - time));
    //int tmpgreater = 0;//
    //int tmpless = 0;//
    //int fingernum = 0;//
    //int Maxindex = 0;//
    //float MaxProb = 0.0;//
    //MV3POINT sf6finger = { 0 };//
    //int sf6Distance[100] = { 0 };//
    //int dMinDist = 0xFFFF;//
    //double MeterValue = 0;//
	for (int i = 0; i<Indexnum; i++)
	{
		if (Indexboxout[i][5] == 0)
			fingernum++;
	}
	if (fingernum == 0){
		printf("未找到指针\n");  //异常处理
		res=-1;
		goto EXT; 
	}
	
	for (int i = 0; i<Indexnum; i++){
		if (Indexboxout[i][5] == 0)
		{
			if (Indexboxout[i][4] > MaxProb){
				MaxProb = Indexboxout[i][4];
				Maxindex = i;
			}
		}
	}
	//printf("left=%f right=%f top=%f bot=%f\n",Indexboxout[Maxindex][0],Indexboxout[Maxindex][1],Indexboxout[Maxindex][2],Indexboxout[Maxindex][3]);
	sf6finger.x = (Indexboxout[Maxindex][1] + Indexboxout[Maxindex][0]) / 2;
	sf6finger.y = (Indexboxout[Maxindex][3] + Indexboxout[Maxindex][2]) / 2;
    //printf("%f,sf6finger.x=%d\n",MaxProb,sf6finger.x);
	
	Maxindex = 0;
	for (int i = 0; i<pPtPos.lPtNum; i++)//寻找离指针最近的刻度
	{
        //printf("pPtPos.ptPosList[%d].x=%d\n", i,pPtPos.ptPosList[i].x);
		sf6Distance[i] = sf6finger.x - pPtPos.ptPosList[i].x;
		if (dMinDist > abs(sf6Distance[i]))
		{
			dMinDist = abs(sf6Distance[i]);
			Maxindex = i;
		}
	}
    //printf("Maxindex=%d\n",Maxindex);
	if (sf6Distance[Maxindex] == 0)//指针在刻度上
	{
   
		MeterValue = pPtPos.dPtValList[Maxindex];
	}
	else//指针不在刻度上
	{
		for (int i = 0; i<pPtPos.lPtNum; i++)
		{
			if (sf6Distance[i] > 0)
				tmpgreater++;
			else
				tmpless++;
		}
		if (tmpgreater == pPtPos.lPtNum)//指针在最右边
		{
         
			MeterValue = pPtPos.dPtValList[pPtPos.lPtNum-1];
		}
		else if (tmpless == pPtPos.lPtNum)//指针在最左边
		{
     
			MeterValue = pPtPos.dPtValList[0];
		}
		else if(Maxindex == pPtPos.lPtNum-1)
		{
         
			MeterValue = (pPtPos.dPtValList[pPtPos.lPtNum-1] * abs(sf6Distance[pPtPos.lPtNum-2]) + pPtPos.dPtValList[pPtPos.lPtNum-2] * abs(sf6Distance[pPtPos.lPtNum-1])) / (abs(sf6Distance[pPtPos.lPtNum-1]) + abs(sf6Distance[pPtPos.lPtNum-2]));
		}
		else if(Maxindex == 0)
		{
            printf("zuixiaokedu\n");
            printf("pPtPos.dPtValList[0]=%f\n",pPtPos.dPtValList[0]);
            printf("pPtPos.dPtValList[1]=%f\n",pPtPos.dPtValList[1]);
            printf("abs(sf6Distance[0])=%d\n",abs(sf6Distance[0]));
            printf("abs(sf6Distance[1])=%d\n",abs(sf6Distance[1]));
			MeterValue = (pPtPos.dPtValList[0] * abs(sf6Distance[1]) + pPtPos.dPtValList[1] * abs(sf6Distance[0])) / (abs(sf6Distance[0]) + abs(sf6Distance[1]));
		}
		else//指针左右都有刻度
		{
			if (abs(sf6Distance[Maxindex + 1])>abs(sf6Distance[Maxindex - 1]))//另一侧为Maxindex-1
            {
             
				MeterValue = (pPtPos.dPtValList[Maxindex] * abs(sf6Distance[Maxindex - 1]) + pPtPos.dPtValList[Maxindex - 1] * abs(sf6Distance[Maxindex])) / (abs(sf6Distance[Maxindex - 1]) + abs(sf6Distance[Maxindex]));
			}
			else//另一侧为Maxindex+1
			{
                
				MeterValue = (pPtPos.dPtValList[Maxindex] * abs(sf6Distance[Maxindex + 1]) + pPtPos.dPtValList[Maxindex + 1] * abs(sf6Distance[Maxindex])) / (abs(sf6Distance[Maxindex + 1]) + abs(sf6Distance[Maxindex]));
			}
		}
	}
	//printf("MeterValue=%f\n",MeterValue);
	MeterResult[0].MeterValue=MeterValue;								  
EXT:
	if(Indexboxout)
	{
		for (int i = 0; i<Indexboxnum; i++){
			if(Indexboxout[i])
			{
				free(Indexboxout[i]);
				Indexboxout[i] = NULL;
			}
		}
	}
	if(Indexboxout)
	{
		free(Indexboxout);
		Indexboxout=NULL;
	}
	if(IndexImage.pixelArray.chunky.pPixel)
	{
		free(IndexImage.pixelArray.chunky.pPixel);
		IndexImage.pixelArray.chunky.pPixel=NULL;
	}
	if(IndexImage416.pixelArray.chunky.pPixel)
	{
		free(IndexImage416.pixelArray.chunky.pPixel);
		IndexImage416.pixelArray.chunky.pPixel=NULL;
	}
	return res;
}
