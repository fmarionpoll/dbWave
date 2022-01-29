// AsystHeader.h
#pragma once

#define ASYSTHEADER	1024

//-----------------  structure du fichier Asyst -----------------------------------------------------------

using FILE_TMP_HEADER = struct
{
	short nb_subfile; // total nb of subfiles
	short btaille;
	// total size of template (bytes) including file template header) (= offset next avail subfile template)
	short nb_data_enr; // nb data records in file (1 record = 128 byte)
	short nb_ligne_com; // nb comment lines (1 line = 64 chars)
	unsigned char inf_asyst[4]; // signature Asyst
	short nb_byte_inf; // amount of data file template currently in use
	short free_fth[5];
}; // ASYST FILE HEADER FORMAT

using SUBFILE_TMP_FORMAT = struct
{
	short nb_subfile; // nb data subfiles
	short btaille; // taile subfile template
	short nb_enr; // nb 128 bytes records / subfile (data size (bytes) = 128*nb_enr*nb_subfile
	short type_data; // type of data entier = 4 (!)
	short nb_dimensions;
	short taille_dim1; // taille dimension 1 (nb elements / canal ou nb canaux
	short taille_dim2; // ???
}; // SUBFILE DATA HEADER FORMAT
