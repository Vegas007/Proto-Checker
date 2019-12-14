#ifndef __CSVFILE_H__
#define __CSVFILE_H__

#include <string>
#include <vector>

class cCsvAlias
{
	private:
		using NAME2INDEX_MAP = std::map<std::string, size_t>;
		using INDEX2NAME_MAP = std::map<size_t, std::string>;

		NAME2INDEX_MAP m_Name2Index;
		INDEX2NAME_MAP m_Index2Name;

	public:
		cCsvAlias() {}
		virtual ~cCsvAlias() = default;

	public:
		auto AddAlias(const char* name, size_t index) -> void;
		auto Destroy() -> void;
		auto operator [](size_t index) const -> const char*;
		auto operator [](const char* name) const -> size_t;

	private:
		cCsvAlias(const cCsvAlias&) {}

		auto operator =(const cCsvAlias&) const -> const cCsvAlias&
		{
			return *this;
		}
};

class cCsvRow : public std::vector<std::string>
{
	public:
		cCsvRow() {}
		~cCsvRow() {}

	public:
		auto AsInt(size_t index) const -> int
		{
			return atoi(at(index).c_str());
		}

		auto AsDouble(size_t index) const -> double
		{
			return atof(at(index).c_str());
		}

		auto AsString(size_t index) const -> const char*
		{
			return at(index).c_str();
		}

		auto AsInt(const char* name, const cCsvAlias& alias) const -> int
		{
			return atoi(at(alias[name]).c_str());
		}

		auto AsDouble(const char* name, const cCsvAlias& alias) const -> double
		{
			return atof(at(alias[name]).c_str());
		}

		auto AsString(const char* name, const cCsvAlias& alias) const -> const char*
		{
			return at(alias[name]).c_str();
		}

	private:
		cCsvRow(const cCsvRow&) {}
		const cCsvRow& operator = (const cCsvRow&) const
		{
			return *this;
		}
};

class cCsvFile
{
	private:
		typedef std::vector<cCsvRow*> ROWS;
		ROWS m_Rows;

	public:
		cCsvFile() = default;
		virtual ~cCsvFile() { Destroy(); }

	public:
		auto Load(const char* fileName, const char seperator = ',', const char quote = '"') -> bool;
		auto Save(const char* fileName, bool append = false, char seperator = ',', char quote = '"') const -> bool;
		auto Destroy() -> void;

		auto operator [](size_t index) -> cCsvRow*;
		auto operator [](size_t index) const -> const cCsvRow*;

		auto GetRowCount() const -> size_t
		{
			return m_Rows.size();
		}

	private:
		cCsvFile(const cCsvFile &) {}

		auto operator =(const cCsvFile&) const -> const cCsvFile&
		{
			return *this;
		}
};

class cCsvTable
{
	public:
		cCsvFile  m_File;
	private:
		cCsvAlias m_Alias;
		int       m_CurRow;

	public:
		cCsvTable();
		virtual ~cCsvTable();


	public:
		bool Load(const char* fileName, const char seperator = ',', const char quote = '"');
		auto Destroy() -> void;

		auto Next() -> bool;
		auto ColCount() const -> size_t;
		auto AsInt(size_t index) const -> int;
		auto AsDouble(size_t index) const -> double;
		auto AsStringByIndex(size_t index) const -> std::string;

		auto AsInt(const char* name) const -> int
		{
			return AsInt(m_Alias[name]);
		}

		auto AsDouble(const char* name) const -> double
		{
			return AsDouble(m_Alias[name]);
		}

		auto AsString(const char* name) const -> const char*
		{
			return AsStringByIndex(m_Alias[name]).c_str();
		}

		auto AddAlias(const char* name, size_t index) -> void
		{
			m_Alias.AddAlias(name, index);
		}

	private:
		auto CurRow() const -> const cCsvRow*;
		cCsvTable(const cCsvTable&)
		{
		}

		auto operator =(const cCsvTable&) const -> const cCsvTable&
		{
			return *this;
		}
};
#endif //__CSVFILE_H__
