/*
 This file is part of Banksia, distributed under MIT license.
 
 Copyright (c) 2019 Nguyen Hong Pham
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */


#include <cstdarg>
#include <regex>
#include <fstream>
#include <iomanip> // for setfill, setw

#include "comm.h"

namespace banksia {
    bool banksiaVerbose = false;
    
    extern const char* pieceTypeName;
    extern const char* reasonStrings[11];
    
    const char* pieceTypeName = ".kqrbnp";
    const char* reasonStrings[] = {
        "*", "mate", "stalemate", "repetition", "resign", "fifty moves", "insufficient material", "illegal move", "timeout", "crash", nullptr
    };
    
    // noresult, win, draw, loss
    const char* resultStrings[] = {
        "*", "1-0", "1/2-1/2", "0-1", nullptr
    };

    static std::mutex consoleMutex;
    
    std::string resultType2String(ResultType type) {
        auto t = static_cast<int>(type);
        if (t < 0 || t > 3) t = 0;
        return resultStrings[t];
    }
    
    ResultType string2ResultType(const std::string& s) {
        for(int i = 0; resultStrings[i]; i++) {
            if (resultStrings[i] == s) {
                return static_cast<ResultType>(i);
            }
        }
        return ResultType::noresult;
    }
    
    std::string reasonType2String(ReasonType type) {
        auto t = static_cast<int>(type);
        if (t < 0) t = 0;
        return reasonStrings[t];
    }
    
    ReasonType string2ReasonType(const std::string& s) {
        for(int i = 0; reasonStrings[i]; i++) {
            if (reasonStrings[i] == s) {
                return static_cast<ReasonType>(i);
            }
        }
        return ReasonType::noreason;
    }
    
    void printText(const std::string& str)
    {
        std::lock_guard<std::mutex> dolock(consoleMutex);
        std::cout << str << std::endl;
    }
    
    void toLower(std::string& str) {
        for (int i = 0; i < str.size(); ++i) {
            str[i] = tolower(str[i]);
        }
    }
    
    void toLower(char* str) {
        for (int i = 0; str[i]; ++i) {
            str[i] = tolower(str[i]);
        }
    }
    
    std::string posToCoordinateString(int pos) {
        int row = pos / 8, col = pos % 8;
        std::ostringstream stringStream;
        stringStream << char('a' + col) << 8 - row;
        return stringStream.str();
    }
    
    int coordinateStringToPos(const char* str) {
        auto colChr = str[0], rowChr = str[1];
        if (colChr >= 'a' && colChr <= 'h' && rowChr >= '1' && rowChr <= '8') {
            int col = colChr - 'a';
            int row = rowChr - '1';
            
            return (7 - row) * 8 + col;
        }
        return -1;
    }
    
    std::string getFileName(const std::string& path) {
        auto pos = path.find_last_of("/\\");
        std::string str = pos != std::string::npos ? path.substr(pos + 1) : path;
        pos = str.find_last_of(".");
        if (pos != std::string::npos) {
            str = str.substr(0, pos);
        }
        return str;
    }

    std::string getFolder(const std::string& path) {
        auto pos = path.find_last_of("/\\");
        std::string str = pos != std::string::npos ? path.substr(0, pos) : "";
        return str;
    }

    std::string getVersion() {
        char buf[10];
        snprintf(buf, sizeof(buf), "%d.%d", BANKSIA_VERSION >> 8, BANKSIA_VERSION & 0xff);
        return buf;
    }
    
    std::string getAppName() {
        return "banksia";
    }
    
    static const char* trimChars = " \t\n\r\f\v";
    
    // trim from left
    std::string& ltrim(std::string& s)
    {
        s.erase(0, s.find_first_not_of(trimChars));
        return s;
    }
    
    // trim from right
    std::string& rtrim(std::string& s)
    {
        s.erase(s.find_last_not_of(trimChars) + 1);
        return s;
    }
    
