/*!
* \file LiErrFunction.h
* \brief  the function related to error function and gamma function 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
/*这部分代码是根据《数字信号处理c代码》修改，经验证该部分代码可行*/


#include "limath.h"
#include "lidebug.h"
#include "liblock.h"
#include "liconv.h"
#include"limatrix.h"
#include "limits.h"
#include "LiErrFunc.h"


#define PI 3.141526

/// \brief  LiGammcdf  calculate the cumulative distribution of gamma
/// function
/// \param  Res:the result
/// \param  data1:the source data
/// \param  shape the shape parameter
/// \param  scale the scale parameter 
/// \return  error code
MRESULT LiGamcdf(MFloat *Res,MFloat data1,MFloat shape,MFloat scale)
{
    MRESULT res=LI_ERR_NONE;
    MFloat temp,temp1;
    MFloat eps=0.00000001;
    if (shape<eps||scale<eps)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    if (data1<eps)
    {
        *Res=0;
    }
    temp=data1/scale;
    LiGaminc(&temp1,shape,temp);

    *Res=temp1;

    return res;
}


/// \brief  LiGammpdf  calculate the probability distribution of gamma
/// function
/// \param  Res:the result
/// \param  data1:the source data
/// \param  shape the shape parameter
/// \param  scale the scale parameter 
/// \return  error code
MRESULT LiGampdf(MFloat *Res,MFloat data1,MFloat shape,MFloat scale)
{
    MRESULT res=LI_ERR_NONE;
    MFloat temp,temp1,temp2;
    MFloat eps=0.00000001;
    if (shape<eps||scale<eps)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    if (data1<eps)
    {
        *Res=0;
    }
    temp=data1/scale;
    LiLogGam(&temp2,shape);
    temp1=(shape-1)*log((float)temp)-temp-temp2;
    *Res=exp((float)temp1)/scale;
    
    return res;
}


/// \brief  LiGamma  calculate the  gamma function
/// \param  result the result
/// \param  data1:the source data
/// \return  error code
MRESULT LiGamma(MDouble *result,MDouble data1)
{
    MLong res=LI_ERR_NONE;
    MLong  i;
    MDouble y,t,s,u;
    static double a[11]={ 0.0000677106,-0.0003442342,
        0.0015397681,-0.0024467480,0.0109736958,
        -0.0002109075,0.0742379071,0.0815782188,
        0.4118402518,0.4227843370,1.0};
    MDouble x=data1;
    if (x<=0.0)
    {
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }
    y=x;
    if (y<=1.0)
    { 
        t=1.0/(y*(y+1.0));
        y=y+2.0;}
    else if (y<=2.0)
    { 
        t=1.0/y; 
        y=y+1.0;}
    else if (y<=3.0) 
        {
            t=1.0;
    }
    else
    {
        t=1.0;
        while (y>3.0)
        {
            y=y-1.0; 
            t=t*y;
        }
    }
    s=a[0]; u=y-2.0;
    for (i=1; i<=10; i++)
    {
        s=s*u+a[i];
    }
    s=s*t;
    *result=s;
    return res;

}



/// \brief  LiGaminc calculate the incomplete gamma function
/// \param  result :the result
/// \param  data1:the source data1
/// \param  data2:the source data2
/// \return  error code
MRESULT LiGaminc(MFloat *Result,MFloat data1,MFloat data2)
{
    MLong n;
    MDouble a=data1;
    MDouble x=data2;
    MRESULT res=LI_ERR_NONE;
    MDouble p,q,d,s,s1,p0,q0,p1,q1,qq;
    MDouble temp=0.0;
    MDouble temp1=0.0;
    MFloat eps=0.00000001;
    if ((a<=eps)||(x<eps))
    { 
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }
    q=log(x);
    q=a*q; 
    qq=exp(q);
    if (x<1.0+a)
    { 
        p=a; 
        d=1.0/a; 
        s=d;
    for (n=1; n<=100; n++)
    { 
        p=1.0+p; 
        d=d*x/p; 
        s=s+d;
    if (fabs(d)<fabs(s)*1.0e-07)
    { 
        LiGamma(&temp,a);
        s=s*exp(-x)*qq/temp;
        temp1=s;
        break;
    }
    }
    }
    else
    { 
        s=1.0/x;
        p0=0.0; 
        p1=1.0;
        q0=1.0;
        q1=x;
    for (n=1; n<=100; n++)
    {
        p0=p1+(n-a)*p0; q0=q1+(n-a)*q0;
        p=x*p0+n*p1; q=x*q0+n*q1;
        if (fabs(q)+1.0!=1.0)
        {
            s1=p/q;
            p1=p;
            q1=q;
    if (fabs((s1-s)/s1)<1.0e-07)
    { 
        LiGamma(&temp,a);
        s=s1*exp(-x)*qq/temp;
         temp1=1-s;
    }
    s=s1;
    }
    p1=p;
    q1=q;
    }

    LiGamma(&temp,a);
    s=1.0-s*exp(-x)*qq/temp;
    temp1=s;
    }
    *Result=temp1;
    
    return res;
}


/// \brief  LiLogGam  calculate gamma function after  log
/// \param  Res :the result
/// \param  data1:the source data
/// \return  error code
MRESULT LiLogGam(MFloat *Res,MFloat data1)
{
    MRESULT res=LI_ERR_NONE;
    MDouble gamma;
    MDouble data=data1;
    LiGamma(&gamma,data);
    *Res=log((float)gamma);
    return res;

}



