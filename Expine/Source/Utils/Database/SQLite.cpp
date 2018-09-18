#include <Database/SQLite.h>

namespace Database
{
	namespace SQLite
	{
		static CDatabaseManager GDatabaseManager;

		const CDatabase::TExecuteCallback CDatabase::CBFunction = CDatabase::ExecuteCallback;

		int CDatabase::ExecuteCallback(int ColumnCount, char ** ColumnData, char ** ColumnNames)
		{
			for (unsigned C = 0; C < ColumnCount; ++C)
			{
				if (ColumnData[C] != NULL)
				{
					size_t Length = std::strlen(ColumnData[C]);
					{
						TVector<Byte> Result(
							ColumnData[C],
							ColumnData[C] + Length);
						LastResult[ColumnNames[C]].push_back(
							Result);
					}
				}
			}

			return 0;
		}

		CDatabase::~CDatabase()
		{
			if (Database)
			{
				sqlite3_close(Database);
			}
		}

		int CDatabase::OpenDB(const WString & Path)
		{
			return sqlite3_open16(Path.c_str(), &Database);
		}

		int CDatabase::OpenDB(const String & Path)
		{
			return sqlite3_open(Path.c_str(), &Database);
		}

		int CDatabase::Execute(const String & Query, DataSet & QueryResult)
		{
			std::scoped_lock<TMutex> Lock(ExecuteMutex);

			char * Error;

			constexpr uintptr_t * P = reinterpret_cast<uintptr_t*>(*reinterpret_cast<TCallback*>(&CBFunction));

			int Result = sqlite3_exec(Database, Query.c_str(), reinterpret_cast<TCallback*>(*reinterpret_cast<uintptr_t*>(*reinterpret_cast<TCallback*>(&CBFunction))), this, &Error);
			
			QueryResult = LastResult;
			{
				LastResult.clear();
			}

			if (Result != SQLITE_OK)
			{
				throw(std::exception(Error));
			}

			return Result;
		}

		int CDatabase::ExecutePrepared(const String & Query, DataSet & QueryResult)
		{
			Int32 ErrorCode;
			sqlite3_stmt *SelectStatement;

			if ((ErrorCode = sqlite3_prepare_v2(Database, Query.c_str(), Query.size(), &SelectStatement, nullptr)) != SQLITE_OK)
			{
				return ErrorCode;
			}

			Int32 Columns = sqlite3_column_count(SelectStatement);

			TVector<String> ColumnNames(Columns);
			{
				for (Int32 C = 0; C < Columns; ++C)
				{
					ColumnNames[C] = sqlite3_column_name(SelectStatement, C);
				}
			}

			while (true)
			{
				ErrorCode = sqlite3_step(SelectStatement);

				if (ErrorCode == SQLITE_ROW)
				{
					for (Int32 C = 0; C < Columns; ++C)
					{
						const Byte* Data = reinterpret_cast<const Byte*>(
							sqlite3_column_blob(SelectStatement, C));

						if (!Data)
						{
							continue;
						}

						const Int32 DataLength = 
							sqlite3_column_bytes(SelectStatement, C);

						auto & Result = QueryResult[ColumnNames[C]];
						auto & ResultContainer = *Result.emplace(Result.end());
						
						ResultContainer.reserve(DataLength + 1);
						ResultContainer.insert(ResultContainer.end(),
							Data,
							Data + DataLength
						);

						Byte * Last = ResultContainer.data() + ResultContainer.size();

						// Null termination
						*Last = '\0';
					}
				}
				else
				{
					break;
				}
			}

			if (ErrorCode != SQLITE_DONE)
			{
				sqlite3_finalize(SelectStatement);
				return ErrorCode;
			}

			ErrorCode = sqlite3_finalize(SelectStatement);

			return ErrorCode;
		}

		SharedPointer<CDatabase> CDatabaseManager::OpenDatabase(const WString & Path)
		{
			std::scoped_lock<TMutex> Lock(Mutex);

			auto DB = OpenedDatabases.find(Path);
			{
				if (DB != OpenedDatabases.end())
				{
					return DB->second;
				}
			}

			SharedPointer<CDatabase> Database = new CDatabase();

			if (Database->OpenDB(Path) != SQLITE_OK)
			{
				return NULL;
			}

			OpenedDatabases[Path] = Database;

			return Database;
		}
	}
}