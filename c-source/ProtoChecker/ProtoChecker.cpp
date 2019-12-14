/***********************************
__name__    = "ProtoChecker"
__author__  = "VegaS"
__date__    = "2019-12-05"
__version__ = "0.0.1"
***********************************/
#include "pch.h"
#include "ProtoChecker.h"
#include "CsvReader.h"

/**
 * \brief
 * Character delimiters.
 */
enum EDelimiter : char
{
	DELIMITER_RANGE = '~',
	DELIMITER_TAB	= '\t',
};

/**
 * \brief
 * Vnum range config.
 */
enum EVnumRangeConfig : uint32_t
{
	VNUM_RANGE_START    = 110000,
	VNUM_RANGE_END      = 165400,
	VNUM_RANGE          = 99,
};

/**
 * \brief
 * Path configuration.
 */
static const auto ROOT_PATH             = "resource\\";
static const auto LOG_FILE_NAME         = "syslog.txt";

static const auto ITEM_PROTO_FILE_NAME  = "item_proto.txt";
static const auto ITEM_NAMES_FILE_NAME  = "item_names.txt";
static const auto MOB_PROTO_FILE_NAME   = "mob_proto.txt";
static const auto MOB_NAMES_FILE_NAME   = "mob_names.txt";

/**
 * \brief
 * Builtin translation.
 */
using TLocaleStringMap = std::map<std::string, std::string>;
TLocaleStringMap TRANSLATE_MAP =
{
	{"FILE_EMPTY",          format(COLOR_GREEN, "File %s is empty.")},
	{"FILE_OK",             format(COLOR_GREEN, "\tOK")},
	{"FILE_NAME",           format(COLOR_GRAY,  "Reading file: %s")},
	{"FILE_TOTAL_LINES",    format(COLOR_RED,   "\tTotal lines: %d")},
	{"FILE_DUPLICATE_LINE", format(COLOR_RED,   "\tDuplicated itemVnum: %s at line: %d")},
	{"FILE_COMPARING_LINE", format(COLOR_RED,   "\tMissing itemVnum: %s")},
};

/**
 * \brief
 * Called the constructor/destructor method.
 */
CProtoChecker::CProtoChecker() :
	m_ItemProtoFile(load_file(ITEM_PROTO_FILE_NAME)),
	m_ItemNamesFile(load_file(ITEM_NAMES_FILE_NAME)),
	m_MobProtoFile(load_file(MOB_PROTO_FILE_NAME)),
	m_MobNamesFile(load_file(MOB_NAMES_FILE_NAME))
{
}

CProtoChecker::~CProtoChecker()
{
}

/**
 * \brief
 * Load a specific file and save the data into a tuple.
 * \param: stFileName: string
 * \return: tuple
 */
auto CProtoChecker::load_file(const std::string& stFileName) const -> TFileTuple
{
	cCsvTable csv_reader;
	std::vector<std::string> vecFileData;
	std::vector<std::tuple<uint32_t, uint32_t>> vecRangeVnum;

	std::string stFilePath(ROOT_PATH);
	stFilePath.append(stFileName);

	const auto bFileExists = csv_reader.Load(stFilePath.c_str(), DELIMITER_TAB);
	if (bFileExists)
	{
		csv_reader.Next();
		while (csv_reader.Next())
		{
			const auto & stItemVnum = csv_reader.AsStringByIndex(0);
			if (stItemVnum.find(DELIMITER_RANGE) != std::string::npos)
			{
				const auto & vecVnumRange = split_range(stItemVnum, std::string(1, DELIMITER_RANGE));
				vecRangeVnum.emplace_back(strtoul(vecVnumRange[0]), strtoul(vecVnumRange[1]));
			}

			vecFileData.emplace_back(stItemVnum);
		}
	}

	return std::make_tuple(stFileName, vecFileData, bFileExists, vecRangeVnum);
}

/**
 * \brief
 * Read a specific file and find the duplicates lines.
 * \param: file: tuple
 */
auto CProtoChecker::find_duplicate(const TFileTuple& file) -> void
{
	if (!std::get<EXISTS>(file))
		return;

	const auto & stFileName = std::get<NAME>(file);
	const auto & vecFileData = std::get<DATA>(file);

	write_log(TRANSLATE_MAP["FILE_NAME"], stFileName.c_str());

	std::map<std::string, uint32_t> mapFileDuplicate;
	uint32_t lineIndex = 1;
	for (const auto & stItemVnum : vecFileData)
	{
		++lineIndex;
		const auto iCount = std::count(vecFileData.begin(), vecFileData.end(), stItemVnum);
		if (iCount > 1)
			mapFileDuplicate[stItemVnum] = lineIndex;
	}

	for (const auto & it : mapFileDuplicate)
		write_log(TRANSLATE_MAP["FILE_DUPLICATE_LINE"], it.first.c_str(), it.second);

	write_log(mapFileDuplicate.empty() ? TRANSLATE_MAP["FILE_OK"] : TRANSLATE_MAP["FILE_TOTAL_LINES"], mapFileDuplicate.size());
}

/**
 * \brief
 * Compare two files and find the differences between them.
 * \param: fileExamine: tuple
 * \param: fileSearch: tuple
 */
