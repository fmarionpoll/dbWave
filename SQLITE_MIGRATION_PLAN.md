# SQLITE MIGRATION PLAN FOR DBWAVE2 WITH MDB PLUGIN SUPPORT

## Overview
This document outlines a comprehensive migration strategy for dbWave2 from 32-bit DAO/MDB to 64-bit SQLite while maintaining backward compatibility with existing 32-bit MDB files through a plugin architecture.

## Migration Architecture

### Core Components
1. **64-bit dbWave2 Application** (Primary)
   - SQLite as main database engine
   - Modern 64-bit architecture
   - Enhanced performance and memory capacity

2. **MDB Compatibility Plugin** (Secondary)
   - 32-bit process for MDB file access
   - Inter-process communication (IPC)
   - Data translation layer

3. **Unified Data Interface**
   - Common API for both SQLite and MDB data
   - Seamless data migration tools
   - Hybrid database support

## 1. Project Structure

### New File Organization
```
dbWave2/
├── src/
│   ├── core/                    # 64-bit main application
│   │   ├── database/
│   │   │   ├── SQLiteDatabase.h
│   │   │   ├── SQLiteDatabase.cpp
│   │   │   ├── DatabaseInterface.h
│   │   │   └── DatabaseInterface.cpp
│   │   ├── plugins/
│   │   │   ├── PluginManager.h
│   │   │   ├── PluginManager.cpp
│   │   │   └── MDBPlugin/
│   │   │       ├── MDBPlugin.h
│   │   │       ├── MDBPlugin.cpp
│   │   │       └── MDBBridge.cpp
│   │   └── migration/
│   │       ├── DataMigrator.h
│   │       ├── DataMigrator.cpp
│   │       └── SchemaConverter.h
│   └── legacy/                  # 32-bit MDB support
│       ├── MDBReader/
│       │   ├── MDBReader.h
│       │   ├── MDBReader.cpp
│       │   └── MDBReader.exe    # 32-bit executable
│       └── IPC/
│           ├── SharedMemory.h
│           ├── NamedPipes.h
│           └── MessageQueue.h
```

## 2. Database Interface Design

### Unified Database Interface
```cpp
// DatabaseInterface.h
class IDatabaseInterface
{
public:
    virtual ~IDatabaseInterface() = default;
    
    // Connection management
    virtual BOOL OpenDatabase(const CString& filename) = 0;
    virtual BOOL CloseDatabase() = 0;
    virtual BOOL IsOpen() const = 0;
    
    // Record operations
    virtual BOOL MoveFirst() = 0;
    virtual BOOL MoveNext() = 0;
    virtual BOOL MovePrev() = 0;
    virtual BOOL IsEOF() const = 0;
    virtual BOOL IsBOF() const = 0;
    
    // Data access
    virtual CString GetFieldValue(const CString& fieldName) = 0;
    virtual __int64 GetFieldValueInt64(const CString& fieldName) = 0;
    virtual BOOL SetFieldValue(const CString& fieldName, const CString& value) = 0;
    virtual BOOL SetFieldValueInt64(const CString& fieldName, __int64 value) = 0;
    
    // Record editing
    virtual BOOL Edit() = 0;
    virtual BOOL Update() = 0;
    virtual BOOL AddNew() = 0;
    virtual BOOL Delete() = 0;
    
    // Metadata
    virtual int GetRecordCount() const = 0;
    virtual CString GetDatabaseType() const = 0;
    virtual CString GetDatabasePath() const = 0;
};
```

