#include "../includes/Storage.h"

bool Storage::haveWord(const string& word) {
    if (words.empty())return false;
    for (const auto& i : words) {
        if (i.word == word) {
            return true;
        }
    }
    return false;
}

int Storage::findWordNum(const string& word) {
    int iter = 0;
    for (const auto& i : words) {
        if (i.word == word) {
            return iter;
        }
        iter++;
    }
    return -1;
}

bool Storage::isUsefulWord(const string& word) {
    if (word.size() <= 1)return false;
    std::vector<std::string> badWords = { "the", "in", "at", "on", "to", "out", "by", "for", "an", "is", "of", "or", "and" };
    for (const auto& c : badWords) {
        if (word == c)return false;
    }
    return true;
}

string Storage::fixWord(string word) {
    char badThings[4] = { ',', '.', '-', ':' };
    for (auto c : badThings) {
        if (word[word.size() - 1] == c)word.pop_back();
    }
    int i = 0;
    for (auto c : word) {
        if (c >= 65 and c <= 90)word[i] += 32;
        i++;
    }
    return word;
}

string Storage::fixLine(string line) {
    string buff = line;
    while (line[buff.size() - 1] == ' ') {
        buff.pop_back();
    }
    return buff;
}

string Storage::getFileName(string path) {
    int lastSlPos = 0;
    for (int i = 0; i < path.size(); i++)
        if (path[i] == '\\')
            lastSlPos = i + 1;
    string name;
    for (; lastSlPos < path.size(); lastSlPos++) {
        name.push_back(path[lastSlPos]);
    }
    return name;
}

void Storage::readFile(const string& path) {//read words and their num directly from file (slow)
    logger->newLog("Start Reading From File...");
    std::string text, word;
    text = fileToString(path);
    while (getWord(text, word)) {
        word = fixWord(word);
        if (isUsefulWord(word)) {
            if (haveWord(word)) {
                words[findWordNum(word)].num++;
            }
            else {
                words.push_back({ word, 1 });
            }
        }
    }
    logger->newLog("Successfully");
}

void Storage::readFromDB(const string& key) {
    logger->newLog("Start Reading From DataBase");
    std::ifstream file(dbPath);
    if (file.is_open())
    {
        std::string line, word;
        while (getline(file, line))
        {
            //word = line;    //для безопастности
            int i = 0;
            while (true)
            {
                word.push_back(line[i]);
                if (line[i + 1] == '~')break;
                i++;
            }
            //cout << "readFromFile - checker: word:" << word << " idName:" << idName <<endl;//chekcer
            if (word == key)
            {
                std::string buffStr;
                int buffInt;
                i += 2;     //idName~word [w]<-
                word = "";  //word = idName<---
                while (true)
                {
                    word.push_back(line[i]);
                    if (line[i + 1] == '~')break;
                    i++;
                }
                buffStr = word;
                word = "";
                i += 2;     //idName~word~num [n]<-
                while (i < line.size())
                {
                    word.push_back(line[i]);
                    i++;
                }
                buffInt = stoi(word);
                word = "";
                words.push_back({ buffStr, buffInt });
            }
            word = ""; line = "";
        }
        logger->newLog("Successfully");
        return;
    }
    logger->newLog("File: " + dbPath + " adbsent...");
}

void Storage::writeToDB(const string& key) {
    logger->newLog("Writing to DataBase...");
    std::ofstream file(dbPath, std::ios_base::app);
    if (!words.empty())
    {
        for (auto & word : words)
        {
            file << key << '~' << word.word << '~' << word.num << std::endl;
        }
    }
    file.close();
    logger->newLog("Successfully");
}

void Storage::clearDB(string key) {//if some file's data was changed, means that the information about this file in the database is out of date and needs to be cleared
    logger->newLog("Clearing DataBase lines with key - \"" + key + "\"...");
    ifstream oldDB(dbPath);
    string line, newDB;
    while (getline(oldDB, line)) {
        bool safeLine = true;
        for (int i = 0; i < key.size(); i++)
            if (key[i] != line[i]) {
                safeLine = false;
                break;
            }
        if (safeLine) {
            newDB += line + '\n';
        }
    }
    ofstream db(dbPath);
    db << newDB;
    db.close();
    logger->newLog("Successfully");
}

Storage::Storage(char* path, const string& filePath, Logger& logger, int docID) {
    this->docID = std::to_string(docID);
    this->logger = &logger;
    dbPath = path;
    while (dbPath[dbPath.size() - 1] != '\\')//clearing executable name of this program. because argv[0] have it and we dont need it...
        dbPath.pop_back();
    db1Path = dbPath;
    dbPath += ".files\\db.txt";
    db1Path = ".files\\" +std::to_string(docID) + ".db";
    ifstream IfileDb(db1Path);
    if (IfileDb.is_open()) {//if file is not new
        if (fileToString(db1Path) == fileToString(filePath)) {//if file is not changed after last run this program
            readFromDB(this->docID);
        }
        else {//if it was changed after last run
            ofstream OfileDB(db1Path);
            OfileDB << fileToString(filePath);
            OfileDB.close();
            clearDB(this->docID);
            readFile(filePath);
            writeToDB(this->docID);
        }
    }
    else {//if file is new
        ofstream OfileDb(db1Path);
        OfileDb << fileToString(filePath);
        OfileDb.close();
        readFile(filePath);
        writeToDB(this->docID);
    }
}

[[maybe_unused]] void Storage::print() {
    std::cout << "Words: \n";
    if (!words.empty()) {
        for (int i = 0; i < words.size(); i++) {
            std::cout << "Word: " << words[i].word << " Num: " << words[i].num << std::endl;
        }
    }
}
