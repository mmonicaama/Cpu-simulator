#pragma once
#include <vector>
#include <map>
#include <string>


class Cpu
{
public:
	Cpu();

public:
	void load(const std::string& file);
	void execute(const std::string& file);
	void dump_memory() const;
	void clear();

private:
	int findLabelAddress(const std::string& label);

private:
	const std::size_t memorySize = 32;
	std::map<std::string, int> registers;
	std::vector<std::string> memory;
	std::size_t instSize;
	std::map<int, std::string> labels;
	bool smthWentWrong;
};