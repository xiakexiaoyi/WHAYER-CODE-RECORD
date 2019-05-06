#include<iostream>
#include "HalconCpp.h"
#include "HDevThread.h"
#include "HY_MeterRecogV3.h"

#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
extern "C"
{
#endif

#include "detector.h"

#ifdef __cplusplus
}
#endif
using namespace HalconCpp;

#define GrayStep(x)			(( (x) + 3 ) & (~3) )
typedef struct 
{
	network net;
	float lThresh;
}MRParam;
void HImageToHYImage(HObject Hobj, HYLPMRV3_IMAGES mask)
{
	HTuple htChannels;
	HTuple cType, width, height;
	ConvertImageType(Hobj, &Hobj, "byte");
	CountChannels(Hobj, &htChannels);
	if (htChannels[0].I() == 1)
	{
		//HTuple ptr;
		//GetImagePointer1(Hobj, &ptr, &cType, &width, &height);
		GetImageSize(Hobj, &width, &height);
		unsigned char *dstData = (unsigned char *)mask->pixelArray.chunky.pPixel;
		for (int j = 0; j < height.I(); j++)
		{
			for (int i = 0; i < width.I(); i++)
			{
				HTuple temp;
				GetGrayval(Hobj, j, i, &temp);
				dstData[i + j*mask->pixelArray.chunky.lLineBytes] = (unsigned char)((Hlong)temp);
			}
		}
	}
	else if (htChannels[0].I() == 3)
	{
		HObject ImageR, ImageG, ImageB;
		//HTuple ptrRed, ptrGreen, ptrBlue;
		//GetImagePointer3(Hobj, &ptrRed, &ptrGreen, &ptrBlue, &cType, &width, &height);
		GetImageSize(Hobj, &width, &height);
		Decompose3(Hobj, &ImageR, &ImageG, &ImageB);
		unsigned char *dstData = (unsigned char *)mask->pixelArray.chunky.pPixel;
		for (int j = 0; j < height.I(); j++)
		{
			for (int i = 0; i < width.I(); i++)
			{
				HTuple tempR, tempG, tempB;
				GetGrayval(ImageR, j, i, &tempR);
				GetGrayval(ImageG, j, i, &tempG);
				GetGrayval(ImageB, j, i, &tempB);
				dstData[j*mask->pixelArray.chunky.lLineBytes + i * 3 + 0] = (unsigned char)((Hlong)tempR);
				dstData[j*mask->pixelArray.chunky.lLineBytes + i * 3 + 1] = (unsigned char)((Hlong)tempG);
				dstData[j*mask->pixelArray.chunky.lLineBytes + i * 3 + 2] = (unsigned char)((Hlong)tempB);
			}
		}
	}
}