/// \brief  LiGamInv  calculate inverse gamma function 
/// \param  Res the result
/// \param data1 the source data
/// \param  shape the shape parameters
/// \param  scale the scale parameters 
/// \return  error code
MRESULT LiGamInv(MFloat *Res,MFloat data1,MFloat shape,MFloat scale)
{
    /// \Initial the parameters
    MLong res=LI_ERR_NONE;
    MFloat q=0;
    MFloat p1=0;
    MFloat loga=0.0;
    MFloat sigsq=0.0;
    MFloat mu=0.0;
    MLong maxiter=0;
    MDouble eps=1.0e-007;
    MLong i;
    MFloat F,dF,f,h,qnew;
    MFloat temp;
    /// \Judgement of the parameters
    if (shape<0||scale<0)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    if (data1>1||data1<0)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    /// \ Initial guess of q
    loga=log((float)shape);
    sigsq=log((float)(1+shape))-loga;
    mu=loga-0.5*sigsq;
    ErfCinv(&p1,(2*data1));
    q=exp((float)(mu-(sqrt((float)2*sigsq))*p1));
    maxiter=500;
    LiGamcdf(&F,q,shape,1);
    dF=F-data1;
    for(i=0;i<maxiter;i++)
    {
        LiGampdf(&f,q,shape,1);
        h=dF/f;
        qnew=MAX(q/10,MIN(10*q,q-h));
        if (fabs(h)<eps)
        {
            q=qnew;
            break;
        }
    }
    temp=q*scale;
    *Res=temp;

    return res;
}

/// \brief  Erfinv  calculate the inverse error function
/// \param  datares:the result
/// \param  datasrc:the source data
/// \return  error code
MRESULT Erfinv(MFloat* datares,MFloat datasrc)
{
    MLong res=LI_ERR_NONE;
    MDouble d1,d2,d3,d4,d5,d6,d7,d8;
    MFloat D;
    MFloat temp;
    if (datasrc>1||datasrc<-1)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    d1=datasrc;
    d2=(PI)*(pow((double)d1,3))/12;
    d3=(7*pow((double)PI,2)*pow((double)d1,5))/480;
    d4=(127*pow((double)PI,3)*pow((double)d1,7))/40320;
    d5=(4369*pow((double)PI,4)*pow((double)d1,9))/5806080;
    d6=(34807*pow((double)PI,5)*pow((double)d1,11))/182476800;
    d7=(5.76*pow((float)PI,6)*pow((double)d1,13))/(4096*13);
    d8=(10.06*pow((float)PI,7)*pow((double)d1,15))/(16384*15);
    
    D=d1+d2+d3+d4+d5+d6+d7+d8;
    temp=sqrt((float)PI);
    *datares=0.5*temp*D;
    return res;
}


/// \brief  ErfCinv  calculate the inverse  complement error function
/// \param  datares:the result
/// \param  datasrc:the source data
/// \return  error code
MRESULT ErfCinv(MFloat* datares,MFloat datasrc)
{
    MLong res=LI_ERR_NONE;
    if (datasrc>2||datasrc<0)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    
    Erfinv(datares,1-datasrc);

    return res;
}

/// \brief  Erfc  calculate the complement error function
/// \param  datares:the result
/// \param  datasrc:the source data
/// \return  error code
MRESULT Erfc(MFloat* datares,MFloat datasrc)
{
    MLong res=LI_ERR_NONE;
    MFloat temp=0.0;
    Erf(&temp,datasrc);
    *datares=1-temp;
    return res;
}


/// \brief  Erf  calculate  error function
/// \param  datares:the result
/// \param  datasrc:the source data
/// \return  error code
MRESULT Erf(MFloat *datares,MFloat datasrc)
{
    MLong res=LI_ERR_NONE;
    MFloat tmp;
    MFloat x=datasrc;
    MFloat a,b,eps;
    MFloat fftsf(MFloat);
    
    a=0.0;
    b=x;
    eps=0.00001;
    tmp=0;
    LiIntrgral(&tmp,a,b,eps,fftsf);
    tmp=2/sqrt((float)PI)*tmp;
    *datares=tmp;
    
return res;

}

/// \brief  fftsf  calculate exp(-x*x)
/// \param  x the index
/// \return  y=exp(-x*x)
MFloat fftsf(MFloat x)

{
    MFloat y;
    y=exp(-x*x);
    return y;
}

/// \brief  LiIntrgral  calculate  the integral 
/// \param  datares:  result
/// \param  a upper limit of the integral 
/// \param  b lower limit of the integral 
/// \return  error code
MRESULT LiIntrgral(MFloat* datares, MLong a,MLong b, MFloat eps,MFloat IntrgraF(MFloat))
{
    MLong res=LI_ERR_NONE;
    MLong n,k;
    MFloat fa,fb,h,t1,p,s,x,t;
    fa=IntrgraF(a); fb=IntrgraF(b);
    n=1; h=b-a;
    t1=h*(fa+fb)/2.0;
    p=eps+1.0;
    while (p>=eps)
    { s=0.0;
    for (k=0;k<=n-1;k++)
    {
        x=a+(k+0.5)*h;
        s=s+IntrgraF(x);
    }
    t=(t1+h*s)/2.0;
    p=fabs(t1-t);
    t1=t; n=n+n; h=h/2.0;
    }
    *datares=t;


    return res;
}
