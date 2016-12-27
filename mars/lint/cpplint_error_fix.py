#!/usr/bin/env python
#check all files cpplint issues in folder and output a xml report

import sys
import codecs
import os
import stat
import platform
import re
import sre_compile

_regexp_compile_cache = {}

def Match(pattern, s):
  """Matches the string with the pattern, caching the compiled regexp."""
  # The regexp compilation caching is inlined in both Match and Search for
  # performance reasons; factoring it out into a separate function turns out
  # to be noticeably expensive.
  if pattern not in _regexp_compile_cache:
    _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
  return _regexp_compile_cache[pattern].match(s)

def ReplaceAll(pattern, rep, s):
  """Replaces instances of pattern in a string with a replacement.

  The compiled regex is kept in a cache shared by Match and Search.

  Args:
    pattern: regex pattern
    rep: replacement text
    s: search string

  Returns:
    string with replacements made (or original string if no replacements)
  """
  if pattern not in _regexp_compile_cache:
    _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
  return _regexp_compile_cache[pattern].sub(rep, s)


def Replace(pattern, rep, s, count):
  """Replaces instances of pattern in a string with a replacement.

  The compiled regex is kept in a cache shared by Match and Search.

  Args:
    pattern: regex pattern
    rep: replacement text
    s: search string

  Returns:
    string with replacements made (or original string if no replacements)
  """
  if pattern not in _regexp_compile_cache:
    _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
  return _regexp_compile_cache[pattern].sub(rep, s, count)
    
    
def Search(pattern, s):
  """Searches the string for the pattern, caching the compiled regexp."""
  if pattern not in _regexp_compile_cache:
    _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
  return _regexp_compile_cache[pattern].search(s)


def FindAll(pattern, s):
  """Searches the string for the pattern, caching the compiled regexp."""
  if pattern not in _regexp_compile_cache:
    _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
  return _regexp_compile_cache[pattern].findall(s)


def ToHex(_string):
    string = ''
    for ch in _string:
        string +=  r'\x%02x'%(ord(ch))
    return string
    
def IsCanAutoFix(_match, _lines, _linenum):
    if _linenum < 0 or _linenum >= len(_lines):
        return False
    line = _lines[_linenum]
    try:
        if -1==line.find(_match): return False
    except UnicodeDecodeError:
        return False
    
    match_re = ToHex(_match)
    
    if _match != '//' \
    and Search(r'//.*%s.*$'%(match_re), line) : 
        return False
    
    for string in FindAll(r'"[^"]*"', line):
        if _match in string :
            return False
        
    return True
    
class __ErrorInfo(object):
  """Maintains module-wide state.."""

  def __init__(self, line):
      self.filename = None
      match = Match(r'^(.+):(\d+):  (.+)  \[(.+)\] \[(\d)\]', line)
      if not match: 
          return
          
      self.filename = match.group(1) 
      self.linenum = int(match.group(2)) -1
      self.message = match.group(3) 
      self.category = match.group(4)
      self.confidence = int(match.group(5))


def ProcessErrorList(_filename):
  error_dic_list= {} 
  lines = []
  # Support the UNIX convention of using "-" for stdin.  Note that
  # we are not opening the file with universal newline support
  # (which codecs doesn't support anyway), so the resulting lines do
  # contain trailing '\r' characters if we are reading a file that
  # has CRLF endings.
  # If after the split a trailing '\r' is present, it is removed
  # below.
  if _filename == '-':
    lines = codecs.StreamReaderWriter(sys.stdin, codecs.getreader('utf8'), codecs.getwriter('utf8'), 'replace').readlines()
  else:
    lines = codecs.open(_filename, 'rb', 'utf8', 'replace').readlines()
    
  for line in lines:
     info = __ErrorInfo(line) 
     if not info.filename:
         continue
     if info.filename not in error_dic_list:
          error_dic_list[info.filename] = []
            
     error_dic_list[info.filename].append(info)
    
  return error_dic_list

