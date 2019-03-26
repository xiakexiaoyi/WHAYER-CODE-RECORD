//------------------------------------------------------------------------------
// DriveInfo.cpp
#include "DriveInfo.h"
#include <sstream>
using namespace std;

const int DriveInfo::STRINGSIZE = 1024;
const int MEGA_UNIT=1024*1024;
vector<DriveStats> DriveInfo::m_vdriveStats;

DriveStats::DriveStats()
{
	m_strType = "Unknown";
}

DriveStats::DriveStats(const DriveStats& source)
{
	assign(source);
}

DriveStats& DriveStats::operator=(const DriveStats& right)
{
	if (this != &right)
	{
		assign(right);
	}

	return (*this);
}

DriveStats::~DriveStats()
{
	// nothing to do yet
}
void DriveStats::assign(const DriveStats& source)
{
	m_strType = source.m_strType;
	m_strName = source.m_strName;
	m_strTotalSpace = source.m_strTotalSpace;
	m_strFreeSpace = source.m_strFreeSpace;
	m_strVolumeName = source.m_strVolumeName;
	m_strSerialNumber = source.m_strSerialNumber;
	m_strFileSystemType = source.m_strFileSystemType;
	m_fFreeRatio = source.m_fFreeRatio;
	m_iTotalSpace = source.m_iTotalSpace;
	m_iFreeSpace = source.m_iFreeSpace;
}
DriveInfo::DriveInfo(const unsigned int& unDriveTypes/* = HARDDRIVE*/)
{
	determineDriveInfo(unDriveTypes);
}
DriveInfo::~DriveInfo()
{
	// nothing to do yet
}
string DriveInfo::MegaToStr(const size_t n64ByteCount /* = 0 */, const int& nPrecision /* = 2 */)
{
	const int NUMFORMATTERS = 3;
	char szFormatters[NUMFORMATTERS][10] = {" MB", " GB", " TB" };
	double dblBase = n64ByteCount;
	int nNumConversions = 0;

	while (dblBase > 1000)
	{
		dblBase /= 1024;
		nNumConversions++;
	}

	string strUnits;
	if ((0 <= nNumConversions) && (nNumConversions <= NUMFORMATTERS))
	{
		strUnits = szFormatters[nNumConversions];
	}

	ostringstream os;
	os.setf(ostringstream::fixed);
	os.precision(nPrecision);
	os << dblBase << strUnits << ends;
	return (os.str());   
}
#ifdef WIN32
void DriveInfo::determineDriveInfo(const unsigned int& unDriveTypes/* = HARDDRIVE*/)
{
	HINSTANCE hModule = NULL;                     
	if ((hModule = LoadLibrary((LPCSTR)"KERNEL32.DLL")) != NULL)
	{
		DWORD dwDriveList = ::GetLogicalDrives();
		DWORD dwMinMask = 0x00000001;
		m_vdriveStats.clear();

		// check to see if they want to test for floppy drives
		for (int i = (unDriveTypes & FLOPPY) == FLOPPY ? 0 : 2; i < 26; i++)
		{
			DriveStats driveStats;

			if (dwDriveList & (dwMinMask << i))
			{
				// set the drive name
				ostringstream os;
				os << (char)('A' + i) << ":\\" << ends;
				driveStats.setName(os.str());

				// make sure the drive is a valid type
				// we don't want floppy or cdrom, because they will
				// require the user to insert a disk if there is no disk....
				unsigned int unType = GetDriveType((LPCSTR)driveStats.getName().c_str());
				if (unType != DRIVE_NO_ROOT_DIR)
				{
					if ( (((unDriveTypes & CDROM) == CDROM) && (unType == DRIVE_CDROM)) ||
						(((unDriveTypes & HARDDRIVE) == HARDDRIVE) && (unType == DRIVE_FIXED)) ||
						(((unDriveTypes & NETWORK) == NETWORK) && (unType == DRIVE_REMOTE)) ||
						(((unDriveTypes & FLOPPY) == FLOPPY) && (unType == DRIVE_REMOVABLE)) )
					{
						PFNGETDISKFREESPACEEX pDiskFreeSpaceEx = NULL;

						// set the string version of the type based on unType
						switch (unType)
						{
						case DRIVE_UNKNOWN:
							driveStats.setType("Unknown drive");
							break;
						case DRIVE_REMOVABLE:
							driveStats.setType("Removable drive");
							break;
						case DRIVE_FIXED:
							driveStats.setType("Fixed drive");
							break;
						case DRIVE_REMOTE:
							driveStats.setType("Network drive");
							break;
						case DRIVE_CDROM:
							driveStats.setType("CD-ROM drive");
							break;
						case DRIVE_RAMDISK:
							driveStats.setType("RAM disk");
							break;
						}

						// use GetDiskFreeSpaceEx, if available. otherwise, use the crappy version [GetDiskFreeSpace]
						if ((pDiskFreeSpaceEx = (PFNGETDISKFREESPACEEX)GetProcAddress(hModule, "GetDiskFreeSpaceExA")) == NULL)
						{	
							DWORD dwSectorsPerCluster = 0; 
							DWORD dwBytesPerSector = 0;    
							DWORD dwFreeClusters = 0;      
							DWORD dwClusters = 0;          		
							BOOL fResult = GetDiskFreeSpace((LPCSTR)driveStats.getName().c_str(), &dwSectorsPerCluster, &dwBytesPerSector, 
								&dwFreeClusters, &dwClusters);

							DWORD dwTotal = fResult ? (dwClusters * dwBytesPerSector * dwSectorsPerCluster) : 0;
							DWORD dwFree = fResult ? (dwFreeClusters * dwBytesPerSector * dwSectorsPerCluster) : 0;

							dwTotal/=MEGA_UNIT;
							dwFree/=MEGA_UNIT;

							driveStats.setTotalSpace(MegaToStr(dwTotal));
							driveStats.setFreeSpace(MegaToStr(dwFree));
							driveStats.SetSpaceParam(dwTotal, dwFree);
						}
						else
						{
							ULARGE_INTEGER uliTotalNumBytes;         
							ULARGE_INTEGER uliTotalFreeBytes;        
							ULARGE_INTEGER uliTotalAvailableToCaller;

							BOOL fResult = pDiskFreeSpaceEx((LPCSTR)driveStats.getName().c_str(), &uliTotalAvailableToCaller, &uliTotalNumBytes, &uliTotalFreeBytes);

							uliTotalNumBytes.QuadPart/=MEGA_UNIT;
							uliTotalFreeBytes.QuadPart/=MEGA_UNIT;

							driveStats.setTotalSpace(fResult ? MegaToStr((size_t)uliTotalNumBytes.QuadPart) : "0");
							driveStats.setFreeSpace(fResult ? MegaToStr((size_t)uliTotalFreeBytes.QuadPart) : "0");
							driveStats.SetSpaceParam((size_t)uliTotalNumBytes.QuadPart, (size_t)uliTotalFreeBytes.QuadPart);
						}

						determineVolumeInfo(&driveStats);
						m_vdriveStats.push_back(driveStats);
					}
				}
			}
		}

		FreeLibrary(hModule);
	}
}

