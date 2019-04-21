#include "pch.h"
#include <vector>
#include "Q3Model.h"

///////////////////////////////////////////////////////////////////////////////
// MD3Mesh
///////////////////////////////////////////////////////////////////////////////

MD3Mesh::MD3Mesh(std::string name, unzFile source) : m_sName(name)
{
	USES_CONVERSION;
	unz_file_info fi;

	if (unzLocateFile(source, m_sName.c_str(), 2) == UNZ_OK)
	{
		ATLTRACE(_T("Loading \"%s\"...\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
		ATLENSURE(unzGetCurrentFileInfo(source, &fi, nullptr, 0, nullptr, 0, nullptr, 0) == UNZ_OK);

		std::vector<unsigned char> mem(fi.uncompressed_size);
		ATLENSURE(unzOpenCurrentFile(source) == UNZ_OK);
		ATLENSURE(unzReadCurrentFile(source, mem.data(), fi.uncompressed_size) == fi.uncompressed_size);
		ATLENSURE(unzCloseCurrentFile(source) == UNZ_OK);
	}
	else ATLTRACE(_T("Failed loading \"%s\"...\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
}

MD3Mesh::~MD3Mesh()
{
	ATLTRACE(_T("Cleaning \"%s\".\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
// Q3Model
///////////////////////////////////////////////////////////////////////////////

Q3Model::Q3Model(LPCTSTR szFile)
{
	USES_CONVERSION;
	ATLTRACE(_T("Try loading model from %s...\n"), szFile);

	unzFile zip = unzOpen(CT2CA(szFile));
	if (zip)
	{
		int result = unzGoToFirstFile(zip);

		unz_file_info fi;
		char szEntry[MAX_PATH];
		char szModelPath[MAX_PATH];
		memset(szModelPath, 0, MAX_PATH);

		// find model path by iterating over all compressed files
		while (result == UNZ_OK)
		{
			if (unzGetCurrentFileInfo(zip, &fi, szEntry, MAX_PATH, nullptr, 0, nullptr, 0) == UNZ_OK)
			{
				ATLTRACE(_T("Zip entry: %s\n"), (LPCTSTR)CA2CT(szEntry));
				if (fi.size_filename > 8 && _stricmp(szEntry + fi.size_filename - 8, "head.md3") == 0)
				{
					strncpy_s(szModelPath, szEntry, fi.size_filename - 8);
					// break;
				}
			}

			result = unzGoToNextFile(zip);
		}

		if (strlen(szModelPath))
		{
			ATLTRACE(_T("Model path: %s\n"), (LPCTSTR)CA2CT(szModelPath));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "head.md3");
			AddChild(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "upper.md3");
			AddChild(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "lower.md3");
			AddChild(new MD3Mesh(szEntry, zip));
		}

		unzClose(zip);
	}
	else ATLTRACE(_T("Load failed. Could not open file!\n"));
}

Q3Model::~Q3Model()
{
}
