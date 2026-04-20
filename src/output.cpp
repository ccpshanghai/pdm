// Copyright © 2026 CCP ehf.

#include "output.h"
#include "utilities.h"

#include <algorithm>
#include <sstream>

namespace PDM
{
	void Output(const std::vector<DataField>& items, std::ostream& stream, int indentation)
	{
		if (items.empty()) return;

		auto maxlen = std::max_element(begin(items), end(items), [](const DataField& item1, const DataField& item2)
		{
			return item1.name.length() < item2.name.length();
		})->name.length();

		std::for_each(cbegin(items), cend(items), [&stream, indentation, maxlen](const DataField& item)
		{
			for (auto i = indentation; i--;) stream << '\t';
			stream << item.name;

			for (auto i = item.name.length(); i < maxlen; i++) stream << ' ';
			auto& val = item.value.empty() ? "{EMPTY}" : item.value;
			stream << ": " << val << '\n';
		});

		stream << '\n';
	}

	void Output(const std::vector<SubItem>& items, std::ostream& stream, int indentation)
	{
		std::for_each(cbegin(items), cend(items), [&stream, indentation](const SubItem& item)
		{
			Output(item, stream, indentation);
		});
	}

	void Output(const SubItem& item, std::ostream& stream, int indentation)
	{
		if (item.items.empty() && item.subitems.empty()) return;

		for (auto i = indentation; i--;) stream << '\t';
		stream << "{" << item.name.c_str() << "}\n";

		Output(item.items, stream, indentation + 1);
		Output(item.subitems, stream, indentation + 1);
	}

	void Output(const PDMData& data, std::ostream& stream)
	{
		Output(data.data, stream);
		stream.flush();
	}

	std::string PDMDataToFormattedString(const PDMData& pdmData)
	{
		std::stringstream str;
		Output(pdmData, str);
		return str.str();
	}
}