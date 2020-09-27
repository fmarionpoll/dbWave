#pragma once

// Make sure your file paths for MSO.DLL, VBE6EXT.OLB and EXCEL.EXE are 100% correct
// For 32-bit systems the file path is usually "C:\\Program Files\\..."
// For 64-bit systems the file path is usually "C:\\Program Files (x86)\\..."

//import entry points for excel and access

#import "./include/office/mso.dll"  \
		rename("RGB",			"MSORGB")
using namespace Office;

#import "./include/office/VBE6EXT.OLB" \
		raw_interfaces_only, \
		rename("Reference",		"ignorethis"), \
		rename("VBE",			"testVBE")
using namespace VBIDE;

#import "./include/office/excel.exe" \
		exclude("IFont",		"IPicture") \
		rename("VBE",			"testVBE") \
		rename("FindText",		"ExcelFindText") \
		rename("NoPrompt",		"ExcelNoPrompt") \
		rename("CopyFile",		"ExcelCopyFile") \
		rename("ReplaceText",	"ExcelReplaceText") \
		rename("RGB",			"ExcelRGB") \
		rename("DialogBox",		"ExcelDialogBox") 	\
		no_auto_exclude
using namespace Excel;

#import "./include/office/ACEDAO.DLL"  \
	rename( "EOF", "AdoNSEOF" ) \
	auto_rename
using namespace DAO;
