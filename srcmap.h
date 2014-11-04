#ifdef __cplusplus

#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#include "json.hpp"
#include "base64vlq.hpp"

#ifndef VERSION
#define VERSION "[NA]"
#endif

// using std::string
using namespace std;

// add namespace for c++
namespace SrcMap
{

	class Entry
	{
		public:
			Entry(); // ctor
			Entry(size_t col); // ctor
			Entry(size_t col, size_t src_idx, size_t src_ln, size_t src_col); // ctor
			Entry(size_t col, size_t src_idx, size_t src_ln, size_t src_col, size_t token); // ctor
			const size_t getLength();
			const size_t getCol();
			const size_t getSource();
			const size_t getSrcLine();
			const size_t getSrcCol();
			const size_t getToken();
		private:
			vector<size_t> values;
	};

	class Row
	{
		public:
			Row(); // ctor
			const size_t getLength();
			Entry* getEntry(size_t idx);
			void addEntry(Entry* entry);
		private:
			vector<Entry*> entries;
	};

	class Mapping
	{
		public:
			Mapping(); // ctor
			Mapping(string VLQ); // ctor
			const size_t getLength();
			Row* getRow(size_t idx);
			void addRow(Row* row);
			string serialize();
		private:
			vector<Row*> rows;
			void init(string VLQ);
	};

	class SrcMap
	{
		public:
			SrcMap(); // ctor
			SrcMap(string json_str); // ctor
			SrcMap(JsonNode* json_node); // ctor
			string getFile();
			string getRoot();
			Mapping* getMap();
			string getToken(size_t idx);
			string getSource(size_t idx);
			string getContent(size_t idx);
			// use enc to disable map encoding
			string serialize(bool enc = true);
		private:
			string file;
			string root;
			Mapping* map;
			string version = "3";
			vector<string> tokens;
			vector<string> sources;
			vector<string> contents;
			void init(JsonNode* json_node);
	};

}
// EO namespace

// declare for c
extern "C" {
#endif

	void testme();

#ifdef __cplusplus
}
#endif