// this function's just used to reduce complexity in determineDriveInfo
void DriveInfo::determineVolumeInfo(DriveStats* pDriveStats) const
{
	char szVolumeName[STRINGSIZE] = {0};
	char szFileSystemName[STRINGSIZE] = {0};
	DWORD dwVolumeSerialNumber = 0;
	DWORD dwMaxComponentLength = 0;  // not used yet
	DWORD dwFileSystemFlags = 0;     // not used yet

	GetVolumeInformation((LPWSTR)pDriveStats->getName().c_str(), 
		(LPWSTR)szVolumeName, STRINGSIZE, 
		&dwVolumeSerialNumber, &dwMaxComponentLength,
		&dwFileSystemFlags, 
		(LPCSTR)szFileSystemName, STRINGSIZE);

	// convert serial number to string
	ostringstream os;
	os << dwVolumeSerialNumber << ends;

	pDriveStats->setSerialNumber(os.str());
	pDriveStats->setVolumeName(szVolumeName);
	pDriveStats->setFileSystemType(szFileSystemName);
}
#else

#include "ctutil.h"
#include "process_op_fun.h"
#include <patterns/util/StringUtil.hpp>
using namespace suntek::util;
#include<iostream>
using namespace std;
void DriveInfo::determineDriveInfo(const unsigned int& unDriveTypes/* = HARDDRIVE*/)
{	
	const char* szDriverFile="drivers.data";
	const char* szShellFile="driver.sh";

	const int MAX_PATH=256;
	
	 if (access(szShellFile,0))
	 {
		char szShellCmd[MAX_PATH+1]="/bin/df -l -P -T > ";
		strcat(szShellCmd,szDriverFile);

		FILE* fp=fopen(szShellFile,"a+");
		if(!fp)
		{
			fprintf(stderr,"Open file %s fail!\n",szShellFile);
			return;
		}
		fwrite(szShellCmd,strlen(szShellCmd),1,fp);
		fclose(fp);
	 }

	char szCmdLine[MAX_PATH+1]="/bin/bash ";	
	strcat(szCmdLine,szShellFile);

	DWORD pi(0);
	//fprintf(stderr,"Create process(%s)...\n",szCmdLine);
	pi=RunProcess(szCmdLine,"/bin/");	
	//fprintf(stderr,"%d Done!!!\n",pi);

	std::string strDriverInfo;
	ReadDataFromFile(szDriverFile,strDriverInfo);
	// cout<<"strDriverInfo:"<<strDriverInfo<<endl;
	//fprintf(stderr,"%s",strDriverInfo.c_str());

	m_vdriveStats.clear();
	DriveStats driveStats;     

	char strLn[2]={0xA,0};
	vector<string> vctDrivers;
	StringUtil::split(vctDrivers,strDriverInfo,strLn);

	for(size_t i=1;i<vctDrivers.size();i++)
	{
		vector<string> vctDriver;
		StringUtil::split(vctDriver,vctDrivers[i]," ",false);

		//fprintf(stderr,"%s\n",vctDrivers[i].c_str());
		if (vctDriver.size()<7)
		{
			fprintf(stderr,"Invalid data segments(%d)!!!\n",vctDriver.size());
			continue;
		}		
		driveStats.setName(vctDriver[0]);
		driveStats.setFileSystemType(vctDriver[1]);
		driveStats.setType("Fixed drive");

		size_t dwTotal = atol(vctDriver[2].c_str())/1024;
		size_t dwFree = atol(vctDriver[4].c_str())/1024;

		driveStats.setTotalSpace(MegaToStr(dwTotal));
		driveStats.setFreeSpace(MegaToStr(dwFree));
		driveStats.SetSpaceParam(dwTotal, dwFree);		
		driveStats.setVolumeName(vctDriver[6]);
		m_vdriveStats.push_back(driveStats);
	}	
}
#endif
