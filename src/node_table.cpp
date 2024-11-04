#include <ostream>
#include <string>

#include "node_table.h"

namespace spic {

	/* Methods of NodeTable */
	node_id_t NodeTable::append_node(std::string *name) {
		node_id_t nid = table.size();
		table[*name] = nid; // ERROR
		delete(name);
		return nid;
	}

	node_id_t NodeTable::append_node(int name) {
		node_id_t nid = table.size();
		table[std::to_string(name)] = nid;
		return nid;
	}

	std::string NodeTable::get_node_name(int node_id) {
	for(auto &it : table) { 
		if(it.second == node_id) { 
			return it.first; 
		}
	}
	return nullptr;
}
}

/* Support of << operator for printing a NodeTable */
std::ostream& operator<<(std::ostream &out, spic::NodeTable const& nt) {
	for (auto it = nt.table.begin(); it != nt.table.end(); ++it) {
		out << it->first << " -> " << it->second << std::endl;
	}
	// and so on... 
	return out;
}

// int main() {
// 	spic::NodeTable *nt = new spic::NodeTable();

// 	std::string *str = new std::string("hello");

// 	nt->append_node(str);

// 	std::cout << *nt;
// }