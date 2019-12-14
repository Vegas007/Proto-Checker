#include "pch.h"
#include "CsvReader.h"
#include <fstream>
#include <algorithm>

#ifndef Assert
    #include <assert.h>
    #define Assert assert
    #define LogToFile (void)(0);
#endif

namespace
{
    enum ParseState
    {
        STATE_NORMAL = 0,
        STATE_QUOTE
    };

    std::string Trim(std::string str)
    {
        str = str.erase(str.find_last_not_of(" \t\r\n") + 1);
        str = str.erase(0, str.find_first_not_of(" \t\r\n"));
        return str;
    }

    std::string Lower(std::string original)
    {
        std::transform(original.begin(), original.end(), original.begin(), tolower);
        return original;
    }
}

auto cCsvAlias::AddAlias(const char* name, size_t index) -> void
{
	const auto & converted(Lower(name));

    Assert(m_Name2Index.find(converted) == m_Name2Index.end());
    Assert(m_Index2Name.find(index) == m_Index2Name.end());

    m_Name2Index.insert(NAME2INDEX_MAP::value_type(converted, index));
    m_Index2Name.insert(INDEX2NAME_MAP::value_type(index, name));
}

auto cCsvAlias::Destroy() -> void
{
    m_Name2Index.clear();
    m_Index2Name.clear();
}

auto cCsvAlias::operator [](size_t index) const -> const char*
{
	const auto & it = m_Index2Name.find(index);
    if (it == m_Index2Name.end())
    {
        Assert(false && "cannot find suitable conversion");
        return nullptr;
    }

    return it->second.c_str();
}

auto cCsvAlias::operator [](const char* name) const -> size_t
{
	const auto & it = m_Name2Index.find(Lower(name));
    if (it == m_Name2Index.end())
    {
        Assert(false && "cannot find suitable conversion");
        return 0;
    }

    return it->second;
}

auto cCsvFile::Load(const char* fileName, const char seperator, const char quote) -> bool
{
    Assert(seperator != quote);

    std::ifstream file(fileName, std::ios::in);
    if (!file) 
		return false;

    Destroy(); 

    cCsvRow * row = nullptr;
    ParseState state = STATE_NORMAL;
    std::string token;
	
    char buf[2048+1] = {0,};

    while (file.good())
    {
        file.getline(buf, 2048);
        buf[sizeof(buf)-1] = 0;

        std::string line(Trim(buf));
        if (line.empty() || (state == STATE_NORMAL && line[0] == '#'))
			continue;

		const auto & text  = std::string(line) + "  ";
        size_t cur = 0;

        while (cur < text.size())
        {
            if (state == STATE_QUOTE)
            {
                if (text[cur] == quote)
                {
                    if (text[cur+1] == quote)
                    {
                        token += quote;
                        ++cur;
                    }
                    else
                    {
                        state = STATE_NORMAL;
                    }
                }
                else
                {
                    token += text[cur];
                }
            }
            else if (state == STATE_NORMAL)
            {
                if (row == nullptr)
                    row = new cCsvRow();
                if (text[cur] == seperator)
                {
                    row->push_back(token);
                    token.clear();
                }
                else if (text[cur] == quote)
                {
                    state = STATE_QUOTE;
                }
                else
                {
                    token += text[cur];
                }
            }

            ++cur;
        }

        if (state == STATE_NORMAL)
        {
            Assert(row != NULL);
            row->push_back(token.substr(0, token.size()-2));
            m_Rows.push_back(row);
            token.clear();
            row = nullptr;
        }
        else
        {
            token = token.substr(0, token.size()-2) + "\r\n";
        }
    }

	file.close();
    return true;
}

auto cCsvFile::Save(const char* fileName, bool append, char seperator, char quote) const -> bool
{
    Assert(seperator != quote);
    std::ofstream file;
	file.open(fileName, std::ios::out | (append ? std::ios::app : std::ios::trunc));

    if (!file) 
		return false;

    char special_chars[5] = { seperator, quote, '\r', '\n', 0 };
    char quote_escape_string[3] = { quote, quote, 0 };

    for (size_t i = 0; i < m_Rows.size(); i++)
    {
        const auto & row = *((*this)[i]);

        std::string line;
        for (size_t j = 0; j < row.size(); j++)
        {
            const auto & token = row[j];
            if (token.find_first_of(special_chars) == std::string::npos)
            {
                line += token;
            }
            else
            {
                line += quote;
                for (auto k : token)
                {
                    if (k == quote) 
						line += quote_escape_string;
                    else 
						line += k;
                }

                line += quote;
            }

            if (j != row.size() - 1)
				line += seperator;
        }

        file << line << std::endl;
    }

    return true;
}

auto cCsvFile::Destroy() -> void
{
    for (auto itr(m_Rows.begin()); itr != m_Rows.end(); ++itr)
        delete *itr;

    m_Rows.clear();
}

auto cCsvFile::operator [](size_t index) -> cCsvRow *
{
    Assert(index < m_Rows.size());
    return m_Rows[index];
}

auto cCsvFile::operator [](size_t index) const -> const cCsvRow *
{
    Assert(index < m_Rows.size());
    return m_Rows[index];
}

cCsvTable::cCsvTable()
: m_CurRow(-1)
{
}

cCsvTable::~cCsvTable()
{
}

auto cCsvTable::Load(const char* fileName, const char seperator, const char quote) -> bool
{
    Destroy();
    return m_File.Load(fileName, seperator, quote);
}

auto cCsvTable::Next() -> bool
{
    return ++m_CurRow < static_cast<int>(m_File.GetRowCount());
}

auto cCsvTable::ColCount() const -> size_t
{
    return CurRow()->size();
}

auto cCsvTable::AsInt(size_t index) const -> int
{
    const cCsvRow* const row = CurRow();
    Assert(row);
    Assert(index < row->size());
    return row->AsInt(index);
}

auto cCsvTable::AsDouble(size_t index) const -> double
{
    const auto row = CurRow();
    Assert(row);
    Assert(index < row->size());
    return row->AsDouble(index);
}

auto cCsvTable::AsStringByIndex(size_t index) const -> std::string
{
    const auto row = CurRow();
    Assert(row);
    Assert(index < row->size());
    return row->AsString(index);
}

auto cCsvTable::Destroy() -> void
{
    m_File.Destroy();
    m_Alias.Destroy();
    m_CurRow = -1;
}

auto cCsvTable::CurRow() const -> const cCsvRow*
{
    if (m_CurRow < 0)
    {
        Assert(false && "call Next() first!");
        return nullptr;
    }
    else if (m_CurRow >= static_cast<int>(m_File.GetRowCount()))
    {
        Assert(false && "no more rows!");
        return nullptr;
    }

    return m_File[m_CurRow];
}
