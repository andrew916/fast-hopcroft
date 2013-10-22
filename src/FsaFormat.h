#pragma once

enum class FsaFormat
{
	None,
	ZeroBasedPlainText,
	OneBasedPlainText,
	GraphViz,
	AlmeidaPlainTextReader
};

#include <stdexcept>
#include <iostream>

std::istream& operator>>(std::istream& in, FsaFormat& fmt)
{
	std::string token;
	in >> token;
	if (token == "none") fmt = FsaFormat::None;
	else if (token == "graphviz") fmt = FsaFormat::GraphViz;
	else if (token == "one-based-text") fmt = FsaFormat::OneBasedPlainText;
	else if (token == "zero-based-text") fmt = FsaFormat::ZeroBasedPlainText;
	else if(token == "almeida") fmt = FsaFormat::AlmeidaPlainTextReader;
	else throw std::invalid_argument("unknown format");
	return in;
}

std::ostream& operator<<(std::ostream& on, const FsaFormat& fmt)
{
	std::string token;
	if(fmt == FsaFormat::None) token = "none";
	else if(fmt == FsaFormat::GraphViz) token = "graphviz";
	else if(fmt == FsaFormat::OneBasedPlainText) token = "one-based-text";
	else if(fmt == FsaFormat::ZeroBasedPlainText) token = "zero-based-text";
	else if(fmt == FsaFormat::AlmeidaPlainTextReader) token = "almeida";
	else throw std::invalid_argument("unknown format");    
	return on << token;
}

#include <memory>
#include "FsaPlainTextReader.h"
#include "AlmeidaPlainTextReader.h"

template<typename TFsa>
std::unique_ptr<IFsaReader<TFsa>> new_reader(FsaFormat format)
{
	using namespace std;
	switch (format)
	{
	case FsaFormat::None: throw logic_error("invalid format");		
	case FsaFormat::ZeroBasedPlainText: return unique_ptr<IFsaReader<TFsa>>(new FsaPlainTextReader<TFsa>());		
	case FsaFormat::OneBasedPlainText: return unique_ptr<IFsaReader<TFsa>>(new FsaPlainTextReaderOneBased<TFsa>());		
	case FsaFormat::AlmeidaPlainTextReader: return unique_ptr<IFsaReader<TFsa>>(new AlmeidaPlainTextReader<TFsa>());
	case FsaFormat::GraphViz: 	
	default:
		throw logic_error("unsupported format");
	}
}

#include "FsaGraphVizWriter.h"
#include "FsaPlainTextWriter.h"

template<typename TFsa>
std::unique_ptr<IFsaWriter<TFsa>> new_writer(FsaFormat format)
{
	using namespace std;
	switch (format)
	{
	case FsaFormat::None: throw logic_error("invalid format");
	case FsaFormat::ZeroBasedPlainText: return unique_ptr<IFsaWriter<TFsa>>(new FsaPlainTextWriter<TFsa>());	
	case FsaFormat::GraphViz: return unique_ptr<IFsaWriter<TFsa>>(new FsaGraphVizWriter<TFsa>());
	case FsaFormat::OneBasedPlainText:
	case FsaFormat::AlmeidaPlainTextReader:	
	default:
		throw logic_error("unsupported format");
	}
}