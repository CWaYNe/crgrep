#ifndef CGREP_BOOK_FORMATTER_H_
#define	CGREP_BOOK_FORMATTER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <set>
#include <time.h>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <stack>
#include <queue>
#include <deque>
#include <tuple>
#include <regex>
#include "load_data.h"


void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

std::tuple<std::string, int> getTagTuple(std::string tagType, long tagLocation) 
{
  return std::make_tuple(tagType, tagLocation); // Always works
}

class BookFormatter{
private:
	std::string pathToFormattedDir;
	std::string pathToDownloadDir;
	std::string pathToMergedFile;
	std::string pathToDownloadedList;
	std::string pathToStopWords;

	std::vector <std::string> stopWords;
	std::set <std::string> alreadyMerged;
	// std::vector <std::string> getToMergeChaptersPath(std::string bookTitle){
	// 	std::string bookPath;
	// 	bookPath = pathToDownloadDir + bookTitle;
	// 	return;
	// };

	std::vector <std::string> getNotMergedBookList(){
		// if(exists(pathToDownloadedList))
		// 	std::cout << "exists" << std::endl;
		// if(exists(pathToDownloadDir))
		// 	std::cout << "exists" << std::endl;
		// if(exists(pathToMergedFile))
		// 	std::cout << "exists" << std::endl;
		// if(exists(pathToFormattedDir))
		// 	std::cout << "exists" << std::endl;

		std::ifstream mergedFile(pathToMergedFile);
		std::string bookTitle;
		while(mergedFile >> bookTitle) {
			alreadyMerged.insert(bookTitle);
		}

		std::vector <std::string> toMergeList;
		std::ifstream downloadedFile(pathToDownloadedList);
		std::string bookUrl;
		while(downloadedFile >> bookTitle >> bookUrl){
			if(alreadyMerged.find(bookTitle) == alreadyMerged.end())
				toMergeList.push_back(bookTitle);
		}
		return toMergeList;
	}

	void mergeWithoutTags(){
		std::vector <std::string> toMergeList;
		toMergeList = getNotMergedBookList();
		std::ofstream mergedFile;
		mergedFile.open (pathToMergedFile, std::ios::in | std::ios::out | std::ios::ate);
		for (int i=0; i<toMergeList.size(); i++){
			mergeChapters(toMergeList[i]);
			mergedFile << toMergeList[i] << std::endl;
		}
		mergedFile.close();
	}

	void mergeChapters(std::string bookTitle) {
		// NOTE: check if slash exist
		std::vector <std::string> pathToRawChapters = getToMergeChaptersPath(bookTitle);
		writeRecordHeaderToFile(bookTitle);
		long char_count = 0; 
		int chapter_num = 0, title_num = 0, paragraph_num = 0, sentense_num =0;
		for (int i=0; i< pathToRawChapters.size(); i++)
			formatThenMerge(pathToRawChapters[i], 
				char_count, chapter_num, title_num, 
				paragraph_num, sentense_num, bookTitle);
			//std::cout << pathToRawChapters[i] << std::endl;
	}

	void writeRecordHeaderToFile(std::string bookTitle){
		std::string formattedDestination = pathToFormattedDir + bookTitle + ".txt";
		std::ofstream formattedFile;
		formattedFile.open(formattedDestination);
		formattedFile << "@id:" << bookTitle << std::endl;
		formattedFile << "@title:" << bookTitle << std::endl;
		formattedFile << "@content:";
		formattedFile.close();
	}

	void formatThenMerge(std::string pathToRawChapter, long &char_count, int &chapter_num, 
				int &title_num, int &paragraph_num, int &sentense_num, std::string bookTitle){
		std::cout << "Current processing chapter: " << pathToRawChapter << std::endl; 
		std::ifstream chapterFile(pathToRawChapter);
		std::string line, text = "";
		while (std::getline(chapterFile, line)){
			for (int i=0; i< stopWords.size();i++)
				ReplaceStringInPlace(line, stopWords[i], "");
			if (line.length()>2)
				text += (line + '\n');
		}

		std::string formattedDestination = pathToFormattedDir + bookTitle + ".txt";
		std::ofstream formattedFile;
		formattedFile.open(formattedDestination, std::ios::in | std::ios::out | std::ios::ate);

		bool titleFlag = true;
		std::deque <std::tuple <std::string, long>> tagQueue;
		chapter_num += 1;
		tagQueue.push_back(getTagTuple("c_"+str(chapter_num), char_count));
		std::stringstream stext(text);
		int lineCOunt = 0;
		while (std::getline(stext, line)){
			if (!titleFlag){
				line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
				paragraph_num += 1;
				tagQueue.push_back(getTagTuple("p_"+str(paragraph_num), char_count));
				lineFormatter(line, sentense_num, char_count, tagQueue);
				tagQueue.push_back(getTagTuple("p_"+str(paragraph_num), char_count));
			}else {
				std::regex e("\\s+");
				std::smatch m;
				std::regex_search(line, m, e);
				//formattedFile << line;
				line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
				title_num += 1;
				tagQueue.push_back(getTagTuple("t_"+str(title_num), char_count));
				char_count += line.length();
				tagQueue.push_back(getTagTuple("t_"+str(title_num), char_count));
				titleFlag = false;
			}
			formattedFile << line;
			line = "";
		}
		tagQueue.push_back(getTagTuple("c_"+str(chapter_num), char_count));
		formattedFile.close();
		writeTagInfoToFile(tagQueue, bookTitle);
		//std::cout << line << std::endl;
		// std::cout << text << std::endl;
	}

