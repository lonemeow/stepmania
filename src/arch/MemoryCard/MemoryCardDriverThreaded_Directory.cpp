#include "global.h"
#include "MemoryCardDriverThreaded_Directory.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageTimer.h"
#include "Foreach.h"
#include "PlayerNumber.h"
#include "MemoryCardManager.h"

static int g_currentSerial = 0;

static struct timespec invalid_timespec = { 0, 0 };

bool operator==( const struct timespec &a, const struct timespec &b )
{
	return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}

bool operator!=( const struct timespec &a, const struct timespec &b )
{
	return !(a == b);
}

bool MemoryCardDriverThreaded_Directory::TestWrite( UsbStorageDevice* pDevice )
{
	return true;
}

bool MemoryCardDriverThreaded_Directory::USBStorageDevicesChanged()
{
	if( m_ForceChanged )
	{
		LOG->Trace( "Forced change in USB devices." );
		m_ForceChanged = false;
		return true;
	}

	bool bChanged = false;
	FOREACH_ENUM( PlayerNumber, p )
	{
		const RString &profileDir = MEMCARDMAN->m_sMemoryCardOsMountPoint[p];
		struct stat statbuf;

		if( stat( profileDir.c_str(), &statbuf ) < 0 )
			statbuf.st_mtim = invalid_timespec;

		if( m_LastModified[p] == statbuf.st_mtim )
			continue;

		bChanged = true;
	}
	if( bChanged )
		LOG->Trace( "Change in USB storage devices detected." );
	return bChanged;
}

void MemoryCardDriverThreaded_Directory::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	vDevicesOut.clear();

	FOREACH_ENUM(PlayerNumber, p)
	{
		const RString &profileDir = MEMCARDMAN->m_sMemoryCardOsMountPoint[p];
		struct stat statbuf;

		if( stat( profileDir.c_str(), &statbuf ) < 0 )
			statbuf.st_mtim = invalid_timespec;

		if( m_LastModified[p] != statbuf.st_mtim )
		{
			m_LastModified[p] = statbuf.st_mtim;
			m_ForceChanged = true;
			continue;
		}

		if( statbuf.st_mtim != invalid_timespec )
		{
			UsbStorageDevice dev;
			dev.sOsMountDir = profileDir;
 			dev.sSerial = StringConversion::ToString(g_currentSerial++);
			vDevicesOut.push_back( dev );
		}
	}
}

bool MemoryCardDriverThreaded_Directory::Mount( UsbStorageDevice* pDevice )
{
	return true;
}

void MemoryCardDriverThreaded_Directory::Unmount( UsbStorageDevice* pDevice )
{
}

/*
 * (c) 2017 Ilpo Ruotsalainen
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
