//
// iitp
// imageinfo.txt producer
//
// (c)2009 Piotr "utak3r" Borys
//

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <boost/lexical_cast.hpp>

typedef struct ROMHDR {
    ULONG   dllfirst;               // first DLL address
    ULONG   dlllast;                // last DLL address
    ULONG   physfirst;              // first physical address
    ULONG   physlast;               // highest physical address
    ULONG   nummods;                // number of TOCentry's
    ULONG   ulRAMStart;             // start of RAM
    ULONG   ulRAMFree;              // start of RAM free space
    ULONG   ulRAMEnd;               // end of RAM
    ULONG   ulCopyEntries;          // number of copy section entries
    ULONG   ulCopyOffset;           // offset to copy section
    ULONG   ulProfileLen;           // length of PROFentries RAM 
    ULONG   ulProfileOffset;        // offset to PROFentries
    ULONG   numfiles;               // number of FILES
    ULONG   ulKernelFlags;          // optional kernel flags from ROMFLAGS .bib config option
    ULONG   ulFSRamPercent;         // Percentage of RAM used for filesystem 
                                        // byte 0 = #4K chunks/Mbyte of RAM for filesystem 0-2Mbytes 0-255
                                        // byte 1 = #4K chunks/Mbyte of RAM for filesystem 2-4Mbytes 0-255
                                        // byte 2 = #4K chunks/Mbyte of RAM for filesystem 4-6Mbytes 0-255
                                        // byte 3 = #4K chunks/Mbyte of RAM for filesystem > 6Mbytes 0-255

    ULONG   ulDrivglobStart;        // device driver global starting address
    ULONG   ulDrivglobLen;          // device driver global length
    USHORT  usCPUType;              // CPU (machine) Type
    USHORT  usMiscFlags;            // Miscellaneous flags
    void    *pExtensions;           // pointer to ROM Header extensions
    ULONG   ulTrackingStart;        // tracking memory starting address
    ULONG   ulTrackingLen;          // tracking memory ending address
} ROMHDR;

#define ROM_EXTRA 9

struct info {                       /* Extra information header block      */
    unsigned long   rva;            /* Virtual relative address of info    */
    unsigned long   size;           /* Size of information block           */
};

typedef struct e32_rom {
    unsigned short  e32_objcnt;     /* Number of memory objects            */
    unsigned short  e32_imageflags; /* Image flags                         */
    unsigned long   e32_entryrva;   /* Relative virt. addr. of entry point */
    unsigned long   e32_vbase;      /* Virtual base address of module      */
    unsigned short  e32_subsysmajor;/* The subsystem major version number  */
    unsigned short  e32_subsysminor;/* The subsystem minor version number  */

    unsigned long   e32_stackmax;   /* Maximum stack size                  */
    unsigned long   e32_vsize;      /* Virtual size of the entire image    */
    unsigned long   e32_sect14rva;  /* section 14 rva */
    unsigned long   e32_sect14size; /* section 14 size */
	unsigned long   e32_timestamp;  /* Timestamp */

    struct info     e32_unit[ROM_EXTRA]; /* Array of extra info units      */
    unsigned short  e32_subsys;     /* The subsystem type                  */
} e32_rom;

typedef struct o32_rom {
    unsigned long       o32_vsize;      /* Virtual memory size              */
    unsigned long       o32_rva;        /* Object relative virtual address  */
    unsigned long       o32_psize;      /* Physical file size of init. data */
    unsigned long       o32_dataptr;    /* Image pages offset               */
    unsigned long   o32_realaddr;       /* pointer to actual                */
    unsigned long       o32_flags;      /* Attribute flags for the object   */
} o32_rom;

inline int stricmp (const std::string &s1, const std::string &s2)
{
	return stricmp (s1.c_str(), s2.c_str());
}

template<typename T>
T fromString(const std::string& s, 
			 std::ios_base& (*f)(std::ios_base&) = std::dec)
{
	std::istringstream is(s);
	T t;
	is >> f >> t; 
	return t;
} 

template<typename T>
std::string toString(const T& t, 
					 std::ios_base& (*f)(std::ios_base&) = std::dec) 
{
	std::ostringstream s;
	s << f << t; 
	return s.str();
}

void PrintHeader();
void PrintUsage();
bool find_file(const boost::filesystem::path & dir_path, 
			   const std::string & file_name, 
			   boost::filesystem::path & path_found);

int DecodeRomHdr();
int ConvertBin2Txt(std::string sModuleName, HANDLE fin, HANDLE fout);

ROMHDR romheader;


int _tmain(int argc, char *argv[])
{
	PrintHeader();

	// albo podajemy w parametrze physfirst, albo dekodujemy romhdr.bin
	if ((argc == 2) || (argc > 3))
	{
		PrintUsage();
		return 1;
	}
	if (argc == 1)
	{
		DecodeRomHdr();
	}
	if (argc == 3)
	{
		if (strcmp(argv[1], "-p") == 0)
		{
			std::string sPhysFirst(argv[2]);
			unsigned long uPhysFirst = fromString<unsigned long>(sPhysFirst, std::hex);
			romheader.physfirst = uPhysFirst;
		}
		else
		{
			PrintUsage();
			return 1;
		}
	}

	std::cerr << "physfirst = " << toString<unsigned long>(romheader.physfirst, std::hex) << std::endl;

	boost::filesystem::path file_found;
	find_file(".", "imageinfo.bin", file_found);

	return 0;
}

