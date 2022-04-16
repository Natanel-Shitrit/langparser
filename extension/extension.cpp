#include "extension.h"
#include "context.h"

CExtension g_Extension;
SMEXT_LINK(&g_Extension);

ParseError_t CExtension::ParseFile(const char* pszRelativePath, ILanguageFileParserListener* pListener, char* error, size_t maxlength)
{
	// Convert the relative path to an absolute path.
	char realpath[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", pszRelativePath);

	// Read the file into a buffer.
	std::u16string file_content = ReadUTF16LEFile(realpath);

	if (file_content.empty())
	{
		// Failed to read the file.
        std::cout << "Failed to read file: " << realpath << std::endl;
		return ParseError_StreamOpen;
	}

    // check the first character, make sure this a little-endian unicode file.
    if (file_content[0] != 0xFEFF)
    {
        std::cout << "File is not little-endian unicode: " << realpath << std::endl;
        return ParseError_StreamOpen;
    }

    // remove the BOM.
    file_content.erase(0, 1);

	// Parse the file.
	ParseError_t result = CExtension::ParseBuffer((const ucs2_t*)file_content.c_str(), pListener, error, maxlength);
	
	// free the memory.
	file_content.clear();
	
	return result;
}

ParseError_t CExtension::ParseBuffer(const ucs2_t* pfile_contentIn, ILanguageFileParserListener* pListener, char* error, size_t maxlength)
{
	ParserContext parserCtx;
	return parserCtx.Process(pfile_contentIn, pListener, error, maxlength);
}

std::u16string CExtension::ReadUTF16LEFile(const char* filepath)
{
    std::ifstream file(filepath, std::ios::binary);

    // Check if file is open
    if (!file)
    {
        return u"";
    }

    // Get file size
    auto size = std::filesystem::file_size(filepath);

    // Allocate buffer
    std::u16string u16;
    u16.resize(size / 2);

    // Read file
    file.read((char*)&u16[0], size);

	// ensure that the file is null-terminated.
    u16.push_back(0);

    // Close file
    file.close();

    return u16;
}

bool CExtension::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
	sharesys->AddInterface(myself, this);
	return true;
}
