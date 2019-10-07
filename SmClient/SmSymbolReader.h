#pragma once
#include "Global/TemplateSingleton.h"
#include <string>

class SmSymbolReader : public TemplateSingleton<SmSymbolReader>
{
public:
	SmSymbolReader();
	~SmSymbolReader();

public:
	std::string GetWorkingDir();
	void ReadSymbolFromFile(int index, std::string fullPath);
private:
	void ReadMarketFile(std::string fullPath);
	void ReadPmFile(std::string fullPath);
	void ReadJmFile(std::string fullPath);
	void ReadMarketFile();
	void ReadPmFile();
	void ReadJmFile();
	void ReadKospiFutureFile(std::string fullPath);
	void ReadKospiOptionFile(std::string fullPath);
	void ReadKospiWeeklyOptionFile(std::string fullPath);
	void ReadKosdaqFutureFile(std::string fullPath);
	void ReadMiniKospiFutureFile(std::string fullPath);
	void ReadCommodityFutureFile(std::string fullPath);
	void ReadKospiFutureInfo(std::string fullPath);
	void ReadKospiOptionInfo(std::string fullPath);
	void ReadUsDollarFutureInfo(std::string fullPath);
};

