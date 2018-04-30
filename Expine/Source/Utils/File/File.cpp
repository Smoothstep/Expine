#include "File/File.h"

#include <fstream>
#include <ostream>

namespace File
{
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
