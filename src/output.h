// Copyright © 2026 CCP ehf.

#include "../include/pdm.h"
#include <iostream>

namespace PDM
{
	void Output(const PDMData& data, std::ostream& stream);
	void Output(const SubItem& item, std::ostream& stream, int indentation = 0);
	void Output(const std::vector<DataField>& items, std::ostream& stream, int indentation);
	void Output(const std::vector<SubItem>& items, std::ostream& stream, int indentation);
	void Output(const SubItem& item, std::ostream& stream, int indentation);
}