auto CProtoChecker::compare(const TFileTuple& fileExamine, const TFileTuple& fileSearch) -> void
{
	if (!std::get<EXISTS>(fileExamine) || !std::get<EXISTS>(fileSearch))
		return;
	 
	auto vecFileExamine = std::get<DATA>(fileExamine);
	auto vecFileSearch = std::get<DATA>(fileSearch);
	decltype(vecFileSearch) vecFileDifference;

	std::sort(vecFileExamine.begin(), vecFileExamine.end());
	std::sort(vecFileSearch.begin(), vecFileSearch.end());

	const auto bCmpItemProto = std::get<NAME>(fileSearch) == ITEM_PROTO_FILE_NAME;
	const auto bCmpItemNames = std::get<NAME>(fileSearch) == ITEM_NAMES_FILE_NAME;

	if (bCmpItemProto || bCmpItemNames)
	{
		for (const auto & stItemVnum : vecFileExamine)
		{
			auto stBaseItemVnum = stItemVnum;
			if (bCmpItemProto)
			{
				const auto itemVnum = strtoul(stItemVnum);
				if (itemVnum >= VNUM_RANGE_START && itemVnum <= VNUM_RANGE_END)
				{
					char cVnumRange[64];
					_snprintf_s(cVnumRange, sizeof(cVnumRange), "%u%c%u", itemVnum, DELIMITER_RANGE, itemVnum + VNUM_RANGE);
					stBaseItemVnum = cVnumRange;
				}
			}
			if (bCmpItemNames)
			{
				if (stItemVnum.find(DELIMITER_RANGE) != std::string::npos)
				{
					const auto stItemVnumStart = split_range(stItemVnum, std::string(1, DELIMITER_RANGE)).at(0);
					stBaseItemVnum = stItemVnumStart;
				}
			}
			if (std::find(vecFileSearch.begin(), vecFileSearch.end(), stBaseItemVnum) == vecFileSearch.end())
				vecFileDifference.emplace_back(stBaseItemVnum);
		}
	}
	else
	{
		std::set_difference(vecFileExamine.begin(), vecFileExamine.end(), vecFileSearch.begin(), vecFileSearch.end(), std::inserter(vecFileDifference, vecFileDifference.begin()));
	}

	write_log(TRANSLATE_MAP["FILE_NAME"], std::get<EFile::NAME>(fileSearch).c_str());

	if (vecFileDifference.empty())
	{
		write_log(TRANSLATE_MAP["FILE_OK"]);
		return;
	}

	for (const auto & stItemVnum : vecFileDifference)
		write_log(TRANSLATE_MAP["FILE_COMPARING_LINE"], stItemVnum.c_str());

	write_log(TRANSLATE_MAP["FILE_TOTAL_LINES"], vecFileDifference.size());
}

/**
 * \brief
 * Split a string by a specific regular expression.
 * \param:: stItemVnum: string
 * \param:: stRegex: string
 * \return: vector
 */
auto CProtoChecker::split_range(const std::string& stItemVnum, const std::string& stRegex) -> std::vector<std::string>
{
	try
	{
		const std::regex re(stRegex);
		const std::sregex_token_iterator first{stItemVnum.begin(), stItemVnum.end(), re, -1};
		const std::sregex_token_iterator last;
		return { first, last };
	}
	catch (const std::regex_error & e)
	{
		std::cout << e.what() << e.code() << std::endl;
	}
	return {};
}

/**
 * \brief
 * Strip a specific string from the beginning and at the end of the string.
 * \param: stSrc: string
 * \param: stFrom: string
 */
auto CProtoChecker::strip_color(std::string & stSrc, const std::string& stFrom) -> void
{
	size_t iStartPos = 0;
	while ((iStartPos = stSrc.find(stFrom, iStartPos)) != std::string::npos)
		stSrc.replace(iStartPos, stFrom.length(), "");
}

/**
 * \brief
 * Write a log line to console and specific log file.
 * \param: fmt: string
 * \param: ...: any
 */
auto CProtoChecker::write_log(const std::string fmt, ...) -> void
{
	char cLogLine[512];
	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(cLogLine, sizeof(cLogLine), fmt.c_str(), args);
	va_end(args);

	std::string stLogLine = cLogLine;
	std::cout << stLogLine << std::endl;

	for (const auto & color: {COLOR_GREEN, COLOR_GRAY, COLOR_RED})
	{
		for (const auto & item : split_range(color, "%s"))
			strip_color(stLogLine, item);
	}
	
	m_vecLogFile.emplace_back(stLogLine);
}

/**
 * \brief
 * Create the syslog file.
 */
auto CProtoChecker::create_file_log() -> void
{
	std::ofstream outfile(LOG_FILE_NAME);
	for (const auto & line : m_vecLogFile)
		outfile << line << std::endl;

	outfile.close();
}

/**
 * \brief
 * Run the program.
 */
auto CProtoChecker::run() -> void
{
	write_log("###### START_CHECKING_FOR_DUPLICATE ######");
	for (const auto & file : { m_ItemProtoFile, m_ItemNamesFile, m_MobProtoFile, m_MobNamesFile })
		find_duplicate(file);
	write_log("###### END_CHECKING_FOR_DUPLICATE ######");

	write_log("###### START_COMPARING ######");
	compare(m_ItemProtoFile, m_ItemNamesFile);
	compare(m_ItemNamesFile, m_ItemProtoFile);
	compare(m_MobProtoFile, m_MobNamesFile);
	compare(m_MobNamesFile, m_MobProtoFile);
	write_log("###### END_COMPARING ######");

	create_file_log();
}