    // trim from left & right
    std::string& trim(std::string& s)
    {
        return ltrim(rtrim(s));
    }
    
    std::vector<std::string> splitString(const std::string& string, const std::string& regexString)
    {
        std::regex re(regexString);
        std::sregex_token_iterator first{ string.begin(), string.end(), re }, last;
        return { first, last };
    }
    
    std::vector<std::string> splitString(const std::string &s, char delim)
    {
        std::vector<std::string> elems;
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            auto s = trim(item);
            if (!s.empty())
                elems.push_back(s);
        }
        return elems;
    }
    
    std::vector<std::string> readTextFileToArray(const std::string& path)
    {
        std::ifstream inFile;
        inFile.open(path);
        
        std::string line;
        std::vector<std::string> vec;
        while (getline(inFile, line)) {
            vec.push_back(line);
        }
        return vec;
    }
    
    std::string formatPeriod(int seconds)
    {
        int s = seconds % 60, minutes = seconds / 60, m = minutes % 60, hours = minutes / 60, h = hours % 24, d = hours / 24;
        
        std::ostringstream stringStream;
        if (d > 0) {
            stringStream << d << "d ";
        }
        if (h > 0) {
            stringStream << h << ":"
            << std::setfill('0') << std::setw(2);
        }
        
        stringStream
        << m << ":"
        << std::setfill('0') << std::setw(2)
        << s
        << std::setfill(' ') << std::setw(0);
        return stringStream.str();
    }
    
    // https://stackoverflow.com/questions/38034033/c-localtime-this-function-or-variable-may-be-unsafe
    std::tm localtime_xp(std::time_t timer)
    {
        std::tm bt{};
#if defined(__unix__)
        localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
#endif
        return bt;
    }
}


////////////////////////
using namespace banksia;

void Obj::printOut(const char* msg) const
{
    if (msg) {
        std::cout << msg << std::endl;
    }
    std::cout << toString() << std::endl;
}

bool JsonSavable::loadFromJsonFile(const std::string& jsonPath, bool verbose)
{
    try {
        Json::Value obj;
        return JsonSavable::loadFromJsonFile(jsonPath, obj, verbose) && parseJsonAfterLoading(obj);
    } catch (Json::Exception const& e) {
        if (verbose)
            std::cerr << "Error: Exception when parsing json file - " << e.what() << std::endl;
    }
    return false;
}

bool JsonSavable::_loadFromJsonFile(const std::string& path, Json::Value& jsonData, bool verbose)
{
    jsonPath = path;
    return loadFromJsonFile(path, jsonData, verbose);
}

bool JsonSavable::loadFromJsonFile(const std::string& path, Json::Value& jsonData, bool verbose)
{
    std::ifstream ifs(path);
    
    Json::CharReaderBuilder jsonReader;
    std::string errorString;
    if (Json::parseFromStream(jsonReader, ifs, &jsonData, &errorString)) {
        return true;
    }
    if (verbose) {
        std::cerr << "Error: cannot load (or broken) json file " << path << ", error: " << errorString << std::endl;
    }
    return false;
}

bool JsonSavable::saveToJsonFile()
{
    Json::Value jsonData = createJsonForSaving();
    return saveToJsonFile(jsonPath, jsonData);
}

bool JsonSavable::saveToJsonFile(Json::Value& jsonData)
{
    return saveToJsonFile(jsonPath, jsonData);
}

bool JsonSavable::saveToJsonFile(const std::string& path, Json::Value& jsonData)
{
    auto r = false;
    std::ofstream outFile;
    outFile.open(path, std::ofstream::trunc);
    if (outFile.is_open())
    {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = " ";
        builder["precision"] = 1;
//        builder["commentStyle"] = "None";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(jsonData, &outFile);
        r = true;
    }
    outFile.close();
    return r;
}

std::string JsonSavable::getJsonPath() const
{
    return jsonPath;
}

void JsonSavable::setJsonPath(const std::string& _jsonPath)
{
    jsonPath = _jsonPath;
}