HObject HYImageToHImage(HYMRV3_IMAGES *pImage,int *flag)
{
	HObject Hobj;
	if (pImage->lPixelArrayFormat == HYAMR_IMAGE_GRAY)
	{
		int height = pImage->lHeight;
		int width = pImage->lWidth;
		int tmp;
		tmp = width*height;
		unsigned char *dataGray = new unsigned char[tmp];
		if(dataGray==NULL)//vc6.0
		{
			*flag=-1;
			return Hobj;
		}
		unsigned char *data = (unsigned char *)pImage->pixelArray.chunky.pPixel;
		for (int i = 0; i<height; i++)
		{
			memcpy(dataGray + width*i, data + pImage->pixelArray.chunky.lLineBytes*i, width);
		}
		GenImage1(&Hobj, "byte", pImage->lWidth, pImage->lHeight, (Hlong)(dataGray));
		delete[] dataGray;
	}
	if (pImage->lPixelArrayFormat == HYAMR_IMAGE_BGR)
	{
		int i = 0, j = 0;
		unsigned char* data = (unsigned char *)pImage->pixelArray.chunky.pPixel;
		unsigned char* dataRed = new unsigned char[pImage->lWidth*pImage->lHeight];
		unsigned char* dataGreen = new unsigned char[pImage->lWidth*pImage->lHeight];
		unsigned char* dataBlue = new unsigned char[pImage->lWidth*pImage->lHeight];
		if(dataRed==NULL||dataGreen==NULL||dataBlue==NULL)//vc6.0
		{
			*flag=-1;
			return Hobj;
		}
		int height = pImage->lHeight;
		int width = pImage->lWidth;
		for (j = 0; j<height; j++) {
			for (i = 0; i<width; i++) {
				dataRed[width*j + i] = data[j*pImage->pixelArray.chunky.lLineBytes + i * 3 + 0];
				dataGreen[width*j + i] = data[j*pImage->pixelArray.chunky.lLineBytes + i * 3 + 1];
				dataBlue[width*j + i] = data[j*pImage->pixelArray.chunky.lLineBytes + i * 3 + 2];
			
				
			}
		}
		GenImage3(&Hobj, "byte", pImage->lWidth, pImage->lHeight, (Hlong)(dataRed), (Hlong)(dataGreen), (Hlong)(dataBlue));
		delete[] dataRed;
		delete[] dataGreen;
		delete[] dataBlue;
	}
	return Hobj;
}
int polartrans(HYLPMRV3_IMAGES src, HYLPMRV3_IMAGES dst, TmpPara Meterdescrip, MV3PT *SF)
{
	int res=0;
	HObject  ho_Image, ho_ImagePolar;
	HTuple hv_WindowHandle;
	int hv_Column1 = Meterdescrip.Col;
	int hv_Row1 = Meterdescrip.Row;
	int hv_Radius1 = Meterdescrip.RadiusOut;
	int hv_Radius2 = Meterdescrip.RadiusIn;
	int hv_AngleStart = Meterdescrip.AngleStart;
	int hv_AngleEnd = Meterdescrip.AngleEnd;
	int hv_WidthP = dst->lWidth;
	int hv_HeightP = dst->lHeight;
	
	HObject ho_test, ho_ImagePolar2;

	int lWidth = src->lWidth;
	int lHeight = src->lHeight;
	unsigned char *imagegray = (unsigned char *)calloc(lWidth*lHeight, sizeof(unsigned char));
	if(imagegray==NULL)
	{
		res=-1;
		goto EXT;
	}

	ho_Image = HYImageToHImage(src,&res);
	if(res<0)
	{
		goto EXT;
	}

	PolarTransImageExt(ho_Image, &ho_ImagePolar, hv_Row1, hv_Column1, HTuple(hv_AngleStart).TupleRad(),
		HTuple(hv_AngleEnd).TupleRad(), hv_Radius1, hv_Radius2, hv_WidthP, hv_HeightP, "bilinear");
	//WriteImage(ho_ImagePolar, "bmp", 0, "D:/t50");
	HImageToHYImage(ho_ImagePolar, dst);

	
	HYMRV3_IMAGES pointtemp = { 0 };
	pointtemp.lHeight = dst->lHeight;//64
	pointtemp.lWidth = dst->lWidth;//416
	pointtemp.pixelArray.chunky.lLineBytes = GrayStep(pointtemp.lWidth);
	pointtemp.pixelArray.chunky.pPixel = (unsigned char *)calloc(pointtemp.lHeight*pointtemp.pixelArray.chunky.lLineBytes, sizeof(unsigned char));
	if(pointtemp.pixelArray.chunky.pPixel==NULL)
	{
		res=-1;
		goto EXT;
	}
	pointtemp.lPixelArrayFormat = HYAMR_IMAGE_GRAY;
	int i, j, m, n;
	
	//unsigned char *imagegray = new unsigned char[lWidth*lHeight];
	SF->lPtNum = 0;
	for (i = 0; i < Meterdescrip.lPtNum; i++)
	{
		for (j = 0; j<lHeight; j++)
		{
			for (n = 0; n<lWidth; n++)
			{
				//pData[j*mask.pixelArray.chunky.lLineBytes + i] = 255;
				imagegray[j*lWidth + n] = 255;
			}
		}
		//AffineTransPoint2d(hv_HomMat2DObject, pOutPattern->ptPosList[i].y, pOutPattern->ptPosList[i].x, &hv_Qy, &hv_Qx);
		//pOutPattern->ptPosList[i].x = int(hv_Qx.D());
		//pOutPattern->ptPosList[i].y = int(hv_Qy.D());
		//pData[pOutPattern->ptPosList[i].y*mask.pixelArray.chunky.lLineBytes + pOutPattern->ptPosList[i].x] = 0;
		imagegray[Meterdescrip.ptPosList[i].y*lWidth + Meterdescrip.ptPosList[i].x] = 0;
		imagegray[Meterdescrip.ptPosList[i].y*lWidth + Meterdescrip.ptPosList[i].x - 1] = 0;
		imagegray[Meterdescrip.ptPosList[i].y*lWidth + Meterdescrip.ptPosList[i].x + 1] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y - 1)*lWidth + Meterdescrip.ptPosList[i].x] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y + 1)*lWidth + Meterdescrip.ptPosList[i].x] = 0;

		imagegray[(Meterdescrip.ptPosList[i].y - 1)*lWidth + Meterdescrip.ptPosList[i].x - 1] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y + 1)*lWidth + Meterdescrip.ptPosList[i].x - 1] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y - 1)*lWidth + Meterdescrip.ptPosList[i].x + 1] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y + 1)*lWidth + Meterdescrip.ptPosList[i].x + 1] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y - 2)*lWidth + Meterdescrip.ptPosList[i].x] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y)*lWidth + Meterdescrip.ptPosList[i].x - 2] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y)*lWidth + Meterdescrip.ptPosList[i].x + 2] = 0;
		imagegray[(Meterdescrip.ptPosList[i].y + 2)*lWidth + Meterdescrip.ptPosList[i].x] = 0;
		//count++;
		GenImage1(&ho_test, "byte", lWidth, lHeight, (Hlong)imagegray);
		PolarTransImageExt(ho_test, &ho_ImagePolar2, hv_Row1, hv_Column1, HTuple(hv_AngleStart).TupleRad(),
			HTuple(hv_AngleEnd).TupleRad(), hv_Radius1 + 5, hv_Radius2, hv_WidthP, hv_HeightP, "bilinear");
		long line = pointtemp.pixelArray.chunky.lLineBytes;

		//dev_open_window_fit_image(ho_ImagePolar2, 0, 0, -1, -1, &hv_WindowHandle);
		//WriteImage(ho_ImagePolar2, "bmp", 0, "D:/t50");
		
		HImageToHYImage(ho_ImagePolar2, &pointtemp);
		unsigned char *Qdata = (unsigned char*)pointtemp.pixelArray.chunky.pPixel;
		int pointflag = 0;
		for (j = 0; j < hv_HeightP; j++)
		{
			for (m = 0; m < hv_WidthP; m++)
			{
				if (Qdata[line*j + m] != 255)
				{
					if (pointflag == 0)pointflag = 1;
					SF->ptPosList[i].x = m;
					SF->ptPosList[i].y = j;
					SF->dPtValList[i] = Meterdescrip.dPtValList[i];
				}
			}
		}
		if (pointflag == 1)
			SF->lPtNum = SF->lPtNum + 1;
	}
