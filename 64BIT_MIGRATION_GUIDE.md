# 64-BIT MIGRATION GUIDE FOR DBWAVE2

## Overview
This document provides a comprehensive guide for converting the dbWave2 C++ MFC application from 32-bit to 64-bit. The application currently uses DAO (Data Access Objects) which is 32-bit only, requiring significant changes to the database access layer.

## Migration Prerequisites

### System Requirements
- Visual Studio 2019 or later
- Windows 10/11 64-bit
- 64-bit compatible database engine
- Updated development tools

### Backup Strategy
1. **Create complete project backup** before starting
2. **Document current database schema**
3. **Export all data** from existing Access database
4. **Version control** all changes

## 1. Visual Studio Project Configuration

### Update Project Settings
1. **Open Project Properties** (Right-click project → Properties)
2. **Platform Configuration**:
   - Change `Platform` from `Win32` to `x64`
   - Or add `x64` as an additional platform

### Build Configuration Changes
```
Configuration Manager → Active solution platform → x64
```

### Update Project Files
```xml
<!-- In .vcxproj file -->
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <PlatformToolset>v143</PlatformToolset>
  <TargetFrameworkVersion>v4.8</TargetFrameworkVersion>
  <Platform>x64</Platform>
</PropertyGroup>
```

### Update Preprocessor Definitions
```
WIN32;_DEBUG;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)
```
to:
```
WIN32;_DEBUG;_WINDOWS;_USRDLL;_WIN64;%(PreprocessorDefinitions)
```

## 2. Database Access Layer Migration

### Current State (32-bit DAO)
```cpp
// Current includes
#include <afxdao.h>

// Current classes
CDaoDatabase* p_database;
CDaoRecordset* p_recordset;
```

### Migration Options

#### Option A: Migrate to ADO (Recommended)
```cpp
// New includes
#include <adodb.h>
#include <adoint.h>

// New class declarations
_ConnectionPtr m_pConnection;
_RecordsetPtr m_pRecordset;

// Connection string
CString connectionString = _T("Provider=Microsoft.ACE.OLEDB.16.0;Data Source=dbWave.mdb;");
```

#### Option B: Use SQLite (Lightweight Alternative)
```cpp
// Add SQLite support
#include <sqlite3.h>

// Database handle
sqlite3* m_pDatabase;
sqlite3_stmt* m_pStatement;

// Connection
int result = sqlite3_open("dbWave.db", &m_pDatabase);
```

#### Option C: Use ODBC with SQL Server
```cpp
// ODBC includes
#include <afxdb.h>

// Database classes
CDatabase m_database;
CRecordset m_recordset;

// Connection string
CString connectionString = _T("ODBC;DSN=dbWave;UID=;PWD=;");
```

## 3. Code Changes Required

### Data Type Updates

#### Update dbTableMain.h
```cpp
// Change field types from long to __int64
class CdbTableMain : public CRecordset  // or _RecordsetPtr
{
public:
    __int64 m_id {0};                    // Was: long m_id
    __int64 m_id_insect {0};             // Was: long m_id_insect
    __int64 m_id_sensillum {0};          // Was: long m_id_sensillum
    __int64 m_insect_id {0};             // Was: long m_insect_id
    __int64 m_location_id {0};           // Was: long m_location_id
    __int64 m_operator_id {0};           // Was: long m_operator_id
    __int64 m_sensillum_id {0};          // Was: long m_sensillum_id
    __int64 m_path_id {0};               // Was: long m_path_id
    __int64 m_path2_id {0};              // Was: long m_path2_id
    __int64 m_data_len {0};              // Was: long m_data_len
    __int64 m_n_spikes {0};              // Was: long m_n_spikes
    __int64 m_n_spike_classes {0};       // Was: long m_n_spike_classes
    __int64 m_stimulus_id {0};           // Was: long m_stimulus_id
    __int64 m_concentration_id {0};      // Was: long m_concentration_id
    __int64 m_stimulus2_id {0};          // Was: long m_stimulus2_id
    __int64 m_concentration2_id {0};     // Was: long m_concentration2_id
    __int64 m_flag {0};                  // Was: long m_flag
    __int64 m_strain_id {0};             // Was: long m_strain_id
    __int64 m_sex_id {0};                // Was: long m_sex_id
    __int64 m_repeat {0};                // Was: long m_repeat
    __int64 m_repeat2 {0};               // Was: long m_repeat2
    __int64 m_experiment_id {0};         // Was: long m_experiment_id
    
    // String fields remain the same
    CString m_file_dat {_T("")};
    CString m_file_spk {_T("")};
    CString m_acq_comment {_T("")};
    CString m_more {_T("")};
    
    // Date field remains the same
    COleDateTime m_table_acq_date {static_cast<DATE>(0)};
};
```

