#pragma once

#include <iostream>
#include <unordered_map>


namespace spic {
	using node_id_t = int;

	class NodeTable {
		public:
		std::unordered_map<std::string, node_id_t> table;

		// Methods
		NodeTable() {
			// table = std::unordered_map<std::string&, node_id_t>();
			table["0"] = 0;
		}
		node_id_t find_node(int name);
		node_id_t find_node(std::string *name);
		node_id_t append_node(std::string *name);
		node_id_t append_node(int name);
		std::string get_node_name(int node_id);
	};
}

std::ostream& operator<<(std::ostream &out, spic::NodeTable const& nt);

