// Protocol Buffers for Objective C
//
// Copyright 2010 Booyah Inc.
// Copyright 2008 Cyrus Najmabadi
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#import "Message.h"

/**
 * A partial implementation of the {@link Message} interface which implements
 * as many methods of that interface as possible in terms of other methods.
 *
 * @author Cyrus Najmabadi
 */
@interface PBAbstractMessage : NSObject<PBMessage> {
@private
}

/**
 * Writes a string description of the message into the given mutable string
 * respecting a given indent.
 */
- (void)writeDescriptionTo:(NSMutableString*) output
                withIndent:(NSString*) indent;

- (void) storeInDictionary: (NSMutableDictionary *) dic;

@end