### Database Operation Updates

#### Replace DAO Operations with ADO
```cpp
// Old DAO code:
void DlgdbEditRecord::OnBnClickedButton8()
{
    auto* p_db_table = m_pdb_doc->db_table;
    p_db_table->m_main_table_set.MoveFirst();
    while (!p_db_table->m_main_table_set.IsEOF())
    {
        CString current_filename = p_db_table->m_main_table_set.m_file_dat;
        if (!current_filename.IsEmpty() && current_filename[0] == _T(' '))
        {
            CString trimmed_filename = current_filename;
            trimmed_filename.TrimLeft();
            if (trimmed_filename != current_filename)
            {
                p_db_table->m_main_table_set.Edit();
                p_db_table->m_main_table_set.m_file_dat = trimmed_filename;
                p_db_table->m_main_table_set.Update();
            }
        }
        p_db_table->m_main_table_set.MoveNext();
    }
}

// New ADO code:
void DlgdbEditRecord::OnBnClickedButton8()
{
    try
    {
        m_pRecordset->MoveFirst();
        while (!m_pRecordset->EndOfFile)
        {
            CString current_filename = (LPCTSTR)(_bstr_t)m_pRecordset->Fields->Item["filename"]->Value;
            if (!current_filename.IsEmpty() && current_filename[0] == _T(' '))
            {
                CString trimmed_filename = current_filename;
                trimmed_filename.TrimLeft();
                if (trimmed_filename != current_filename)
                {
                    m_pRecordset->Fields->Item["filename"]->Value = (_bstr_t)trimmed_filename;
                    m_pRecordset->Update();
                }
            }
            m_pRecordset->MoveNext();
        }
    }
    catch (_com_error& e)
    {
        CString errorMsg = (LPCTSTR)e.Description();
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);
    }
}
```

### Pointer and Size Updates
```cpp
// Update array size handling
SIZE_T array_size = p_id_array->GetSize();  // Was: int array_size

// Update loop counters
for (SIZE_T i = 0; i < array_size; i++)     // Was: for (int i = 0; i < array_size; i++)

// Update memory allocations
void* p_buffer = malloc(sizeof(__int64) * count);  // Was: malloc(sizeof(long) * count)
```

## 4. Database Migration Strategy

### Option 1: Convert Access Database to SQLite
```cpp
// Migration script example
void MigrateToSQLite()
{
    // Create SQLite database
    sqlite3* db;
    int result = sqlite3_open("dbWave.db", &db);
    
    // Create tables
    const char* createTableSQL = 
        "CREATE TABLE main_table ("
        "id INTEGER PRIMARY KEY, "
        "filename TEXT, "
        "filespk TEXT, "
        "acq_date DATETIME, "
        "acq_comment TEXT, "
        "id_insect INTEGER, "
        "id_sensillum INTEGER, "
        "more TEXT, "
        "insect_id INTEGER, "
        "location_id INTEGER, "
        "operator_id INTEGER, "
        "sensillum_id INTEGER, "
        "path_id INTEGER, "
        "path2_id INTEGER, "
        "data_len INTEGER, "
        "n_spikes INTEGER, "
        "n_spike_classes INTEGER, "
        "stimulus_id INTEGER, "
        "concentration_id INTEGER, "
        "stimulus2_id INTEGER, "
        "concentration2_id INTEGER, "
        "flag INTEGER, "
        "strain_id INTEGER, "
        "sex_id INTEGER, "
        "repeat_val INTEGER, "
        "repeat2_val INTEGER, "
        "acqdate_day DATE, "
        "acqdate_time TIME, "
        "expt_id INTEGER"
        ");";
    
    sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);
}
```

### Option 2: Use SQL Server Express
```cpp
// Connection string for SQL Server
CString connectionString = _T("Provider=SQLOLEDB;Server=localhost\\SQLEXPRESS;Database=dbWave;Trusted_Connection=yes;");

// Migration using SQL Server Management Objects (SMO)
void MigrateToSQLServer()
{
    // Implementation for SQL Server migration
    // This would involve creating tables and importing data
}
```

## 5. Updated File Structure