void PrintHeader()
{
	std::cerr << "IITP - imageinfo.txt producer - v0.1" << std::endl;
	std::cerr << "(c)2009 utak3r" << std::endl;
	std::cerr << "Catch me @ mobione.pl" << std::endl << std::endl;
}

void PrintUsage()
{
	std::cerr << "Usage:" << std::endl;
	std::cerr << "iitp [-p physfirst]" << std::endl << std::endl;
	std::cerr << "Value of physfirst is expected to be in hex."  << std::endl;
	std::cerr << "You can type it both as 0x1234abcd or 1234abcd." << std::endl;
	std::cerr << "If physfirst is not given, it will search for the romhdr.bin file" << std::endl;
	std::cerr << "and decode it into ROMHDR.txt" << std::endl << std::endl;
	std::cerr << "Thanks go to globalbus for the help and to c_shekhar" << std::endl;
	std::cerr << "for pushing me to make this little tool :=)" << std::endl << std::endl;
}

bool find_file(const boost::filesystem::path & dir_path, 
			   const std::string & file_name, 
			   boost::filesystem::path & path_found)
{
	bool found = false;
    for (boost::filesystem::recursive_directory_iterator itr(dir_path); 
		itr != boost::filesystem::recursive_directory_iterator(); 
		++itr)
    {
		if (is_regular_file(itr->status()))
		{
			if (itr->path().filename() == file_name)
			{
				path_found = itr->path();
				found = true;
				HANDLE hFBinHandle, hFTxtHandle;

				std::string binfilename = itr->path().string();
				boost::algorithm::replace_all(binfilename, "/", "\\");
				boost::algorithm::replace_all(binfilename, ".\\", "");
				if ((hFBinHandle = CreateFile((LPCTSTR)(binfilename.c_str()), GENERIC_READ,
												0, 0, OPEN_EXISTING, 0, 0))
								== INVALID_HANDLE_VALUE) {
									std::cerr << "Error opening " << binfilename << "!!\nError code " << GetLastError() << std::endl;
					exit(1);
				}
				std::string txtfilename = binfilename;
				boost::algorithm::replace_last(txtfilename, ".bin", ".txt");
				if ((hFTxtHandle = CreateFile((LPCTSTR)(txtfilename.c_str()), GENERIC_WRITE,
												0, 0, CREATE_ALWAYS, 0, 0))
								== INVALID_HANDLE_VALUE) {
					std::cerr << "Error creating " << txtfilename << "!!\nError code " << GetLastError() << std::endl;
					exit(1);
				}

				ConvertBin2Txt(itr->path().branch_path().filename(), hFBinHandle, hFTxtHandle);
				CloseHandle(hFBinHandle);
				CloseHandle(hFTxtHandle);

			}
		}
    }

  return found;
}

int DecodeRomHdr()
{
	HANDLE hFBin, hFTxt;
	if ((hFBin = CreateFile(TEXT("romhdr.bin"), GENERIC_READ,
									0, 0, OPEN_EXISTING, 0, 0))
					== INVALID_HANDLE_VALUE) {
		std::cerr << "Error opening romhdr.bin!!" << std::endl;
        exit(1);
    }
	if ((hFTxt = CreateFile(TEXT("ROMHDR.txt"), GENERIC_WRITE,
									0, 0, CREATE_ALWAYS, 0, 0))
					== INVALID_HANDLE_VALUE) {
		std::cerr << "Error creating ROMHDR.txt!!" << std::endl;
        exit(1);
    }

	DWORD iBytesRead = 0;
	DWORD iBytesWritten = 0;
	ReadFile(hFBin, &romheader, sizeof(ROMHDR), &iBytesRead, NULL);

	char hdrtxt[1024];
	//printf("ROM header:\n\n");

	sprintf(hdrtxt, "   dllfirst:            D=%08X \n", romheader.dllfirst);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   dlllast:               %08X \n", romheader.dlllast);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   physfirst:           P=%08X \n", romheader.physfirst);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   physlast:              %08X \n", romheader.physlast);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   nummods:              (%08X) \n", romheader.nummods);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulRAMStart:          R=%08X \n", romheader.ulRAMStart);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulRAMFree:             %08X \n", romheader.ulRAMFree);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulRAMEnd:              %08X \n", romheader.ulRAMEnd);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulCopyEntries:        (%08X) \n", romheader.ulCopyEntries);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulCopyOffset:        P+%08X \n", romheader.ulCopyOffset - romheader.physfirst);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulProfileLen:          %08X \n", romheader.ulProfileLen);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulProfileOffset:       %08X \n", romheader.ulProfileOffset);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   numfiles:             (%08X) \n", romheader.numfiles);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulKernelFlags:         %08X \n", romheader.ulKernelFlags);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulFSRamPercent:        %08X \n", romheader.ulFSRamPercent);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulDrivglobStart:       %08X \n", romheader.ulDrivglobStart);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulDrivglobLen:         %08X \n", romheader.ulDrivglobLen);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   usCPUType:             %08X \n", romheader.usCPUType);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   usMiscFlags:           %08X \n", romheader.usMiscFlags);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   pExtensions:         P+%08X \n", (ULONG)(romheader.pExtensions) - romheader.physfirst);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulTrackingStart:       %08X \n", romheader.ulTrackingStart);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   ulTrackingLen:         %08X \n", romheader.ulTrackingLen);
	//printf("%s", hdrtxt);
	WriteFile(hFTxt, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	//printf("\n\n");

	CloseHandle(hFBin);
	CloseHandle(hFTxt);
	return 0;
}

