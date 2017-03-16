#ifndef CRGREP_RECORD_H_
#define CRGREP_RECORD_H_
#include "ranking.h"

struct record {
	char *language;
	char *id;
	char *title;
	char *content;
};

class Record {
private:
	struct record* data;
	std::vector<Ranking> rank;
	int fileCount;
	std::string inputPath;
	std::vector<std::string> rawfiles;
	std::vector<std::string> tagFiles;

	void searchId(char *id, char *recordLanguage, std::string pattern, int &searchScore, int &searchMatchCount,bool caseInsensitive = 0, unsigned int editDistance = 0);
	void searchTitle(char *title, char *recordLanguage, std::string pattern, int &searchScore, int &searchMatchCount, bool caseInsensitive = 0, unsigned int editDistance = 0);
	void searchContent(char *content, char *recordLanguage, std::vector <std::string> &searchPatterns, int recordIndex, int &searchScore, int &searchMatchCount, bool caseInsensitive = 0, unsigned int editDistance = 0);
	char * searchFactory(char *text, char *recordLanguage, std::string pattern, bool caseInsensitive = 0, unsigned int editDistance = 0);
	void buildRecord();
	void readFileThenSetRecordAndRank();
	void checkPathAndSetFileVectors();
	void detectLanguage(const char* src, char *&recordLanguage);

public:
	Record(std::string path);
	~Record();
	
	int getFileCount();
	std::vector <std::tuple <std::string, int, int>> searchAndSortWithRank(std:: string pattern, bool caseInsensitive = 0, unsigned int editDistance = 0);

};

#endif