### SQLite Implementation
```cpp
// SQLiteDatabase.h
class CSQLiteDatabase : public IDatabaseInterface
{
private:
    sqlite3* m_pDatabase;
    sqlite3_stmt* m_pCurrentStatement;
    CString m_databasePath;
    CString m_currentTable;
    
    // Prepared statements cache
    std::map<CString, sqlite3_stmt*> m_statementCache;
    
public:
    CSQLiteDatabase();
    virtual ~CSQLiteDatabase();
    
    // IDatabaseInterface implementation
    BOOL OpenDatabase(const CString& filename) override;
    BOOL CloseDatabase() override;
    BOOL IsOpen() const override;
    
    // SQLite specific methods
    BOOL CreateTable(const CString& tableName, const CString& schema);
    BOOL ExecuteQuery(const CString& sql);
    BOOL BeginTransaction();
    BOOL CommitTransaction();
    BOOL RollbackTransaction();
    
private:
    BOOL PrepareStatement(const CString& sql, sqlite3_stmt** ppStmt);
    void ClearStatementCache();
};
```

### MDB Plugin Implementation
```cpp
// MDBPlugin.h
class CMDBPlugin : public IDatabaseInterface
{
private:
    HANDLE m_hMDBProcess;
    HANDLE m_hSharedMemory;
    HANDLE m_hNamedPipe;
    CString m_mdbFilePath;
    
    // IPC structures
    struct MDBCommand
    {
        enum CommandType
        {
            CMD_OPEN,
            CMD_CLOSE,
            CMD_MOVE_FIRST,
            CMD_MOVE_NEXT,
            CMD_GET_FIELD,
            CMD_SET_FIELD,
            CMD_EDIT,
            CMD_UPDATE
        };
        
        CommandType type;
        char fieldName[256];
        char fieldValue[1024];
        __int64 fieldValueInt64;
    };
    
    struct MDBResponse
    {
        BOOL success;
        char result[1024];
        __int64 resultInt64;
    };
    
public:
    CMDBPlugin();
    virtual ~CMDBPlugin();
    
    // IDatabaseInterface implementation
    BOOL OpenDatabase(const CString& filename) override;
    BOOL CloseDatabase() override;
    
private:
    BOOL StartMDBProcess();
    BOOL SendCommand(const MDBCommand& cmd, MDBResponse& response);
    BOOL InitializeIPC();
};
```

## 3. Plugin Manager Architecture

### Plugin Manager Implementation
```cpp
// PluginManager.h
class CPluginManager
{
private:
    std::map<CString, std::unique_ptr<IDatabaseInterface>> m_plugins;
    IDatabaseInterface* m_pCurrentDatabase;
    
public:
    CPluginManager();
    ~CPluginManager();
    
    // Plugin management
    BOOL RegisterPlugin(const CString& name, std::unique_ptr<IDatabaseInterface> plugin);
    BOOL LoadPlugin(const CString& pluginPath);
    BOOL UnloadPlugin(const CString& name);
    
    // Database operations
    BOOL OpenDatabase(const CString& filename);
    BOOL CloseDatabase();
    IDatabaseInterface* GetCurrentDatabase() const;
    
    // Auto-detection
    CString DetectDatabaseType(const CString& filename);
    BOOL MigrateDatabase(const CString& sourcePath, const CString& targetPath);
    
private:
    BOOL IsSQLiteFile(const CString& filename);
    BOOL IsMDBFile(const CString& filename);
};
```

## 4. Data Migration Strategy

### Migration Process
```cpp
// DataMigrator.h
class CDataMigrator
{
public:
    enum MigrationMode
    {
        MIGRATE_COPY,      // Copy data, keep original
        MIGRATE_MOVE,      // Move data, delete original
        MIGRATE_HYBRID     // Keep both, sync changes
    };
    
    struct MigrationOptions
    {
        MigrationMode mode;
        BOOL validateData;
        BOOL createBackup;
        BOOL preserveTimestamps;
        CString backupPath;
    };
    
public:
    CDataMigrator();
    
    // Migration operations
    BOOL MigrateMDBToSQLite(const CString& mdbPath, const CString& sqlitePath, 
                           const MigrationOptions& options);
    BOOL SyncDatabases(const CString& sourcePath, const CString& targetPath);
    BOOL ValidateMigration(const CString& sourcePath, const CString& targetPath);
    
    // Schema conversion
    CString ConvertMDBSchemaToSQLite(const CString& mdbPath);
    BOOL CreateSQLiteTables(const CString& sqlitePath, const CString& schema);
    
private:
    BOOL CopyTableData(const CString& sourceTable, const CString& targetTable);
    BOOL ValidateTableData(const CString& sourceTable, const CString& targetTable);
    CString GenerateSQLiteSchema(const CString& mdbSchema);
};
```

