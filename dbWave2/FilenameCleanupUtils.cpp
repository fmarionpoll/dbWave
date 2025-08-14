#include "StdAfx.h"
#include "dbWave.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "FilenameCleanupUtils.h"

// Utility function to clean up data file filenames
BOOL CleanupDataFileFilenames(CdbWaveDoc* p_db_doc)
{
	if (!p_db_doc)
		return FALSE;

	auto* p_db_table = p_db_doc->db_table;
	if (!p_db_table || !p_db_table->m_main_table_set.IsOpen())
	{
		AfxMessageBox(_T("Database is not open."), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	try
	{
		int total_records = 0;
		int records_updated = 0;
		// Move to first record
		p_db_table->m_main_table_set.MoveFirst();
		
		// Iterate through all records
		while (!p_db_table->m_main_table_set.IsEOF())
		{
			total_records++;
			
			// Get current filename
			CString current_filename = p_db_table->m_main_table_set.m_file_dat;
			
			// Check if filename has leading spaces
			if (!current_filename.IsEmpty() && current_filename[0] == _T(' '))
			{
				// Remove leading spaces
				CString trimmed_filename = current_filename;
				trimmed_filename.TrimLeft();
				
				// Update the record if filename changed
				p_db_table->m_main_table_set.Edit();
				p_db_table->m_main_table_set.m_file_dat = trimmed_filename;
				p_db_table->m_main_table_set.Update();
				records_updated++;
			}
			
			// Move to next record
			p_db_table->m_main_table_set.MoveNext();
		}
		p_db_table->m_main_table_set.MoveFirst();

		// Show results
		CString result_msg;
		result_msg.Format(_T("Data file filename cleanup completed.\n\nTotal records processed: %d\nRecords updated: %d"), 
						 total_records, records_updated);
		AfxMessageBox(result_msg, MB_OK | MB_ICONINFORMATION);
		
		// Refresh views
		p_db_doc->update_all_views_db_wave(nullptr, 0L, nullptr);
		
		return TRUE;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 66);
		e->Delete();
		return FALSE;
	}
}

// Utility function to clean up spike file filenames
BOOL CleanupSpikeFileFilenames(CdbWaveDoc* p_db_doc)
{
	if (!p_db_doc)
		return FALSE;

	auto* p_db_table = p_db_doc->db_table;
	if (!p_db_table || !p_db_table->m_main_table_set.IsOpen())
	{
		AfxMessageBox(_T("Database is not open."), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	try
	{
		int total_records = 0;
		int records_updated = 0;
		// Move to first record
		p_db_table->m_main_table_set.MoveFirst();
		
		// Iterate through all records
		while (!p_db_table->m_main_table_set.IsEOF())
		{
			total_records++;
			
			// Get current filename
			CString current_filename = p_db_table->m_main_table_set.m_file_spk;
			
			// Check if filename has leading spaces
			if (!current_filename.IsEmpty() && current_filename[0] == _T(' '))
			{
				// Remove leading spaces
				CString trimmed_filename = current_filename;
				trimmed_filename.TrimLeft();
				
				// Update the record if filename changed
				p_db_table->m_main_table_set.Edit();
				p_db_table->m_main_table_set.m_file_spk = trimmed_filename;
				p_db_table->m_main_table_set.Update();
				records_updated++;
			}
			
			// Move to next record
			p_db_table->m_main_table_set.MoveNext();
		}
		p_db_table->m_main_table_set.MoveFirst();
		
		// Show results
		CString result_msg;
		result_msg.Format(_T("Spike file filename cleanup completed.\n\nTotal records processed: %d\nRecords updated: %d"), 
						 total_records, records_updated);
		AfxMessageBox(result_msg, MB_OK | MB_ICONINFORMATION);
		
		// Refresh views
		p_db_doc->update_all_views_db_wave(nullptr, 0L, nullptr);
		
		return TRUE;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 131);
		e->Delete();
		return FALSE;
	}
}
