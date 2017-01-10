// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "GPBBootstrap.h"

#import "GPBArray.h"
#import "GPBCodedInputStream.h"
#import "GPBCodedOutputStream.h"
#import "GPBDescriptor.h"
#import "GPBDictionary.h"
#import "GPBExtensionRegistry.h"
#import "GPBMessage.h"
#import "GPBRootObject.h"
#import "GPBUnknownField.h"
#import "GPBUnknownFieldSet.h"
#import "GPBUtilities.h"
#import "GPBWellKnownTypes.h"
#import "GPBWireFormat.h"

// This CPP symbol can be defined to use imports that match up to the framework
// imports needed when using CocoaPods.
#if !defined(GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS)
 #define GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS 0
#endif

// Well-known proto types
#if GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS
 #import <Protobuf/Any.pbobjc.h>
 #import <Protobuf/Api.pbobjc.h>
 #import <Protobuf/Duration.pbobjc.h>
 #import <Protobuf/Empty.pbobjc.h>
 #import <Protobuf/FieldMask.pbobjc.h>
 #import <Protobuf/SourceContext.pbobjc.h>
 #import <Protobuf/Struct.pbobjc.h>
 #import <Protobuf/Timestamp.pbobjc.h>
 #import <Protobuf/Type.pbobjc.h>
 #import <Protobuf/Wrappers.pbobjc.h>
#else
 #import "Any.pbobjc.h"
 #import "Api.pbobjc.h"
 #import "Duration.pbobjc.h"
 #import "Empty.pbobjc.h"
 #import "FieldMask.pbobjc.h"
 #import "SourceContext.pbobjc.h"
 #import "Struct.pbobjc.h"
 #import "Timestamp.pbobjc.h"
 #import "Type.pbobjc.h"
 #import "Wrappers.pbobjc.h"
#endif