### Schema Conversion
```cpp
// SchemaConverter.h
class CSchemaConverter
{
public:
    struct FieldDefinition
    {
        CString name;
        CString type;
        BOOL isPrimaryKey;
        BOOL isNullable;
        CString defaultValue;
    };
    
    struct TableDefinition
    {
        CString name;
        std::vector<FieldDefinition> fields;
        std::vector<CString> indexes;
    };
    
public:
    // MDB to SQLite conversion
    TableDefinition ConvertMDBTable(const CString& tableName, CDaoRecordset* pRecordset);
    CString GenerateSQLiteCreateTable(const TableDefinition& table);
    
    // Data type mapping
    CString MapMDBTypeToSQLite(const CString& mdbType);
    CString MapSQLiteTypeToMDB(const CString& sqliteType);
    
private:
    BOOL AnalyzeMDBStructure(const CString& mdbPath, std::vector<TableDefinition>& tables);
};
```

## 5. IPC Implementation for MDB Access

### 32-bit MDB Reader Process
```cpp
// MDBReader.cpp (32-bit executable)
class CMDBReader
{
private:
    CDaoDatabase m_database;
    CDaoRecordset m_recordset;
    HANDLE m_hSharedMemory;
    HANDLE m_hNamedPipe;
    
public:
    CMDBReader();
    ~CMDBReader();
    
    BOOL Initialize();
    BOOL ProcessCommands();
    
private:
    BOOL HandleOpenCommand(const MDBCommand& cmd, MDBResponse& response);
    BOOL HandleMoveCommand(const MDBCommand& cmd, MDBResponse& response);
    BOOL HandleGetFieldCommand(const MDBCommand& cmd, MDBResponse& response);
    BOOL HandleSetFieldCommand(const MDBCommand& cmd, MDBResponse& response);
    BOOL SendResponse(const MDBResponse& response);
};
```

### Shared Memory Structure
```cpp
// SharedMemory.h
struct SharedMemoryData
{
    MDBCommand command;
    MDBResponse response;
    BOOL commandReady;
    BOOL responseReady;
    HANDLE commandEvent;
    HANDLE responseEvent;
};
```

## 6. Updated Application Architecture

### Main Application Changes
```cpp
// dbWaveDoc.h (Updated)
class CdbWaveDoc : public COleDocument
{
private:
    CPluginManager m_pluginManager;
    CDataMigrator m_dataMigrator;
    CString m_currentDatabasePath;
    CString m_currentDatabaseType;
    
public:
    // Database operations
    BOOL OpenDatabase(const CString& filename);
    BOOL CloseDatabase();
    BOOL SaveDatabase();
    BOOL SaveDatabaseAs(const CString& filename);
    
    // Migration operations
    BOOL MigrateToSQLite(const CString& targetPath);
    BOOL ImportFromMDB(const CString& mdbPath);
    BOOL ExportToMDB(const CString& mdbPath);
    
    // Compatibility
    BOOL IsLegacyMDB(const CString& filename);
    BOOL CanOpenFile(const CString& filename);
    
private:
    BOOL InitializePlugins();
    BOOL AutoMigrateIfNeeded(const CString& filename);
};
```