int ConvertBin2Txt(std::string sModuleName, HANDLE fin, HANDLE fout)
{
	DWORD iBytesRead = 0;
	DWORD iBytesWritten = 0;

	e32_rom e32;
	ReadFile(fin, &e32, sizeof(e32_rom), &iBytesRead, NULL);

	char hdrtxt[1024];
	std::cerr << "Working on module: " << sModuleName << std::endl;
	bool bEXE = false;
	if (sModuleName.length() > 4)
	{
		if (stricmp(sModuleName.substr(sModuleName.length()-3, 3), "exe") == 0)
			bEXE = true;
		else
			bEXE = false;
	}
	else
		bEXE = false;

	sprintf(hdrtxt, "  Module name: %s \n", sModuleName.c_str());
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);

	sprintf(hdrtxt, "   e32_objcnt:            %08X \n", e32.e32_objcnt);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_imageflags:        %08X \n", e32.e32_imageflags);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_entryrva:          %08X \n", e32.e32_entryrva);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_vbase:           V=%08X \n", e32.e32_vbase);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_subsysmajor:       %08X \n", e32.e32_subsysmajor);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_subsysminor:       %08X \n", e32.e32_subsysminor);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_stackmax:          %08X \n", e32.e32_stackmax);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_vsize:             %08X \n", e32.e32_vsize);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_sect14rva:         %08X \n", e32.e32_sect14rva);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_sect14size:        %08X \n", e32.e32_sect14size);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	sprintf(hdrtxt, "   e32_timestamp:         %08X \n", e32.e32_timestamp);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);

	for (int i = 0; i < ROM_EXTRA; i++)
	{
		sprintf(hdrtxt, "   e32_unit[%d].rva:       %08X \n", i, e32.e32_unit[i].rva);
		//printf("%s", hdrtxt);
		WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
		sprintf(hdrtxt, "   e32_unit[%d].size:      %08X \n", i, e32.e32_unit[i].size);
		//printf("%s", hdrtxt);
		WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	}

	sprintf(hdrtxt, "   e32_subsys:            %08X \n", e32.e32_subsys);
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	
	int o32_count = 0;
	sprintf(hdrtxt, "\n");
	//printf("%s", hdrtxt);
	WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
	do
	{
		o32_rom o32;
		ReadFile(fin, &o32, sizeof(o32_rom), &iBytesRead, NULL);
		if (iBytesRead > 0)
		{
			sprintf(hdrtxt, "   o32[%d].o32_vsize:      %08X \n", o32_count, o32.o32_vsize);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
			sprintf(hdrtxt, "   o32[%d].o32_rva:        %08X \n", o32_count, o32.o32_rva);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
			sprintf(hdrtxt, "   o32[%d].o32_psize:      %08X \n", o32_count, o32.o32_psize);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
			sprintf(hdrtxt, "   o32[%d].o32_dataptr:  P+%08X \n", o32_count, o32.o32_dataptr - romheader.physfirst);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);


			if (((o32.o32_flags == 0xC0000040)
				|| (o32.o32_flags == 0xC8000040)
				) && (!bEXE))
				sprintf(hdrtxt, "   o32[%d].o32_realaddr: D=%08X \n", o32_count, o32.o32_realaddr);
			else if ((o32.o32_flags == 0x60000020) 
					|| (o32.o32_flags == 0x40000040)
					|| (o32.o32_flags == 0x68000020)
					|| (o32.o32_flags == 0x48000040)
					|| ((o32.o32_flags == 0xC8000040) && bEXE)
					)
				sprintf(hdrtxt, "   o32[%d].o32_realaddr: V+%08X \n", o32_count, o32.o32_realaddr - e32.e32_vbase);
			else
				sprintf(hdrtxt, "   o32[%d].o32_realaddr:   %08X \n", o32_count, o32.o32_realaddr);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);

			sprintf(hdrtxt, "   o32[%d].o32_flags:      %08X \n", o32_count, o32.o32_flags);
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
			sprintf(hdrtxt, "\n");
			//printf("%s", hdrtxt);
			WriteFile(fout, hdrtxt, strlen(hdrtxt), &iBytesWritten, 0);
		}
		o32_count++;
	} while (iBytesRead > 0);
		
	//printf("\n\n");
	return 0;
}