EXT:
	//free(imagegray);
	if(imagegray)
	{
		free(imagegray);
		imagegray=NULL;
	}
	if(pointtemp.pixelArray.chunky.pPixel)
	{
		free(pointtemp.pixelArray.chunky.pPixel);
		pointtemp.pixelArray.chunky.pPixel=NULL;
	}
	//free(pointtemp.pixelArray.chunky.pPixel);
	return res;
}
int init(void **handle,char *cfgfile, char *weightfile,float Thresh)
{
	int res=0;
	MRParam *tmpHandle = NULL;
	tmpHandle = (MRParam*)calloc(1,sizeof(MRParam));
	if(tmpHandle==NULL)
	{
		res=-1;
		goto EXT;
	}
	tmpHandle->lThresh = Thresh;
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
int uninit(void *handle)
{
	MRParam *netParam = (MRParam*)handle;
	COM_free_network(netParam->net);
	if (netParam)
	{
		free(netParam);
		netParam = NULL;
	}		
	return 0;
}
int detec(void *handle,unsigned char* imagdata, int height, int width, int channels, int widstep,int *numDST, float **boxout)
{
	int res=0;
	MRParam *pSRParam = (MRParam*)handle;
	image out = COM_make_image(width, height, channels);
	int i, j, k, count = 0;;
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
int MeterReadRecogV3(HYLPMRV3_IMAGES src,  TmpPara Meterdescrip,HYMR_POINTERRESULT *MeterResult)
{
	int res=0;
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
	MV3PT pPtPos = { 0 };
	//polartrans(src, &IndexImage, Meterdescrip, &pPtPos);
	if(0!=polartrans(src, &IndexImage, Meterdescrip, &pPtPos))
	{
		res=-1;
		goto EXT;
	}
	//showpolartrans(src, &IndexImage, Meterdescrip);
	
	HYMRV3_IMAGES IndexImage416 = { 0 };
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
	unsigned char *IndexData = (unsigned char*)IndexImage.pixelArray.chunky.pPixel;
	unsigned char *Index416Data = (unsigned char*)IndexImage416.pixelArray.chunky.pPixel;
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
	//YoloNet_detect yolo;
	float thresh = 0.1;
	int Indexnum;
	int Indexboxnum = 1000;
	float **Indexboxout;
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
	char *cfgfile = "../zhizhen/tiny-yolo-voc.cfg";
	char *weightfile = "../zhizhen/tiny-yolo-voc_final.weights";
	//time = clock();
	void *handle;
	//init(&handle,cfgfile,weightfile,thresh);
	if(0!=init(&handle,cfgfile,weightfile,thresh))
	{
		printf("error in init.\n");
		uninit(handle);
		res=-1;
		goto EXT;
	}
	//detec(handle,Index416Data,IndexImage416.lHeight, IndexImage416.lWidth, 3, (int)IndexImage416.pixelArray.chunky.lLineBytes,&Indexnum, Indexboxout);
	if(0!=detec(handle,Index416Data,IndexImage416.lHeight, IndexImage416.lWidth, 3, (int)IndexImage416.pixelArray.chunky.lLineBytes,&Indexnum, Indexboxout))
	{
		printf("error in detec.\n");
		uninit(handle);
		res=-1;
		goto EXT;
	}
	uninit(handle);
	//yolo.initNetData(cfgfile, weightfile);
	//yolo.object_detection(Index416Data, IndexImage416.lHeight, IndexImage416.lWidth, 3, (int)IndexImage416.pixelArray.chunky.lLineBytes, thresh, &Indexnum, Indexboxout);
	//yolo.ReleaseNetwork();
	//printf("sf6 %f seconds\n", sec(clock() - time));
	int tmpgreater = 0;
	int tmpless = 0;
	int fingernum = 0;
	int Maxindex = 0;
	float MaxProb = 0.0;
	MV3POINT sf6finger = { 0 };
	int sf6Distance[100] = { 0 };
	int dMinDist = 0xFFFF;
	double MeterValue = 0;
	for (int i = 0; i<Indexnum; i++)
	{
		if (Indexboxout[i][5] == 0)
			fingernum++;
	}
	if (fingernum == 0){
		printf("未找到sf6指针\n");  //异常处理
		res=-1;
		goto EXT;
		/*
		for (int i = 0; i<Indexboxnum; i++){
			free(Indexboxout[i]);
			Indexboxout[i] = NULL;
		}
		free(Indexboxout);
		Indexboxout = NULL;
		free(IndexImage.pixelArray.chunky.pPixel);
		free(IndexImage416.pixelArray.chunky.pPixel);
		return -1;
		*/
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
	//printf("sf6finger.x=%d\n",sf6finger.x);

	//pPtPos.lPtNum = 11;//找到多少个点由sf6函数传给我
	
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
		else if(Maxindex == pPtPos.lPtNum-1)//离指针最近的刻度为最大值
		{
			MeterValue = (pPtPos.dPtValList[pPtPos.lPtNum-1] * abs(sf6Distance[pPtPos.lPtNum-2]) + pPtPos.dPtValList[pPtPos.lPtNum-2] * abs(sf6Distance[pPtPos.lPtNum-1])) / (abs(sf6Distance[pPtPos.lPtNum-1]) + abs(sf6Distance[pPtPos.lPtNum-2]));
		}
		else if(Maxindex == 0)//离指针最近的刻度为最小值
		{
			MeterValue = (pPtPos.dPtValList[0] * abs(sf6Distance[1]) + pPtPos.dPtValList[1] * abs(sf6Distance[0])) / (abs(sf6Distance[0]) + abs(sf6Distance[1]));
		}
		else//离指针最近的刻度左右都有刻度
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
	printf("MeterValue=%f\n",MeterValue);
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