### Updated Dialog Implementation
```cpp
// DlgdbEditRecord.cpp (Updated)
void DlgdbEditRecord::OnBnClickedButton8()
{
    // Get current database interface
    auto* pDatabase = m_pdb_doc->GetCurrentDatabase();
    if (!pDatabase)
    {
        AfxMessageBox(_T("No database is currently open."), MB_OK | MB_ICONERROR);
        return;
    }
    
    if (AfxMessageBox(_T("This will remove leading spaces from all data file filenames in the database.\n\nContinue?"), 
                      MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;
    
    int records_updated = 0;
    int total_records = 0;
    
    try
    {
        // Use unified interface
        pDatabase->MoveFirst();
        
        while (!pDatabase->IsEOF())
        {
            total_records++;
            
            CString current_filename = pDatabase->GetFieldValue(_T("filename"));
            
            if (!current_filename.IsEmpty() && current_filename[0] == _T(' '))
            {
                CString trimmed_filename = current_filename;
                trimmed_filename.TrimLeft();
                
                if (trimmed_filename != current_filename)
                {
                    pDatabase->Edit();
                    pDatabase->SetFieldValue(_T("filename"), trimmed_filename);
                    pDatabase->Update();
                    records_updated++;
                }
            }
            
            pDatabase->MoveNext();
        }
        
        // Show results
        CString result_msg;
        result_msg.Format(_T("Data file filename cleanup completed.\n\nTotal records processed: %d\nRecords updated: %d"), 
                         total_records, records_updated);
        AfxMessageBox(result_msg, MB_OK | MB_ICONINFORMATION);
        
        // Refresh views
        m_pdb_doc->update_all_views_db_wave(nullptr, 0L, nullptr);
    }
    catch (const std::exception& e)
    {
        CString errorMsg = CString(_T("Database operation failed: ")) + CString(e.what());
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);
    }
}
```

## 7. Migration Implementation Steps

### Phase 1: Foundation (Week 1-2)
1. **Create new 64-bit project configuration**
2. **Implement SQLite database interface**
3. **Create plugin manager architecture**
4. **Set up IPC framework for MDB access**

### Phase 2: MDB Plugin (Week 3-4)
1. **Develop 32-bit MDB reader process**
2. **Implement IPC communication**
3. **Create MDB plugin wrapper**
4. **Test MDB file access through plugin**

### Phase 3: Migration Tools (Week 5-6)
1. **Implement data migration engine**
2. **Create schema conversion tools**
3. **Develop validation and backup systems**
4. **Test migration with sample data**

### Phase 4: Integration (Week 7-8)
1. **Update main application to use plugin manager**
2. **Implement auto-detection and migration**
3. **Update all database operations**
4. **Test hybrid database scenarios**

### Phase 5: Testing & Optimization (Week 9-10)
1. **Performance testing**
2. **Memory leak detection**
3. **Error handling validation**
4. **User acceptance testing**

## 8. Configuration and Settings

### Application Settings
```cpp
// Application configuration
struct AppConfig
{
    // Database preferences
    CString defaultDatabaseType;      // "SQLite" or "MDB"
    BOOL autoMigrateMDB;             // Auto-migrate MDB files
    BOOL keepMDBBackup;              // Keep MDB backup after migration
    
    // Plugin settings
    CString mdbPluginPath;           // Path to MDB reader executable
    int mdbTimeout;                  // Timeout for MDB operations (ms)
    
    // Migration settings
    CString migrationBackupPath;     // Default backup location
    BOOL validateAfterMigration;     // Validate data after migration
    BOOL preserveTimestamps;         // Preserve file timestamps
    
    // Performance settings
    int sqliteCacheSize;             // SQLite cache size (KB)
    BOOL enableWAL;                  // Enable Write-Ahead Logging
    int journalMode;                 // SQLite journal mode
};
```

### Registry Settings
```cpp
// Registry keys for configuration
#define REG_KEY_DB_PREFERENCES _T("Software\\dbWave2\\Database")
#define REG_KEY_PLUGINS _T("Software\\dbWave2\\Plugins")
#define REG_KEY_MIGRATION _T("Software\\dbWave2\\Migration")
```

## 9. Error Handling and Recovery

