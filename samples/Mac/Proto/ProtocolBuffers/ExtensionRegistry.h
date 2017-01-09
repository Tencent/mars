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

/**
 * A table of known extensions, searchable by name or field number.  When
 * parsing a protocol message that might have extensions, you must provide
 * an {@code ExtensionRegistry} in which you have registered any extensions
 * that you want to be able to parse.  Otherwise, those extensions will just
 * be treated like unknown fields.
 *
 * <p>For example, if you had the {@code .proto} file:
 *
 * <pre>
 * option java_class = "MyProto";
 *
 * message Foo {
 *   extensions 1000 to max;
 * }
 *
 * extend Foo {
 *   optional int32 bar;
 * }
 * </pre>
 *
 * Then you might write code like:
 *
 * <pre>
 * ExtensionRegistry registry = ExtensionRegistry.newInstance();
 * registry.add(MyProto.bar);
 * MyProto.Foo message = MyProto.Foo.parseFrom(input, registry);
 * </pre>
 *
 * <p>Background:
 *
 * <p>You might wonder why this is necessary.  Two alternatives might come to
 * mind.  First, you might imagine a system where generated extensions are
 * automatically registered when their containing classes are loaded.  This
 * is a popular technique, but is bad design; among other things, it creates a
 * situation where behavior can change depending on what classes happen to be
 * loaded.  It also introduces a security vulnerability, because an
 * unprivileged class could cause its code to be called unexpectedly from a
 * privileged class by registering itself as an extension of the right type.
 *
 * <p>Another option you might consider is lazy parsing: do not parse an
 * extension until it is first requested, at which point the caller must
 * provide a type to use.  This introduces a different set of problems.  First,
 * it would require a mutex lock any time an extension was accessed, which
 * would be slow.  Second, corrupt data would not be detected until first
 * access, at which point it would be much harder to deal with it.  Third, it
 * could violate the expectation that message objects are immutable, since the
 * type provided could be any arbitrary message class.  An unpriviledged user
 * could take advantage of this to inject a mutable object into a message
 * belonging to priviledged code and create mischief.
 *
 * @author Cyrus Najmabadi
 */

#import "ExtensionField.h"

@interface PBExtensionRegistry : NSObject {
@protected
  NSDictionary* classMap;
}

+ (PBExtensionRegistry*) emptyRegistry;
- (id<PBExtensionField>) getExtension:(Class) clazz fieldNumber:(SInt32) fieldNumber;

/* @protected */
- (instancetype) initWithClassMap:(NSDictionary*) classMap;
- (id) keyForClass:(Class) clazz;

@end
