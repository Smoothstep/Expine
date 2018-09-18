#pragma once

#include "Defines.h"
#include "Types.h"

#include <experimental/filesystem>

#include <fstream>

namespace File
{
	enum ErrorTypes
	{
		ErrorNone,
		ErrorOpen,
		ErrorRead,
		ErrorWrite
	};

	class FileMappingStream
	{
	private:

		size_t			MappingSize;
		size_t			MappingSizeMax;
		TVector<Byte>	MappingBytes;
		void*			MappingHandle;
		DWORD			AccessMem;

	public:

		 FileMappingStream() = default;
		~FileMappingStream();

		bool Open(const WStringView& File, std::ios::openmode Mode);
		bool Open(const StringView& File, std::ios::openmode Mode);
		bool Open(void* FileHandle);

		bool Close();

				TVector<Byte> & GetContent(const size_t MinBytes = -1);
		const	TVector<Byte> & GetContent(const size_t MinBytes = -1) const;
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

		CFile() = default;
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

		template<class T = Byte> inline const TVector<T> & GetContentRef() const
		{
			return reinterpret_cast<const TVector<T>&>(Content);
		}

		template<class T = Byte> inline TVector<T> & GetContentRef()
		{
			return reinterpret_cast<TVector<T>&>(Content);
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
		return std::experimental::filesystem::exists(std::experimental::filesystem::path(FilePath));
	}

	static inline bool DoesFileExist
	(
		const String & FilePath
	)
	{
		return std::experimental::filesystem::exists(std::experimental::filesystem::path(FilePath));
	}

	static inline bool DoesFileExist
	(
		const StringView FilePath
	)
	{
		return std::experimental::filesystem::exists(std::experimental::filesystem::path(std::begin(FilePath), std::end(FilePath)));
	}

	static inline bool DoesFileExist
	(
		const WStringView FilePath
	)
	{
		return std::experimental::filesystem::exists(std::experimental::filesystem::path(std::begin(FilePath), std::end(FilePath)));
	}

	template<class Callback, class StringType, class... StringTypes>
	int DoFilesExist(Callback&& Cb, const StringType& File, const StringTypes&... Files)
	{
		if (DoesFileExist(File))
		{
			if constexpr (sizeof...(StringTypes) > 0)
			{
				return 1 + DoFilesExist(Cb, Files...);
			}
			else
			{
				return 1;
			}
		}

		Cb(File);

		if constexpr (sizeof...(StringTypes) > 0)
		{
			return DoFilesExist(Cb, Files...);
		}
		else
		{
			return 0;
		}
	}

	template<class Callback, class StringType, class... StringTypes>
	bool DoAllFilesExist(Callback&& Cb, const StringType& File, const StringTypes&... Files)
	{
		if (DoesFileExist(File))
		{
			if constexpr (sizeof...(StringTypes) > 0)
			{
				return DoAllFilesExist(Cb, Files...);
			}
			else
			{
				return true;
			}
		}

		Cb(File);

		if constexpr (sizeof...(StringTypes) > 0)
		{
			return DoAllFilesExist(Cb, Files...);
		}
		else
		{
			return false;
		}
	}

	class CDirectoryReader
	{

	};
	
	template<
		typename ContentType,
		typename const	typename ContentType::value_type *	(ContentType::*GetData)() const = &ContentType::data,
		typename		typename ContentType::size_type		(ContentType::*GetSize)() const	= &ContentType::size>
	class ContentAdapter
	{
	protected:
		
		const ContentType & Content;

	public:

		ContentAdapter(const ContentType & Content) : Content(Content) {}

	public:

		inline const typename ContentType::value_type * Data() const
		{
			return (Content.*GetData)();
		}

		inline typename ContentType::size_type Size() const
		{
			return (Content.*GetSize)();
		}
	};

	template<
		typename ContentType,
		typename const	typename ContentType::value_type *	(ContentType::*GetData)() const = &ContentType::data,
		typename		typename ContentType::size_type		(ContentType::*GetSize)() const	= &ContentType::size>
	class CContentReader : public ContentAdapter<ContentType, GetData, GetSize>
	{
	private:

		const ContentType & Content;

	private:

		using ContentAdapter<ContentType, GetData, GetSize>::Size;
		using ContentAdapter<ContentType, GetData, GetSize>::Data;

		size_t ReadOffset = 0;

		const inline Byte * FindCharacter
		(
			const Byte* P,
			const Byte	C
		)	const
		{
			// Finds a character within a range

			const Byte * Offset = reinterpret_cast<const Byte*>(memchr(P, C, Size() - ReadOffset));

			return Offset;
		}

	public:

		inline CContentReader
		(
			const ContentType & Content
		) : 
			Content(Content), ContentAdapter<ContentType, GetData, GetSize>(Content)
		{}

		void Reset()
		{
			ReadOffset = 0;
		}

		const inline Byte * FindCharacter
		(
			const Byte C
		)	const
		{
			const Byte * Current = reinterpret_cast<const Byte*>(Data() + ReadOffset);

			// Finds a character within a range

			const Byte * Offset = reinterpret_cast<const Byte*>(memchr(Current, C, Size() - ReadOffset));

			return Offset;
		}

		inline bool GetNextLine
		(
			String & Result
		)
		{
			const Byte * Current = reinterpret_cast<const Byte*>(Data() + (ReadOffset == 0 ? 0 : ReadOffset + 1));

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