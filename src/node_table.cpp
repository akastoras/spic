#include <iostream>

#include "node_table.h"

std::ostream& operator<<(std::ostream &out, spic::NodeTable const& nt) {
	for (auto it = nt.table.begin(); it != nt.table.end(); ++it) {
		out << it->first << " -> " << it->second << std::endl;
	}
	// and so on... 
	return out;
}