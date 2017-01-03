/*
 Feather INI Parser - 1.41
 You are free to use this however you wish.
 
 If you find a bug, please attept to debug the cause.
 Post your environment details and the cause or fix in the issues section of GitHub.
 
 Written by Turbine.
 
 Website:
 https://github.com/Turbine1991/feather-ini-parser
 http://code.google.com/p/feather-ini-parser/downloads
 
 Help:
 Bundled example & readme.
 http://code.google.com/p/feather-ini-parser/wiki/Tutorials
 */

#ifndef COMM_INI_H_
#define COMM_INI_H_

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <map>
#include <string>
#include <sstream>

#include "assert/__assert.h"

inline bool VerifyName(const std::string& _name) {
    if (_name.empty())
        return false;

    for (std::string::const_iterator it = _name.begin(); it != _name.end(); ++it) {
        if ('a' <= *it && *it <= 'z')
            continue;
        else if ('A' <= *it && *it <= 'Z')
            continue;
        else if ('0' <= *it && *it <= '9')
            continue;
        else if ('-' == *it)
            continue;
        else if ('_' == *it)
            continue;
        else if ('.' == *it)
            continue;
        else if (' ' == *it)
            continue;
        else
            return false;
    }

    return true;
}

class INI {
  public:
    typedef std::map<std::string, std::string> keys_t;
    typedef std::map<std::string, keys_t> sections_t;

    typedef keys_t::iterator keysit_t;
    typedef sections_t::iterator sectionsit_t;

    typedef std::pair<std::string, std::string> keyspair_t;
    typedef std::pair<std::string, keys_t> sectionspair_t;

  public:
    INI(const std::string& fileName, bool parse = true)
        : filename(fileName) {
        if (parse)
            Parse();
    }

    ~INI() {
        Clear();
    }

    bool Create(const std::string& section) {
        if (!VerifyName(section)) {
            ASSERT2(false, "---%s---", section.c_str());
            return false;
        }

        if (Select(section))
            return false;

        currentsection = section;
        sections[section];

        return true;
    }

    bool Select(const std::string& section) {
        if (!VerifyName(section)) {
            ASSERT2(false, "---%s---", section.c_str());
            return false;
        }

        sectionsit_t sectionsit = sections.find(section);

        if (sectionsit == sections.end())
            return false;

        currentsection = section;

        return true;
    }

    template<class V>
    bool Set(const std::string& key, const V& value)
    { return Set(currentsection, key, value);}

    template<class V>
    bool Set(const std::string& section, const std::string& key, const V& value) {
        if (!VerifyName(section)) {
            ASSERT2(false, "---%s---", section.c_str());
            return false;
        }

        if (!VerifyName(key)) {
            ASSERT2(false, "---%s---", key.c_str());
            return false;
        }

        std::stringstream strstream;
        strstream << value;
        std::string svalue = strstream.str();

        if (BUFFER_SIZE < key.size() + svalue.size() + 1) {
            ASSERT2(false, "%lu", (long unsigned int)(key.size() + svalue.size()));
            return false;
        }

        sections[section][key] = svalue;
        return true;
    }

    const std::string& Get(const std::string& key, const std::string& def)
    { return Get(currentsection, key,  def);}

    const std::string& Get(const std::string& section, const std::string& key, const std::string& def) {
        if (!VerifyName(section)) {
            ASSERT2(false, "---%s---", section.c_str());
            return def;
        }

        if (!VerifyName(key)) {
            ASSERT2(false, "---%s---", key.c_str());
            return def;
        }

        keysit_t keys = sections[section].find(key);

        if (keys == sections[section].end()) return def;

        return keys->second;
    }

    template<typename V>
    V Get(const std::string& key, const V& def);

    template<typename V>
    V Get(const std::string& section, const std::string& key, const V& def);