### Error Handling Strategy
```cpp
// Error handling classes
class DatabaseException : public std::exception
{
private:
    CString m_errorMessage;
    CString m_databaseType;
    int m_errorCode;
    
public:
    DatabaseException(const CString& message, const CString& dbType, int code = 0)
        : m_errorMessage(message), m_databaseType(dbType), m_errorCode(code) {}
    
    const char* what() const override { return CT2A(m_errorMessage); }
    CString GetErrorMessage() const { return m_errorMessage; }
    CString GetDatabaseType() const { return m_databaseType; }
    int GetErrorCode() const { return m_errorCode; }
};

// Recovery mechanisms
class DatabaseRecovery
{
public:
    BOOL RecoverFromBackup(const CString& databasePath);
    BOOL ValidateDatabaseIntegrity(const CString& databasePath);
    BOOL RepairDatabase(const CString& databasePath);
    BOOL CreateBackup(const CString& databasePath);
};
```

## 10. Performance Considerations

### Optimization Strategies
1. **SQLite Optimizations**:
   - Use prepared statements
   - Enable WAL mode
   - Optimize cache size
   - Use transactions for bulk operations

2. **MDB Plugin Optimizations**:
   - Batch operations through IPC
   - Connection pooling
   - Cached field mappings
   - Asynchronous operations

3. **Memory Management**:
   - Smart pointers for database objects
   - RAII for resource management
   - Memory pools for frequent operations

### Performance Monitoring
```cpp
// Performance monitoring
class PerformanceMonitor
{
public:
    struct DatabaseMetrics
    {
        double openTime;             // Database open time (ms)
        double queryTime;            // Average query time (ms)
        double migrationTime;        // Migration time (ms)
        size_t memoryUsage;          // Memory usage (bytes)
        int recordCount;             // Number of records
    };
    
    void StartTimer(const CString& operation);
    void EndTimer(const CString& operation);
    DatabaseMetrics GetMetrics(const CString& databasePath);
    void LogPerformance(const CString& operation, double time);
};
```

## 11. Testing Strategy

### Test Scenarios
1. **Database Operations**:
   - Open/close SQLite databases
   - Open/close MDB files through plugin
   - CRUD operations on both types
   - Large dataset handling

2. **Migration Testing**:
   - MDB to SQLite migration
   - Data integrity validation
   - Performance comparison
   - Error recovery

3. **Hybrid Scenarios**:
   - Mixed database types
   - Cross-database operations
   - Plugin switching
   - Concurrent access

4. **Error Conditions**:
   - Corrupted databases
   - Network failures
   - Memory exhaustion
   - Plugin failures

### Test Data
- Small MDB files (< 1MB)
- Medium MDB files (1-100MB)
- Large MDB files (> 100MB)
- Corrupted MDB files
- Various field types and data

## 12. Deployment Strategy

### Installation Package
1. **64-bit dbWave2 executable**
2. **32-bit MDB reader executable**
3. **SQLite runtime libraries**
4. **Configuration files**
5. **Migration tools**
6. **Documentation**

### User Migration Path
1. **Automatic detection** of existing MDB files
2. **One-click migration** to SQLite
3. **Backup creation** before migration
4. **Validation** of migrated data
5. **Option to keep** original MDB files

### Rollback Strategy
1. **Keep original MDB files** as backup
2. **Export to MDB** functionality
3. **Plugin fallback** for MDB access
4. **Configuration backup** and restore

## Conclusion

This migration plan provides a comprehensive solution for converting dbWave2 to 64-bit while maintaining full backward compatibility with existing 32-bit MDB files. The plugin architecture ensures that users can continue working with their existing data while gradually migrating to the more efficient SQLite format.

The hybrid approach allows for:
- **Immediate 64-bit benefits** (larger memory, better performance)
- **Seamless backward compatibility** with existing MDB files
- **Gradual migration** at user's own pace
- **Data safety** through backup and validation
- **Future-proof architecture** for additional database types

The implementation should be done in phases with thorough testing at each stage to ensure data integrity and user satisfaction.
