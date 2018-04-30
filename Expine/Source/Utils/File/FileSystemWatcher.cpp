#include "File/FileSystemWatcher.h"
#include "Memory.h"

namespace File
{
	bool CFileSystemWatcher::OnChangeNotify()
	{
		if (!GetOverlappedResult(hEvent, this, &FileNotifyInformationCount, false))
		{
			return true;
		}

		if (FileNotifyCallback != NULL)
		{
			return FileNotifyCallback->OnFileChangeNotify(FileNotifyInformation);
		}

		return true;
	}

	void CFileSystemWatcher::OnDirectoryChange(DWORD dwErrorCode, DWORD dwNotifyFilter, LPOVERLAPPED lpOverlapped)
	{
		CFileSystemWatcher * pWatcher = reinterpret_cast<CFileSystemWatcher*>(lpOverlapped);

		if (pWatcher != NULL)
		{
			PostQueuedCompletionStatus(pWatcher->GetCompletionPortHandle(), 0, 0, pWatcher);
		}
	}

	HRESULT CFileSystemWatcher::WatchDirectoryChangesAsync()
	{
		if (!ReadDirectoryChangesW(
			DirectoryHandle,
			FileNotifyInformation,
			sizeof(FILE_NOTIFY_INFORMATION) * FileNotifyInformationMax,
			FALSE,
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
			NULL,
			this,
			NULL))
		{
			return HRESULT_LAST_ERROR;
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::InitializeNotifyArray()
	{
		FileNotifyInformation = new TFileNotifyInformation[FileNotifyInformationMax];

		if (FileNotifyInformation == NULL)
		{
			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::InitializeCompletionRoutine()
	{
		if (!BindIoCompletionCallback(
			DirectoryHandle,
			&OnDirectoryChange,
			0))
		{
			return HRESULT_LAST_ERROR;
		}

		CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
			NULL,
			0,
			0);

		if (IS_INVALID_HANDLE(CompletionPort))
		{
			return HRESULT_LAST_ERROR;
		}

		hEvent = CreateEvent(0, 0, 0, L"SystemFileWatcherEvent");

		if (IS_INVALID_HANDLE(hEvent))
		{
			return HRESULT_LAST_ERROR;
		}

		HRESULT hr;

		if (FAILED(hr = WatchDirectoryChangesAsync()))
		{
			return hr;
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::InitializeDirectory()
	{
		if (IS_VALID_HANDLE(DirectoryHandle))
		{
			CLOSE_HANDLE(DirectoryHandle);
		}

		DirectoryHandle = CreateFile(
			DirectoryPath,
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			0,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			0);

		if (IS_INVALID_HANDLE(DirectoryHandle))
		{
			return HRESULT_LAST_ERROR;
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::Stop()
	{
		if (IS_VALID_HANDLE(hEvent))
		{
			CLOSE_HANDLE(hEvent);
		}

		if (!CancelIo(CompletionPort))
		{
			return HRESULT_LAST_ERROR;
		}

		return S_OK;
	}

	CFileSystemWatcher::CFileSystemWatcher(SIZE_T iMaxNotifies)
	{
		FileNotifyInformationMax = iMaxNotifies;
	}

	CFileSystemWatcher::~CFileSystemWatcher()
	{
		SafeReleaseArray(FileNotifyInformation);
	}

	HRESULT CFileSystemWatcher::Initialize(LPCWSTR pFileDirectory)
	{
		if (pFileDirectory == NULL)
		{
			return E_INVALIDARG;
		}

		HRESULT hr;

		if (FileNotifyInformation == NULL)
		{
			if (FAILED(hr = InitializeNotifyArray()))
			{
				return hr;
			}
		}

		DirectoryPath = pFileDirectory;

		if (FAILED(hr = InitializeDirectory()))
		{
			return hr;
		}

		if (FAILED(hr = InitializeCompletionRoutine()))
		{
			return hr;
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::Watch()
	{
		DWORD dwTrans;
		ULONG_PTR ulCompletionKey;
		LPOVERLAPPED pOverlapped;

		if (GetQueuedCompletionStatus(CompletionPort, &dwTrans, &ulCompletionKey, &pOverlapped, 0))
		{
			if (!OnChangeNotify())
			{
				return E_FAIL;
			}
			
			HRESULT hr;

			if (FAILED(hr = WatchDirectoryChangesAsync()))
			{
				return hr;
			}
		}

		return S_OK;
	}

	HRESULT CFileSystemWatcher::Watch(DWORD dwWaitMS)
	{
		DWORD dwTrans;
		ULONG_PTR ulCompletionKey;
		LPOVERLAPPED pOverlapped;

		if (GetQueuedCompletionStatus(CompletionPort, &dwTrans, &ulCompletionKey, &pOverlapped, dwWaitMS))
		{
			if (!OnChangeNotify())
			{
				return E_FAIL;
			}

			HRESULT hr;

			if (FAILED(hr = WatchDirectoryChangesAsync()))
			{
				return hr;
			}
		}

		return S_OK;
	}
}