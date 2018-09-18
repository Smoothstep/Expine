#pragma once

#include "File.h"
#include <WindowsH.h>

namespace File
{
	class CFileSystemWatcher : protected OVERLAPPED
	{
	public:

		struct TFileNotifyInformation : public FILE_NOTIFY_INFORMATION
		{
			inline TFileNotifyInformation * Next()
			{
				if (NextEntryOffset == 0)
				{
					return NULL;
				}

				return reinterpret_cast<TFileNotifyInformation*>(reinterpret_cast<BYTE*>(this) + NextEntryOffset);
			}

			inline WString GetFileName()
			{
				return WString
				(
					this->FileName,
					this->FileName + this->FileNameLength / sizeof(wchar_t)
				);
			}
		};

		struct IChangeNotifyCallback
		{
		public:
			virtual bool OnFileChangeNotify
			(
				TFileNotifyInformation * pChanges
			) PURE;
		};

	private:

		IChangeNotifyCallback	*	FileNotifyCallback				= NULL;
		TFileNotifyInformation	*	FileNotifyInformation			= NULL;
		DWORD						FileNotifyInformationCount		= 0;
		DWORD						FileNotifyInformationMax		= 0;

		HANDLE						CompletionPort					= INVALID_HANDLE_VALUE;
		HANDLE						DirectoryHandle					= INVALID_HANDLE_VALUE;
		LPCWSTR						DirectoryPath					= NULL;

	public:

		inline HANDLE GetCompletionPortHandle() 
		{ 
			return CompletionPort; 
		}

		inline HANDLE GetDirectoryHandle() 
		{ 
			return DirectoryHandle; 
		}

		inline LPCWSTR GetDirectoryPath() 
		{ 
			return DirectoryPath; 
		}

		inline void SetCallback
		(
			IChangeNotifyCallback * pCallback
		)
		{
			FileNotifyCallback = pCallback;
		}

	private:
		
		bool OnChangeNotify();

		static void CALLBACK OnDirectoryChange
		(
			DWORD			dwErrorCode,
			DWORD			dwNotifyFilter,
			LPOVERLAPPED	lpOverlapped
		);

		HRESULT WatchDirectoryChangesAsync();
		HRESULT InitializeNotifyArray();
		HRESULT InitializeCompletionRoutine();
		HRESULT InitializeDirectory();

	public:

		HRESULT Stop();

		CFileSystemWatcher
		(
			SIZE_T iMaxNotifies
		);

		~CFileSystemWatcher();

		HRESULT Initialize
		(
			LPCWSTR pFileDirectory
		);

		HRESULT Watch();

		HRESULT Watch
		(
			DWORD dwWaitMS
		);
	};
}