### New Include Files
```cpp
// dbWave.h - Updated includes
#include <afxwin.h>
#include <afxext.h>
#include <afxdb.h>        // For ODBC
// #include <afxdao.h>    // Remove DAO
#include <adodb.h>        // Add ADO
#include <adoint.h>       // Add ADO interfaces
```

### Updated Class Declarations
```cpp
// dbWaveDoc.h - Updated database access
class CdbWaveDoc : public COleDocument
{
public:
    // Replace DAO with ADO
    _ConnectionPtr m_pConnection;
    _RecordsetPtr m_pMainRecordset;
    
    // Update method signatures
    BOOL OpenDatabase(const CString& filename);
    BOOL CloseDatabase();
    BOOL ExecuteQuery(const CString& sql);
};
```

## 6. Migration Steps

### Phase 1: Preparation (Week 1)
1. **Backup** entire project
2. **Create** new 64-bit project configuration
3. **Document** all DAO usage in codebase
4. **Set up** version control for migration

### Phase 2: Database Migration (Week 2)
1. **Choose** target database (SQLite recommended)
2. **Create** migration scripts
3. **Test** data integrity after migration
4. **Update** connection strings

### Phase 3: Code Updates (Week 3-4)
1. **Replace** DAO classes with new database access
2. **Update** data types (long → __int64)
3. **Fix** pointer arithmetic issues
4. **Update** string handling

### Phase 4: Testing (Week 5)
1. **Unit tests** for database operations
2. **Integration tests** for full workflow
3. **Performance testing**
4. **User acceptance testing**

## 7. Testing Strategy

### Test Scenarios
1. **Database Operations**:
   - Create, Read, Update, Delete operations
   - Large dataset handling
   - Concurrent access

2. **File Operations**:
   - File path handling
   - File size limitations
   - Unicode filename support

3. **Memory Management**:
   - Memory leak detection
   - Large object handling
   - Garbage collection

4. **Performance**:
   - Query performance
   - Memory usage
   - Startup time

### Validation Checklist
- [ ] All database CRUD operations work
- [ ] File operations handle long paths
- [ ] Memory usage is reasonable
- [ ] Application starts without errors
- [ ] All dialogs display correctly
- [ ] Data integrity is maintained

## 8. Alternative: WOW64 Compatibility

If migration is too complex, consider:
1. **Keep** application 32-bit
2. **Run** on 64-bit Windows using WOW64
3. **Install** 32-bit Access Database Engine
4. **Test** compatibility thoroughly

### WOW64 Setup
```batch
# Install 32-bit Access Database Engine
AccessDatabaseEngine.exe /quiet /passive

# Verify installation
reg query "HKLM\SOFTWARE\Microsoft\Office\16.0\Access\DatabaseEngine\Engines\ACE" /v Engine
```

## 9. Tools and Resources

### Development Tools
- **Visual Studio 2019/2022** with C++ workload
- **SQLite Database Browser** for database management
- **Access Database Engine 2016** (32-bit for WOW64)
- **SQL Server Management Studio** (if using SQL Server)

### Documentation
- Microsoft's "Porting and Updating Visual C++ Applications"
- SQLite C/C++ Interface documentation
- ADO.NET documentation
- MFC 64-bit migration guide

### Useful Links
- [Visual Studio Migration Guide](https://docs.microsoft.com/en-us/cpp/porting/visual-cpp-porting-and-upgrading-guide)
- [SQLite Documentation](https://www.sqlite.org/docs.html)
- [ADO.NET Documentation](https://docs.microsoft.com/en-us/dotnet/framework/data/adonet/)

## 10. Risk Assessment

### High Risk Areas
1. **Database Access Layer**: Complete rewrite required
2. **Data Type Changes**: Potential overflow issues
3. **Third-party Dependencies**: May not support 64-bit
4. **File Path Handling**: Long path support needed

### Mitigation Strategies
1. **Incremental Migration**: Migrate one component at a time
2. **Comprehensive Testing**: Test each phase thoroughly
3. **Rollback Plan**: Keep 32-bit version as backup
4. **User Training**: Prepare users for interface changes

## Conclusion

The migration from 32-bit to 64-bit for dbWave2 is a significant undertaking that requires careful planning and execution. The most critical decision is choosing the right database technology to replace DAO. SQLite is recommended for its simplicity and reliability, while SQL Server offers more enterprise features.

The migration should be done in phases with thorough testing at each stage. Consider the WOW64 alternative if the migration timeline or resources are constrained.

Remember to maintain the existing functionality while gaining the benefits of 64-bit architecture, including larger memory address space and improved performance.
