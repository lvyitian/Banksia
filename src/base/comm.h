/*
 This file is part of Banksia.
 
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


#ifndef comm_h
#define comm_h

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <ctime>

#include <assert.h>

#include "../3rdparty/json/json.h"

namespace banksia {
    
    extern const char* BANKSIA_VERSION;
    
#ifdef _WIN32
    const std::string folderSlash = "\\";
#else
    const std::string folderSlash = "/";
#endif


#define i16 int16_t
#define u16 uint16_t
#define i32 int32_t
#define u32 uint32_t
#define u8  uint8_t
#define u64 uint64_t
#define i64 int64_t
    
    class Obj {
    public:
        virtual ~Obj() {}
        virtual const char* className() const { return "Obj"; }
        
        virtual bool isValid() const { return false; }
        virtual std::string toString() const { return ""; }
        virtual void printOut(const char* msg = nullptr) const;
    };
    
    enum class JsonMerge {
        overwrite, fillmissing, none
    };
    class Jsonable : public Obj {
    public:
        virtual ~Jsonable() {}
        virtual bool load(const Json::Value& obj) = 0;
        virtual Json::Value saveToJson() const = 0;
        static void printOut(const Json::Value&, std::string prefix = "");
        static void merge(Json::Value& main, const Json::Value& from, JsonMerge);
    };
    
    class JsonSavable {
    public:
        virtual ~JsonSavable() {}
        virtual bool loadFromJsonFile(const std::string& jsonPath, bool verbose = true);
        virtual bool saveToJsonFile();
        
        virtual std::string getJsonPath() const;
        virtual void setJsonPath(const std::string&);
        
        static bool loadFromJsonFile(const std::string& jsonPath, Json::Value& jsonData, bool verbose = true);
        static bool saveToJsonFile(const std::string& jsonPath, Json::Value& jsonData);
        static bool loadFromJsonString(const std::string& string, Json::Value& jsonData, bool verbose);
    protected:
        virtual bool saveToJsonFile(Json::Value& jsonData);
        virtual bool parseJsonAfterLoading(Json::Value&) { return false; }
        virtual Json::Value createJsonForSaving() { return Json::Value(); }
        
        virtual bool _loadFromJsonFile(const std::string& jsonPath, Json::Value& jsonData, bool verbose);
        std::string jsonPath;
    };
    
    class Tickable {
    public:
        virtual ~Tickable() {}
        virtual void tick() {
            if (tickMutex.try_lock()) {
                tickWork();
                tickMutex.unlock();
            }
        }
        virtual void tickWork() = 0;
        
    private:
        std::mutex tickMutex;
    };
    
    const int B = 0;
    const int W = 1;
    
    enum class Side {
        black = 0, white = 1, none = 2
    };
    
    enum class PieceType {
        empty, king, queen, rook, bishop, knight, pawn
    };
    
    enum class ResultType {
        noresult, win, draw, loss
    };
    
    enum class ReasonType {
        noreason,
        mate,
        stalemate,
        repetition,
        resign,
        fiftymoves,
        insufficientmaterial,
        illegalmove,
        timeout,
        adjudication,
        crash
    };
    
    enum class MoveNotation {
        san, coordinate
    };
    
//    enum Squares {
//        A8, B8, C8, D8, E8, F8, G8, H8,
//        A7, B7, C7, D7, E7, F7, G7, H7,
//        A6, B6, C6, D6, E6, F6, G6, H6,
//        A5, B5, C5, D5, E5, F5, G5, H5,
//        A4, B4, C4, D4, E4, F4, G4, H4,
//        A3, B3, C3, D3, E3, F3, G3, H3,
//        A2, B2, C2, D2, E2, F2, G2, H2,
//        A1, B1, C1, D1, E1, F1, G1, H1,
//        NoSquare
//    };

    // Some useful / library functions
    extern bool banksiaVerbose;
    extern bool profileMode;
    extern const char* pieceTypeName;
    extern const char* reasonStrings[12];
    
    std::string getVersion();
    std::string getAppName();
    
    void printText(const std::string& str);
    std::tm localtime_xp(std::time_t timer);
    std::string formatPeriod(int seconds);
    
    std::vector<std::string> readTextFileToArray(const std::string& path);
    
    std::string posToCoordinateString(int pos);
    int coordinateStringToPos(const char* str);
    
    void toLower(std::string& str);
    void toLower(char* str);
    std::string& trim(std::string& s);
    std::string replaceString(std::string subject, const std::string& search, const std::string& replace);
    
    std::string getFileName(const std::string& path);
    std::string getFolder(const std::string& path);
    std::string currentWorkingFolder();
    std::string getFullPath(const char* path);
    std::vector<std::string> listdir(std::string dirname);
    i64 getFileSize(const std::string& path);
    bool isExecutable(const std::string& path);
    bool isRunning(int pid);
    int getNumberOfCores();
    size_t getMemorySize();
    
    std::vector<std::string> splitString(const std::string& string, const std::string& regexString);
    std::vector<std::string> splitString(const std::string &s, char delim);
    
    std::string resultType2String(ResultType type);
    ResultType string2ResultType(const std::string& s);
    std::string reasonType2String(ReasonType type);
    ReasonType string2ReasonType(const std::string& s);
    
    std::string side2String(Side side, bool shortFrom = true);
    Side string2Side(std::string s);
    
} // namespace banksia

#endif /* Board_hpp */





