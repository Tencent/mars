#check all files cpplint issues in folder and output a xml report

import sys
import cpplint

import xml
from xml.dom import minidom
import codecs
import os
import stat
import platform

import cpplint_custom_rules

_proj_code_root = ''

class cpplint_ex:
    def __init__(self, report_file):
        self.report_file = report_file
        self.allfiles = []        

    def __walk(self, start_path, path):
        relative_path = path
        if cpplint_custom_rules.is_skip_path(relative_path) :
            print 'skip walk path : %s' %(relative_path)
            return

        target_subfixs = ['cc', 'cpp', 'cxx', 'h', 'hh', 'hpp', 'hxx']
        skip_items = ['.svn']        
        for item in os.listdir(path):
            if (item in skip_items):
                continue
            subpath = os.path.join(path, item)
            relative_file_path = subpath
            if (cpplint_custom_rules.is_skip_file(relative_file_path)):
              continue
            mode = os.stat(subpath)[stat.ST_MODE]
            if (stat.S_ISDIR(mode)):
                self.__walk(start_path,subpath)
            elif subpath.rpartition('.')[2] in target_subfixs:
                print 'add file to list : ' + relative_file_path
                self.allfiles.append(subpath)

    def check(self, max_err_count=None):
        self.allfiles = []        
        for path in cpplint_custom_rules.get_scan_paths() :
            self.__walk(path, path)
            
        if not self.allfiles:
          print "Error: no file is checked"
          return -1

        err_count_total = 0
        dict_file_errcount = {}
        dict_err_category = {}
        for filename in self.allfiles:
          try:
            cpplint._root = _proj_code_root
            cpplint._cpplint_state.SetCountingStyle('detailed')
            cpplint._cpplint_state.SetFilters(cpplint_custom_rules.get_filters())
#             cpplint._cpplint_state.SetOutputFormat('eclipse')
            cpplint._cpplint_state.ResetErrorCounts()
            
            print 'begin analyze file :%s' %(filename)
            cpplint.ProcessFile(filename, cpplint._cpplint_state.verbose_level)
            if (cpplint._cpplint_state.error_count > 0):
              dict_file_errcount[filename] = cpplint._cpplint_state.error_count
              #record category begin
              for cat, count in cpplint._cpplint_state.errors_by_category.iteritems():
                if cat not in dict_err_category:
                  dict_err_category[cat] = 0
                dict_err_category[cat] += count
              #record category end"""
              print 'error found in %s' %(filename)
            err_count_total += cpplint._cpplint_state.error_count
          except Exception, e:
            print "Error: except when check file: %s, err:%s" %(filename, str(e))
          if (max_err_count <> None) and err_count_total > max_err_count:
            break
        
        #create xml report
        try:
            items = dict_file_errcount.items()
            backitems=[[v[1],v[0]] for v in items]
            backitems.sort()
            item_len = len(backitems)
            root = None
            dom = None
            try:
              dom = minidom.parse(self.report_file)
              root = dom.documentElement
            except:
              print 'out_file_name xml can not parse.'
              root = None
              dom = None

            if (None == root):
              impl = minidom.getDOMImplementation()
              dom = impl.createDocument(None,  'Root' , None)
              root = dom.documentElement

            try:
              node_rules = root.getElementsByTagName('CodeStyle')
              for node in node_rules:
                root.removeChild(node)
            except:
              print 'removeChild fail.'

            node_rule = dom.createElement('CodeStyle')
            node_rule.setAttribute('ErrFile', str(item_len))
            node_rule.setAttribute('Err', str(err_count_total))
            root.appendChild(node_rule)
            for i in range(0, item_len):
              node_line = dom.createElement('File')
              file_path = backitems[item_len-1-i][1]
              node_line.setAttribute('Path', file_path)
              node_line.setAttribute('Err', str(backitems[item_len-1-i][0]))
              
              if cpplint_custom_rules.is_old_file(file_path)==False:
                    node_line.setAttribute('NewFile', '1')                    
              else :
                    node_line.setAttribute('NewFile', '0')
              node_rule.appendChild(node_line)
            #write catege begin
            items = dict_err_category.items()
            backitems=[[v[1],v[0]] for v in items]
            backitems.sort()
            item_len = len(backitems)
            for i in range(0, item_len):
              node_cat = dom.createElement('Category')
              node_cat.setAttribute('Name', backitems[item_len-1-i][1])
              node_cat.setAttribute('Err', str(backitems[item_len-1-i][0]))
              node_rule.appendChild(node_cat)
            #write catege end
            f = codecs.open(self.report_file, "w", "utf-8")
            dom.writexml(f, addindent='    ', newl='\r', encoding='utf-8')
            f.close()
        except Exception, e:
            print "Error: write CodeRule result err(%s)" %(str(e))
            return -2
        
        return 0


def main(report_file):
    """Analyze all files cpplint issues in folder and output a xml report.
    
    There must be a BLADE_ROOT file in the source root path
    
    Args:
        report_file: the report file path
    
    Reurns:
        0: successed
        otherwise: failed
        
    Example:
        cpplint_ex.py "d:\\dev\\client" "d:\\client_lint_report.xml"
    """
    cpplinter = cpplint_ex(report_file)
    return cpplinter.check()

main(sys.argv[1])
