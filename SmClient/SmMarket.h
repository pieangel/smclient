#pragma once
#include <string>
#include <vector>
class SmCategory;
class SmMarket
{
public:
	SmMarket();
	~SmMarket();
	std::string Name() const { return _Name; }
	void Name(std::string val) { _Name = val; }
	/// <summary>
	/// 품목을 추가한다.
	/// </summary>
	/// <param name="code"></param>
	/// <returns></returns>
	SmCategory* AddCategory(std::string code);
	SmCategory* FindCategory(std::string code);
	std::vector<SmCategory*>& GetCategoryList() {
		return _CategoryList;
	}
private:
	std::string _Name;
	std::vector<SmCategory*> _CategoryList;
};

