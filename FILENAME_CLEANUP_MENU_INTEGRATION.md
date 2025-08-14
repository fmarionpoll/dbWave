# FILENAME CLEANUP MENU INTEGRATION

## Overview
This document describes the integration of filename cleanup functionality into the main dbWave2 application menu system. The implementation allows users to access the filename cleanup routines directly from the Database submenu in the Tools menu, providing a more accessible interface than the previous dialog-only approach.

## Implementation Details

### Files Modified

#### 1. Resource Files
- **`resource.h`** - Added new menu item ID: `ID_TOOLS_CLEANUP_FILENAMES`
- **`dbWave.rc`** - Added menu item to Database submenu and string resource

#### 2. Source Files
- **`ChildFrm.h`** - Added function declaration for menu handler
- **`ChildFrm.cpp`** - Added menu handler implementation and include
- **`dlgdbeditRecord.cpp`** - Added utility functions for external access

### Menu Integration

#### Menu Structure
The new menu item is added to the Database submenu under Tools:
```
Tools
├── Import
├── Export
├── Database
│   ├── Remove duplicate records...
│   ├── Remove missing files...
│   ├── Restore deleted files...
│   ├── Remove artefact files...
│   ├── Check consistency of spike file names...
│   ├── ────────────────────── (separator)
│   └── Clean up filename spaces... ← NEW ITEM
├── Upload database infos into data files
└── Copy database, data and spike files...
```

#### Menu Item Details
- **ID**: `ID_TOOLS_CLEANUP_FILENAMES` (33118)
- **Text**: "Clean up filename spaces..."
- **Location**: Tools → Database submenu
- **Access Key**: 'f' (Clean up &filename spaces...)

### Functionality

#### Menu Handler: `on_tools_cleanup_filenames()`
Located in `ChildFrm.cpp`, this function:

1. **Validates Active Document**: Checks if a database document is currently open
2. **Presents Options**: Shows a dialog with three cleanup options:
   - **Data files only** (Yes button)
   - **Spike files only** (No button)  
   - **Both data and spike files** (Cancel button)
3. **Executes Cleanup**: Calls appropriate utility functions based on user choice
4. **Updates Views**: Refreshes all views to reflect changes

#### Utility Functions

##### `CleanupDataFileFilenames(CdbWaveDoc* p_db_doc)`
- **Purpose**: Removes leading spaces from data file filenames (`.dat` files)
- **Process**: 
  - Iterates through all database records
  - Checks `m_file_dat` field for leading spaces
  - Trims leading spaces using `TrimLeft()`
  - Updates records that have changes
- **Feedback**: Shows progress dialog with total records processed and updated

##### `CleanupSpikeFileFilenames(CdbWaveDoc* p_db_doc)`
- **Purpose**: Removes leading spaces from spike file filenames (`.spk` files)
- **Process**:
  - Iterates through all database records
  - Checks `m_file_spk` field for leading spaces
  - Trims leading spaces using `TrimLeft()`
  - Updates records that have changes
- **Feedback**: Shows progress dialog with total records processed and updated

### User Experience

#### Menu Access
1. **Open dbWave2** application
2. **Open a database** file (.mdb)
3. **Navigate to**: Tools → Database → Clean up filename spaces...
4. **Select cleanup type** from the options dialog
5. **Review results** in the completion dialog

#### Options Dialog
The options dialog provides clear choices:
```
Which filename cleanup would you like to perform?

• Data files only: Clean up .dat file filenames
• Spike files only: Clean up .spk file filenames  
• Both: Clean up both types of filenames

Click Yes for Data files only
Click No for Spike files only
Click Cancel for Both
```

#### Progress Feedback
Each cleanup operation shows:
- **Total records processed**: Number of records examined
- **Records updated**: Number of records actually modified
- **Success/failure status**: Clear indication of operation result

### Error Handling

#### Database Validation
- **No active document**: Shows error message and exits
- **Database not open**: Shows error message and exits
- **Database access errors**: Catches CDaoException and displays details

#### User Cancellation
- **Options dialog**: User can cancel at any time
- **Confirmation dialogs**: Each cleanup operation asks for confirmation
- **Graceful exit**: All operations can be cancelled without side effects

### Code Structure

#### Message Map Integration
```cpp
// In ChildFrm.cpp
ON_COMMAND(ID_TOOLS_CLEANUP_FILENAMES, &CChildFrame::on_tools_cleanup_filenames)
```

#### Function Declaration
```cpp
// In ChildFrm.h
afx_msg void on_tools_cleanup_filenames();
```

#### Utility Function Access
```cpp
// External utility functions in dlgdbeditRecord.cpp
BOOL CleanupDataFileFilenames(CdbWaveDoc* p_db_doc);
BOOL CleanupSpikeFileFilenames(CdbWaveDoc* p_db_doc);
```

### Benefits

#### Improved Accessibility
- **Centralized access**: Available from main menu instead of hidden dialog
- **Consistent interface**: Follows same pattern as other database operations
- **Logical grouping**: Placed with related database maintenance functions

#### Enhanced User Experience
- **Clear options**: User can choose specific cleanup type
- **Progress feedback**: Shows what's happening and results
- **Error handling**: Graceful handling of all error conditions

#### Code Reusability
- **Utility functions**: Can be called from multiple locations
- **Modular design**: Clean separation of concerns
- **Maintainable**: Easy to modify or extend functionality

### Future Enhancements

#### Potential Improvements
1. **Batch processing**: Allow selection of specific records
2. **Preview mode**: Show what would be changed before applying
3. **Undo functionality**: Ability to revert changes
4. **Logging**: Detailed log of all changes made
5. **Progress bar**: Visual progress indicator for large databases

#### Integration Opportunities
1. **Automated cleanup**: Run as part of database maintenance
2. **Scheduled tasks**: Regular cleanup of new imports
3. **Validation rules**: Prevent leading spaces in new entries
4. **Export options**: Include cleanup in data export processes

## Conclusion

The filename cleanup menu integration provides a user-friendly way to access the existing cleanup functionality. By placing it in the Database submenu alongside other maintenance operations, users can easily find and use this feature to maintain clean, consistent filename data in their databases.

The implementation maintains backward compatibility with the existing dialog-based approach while providing a more accessible and integrated user experience.
