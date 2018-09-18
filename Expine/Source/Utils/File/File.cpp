#include "Utils/File/File.h"
#include <WindowsH.h>
#include <fstream>
#include <ostream>

namespace File
{
	FileMappingStream::~FileMappingStream()
	{
		Close();
	}

	bool FileMappingStream::Open(const WStringView& File, std::ios::openmode Mode)
	{
		DWORD Access = FILE_GENERIC_EXECUTE;
		DWORD AccessMem = PAGE_EXECUTE;

		if (Mode & std::ios::in)
		{
			Access |= FILE_GENERIC_READ;
			AccessMem = PAGE_EXECUTE_READ;
		}

		if (Mode & std::ios::out)
		{
			Access |= FILE_GENERIC_WRITE;
			AccessMem = PAGE_EXECUTE_READWRITE;
		}

		return Open(CreateFile2(File.data(), Access, 0, OPEN_EXISTING, NULL));
	}

	bool FileMappingStream::Open(HANDLE FileHandle)
	{
		if (FileHandle == NULL)
		{
			return false;
		}

		FILE_STANDARD_INFO FileInfo;

		if (!GetFileInformationByHandleEx(FileHandle, FileStandardInfo, &FileInfo, sizeof(FileInfo)))
		{
			return false;
		}

		MappingHandle = CreateFileMapping(FileHandle, NULL, FILE_ALL_ACCESS, FileInfo.AllocationSize.HighPart, FileInfo.AllocationSize.LowPart, NULL);
		MappingSizeMax = FileInfo.AllocationSize.QuadPart;
		MappingSize = 0;

		return MappingHandle != NULL;
	}


	TVector<Byte> & FileMappingStream::GetContent(size_t MinBytes)
	{
		MinBytes = std::min(MinBytes, MappingSizeMax);

		if (MappingSize < MinBytes)
		{
			void * Data = MapViewOfFile(MappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, MinBytes);
		}

		return MappingBytes;
	}

	const TVector<Byte> & FileMappingStream::GetContent(size_t MinBytes) const
	{
		return MappingBytes;
	}

	bool FileMappingStream::Open(const StringView& File, std::ios::openmode Mode)
	{
		DWORD Access = FILE_GENERIC_EXECUTE;
		DWORD AccessMem = PAGE_EXECUTE;

		if (Mode & std::ios::in)
		{
			Access |= FILE_GENERIC_READ;
			AccessMem = PAGE_EXECUTE_READ;
		}

		if (Mode & std::ios::out)
		{
			Access |= FILE_GENERIC_WRITE;
			AccessMem = PAGE_EXECUTE_READWRITE;
		}

		return Open(CreateFileA(File.data(), Access, 0, NULL, OPEN_EXISTING, 0, NULL));
	}

	bool FileMappingStream::Close()
	{
		if (IS_VALID_HANDLE(MappingHandle))
		{
			return CloseHandle(MappingHandle);
		}

		return true;
	}

	bool CFile::OpenFileWrite()
	{
		if (IsOpenForWrite())
		{
			return true;
		}

		FileStream.open(Path, std::ios_base::out | std::ios_base::binary);
		
		if (FileStream.is_open())
		{
			OpenMode = std::ios_base::out | std::ios_base::binary;
			return true;
		}

		return false;
	}

	bool CFile::OpenFileReadWrite()
	{
		if (IsOpenForWrite())
		{
			return true;
		}

		FileStream.open(Path, std::ios_base::in | std::ios_base::out | std::ios_base::binary);

		if (FileStream.is_open())
		{
			OpenMode = std::ios_base::in | std::ios_base::out | std::ios_base::binary;
			return true;
		}

		return false;
	}

	bool CFile::OpenFileRead()
	{
		if (IsOpenForRead())
		{
			return true;
		}

		FileStream.open(Path, std::ios_base::in | std::ios_base::binary);

		if (FileStream.is_open())
		{
			OpenMode = std::ios_base::in | std::ios_base::binary;
			return true;
		}

		return false;
	}

	uint64_t CFile::GetFileSize()
	{
		if (!IsOpen())
		{
			if (!OpenFileRead())
			{
				return -1;
			}
		}

		FileStream.seekg(0, std::ios::end);

		auto Size = FileStream.tellg();

		FileStream.seekg(0, std::ios::beg);

		return Size;
	}

	void CFile::Close()
	{
		FileStream.close();
	}

	bool CFile::IsOpen() const
	{
		return FileStream.is_open();
	}

	bool CFile::IsOpenForRead() const
	{
		return OpenMode & std::ios_base::in;
	}

	bool CFile::IsOpenForWrite() const
	{
		return OpenMode & std::ios_base::out;
	}

	bool CFile::IsClosed() const
	{
		return !IsOpen();
	}

	ErrorTypes CFile::ReadFileContent()
	{
		if (!OpenFileRead())
		{
			return ErrorTypes::ErrorOpen;
		}

		FileStream.unsetf(std::ios::skipws);
		FileStream.seekg(0, std::ios::end);

		Content.ResizeUninitialized(FileStream.tellg());

		FileStream.seekg(0, std::ios::beg);
		FileStream.read(reinterpret_cast<char*>(
			Content.data()), 
			Content.size()
		);

		Close();

		return ErrorNone;
	}

	ErrorTypes CFile::ReadFileContentInto(TVector<Byte> & Stoarge)
	{
		if (!OpenFileRead())
		{
			return ErrorTypes::ErrorOpen;
		}

		FileStream.unsetf(std::ios::skipws);
		FileStream.seekg(0, std::ios::end);

		Stoarge.ResizeUninitialized(FileStream.tellg());

		FileStream.seekg(0, std::ios::beg);
		FileStream.read(reinterpret_cast<char*>(
			Stoarge.data()),
			Stoarge.size()
		);

		Close();

		return ErrorNone;
	}

	ErrorTypes CFile::ReadFileContentInto(void * pBuffer)
	{
		if (!OpenFileRead())
		{
			return ErrorTypes::ErrorOpen;
		}

		FileStream.unsetf(std::ios::skipws);
		FileStream.seekg(0, std::ios::end);
		FileStream.read(reinterpret_cast<char*>(pBuffer), FileStream.tellg());
		FileStream.seekg(0, std::ios::beg);

		Close();

		return ErrorNone;
	}

	ErrorTypes CFile::WriteFileContent()
	{
		if (!OpenFileWrite())
		{
			return ErrorTypes::ErrorOpen;
		}

		if (FileStream.write(reinterpret_cast<char*>(
			Content.data()),
			Content.size()
		).bad())
		{
			return ErrorTypes::ErrorWrite;
		}
		
		return ErrorNone;
	}
}
