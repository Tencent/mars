#!/usr/bin/python

import sys
import os
import glob
import zlib
import struct
import binascii
import traceback


MAGIC_NO_COMPRESS_START = 0x03
MAGIC_NO_COMPRESS_START1 = 0x06
MAGIC_NO_COMPRESS_NO_CRYPT_START = 0x08
MAGIC_COMPRESS_START = 0x04
MAGIC_COMPRESS_START1 = 0x05
MAGIC_COMPRESS_START2 = 0x07
MAGIC_COMPRESS_NO_CRYPT_START = 0x09

MAGIC_END = 0x00

lastseq = 0


def IsGoodLogBuffer(_buffer, _offset, count):

    if _offset == len(_buffer): return (True, '')

    magic_start = _buffer[_offset] 
    if MAGIC_NO_COMPRESS_START==magic_start or MAGIC_COMPRESS_START==magic_start or MAGIC_COMPRESS_START1==magic_start:
        crypt_key_len = 4
    elif MAGIC_COMPRESS_START2==magic_start or MAGIC_NO_COMPRESS_START1==magic_start or MAGIC_NO_COMPRESS_NO_CRYPT_START==magic_start or MAGIC_COMPRESS_NO_CRYPT_START==magic_start:
        crypt_key_len = 64
    else:
        return (False, '_buffer[%d]:%d != MAGIC_NUM_START'%(_offset, _buffer[_offset]))

    headerLen = 1 + 2 + 1 + 1 + 4 + crypt_key_len

    if _offset + headerLen + 1 + 1 > len(_buffer): return (False, 'offset:%d > len(buffer):%d'%(_offset, len(_buffer)))
    length = struct.unpack_from("I", buffer(_buffer, _offset+headerLen-4-crypt_key_len, 4))[0]
    if _offset + headerLen + length + 1 > len(_buffer): return (False, 'log length:%d, end pos %d > len(buffer):%d'%(length, _offset + headerLen + length + 1, len(_buffer)))
    if MAGIC_END!=_buffer[_offset + headerLen + length]: return (False, 'log length:%d, buffer[%d]:%d != MAGIC_END'%(length, _offset + headerLen + length, _buffer[_offset + headerLen + length]))


    if (1>=count): return (True, '')
    else: return IsGoodLogBuffer(_buffer, _offset+headerLen+length+1, count-1)
        
    
def GetLogStartPos(_buffer, _count):
    offset = 0
    while True:
        if offset >= len(_buffer): break
        
        if MAGIC_NO_COMPRESS_START==_buffer[offset] or MAGIC_NO_COMPRESS_START1==_buffer[offset] or MAGIC_COMPRESS_START==_buffer[offset] or MAGIC_COMPRESS_START1==_buffer[offset] or MAGIC_COMPRESS_START2==_buffer[offset] or MAGIC_COMPRESS_NO_CRYPT_START==_buffer[offset] or MAGIC_NO_COMPRESS_NO_CRYPT_START==_buffer[offset]:
            if IsGoodLogBuffer(_buffer, offset, _count)[0]: return offset
        offset+=1
        
    return -1    
    
