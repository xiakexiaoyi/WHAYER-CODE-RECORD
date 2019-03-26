#include "libImageQuality.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
using namespace std;

int main(int argc, const char** argv)
{
    cout<<"usage: testVideo.exe xxx.264 filestartTime processStartTime length gap condition"<<endl;
    if(argc<7)
        return 1;
    ifstream infile(argv[1], ios_base::in|ios_base::binary);

    infile.seekg(0, ios::end);
    streamoff off = infile.tellg();
    cout<<"break1"<<endl;
    cout<<"off is "<<off<<endl;
    vector<char> inData(off);
     cout<<"break2"<<endl;
    infile.seekg(0,ios::beg);
    cout<<"break3"<<endl;
    infile.read(&inData[0], off);

    cout<<"break4"<<endl;
    //filestartTime
    int64_t  filestartTime = atoi(argv[2]);
    int64_t  processStartTime = atoi(argv[3]);
    int64_t  length = atoi(argv[4]);
    int64_t  gap = atoi(argv[5]);
    int  condition = atoi(argv[6]);


    map<int, int> result;

    GetImageQuality(inData, filestartTime, processStartTime, length, gap, 0x0F, result);

    switch(result[BLUR])
    {
    case(IMAGE_UNBLUR):
        fprintf(stdout,"Image is not blur\n");
        break;
    case(IMAGE_TINYBLUR):
        fprintf(stdout,"Image is a little blurred\n");
        break;
    case(IMAGE_MEDBLUR):
        fprintf(stdout,"Image is median blurred\n");
        break;
    case(IMAGE_HEVBLUR):
        fprintf(stdout,"Image is heavy blurred\n");
        break;
    default:
        break;

    }
    switch(result[NOISE])
    {
    case(IMAGE_UNNOISE):
        fprintf(stdout,"Image is not noise\n");
        break;
    case(IMAGE_TINYNOISE):
        fprintf(stdout,"Image is a little noised\n");
        break;
    case(IMAGE_MEDNOISE):
        fprintf(stdout,"Image is median noise\n");
        break;
    case(IMAGE_HEAVENOISE):
        fprintf(stdout,"Image is heavy noise\n");
        break;
    default:
        break;

    }
    switch(result[LIGHT])
    {
    case(IMAGE_TOODARK):
        fprintf(stdout,"Image is too dark\n");
        break;
    case(IMAGE_TOOLIGHT):
        fprintf(stdout,"Image is too light\n");
        break;
    case(0):
        fprintf(stdout,"Image is normal light\n");
        break;

    default:
        break;
    }
    if(result[CAST]==IMAGE_CAST)
    {
        fprintf(stdout,"Image is cast\n");
    }
    else
    {
        fprintf(stdout,"Image is not cast\n");
    }

    return 1;
}
