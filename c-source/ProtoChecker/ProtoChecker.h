#pragma once

using TFileTuple = std::tuple<std::string, std::vector<std::string>, bool, std::vector<std::tuple<uint32_t, uint32_t>>>;

/**
 * \brief
 * Color configuration.
 */
static const auto COLOR_GREEN = "\033[92m%s\033[00m";
static const auto COLOR_RED = "\033[91m%s\033[00m";
static const auto COLOR_GRAY = "\033[37m%s\033[00m";

class CProtoChecker
{
	std::vector<std::string> m_vecLogFile;
	TFileTuple m_ItemProtoFile, m_ItemNamesFile, m_MobProtoFile, m_MobNamesFile;

	enum EFile
	{
		NAME,
		DATA,
		EXISTS,
	};
	
	public:
		CProtoChecker();
		virtual ~CProtoChecker();

		auto run() -> void;
		auto find_duplicate(const TFileTuple &) -> void;
		auto compare(const TFileTuple &, const TFileTuple &) -> void;

		auto write_log(const std::string, ...) -> void;
		auto create_file_log() -> void;

		auto load_file(const std::string&) const -> TFileTuple;

		static auto split_range(const std::string&, const std::string&) -> std::vector<std::string>;
		static auto strip_color(std::string&, const std::string&) -> void;
};