	void writeTagInfoToFile(std::deque <std::tuple <std::string, long>> &tagQueue, std::string bookTitle){
		std::string tagInfoPath = pathToFormattedDir + bookTitle + ".tags";
		std::ofstream tagFile;
		std::tuple <std::string, long> tag;
		tagFile.open(tagInfoPath, std::ios::in | std::ios::out | std::ios::ate);

		tag = tagQueue.front();
		tagFile << std::get<0>(tag) << "\t" << std::get<1>(tag) << "\n";
		tagQueue.pop_front();
		tagQueue.pop_back();
		

		tag = tagQueue.front();
		tagFile << std::get<0>(tag) << "\t" << std::get<1>(tag) << "\n";
		tagQueue.pop_front();
		tagQueue.pop_front();

		int queue_count = 0;
		std::deque <std::tuple <std::string, long>> otherTagQueue;
		std::cout << tagQueue.size() << std::endl;
		for (int i=0; i <tagQueue.size(); i++){
			if (otherTagQueue.size()==0){
				otherTagQueue.push_back(tagQueue.at(i));
			} else if( std::get<0>(tagQueue.at(i)) == std::get<0>(otherTagQueue.at(0))){
				tag = otherTagQueue.front();
				otherTagQueue.pop_front();
				tagFile << std::get<0>(tag) << "\t" << std::get<1>(tag) << "\n";
				//std::cout << std::get<0>(tag) <<"\t" << std::get<1>(tag) << std::endl;
				for (int j=0; j <otherTagQueue.size()/2; j++){
					tag = otherTagQueue.front();
					otherTagQueue.pop_front();
					otherTagQueue.pop_front();
					tagFile << std::get<0>(tag) << "\t" << std::get<1>(tag) << "\n";
					//std::cout << std::get<0>(tag) <<"\t" << std::get<1>(tag) << std::endl;
				}
			} else {
				otherTagQueue.push_back(tagQueue.at(i));
			}
		}
		tagFile.close();

	};


	void lineFormatter(std::string &line,int &sentense_num, long &char_count,
	 std::deque <std::tuple <std::string, long>> &tagQueue){
		std::smatch m;
		std::regex e("(。|！|？|!|\\?)+");
		std::string copiedLine = line;
		int currentP = 0;
		while (regex_search(copiedLine, m, e)) {
			tagQueue.push_back(getTagTuple("s_"+str(sentense_num), char_count + m.position() + currentP));
			tagQueue.push_back(getTagTuple("s_"+str(sentense_num), char_count + m.position() + currentP));
			// std::cout << m[0] << std::endl;
			// std::cout << m.position() + currentP << std::endl;
			currentP += (m.position()+m[0].length());
			copiedLine = m.suffix();
		}
		if (currentP != line.length()){
			tagQueue.push_back(getTagTuple("s_"+str(sentense_num), char_count + line.length()));
		}else {
			tagQueue.pop_back();
		}
		char_count = char_count + line.length();
	}
	


	std::vector <std::string> getToMergeChaptersPath(std::string bookTitle){
		std::string pathToRawBookDir = pathToDownloadDir + bookTitle;
		std::vector <std::string> pathToRawChapters;
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(pathToRawBookDir.c_str())) != NULL) {
			while ((ent = readdir(dir)) != NULL){
				std::string chapterPath = pathToRawBookDir+'/'+ent->d_name;
				if(!isDir(chapterPath)){
					pathToRawChapters.push_back(chapterPath);
				}
			}
		}
		return pathToRawChapters;
	}


public:
	BookFormatter(std::string pathSource, std::string pathDest, std::string pathToDownloadedSource,
	 							std::string pathToMergedDest){
		pathToDownloadDir = pathSource; pathToFormattedDir = pathDest; 
		pathToDownloadedList = pathToDownloadedSource; pathToMergedFile = pathToMergedDest;
		pathToStopWords = "none";
		mergeWithoutTags();
	};
	BookFormatter(std::string pathSource, std::string pathDest, std::string pathToDownloadedSource,
	 							std::string pathToMergedDest, std::string pathStopWords){
		pathToDownloadDir = pathSource; pathToFormattedDir = pathDest; 
		pathToDownloadedList = pathToDownloadedSource; pathToMergedFile = pathToMergedDest;
		pathToStopWords = pathStopWords;
		std::ifstream stopWordsFile(pathToStopWords);
		std::string words;
		std::cout << "Your stop words:" << std::endl;
		while (std::getline(stopWordsFile, words)){
			stopWords.push_back(words);
		}
		stopWords.push_back("\n");
		stopWords.push_back(" ");
		mergeWithoutTags();
	};
	~BookFormatter(){};


};



#endif