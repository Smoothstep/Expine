#pragma once

#include "Defines.h"
#include "Types.h"
#include "WindowsH.h"

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace File
{
	enum ErrorTypes
	{
		ErrorNone,
		ErrorOpen,
		ErrorRead,
		ErrorWrite
	};

	class CFile
	{
	private:

		WString Path;

	private:

		TVector<Byte> Content;

	private:

		std::fstream FileStream;

	private:

		std::ios_base::openmode OpenMode = 0;

	public:

		CFile()
		{}

		CFile
		(
			const WString & Path
		) :
			Path(Path)
		{}

		CFile
		(
			const String & Path
		) :
			Path(WString(Path.begin(), Path.end()))
		{}

		inline const WString & GetPath() const
		{
			return Path;
		}

		template<class T = Byte> inline TVector<T> & GetContentRef() const
		{
			return (TVector<T>&)Content;
		}

		template<class T = Byte> inline TVector<T> GetContent() const
		{
			return TVector<T>(Content.begin(), Content.end());
		}

		template<class T = Byte> inline void DetachContent
		(
			TVector<T> & Result
		)
		{
			Ensure(sizeof(T) <= Content.size());

			TVector<Byte> & Content = reinterpret_cast<TVector<Byte> &>(Result);
			{
				Content.swap(this->Content);
			}
		}

	public:

		bool OpenFileWrite();
		bool OpenFileRead();
		bool OpenFileReadWrite();

		uint64_t GetFileSize();

		ErrorTypes ReadFileContent();
		ErrorTypes ReadFileContentInto
		(
			TVector<Byte> & Storage
		);
		ErrorTypes ReadFileContentInto
		(
			void * pBuffer
		);
		ErrorTypes WriteFileContent();

		void Close();

		bool IsOpen() const;
		bool IsOpenForRead() const;
		bool IsOpenForWrite() const;
		bool IsClosed() const;
	};

	static inline bool DoesFileExist
	(
		const WString & FilePath
	)
	{
		return boost::filesystem::exists(boost::filesystem::path(FilePath));
	}

	static inline bool DoesFileExist
	(
		const String & FilePath
	)
	{
		return boost::filesystem::exists(boost::filesystem::path(WString(FilePath.begin(), FilePath.end())));
	}

	class CDirectoryReader
	{

	};

	class CFileReader
	{
	private:

		const TVector<Byte> & Content;

	private:

		size_t ReadOffset = 0;

		const inline Byte * FindCharacter
		(
			const Byte* P,
			const Byte	C
		)	const
		{
			// Finds a character within a range

			const Byte * Offset = reinterpret_cast<const Byte*>(memchr(P, C, Content.size() - ReadOffset));

			return Offset;
		}

	public:

		inline CFileReader
		(
			const TVector<Byte> & Content
		) : 
			Content(Content) 
		{
		}

		void Reset()
		{
			ReadOffset = 0;
		}

		const inline Byte * FindCharacter
		(
			const Byte C
		)	const
		{
			const Byte * Current = reinterpret_cast<const Byte*>(Content.data() + ReadOffset);

			// Finds a character within a range

			const Byte * Offset = reinterpret_cast<const Byte*>(memchr(Current, C, Content.size() - ReadOffset));

			return Offset;
		}

		inline bool GetNextLine
		(
			String & Result
		)
		{
			const Byte * Current = reinterpret_cast<const Byte*>(Content.data() + (ReadOffset == 0 ? 0 : ReadOffset + 1));

			// Get the end line sign offset

			const Byte * Offset = FindCharacter('\n');

			if (Offset == NULL)
			{
				return false;
			}

			// From last read to next sign

			Result = String(Current, Offset);

			// Add up the line length

			ReadOffset += Offset - Current;

			return true;
		}
	};
}