// Copyright 2008 Cyrus Najmabadi
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



/**
 * All generated protocol message classes extend this class.  This class
 * implements most of the Message and Builder interfaces using Java reflection.
 * Users can ignore this class and pretend that generated messages implement
 * the Message interface directly.
 *
 * @author Cyrus Najmabadi
 */


#import <Foundation/Foundation.h>
#import "WireFormat.h"
#import "GeneratedMessage+ObjC.h"

#ifdef DEBUG
#define PB_PROPERTY_USE_DYNAMIC     0
#else
#define PB_PROPERTY_USE_DYNAMIC     1
#endif

#if PB_PROPERTY_USE_DYNAMIC
#define PB_PROPERTY_TYPE    @dynamic
#else
#define PB_PROPERTY_TYPE    @synthesize
#endif

typedef enum {
    FieldLabelOptional = 1,
    FieldLabelRequired = 2,
    FieldLabelRepeated = 3,
} FieldLabel;

typedef BOOL (*IsEnumValidFunc)(uint32_t);

typedef struct {
    Byte 							_fieldNumber;
    Byte 							_fieldLabel;
    Byte 							_fieldType;
    BOOL 							_isPacked;
    int 							_enumInitValue;
	const char*						_fieldName;
    union {
        const char*					_messageClassName;
        __unsafe_unretained Class	_messageClass;
        IsEnumValidFunc				_isEnumValidFunc;
    };
} PBFieldInfo;

#ifdef __cplusplus
extern "C" {
#endif
void initializePBClassInfo(Class cls, const char* className, PBFieldInfo* fieldInfos, unsigned int fieldCount);
#ifdef __cplusplus
}
#endif