def fix_whitespace_operators(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/operators":
             continue
        operators = info.message[-1:] if info.message[-2].isspace() else info.message[-2:]
        match_re = ToHex(operators)
        
        if IsCanAutoFix(operators, _lines, info.linenum) :
            _lines[info.linenum] = ReplaceAll(r'([\w\])}"])%s(.)'%(match_re), r'\1 %s\2'%(match_re), _lines[info.linenum])
            _lines[info.linenum] = ReplaceAll(r'(.)%s([\w\[({"-])'%(match_re), r'\1%s \2'%(match_re), _lines[info.linenum])
        del _error_list[index-1]
        
        
def fix_whitespace_tab(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/tab":
             continue
        _lines[info.linenum] = ReplaceAll(r'\t', r'    ', _lines[info.linenum])
        del _error_list[index-1]
        
def fix_whitespace_comma(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/comma":
             continue
        if IsCanAutoFix(',', _lines, info.linenum) :
            _lines[info.linenum] = ReplaceAll(r'(.*),([\w\[({"-])', r'\1, \2', _lines[info.linenum])
        del _error_list[index-1] 
        
        
def fix_whitespace_newline(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/newline" and info.message != 'An else should appear on the same line as the preceding }':
             continue
        if IsCanAutoFix('else', _lines, info.linenum) \
        and  Match(r'\s*else\b\s*(?:if\b|\{|$)', _lines[info.linenum])\
        and  Match(r'\s*}\s*$', _lines[info.linenum-1])\
        and  not Search(r'//', _lines[info.linenum])\
        and  not Search(r'//', _lines[info.linenum-1]):
            _lines[info.linenum-1] = Replace(r'(\s*})(\s*)\r?\n$', r'\1\2 ', _lines[info.linenum-1], 1)
            _lines[info.linenum] = Replace(r'(?:\s*)(else\b\s*)(if\b|\{|$)', r'\1\2', _lines[info.linenum], 1)
            if Match(r'\s*{', _lines[info.linenum+1])\
            and not Match(r'\s*{\s*', _lines[info.linenum]):
                _lines[info.linenum] = Replace(r'(.*)\s*\r?\n$', r'\1', _lines[info.linenum], 1)
                _lines[info.linenum+1] = Replace(r'\s*{', r'', _lines[info.linenum+1], 1)
        del _error_list[index-1]  
        
def fix_whitespace_parens(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/parens":
             continue
        match = Match(r'Missing space before \( in (.*)(\()', info.message)
        if  not match:
             continue
        if IsCanAutoFix(match.group(1)+match.group(2), _lines, info.linenum):
            _lines[info.linenum] = ReplaceAll(r'%s'%(ToHex(match.group(1)+match.group(2))), match.group(1)+' '+match.group(2), _lines[info.linenum])
        del _error_list[index-1]  
        
def fix_whitespace_blank_line(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/blank_line":
             continue
        if  Match(r'\s*$', _lines[info.linenum]):
            _lines[info.linenum] = ''
        del _error_list[index-1]  
        
def fix_whitespace_comments(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/comments":
             continue
         
        if info.message == "Should have a space between // and comment"\
        and IsCanAutoFix(r'//', _lines, info.linenum)\
        and Search(r'//\S.*$', _lines[info.linenum]):
            _lines[info.linenum] = Replace(r'(///*)(\S.*$)', r'\1 \2', _lines[info.linenum], 1)
            
        if info.message == "At least two spaces is best between code and comments"\
        and IsCanAutoFix(r'//', _lines, info.linenum)\
        and Search(r'\s?//.*$', _lines[info.linenum]):
            _lines[info.linenum] = Replace(r'(\S)(\s)(///*.*$)', r'\1 \2\3', _lines[info.linenum], 1)
            _lines[info.linenum] = Replace(r'([^\s/])(///*.*$)', r'\1  \2', _lines[info.linenum], 1)
            
        del _error_list[index-1]  
        
def fix_readability_namespace(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "readability/namespace":
             continue
         
        message_start = 'Namespace should be terminated with "'
        if info.message.startswith(message_start)\
        and IsCanAutoFix(r'}', _lines, info.linenum):
            match = info.message[len(message_start):-1]
            _lines[info.linenum] = Replace(r'(\r?\n$)', r'  %s\1'%(match), _lines[info.linenum], 1)
            
        del _error_list[index-1]

    
def fix_whitespace_semicolon(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/semicolon":
             continue
         
        if Search(r':\s*;\s*$', _lines[info.linenum]):
            pass
        elif Search(r'^\s*;\s*$', _lines[info.linenum]):
            pass
        elif (Search(r'\s+;\s*$', _lines[info.linenum]) and not Search(r'\bfor\b', _lines[info.linenum])):
            _lines[info.linenum] = Replace(r'\s+;\s*\n$', r';\n', _lines[info.linenum], 1)
            
        del _error_list[index-1] 
        
        
def fix_whitespace_indent(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "whitespace/indent":
             continue
        os.system('cmd.exe /c AStyle.exe --options=astyle.opt -n %s'%(info.filename))
        del _error_list[index-1]  
        break
        
def fix_legal_copyright(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "legal/copyright":
             continue
         
        if -1 == _lines[0].lower().find('copyright'):
            _lines.insert(0, '/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */\n')
        del _error_list[index-1]   
 

def fix_build_storage_class(_lines, _error_list):
    for index in range(len(_error_list), 0, -1):
        info = _error_list[index-1]
        if info.category != "build/storage_class":
             continue
         
        _lines[info.linenum] = \
        ReplaceAll(r'\b(const|volatile|void|char|short|int|long'
                   r'|float|double|signed|unsigned'
                   r'|schar|u?int8|u?int16|u?int32|u?int64)'
                   r'(\s+)(register|static|extern|typedef)\b',
                   r'\3\2\1', _lines[info.linenum])
            
        del _error_list[index-1]  
    
def FixError(_filename, _error_list):
  file = codecs.open(_filename, 'r+')
  lines = file.readlines()
  newlines = lines[:]
  
#   fix_whitespace_tab(newlines, _error_list)
#   fix_whitespace_parens(newlines, _error_list)
#   fix_whitespace_operators(newlines, _error_list)
#   fix_whitespace_comma(newlines, _error_list) 
  fix_whitespace_comments(newlines, _error_list)
  fix_readability_namespace(newlines, _error_list)
  fix_whitespace_semicolon(newlines, _error_list) 
#   fix_whitespace_newline(newlines, _error_list) 
  fix_whitespace_blank_line(newlines, _error_list) 
#   fix_whitespace_indent(newlines, _error_list) 
  fix_build_storage_class(newlines, _error_list)
  fix_legal_copyright(newlines, _error_list) 
  
  if newlines !=  lines:
      file.truncate(0)
      file.seek(0)
      file.writelines(newlines)

  file.close()      
  
  os.system('AStyle.exe --options=astyle.opt -n %s'%(_filename))

def main():
#   error_msg_filename = sys.argv[1]
  error_dic_list = ProcessErrorList('errorlist.txt')
  for (error_filename,error_list) in error_dic_list.items() :
     FixError(error_filename, error_list)
  

if __name__ == '__main__':
  main()
