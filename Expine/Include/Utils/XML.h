#pragma once

#include <Types.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace XML
{
	using XMLTree = boost::property_tree::basic_ptree<std::string, std::string>;
	using XMLTreeWide = boost::property_tree::basic_ptree<std::wstring, std::wstring>;
	using XMLParseError = boost::property_tree::xml_parser_error;
	using XMLTreeDataError = boost::property_tree::ptree_bad_data;

	class XMLDocument
	{
	private:

		XMLTree Tree;

	public:

		inline XMLTree & GetRoot()
		{
			return Tree;
		}

		inline void ParseXML
		(
			const WString & Filepath
		)
		{
			boost::property_tree::read_xml(String(Filepath.begin(), Filepath.end()), Tree, 0);
		}

		inline void ParseXML
		(
			const String & Filepath
		)
		{
			boost::property_tree::read_xml(Filepath, Tree, 0);
		}
	};

	class XMLDocumentWide
	{
	private:

		XMLTreeWide Tree;

	public:

		inline XMLTreeWide & GetRoot()
		{
			return Tree;
		}

		inline void ParseXML
		(
			const WString & Filepath
		)
		{
			boost::property_tree::read_xml(String(Filepath.begin(), Filepath.end()), Tree, 0);
		}

		inline void ParseXML
		(
			const String & Filepath
		)
		{
			boost::property_tree::read_xml(Filepath, Tree, 0);
		}
	};
}