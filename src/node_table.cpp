#include <ostream>
#include <string>

#include "node_table.h"

namespace spic {
	node_id_t NodeTable::find_node(int name) {
		std::string str = std::to_string(name);
		return find_node(&str);
	}

	node_id_t NodeTable::find_node(std::string *name) {
		auto it = table.find(*name);
		if (it != table.end()) {
			return it->second;
		}
		return -1; // Or some other invalid node_id_t value
	}

	/* Methods of NodeTable */
	node_id_t NodeTable::append_node(std::string *name) {
		node_id_t nid = table.size();
		table[*name] = nid;
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
		std::cout << "Node name not found" << std::endl;
		return nullptr;
	}

	int NodeTable::size() {
		return table.size();
	}
}

/* Support of << operator for printing a NodeTable */
std::ostream& operator<<(std::ostream &out, spic::NodeTable const& nt)
{
#if VERBOSE_NODETABLE == 1
	out << "#----------Node Table----------#\n";
	for (auto it = nt.table.begin(); it != nt.table.end(); ++it) {
		out << "'" << it->first << "'" << " -> " << it->second << std::endl;
	}
	out << "#--------End Node Table--------#\n\n";
#endif
	return out;
}

// int main() {
// 	spic::NodeTable *nt = new spic::NodeTable();

// 	std::string *str = new std::string("hello");

// 	nt->append_node(str);

// 	std::cout << *nt;
// }