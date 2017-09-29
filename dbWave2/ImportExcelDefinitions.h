#pragma once

// Make sure your file paths for MSO.DLL, VBE6EXT.OLB and EXCEL.EXE are 100% correct
// For 32-bit systems the file path is usually "C:\\Program Files\\..."
// For 64-bit systems the file path is usually "C:\\Program Files (x86)\\..."

//import entry points for excel and access

#import "C:\\Program Files\\Common Files\\Microsoft Shared\\OFFICE15\\mso.dll"  \
		rename("RGB",			"MSORGB")
using namespace Office;

#import "C:\\Program Files (x86)\\Common Files\\Microsoft Shared\\VBA\\VBA6\\VBE6EXT.OLB" \
		raw_interfaces_only, \
		rename("Reference",		"ignorethis"), \
		rename("VBE",			"testVBE")
using namespace VBIDE;

#import "C:\\Program Files\\Microsoft Office\\OFFICE15\\excel.exe" \
		exclude("IFont", "IPicture") \
		rename("VBE",			"testVBE") \
		rename("FindText",		"ExcelFindText") \
		rename("NoPrompt",		"ExcelNoPrompt") \
		rename("CopyFile",		"ExcelCopyFile") \
		rename("ReplaceText",	"ExcelReplaceText") \
		rename("RGB",			"ExcelRGB") \
		rename("DialogBox",		"ExcelDialogBox") 	\
		no_auto_exclude
using namespace Excel;

#import <C:\\Program Files\\Common Files\\Microsoft Shared\\OFFICE15\\ACEDAO.dll>  \
	rename( "EOF", "AdoNSEOF" ) \
	auto_rename
using namespace DAO;