def DecodeBuffer(_buffer, _offset, _outbuffer):
    
    if _offset >= len(_buffer): return -1
    # if _offset + 1 + 4 + 1 + 1 > len(_buffer): return -1
    ret = IsGoodLogBuffer(_buffer, _offset, 1)
    if not ret[0]:
        fixpos = GetLogStartPos(_buffer[_offset:], 1)
        if -1==fixpos: 
            return -1
        else:
            _outbuffer.extend("[F]decode_log_file.py decode error len=%d, result:%s \n"%(fixpos, ret[1]))
            _offset += fixpos 

    magic_start = _buffer[_offset]
    if MAGIC_NO_COMPRESS_START==magic_start or MAGIC_COMPRESS_START==magic_start or MAGIC_COMPRESS_START1==magic_start:
        crypt_key_len = 4
    elif MAGIC_COMPRESS_START2==magic_start or MAGIC_NO_COMPRESS_START1==magic_start or MAGIC_NO_COMPRESS_NO_CRYPT_START==magic_start or MAGIC_COMPRESS_NO_CRYPT_START==magic_start:
        crypt_key_len = 64
    else:
        _outbuffer.extend('in DecodeBuffer _buffer[%d]:%d != MAGIC_NUM_START'%(_offset, magic_start))
        return -1

    headerLen = 1 + 2 + 1 + 1 + 4 + crypt_key_len
    length = struct.unpack_from("I", buffer(_buffer, _offset+headerLen-4-crypt_key_len, 4))[0]
    tmpbuffer = bytearray(length)

    seq=struct.unpack_from("H", buffer(_buffer, _offset+headerLen-4-crypt_key_len-2-2, 2))[0]
    begin_hour=struct.unpack_from("c", buffer(_buffer, _offset+headerLen-4-crypt_key_len-1-1, 1))[0]
    end_hour=struct.unpack_from("c", buffer(_buffer, _offset+headerLen-4-crypt_key_len-1, 1))[0]

    global lastseq
    if seq != 0 and seq != 1 and lastseq != 0 and seq != (lastseq+1):
        _outbuffer.extend("[F]decode_log_file.py log seq:%d-%d is missing\n" %(lastseq+1, seq-1))

    if seq != 0:
        lastseq = seq

    tmpbuffer[:] = _buffer[_offset+headerLen:_offset+headerLen+length]

    try:
        decompressor = zlib.decompressobj(-zlib.MAX_WBITS)

        if MAGIC_NO_COMPRESS_START1==_buffer[_offset] or MAGIC_COMPRESS_START2==_buffer[_offset]:
            print("use wrong decode script")
        elif MAGIC_COMPRESS_START==_buffer[_offset] or MAGIC_COMPRESS_NO_CRYPT_START==_buffer[_offset]:
            tmpbuffer = decompressor.decompress(str(tmpbuffer))
        elif MAGIC_COMPRESS_START1==_buffer[_offset]:
            decompress_data = bytearray()
            while len(tmpbuffer) > 0:
                single_log_len = struct.unpack_from("H", buffer(tmpbuffer, 0, 2))[0]
                decompress_data.extend(tmpbuffer[2:single_log_len+2])
                tmpbuffer[:] = tmpbuffer[single_log_len+2:len(tmpbuffer)]

            tmpbuffer = decompressor.decompress(str(decompress_data))

        else:
            pass

            # _outbuffer.extend('seq:%d, hour:%d-%d len:%d decompress:%d\n' %(seq, ord(begin_hour), ord(end_hour), length, len(tmpbuffer)))
    except Exception, e:
        traceback.print_exc()  
        _outbuffer.extend("[F]decode_log_file.py decompress err, " + str(e) + "\n")
        return _offset+headerLen+length+1

    _outbuffer.extend(tmpbuffer)
    
    return _offset+headerLen+length+1


def ParseFile(_file, _outfile):
    fp = open(_file, "rb")
    _buffer = bytearray(os.path.getsize(_file))
    fp.readinto(_buffer)
    fp.close()
    startpos = GetLogStartPos(_buffer, 2)
    if -1==startpos:
        return
    
    outbuffer = bytearray()
    
    while True:
        startpos = DecodeBuffer(_buffer, startpos, outbuffer)
        if -1==startpos: break;
    
    if 0==len(outbuffer): return
    
    fpout = open(_outfile, "wb")
    fpout.write(outbuffer)
    fpout.close()
    
def main(args):
    global lastseq

    if 1==len(args):
        if os.path.isdir(args[0]):
            filelist = glob.glob(args[0] + "/*.xlog")
            for filepath in filelist:
                lastseq = 0
                ParseFile(filepath, filepath+".log")
        else: ParseFile(args[0], args[0]+".log")    
    elif 2==len(args):
        ParseFile(args[0], args[1])    
    else: 
        filelist = glob.glob("*.xlog")
        for filepath in filelist:
            lastseq = 0
            ParseFile(filepath, filepath+".log")

if __name__ == "__main__":
    main(sys.argv[1:])
