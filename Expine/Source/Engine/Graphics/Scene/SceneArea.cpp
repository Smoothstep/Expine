#include "Precompiled.h"

#include "Scene/SceneArea.h"
#include "Scene/Object/ObjectTable.h"

#include "Utils/Database/SQLite.h"

namespace D3D
{
	ErrorCode CSceneArea::LoadStaticObjects()
	{
		return ErrorCode();
	}

	ErrorCode CSceneArea::LoadDynamicObjects()
	{
		return ErrorCode();
	}

	ErrorCode CSceneArea::Load(const AreaLoadParameters & Parameters)
	{
		SharedPointer<Database::SQLite::CDatabase> DB = Database::SQLite::CDatabaseManager::Instance().OpenDatabase(L"C:\\Users\\a\\Documents\\Navicat\\MySQL\\servers\\d\\Game.db");

		StringStream Query;
		{
			Query << "SELECT * FROM ObjectTable WHERE AreaId = " << Parameters.AreaId;
		}

		Database::SQLite::DataSet ResultSet;
		
		if (DB->ExecutePrepared(Query.str(), ResultSet) == SQLITE_OK && !ResultSet.empty())
		{
			Uint32 ObjectTableId;
			TVector<Uint8> ObjectTableData;

			auto Data = ResultSet.Find("Data");

			if (Data)
			{
				for (size_t N = 0; N < Data->size(); ++N)
				{
					if (!ResultSet.GetRecordAs("ObjectTableId", ObjectTableId, N))
					{
						CErrorLog::Log<LogError>() << "No ObjectTable.Id for AreaId: " << Parameters.AreaId;
						continue;
					}

					auto ObjectTableData = Data->at(N);
					auto ObjectTableSize = ObjectTableData.size();

					if (ObjectTableSize % sizeof(ObjectTable) != 0)
					{
						CErrorLog::Log<LogError>() << "Invalid ObjectTable.Size for ObjectTable.Id: " << ObjectTableId;
						continue;
					}

					StaticObjectTable.AddEntries(reinterpret_cast<const ObjectTableEntry*>(
						ObjectTableData.data()), 
						ObjectTableData.size() / sizeof(ObjectTable));
				}
			}
			else
			{
				CErrorLog::Log<LogError>() << "No ObjectTable.Data for AreaId: " << Parameters.AreaId;
			}
		}
		else
		{
			CErrorLog::Log<LogInfo>() << "No ObjectTable for AreaId: " << Parameters.AreaId;
		}

		return S_OK;
	}
	
	ErrorCode CSceneArea::LoadArea(const UINT AreaId)
	{
		Properties.AreaId = AreaId;
		return S_OK;
	}
}
