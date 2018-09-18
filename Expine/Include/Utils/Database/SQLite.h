#pragma once

#include "Types.h"
#include "Singleton.h"
#include "Smart.h"

#include "Utils/StringOp.h"

#include "Private/sqlite3.h"

namespace Database
{
	namespace SQLite
	{
		struct DataSet : public THashMap<String, TVector<TVector<Byte> > >
		{
			inline bool GetRecord
			(
				const	String		& ColumnName,
						StringEmu	& Result,
				const	size_t		RowIndex = 0
			)
			{
				auto Iter = find(ColumnName);

				if (Iter == end())
				{
					return false;
				}

				if (Iter->second.size() <= RowIndex)
				{
					return false;
				}

				Result = StringEmu(
					Iter.value()[RowIndex].begin(),
					Iter.value()[RowIndex].end());

				return true;
			}

			template
			<
				typename	Type,
				typename... Types
			>
			inline bool GetRecordsAs
			(
				const	String	&		ColumnName,
						Type	&		Result,
				const	Types	&&...	Values
			)
			{
			}

			template
			<
				typename Type
			>
			inline bool GetRecordAs
			(
				const	String	& ColumnName,
						Type	& Result,
				const	size_t	RowIndex = 0
			)
			{
				if constexpr (std::is_same<TVector<Byte>, Type>::value)
				{
					auto Iter = find(ColumnName);

					if (Iter == end())
					{
						return false;
					}

					if (Iter->second.size() <= RowIndex)
					{
						return false;
					}

					Result = std::move(Iter.value()[RowIndex]);
				}
				else
				{
					StringEmu Value;

					if (!GetRecord(ColumnName, Value, RowIndex))
					{
						return false;
					}

					if constexpr (std::is_floating_point<Type>::value)
					{
						Ensure(Value.Size() > 2);
						// SQLite formatting logic.
						if (Value[Value.Size() - 1] == '0' && 
							Value[Value.Size() - 2] == '.')
						{
							Value.Resize(Value.Size() - 2);
						};
					}

					if (!(Value >> Result))
					{
						return false;
					}
				}

				return true;
			}
		};

		class CException : public std::exception
		{
		public:
			CException(const char * Error, int ErrorCode) :
				std::exception(Error),
				ErrorCode(ErrorCode)
			{}

			CException(int ErrorCode) :
				ErrorCode(ErrorCode)
			{}

			int GetErrorCode() const
			{
				return ErrorCode;
			}

		private:
			int ErrorCode;
		};

		class ExStepError : public CException
		{
		public:
			ExStepError(const char * Error, int ErrorCode) :
				CException(Error, ErrorCode)
			{}
		};

		class ExInvalidTable : public CException
		{
		public:
			ExInvalidTable(const String& TableName, const char * Error, int ErrorCode) :
				TableName(TableName), CException(Error, ErrorCode)
			{}

		private:
			String TableName;
		};

		class ExInvalidColumn : public CException
		{
		public:
			ExInvalidColumn(const String& TableName, const String& ColumnName, const char * Error, int ErrorCode) :
				TableName(TableName), ColumnName(ColumnName), CException(Error, ErrorCode)
			{}

		private:
			String TableName;
			String ColumnName;
		};

		class ExInvalidRow : public CException
		{
		public:
			ExInvalidRow(const String& TableName, const String& ColumnName, uint64_t RowId, const char * Error, int ErrorCode) :
				TableName(TableName), ColumnName(ColumnName), RowId(RowId), CException(Error, ErrorCode)
			{}

		private:
			String TableName;
			String ColumnName;
			uint64_t RowId;
		};

		class ExInvalidQuery : public CException
		{
		public:
			ExInvalidQuery(const String& Query, const char * Error, int ErrorCode) :
				Query(Query), CException(Error, ErrorCode)
			{}

		private:
			String Query;
		};

		class CDatabase;
		class CDatabaseManager : public CSingleton<CDatabaseManager>
		{
		private:

			TMutex Mutex;

		private:

			TMap<WString, SharedPointer<CDatabase> > OpenedDatabases;

		public:

			SharedPointer<CDatabase> OpenDatabase
			(
				const WString & Path
			);
		};
		
		class CDatabase
		{
		public:

			typedef int(TCallback)(void*, int, char**, char**);
			typedef int(CDatabase::*TExecuteCallback)(int, char**, char**);

		private:

			static const TExecuteCallback CBFunction;

		private:

			TMutex ExecuteMutex;

		private:

			sqlite3 * Database = NULL;

		private:

			DataSet LastResult;

			int ExecuteCallback
			(
				int		ColumnCount,
				char ** ppColumnData,
				char ** ppColumnNames
			);

		public:

			~CDatabase();

			int OpenDB
			(
				const WString & Path
			);
			int OpenDB
			(
				const String & Path
			);
			int Execute
			(
				const String & Query, DataSet & QueryResult
			);
			int ExecutePrepared
			(
				const String & Query, DataSet & QueryResult
			);
		};
	}
}