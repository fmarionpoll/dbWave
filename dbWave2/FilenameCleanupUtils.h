#pragma once

// Forward declarations
class CdbWaveDoc;

// Utility functions for cleaning up filenames in database records
BOOL CleanupDataFileFilenames(CdbWaveDoc* p_db_doc);
BOOL CleanupSpikeFileFilenames(CdbWaveDoc* p_db_doc);