    bool Parse() {
        std::string cur;
        FILE* file = fopen(filename.c_str(), "rb");

        if (NULL == file)
            return false;

        fseek(file, 0, SEEK_END);
        int length = (int)ftell(file);
        fseek(file, 0, SEEK_SET);
        ASSERT(1 * 1024 * 1024 > length);

        while (true) {
            
            if (feof(file))
                break;
            
            char line[BUFFER_SIZE] = {0};
           
            if (NULL == fgets(line, BUFFER_SIZE, file) || ferror(file))
                break;

            size_t line_length = strnlen(line, BUFFER_SIZE);
            if ((line_length >= 2) && line[0] == '/' && line[1] == '/')
                continue;

            if (line[line_length-1] == '\n') {
                line[line_length-1] = '\0';
                line_length -= 1;
            }
            
            if (line[0] == '[') {
                cur = "";

                char ssection[512] = {0};
#ifdef WIN32
                int ret = _snscanf(line, 512, "[%[.0-9a-zA-Z_ -]]", ssection);
#else
                int ret = sscanf(line, "[%[.0-9a-zA-Z_ -]]", ssection);
#endif

                ssection[sizeof(ssection) - 1] = '\0';

                ASSERT(0 < ret);

                if (0 == ret)
                    continue;

                if (!VerifyName(ssection)) {
                    ASSERT2(false, "---%s---", ssection);
                    continue;
                }

                cur = ssection;
                sections[cur];
            } else {
                ASSERT(0 < cur.length());

                if (0 == cur.length()) {
                    ASSERT(false);
                    continue;
                }

                char* token = strchr(line, '=');

                if (NULL == token) {
                    continue;
                }

                *token = '\0';
                const char* skey = line;
                const char* svalue = token + 1;

                if (!VerifyName(skey)) {
                    ASSERT2(false, "---%s---", skey);
                    continue;
                }

                sections[cur][skey] = svalue;
            }
        }

        fclose(file);

        return true;
    }

    void Clear() {
        sections.clear();
        currentsection = "";
    }
    void ClearExceptSection(const std::string& _sectionName) {
    	INI::sectionsit_t iter = sections.begin();
    	for (; iter!=sections.end();) {
    		if (0==iter->first.compare(_sectionName)) {
    			++iter;
    		} else {
    			sections.erase(iter++);
    		}
    	}
    }
   
    void Save(const std::string& filename = "") {
        
        FILE* file = fopen(((filename == "") ? this->filename : filename).c_str(), "wb");
        
        if (NULL == file) {
            return;
        }

      
        for (INI::sectionsit_t i = sections.begin(); i != sections.end(); ++i) {
            if (i->second.size() == 0)
                continue;

         
            const std::string temp = makeSection(i->first);
            fwrite(temp.c_str(), 1, temp.length(), file);
            

            for (INI::keysit_t j = i->second.begin(); j != i->second.end(); ++j) {
                
                const std::string temp = makeKeyValue(j->first, j->second);
                fwrite(temp.c_str(), 1, temp.length(), file);
            }
        }

        fclose(file);
    }

    sections_t& Sections() {
        currentsection = "";
        return sections;
    }

  private:
  
    std::string makeSection(const std::string& section) {
        std::string line;
        line += '[';
        line += section;
        line += ']';
        line += '\n';

        return line;
    }

    std::string makeKeyValue(const std::string& key, const std::string& value) {
        std::string line;
        line += key;
        line += '=';
        line += value;
        line += '\n';

        return line;
    }

  private:

    static const unsigned int BUFFER_SIZE = 4096;

    std::string currentsection;
    sections_t sections;

    std::string filename;
};

template<typename V>
V INI::Get(const std::string& key, const V& def)
{ return Get(currentsection, key, def);}

template<typename V>
V INI::Get(const std::string& section, const std::string& key, const V& def) {
    if (!VerifyName(section)) {
        ASSERT2(false, "---%s---", section.c_str());
        return def;
    }

    if (!VerifyName(key)) {
        ASSERT2(false, "---%s---", key.c_str());
        return def;
    }

    keysit_t keys = sections[section].find(key);

    if (keys == sections[section].end()) return def;

    if (keys->second.empty()) return def;

    V result;
    std::stringstream strstream(keys->second);
    strstream >> result;

    if (strstream.bad()) {
        ASSERT("strstream.bad()");
        return def;
    } else if (strstream.fail()) {
        ASSERT("strstream.fail()");
        return def;
    } else if (strstream.good() || strstream.eof()) {
        return result;
    } else {
        ASSERT("unkown error");
        return def;
    }
}

template<> inline
std::string INI::Get<std::string>(const std::string& section, const std::string& key, const std::string& def)
{ return Get(section, key, def);}

#endif	// COMM_INI_H_
