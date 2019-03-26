//------------------------------------------------------------------------------
// DriveInfo.h
// 
#ifndef DRIVEINFO_H_
#define DRIVEINFO_H_

#include <vector>
#include <iterator>
#include <utility>
#include <string>
using namespace std;

#ifdef WIN32
#include <Windows.h>
typedef BOOL (WINAPI* PFNGETDISKFREESPACEEX)(LPCTSTR, PULARGE_INTEGER,
	 								 PULARGE_INTEGER, PULARGE_INTEGER);
#endif

class DriveStats
{
public:        // object creation/destruction
	DriveStats();
	DriveStats(const DriveStats& source);
	DriveStats& operator=(const DriveStats& right);
	virtual ~DriveStats();

public:      
	float GetFreeRatio() const;
	size_t GetFreeSpaceNum() const;
	size_t GetTotalSpaceNum() const;
	void SetSpaceParam(size_t totalSpace, size_t freeSpace);
	void getType(unsigned int* punType) const;
	std::string getType() const;
	std::string getName() const;
	std::string getTotalSpace() const;
	std::string getFreeSpace() const;
	std::string getVolumeName() const;
	std::string getSerialNumber() const;
	std::string getFileSystemType() const;
	std::string toString() const;	

#if defined(SYSINFO_USEOLDINTERFACE)
	void setType(const unsigned int& unType);
#endif
	void setType(const std::string& strType);
	void setName(const std::string& strName);
	void setTotalSpace(const std::string& strTotalSpace);
	void setFreeSpace(const std::string& strFreeSpace);
	void setVolumeName(const std::string& strVolumeName);
	void setSerialNumber(const std::string& strSerialNumber);
	void setFileSystemType(const std::string& strFileSystemType);

protected:     // protected members
	virtual void assign(const DriveStats& source);

private:       // attributes
	float m_fFreeRatio;
	size_t m_iTotalSpace;
	size_t m_iFreeSpace;
	std::string  m_strType;          // type of drive
	std::string  m_strName;          // drive name
	std::string  m_strTotalSpace;    // total drive space
	std::string  m_strFreeSpace;     // total free space
	std::string  m_strVolumeName;    // name of the volume as set by the user
	std::string  m_strSerialNumber;  // not supported for network drives on Win9x/Me
	std::string  m_strFileSystemType;// NTFS, FAT32 [NOT an enum on purpose]
};
class DriveInfo
{
	enum
	{
		REFRESH_CYCLE = 900// 15 * 60s = 900
	};
public:
	enum DiskTypes { HARDDRIVE = 1, NETWORK = 2, FLOPPY = 4, CDROM = 8 };

	DriveInfo(const unsigned int& unDriveTypes = HARDDRIVE);
	virtual ~DriveInfo();
	const vector<DriveStats>& getDriveStats() const;

private:     
	virtual void determineVolumeInfo(DriveStats* pDriveStats) const;
	virtual void determineDriveInfo(const unsigned int& unDriveTypes = HARDDRIVE);
	string MegaToStr(const size_t n64ByteCount = 0, const int& nPrecision = 2 );
private:      
	static vector<DriveStats> m_vdriveStats;
	static const int STRINGSIZE;
private:
	DriveInfo(const DriveInfo& source);
	DriveInfo& operator=(const DriveInfo& right);
};


inline const vector<DriveStats>& DriveInfo::getDriveStats() const 
{ 
	return m_vdriveStats;
}

inline 	size_t DriveStats::GetFreeSpaceNum() const
{	return m_iFreeSpace;	}

inline 	size_t DriveStats::GetTotalSpaceNum() const
{	return m_iTotalSpace;	}

inline void DriveStats::SetSpaceParam(size_t totalSpace, size_t freeSpace)
{
	m_iTotalSpace = totalSpace;
	m_iFreeSpace = freeSpace;
	m_fFreeRatio = ((float)freeSpace) / ((float)totalSpace);
}

inline float DriveStats::GetFreeRatio() const
{	return m_fFreeRatio;}

// clients of SysInfo get these
inline std::string DriveStats::getType() const { return (m_strType); }
inline std::string DriveStats::getName() const { return (m_strName); }
inline std::string DriveStats::getTotalSpace() const { return (m_strTotalSpace); }
inline std::string DriveStats::getFreeSpace() const {  return (m_strFreeSpace); }
inline std::string DriveStats::getVolumeName() const { return (m_strVolumeName); }
inline std::string DriveStats::getSerialNumber() const { return (m_strSerialNumber); }
inline std::string DriveStats::getFileSystemType() const { return (m_strFileSystemType); }

// ONLY DriveInfo and subclasses get these
inline void DriveStats::setType(const std::string& strType) { m_strType = strType; }
inline void DriveStats::setName(const std::string& strName) { m_strName = strName; }
inline void DriveStats::setTotalSpace(const std::string& strTotalSpace) { m_strTotalSpace = strTotalSpace; }
inline void DriveStats::setFreeSpace(const std::string& strFreeSpace) { m_strFreeSpace = strFreeSpace; }
inline void DriveStats::setVolumeName(const std::string& strVolumeName) { m_strVolumeName = strVolumeName; }
inline void DriveStats::setSerialNumber(const std::string& strSerialNumber) { m_strSerialNumber = strSerialNumber; }
inline void DriveStats::setFileSystemType(const std::string& strFileSystemType) { m_strFileSystemType = strFileSystemType; }

#endif
