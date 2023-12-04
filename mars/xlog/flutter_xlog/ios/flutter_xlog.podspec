#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint flutter_xlog.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'flutter_xlog'
  s.version          = '0.0.1'
  s.summary          = 'A plugin for use Mars XLog in flutter project..'
  s.description      = <<-DESC
A plugin for use Mars XLog in flutter project..
                       DESC
  s.homepage         = 'https://github.com/jqctop1/mars'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'jqctop1@qq.com' }
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*'
  s.vendored_frameworks = "Frameworks/*.framework"
  s.dependency 'Flutter'
  s.platform = :ios, '11.0'
  s.static_framework = false

  # Flutter.framework does not contain a i386 slice.
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386' }
  s.swift_version = '5.0'
end
