bazel run FilePacker $(PWD)/packFilesTest/testData/test.dat $(PWD)/packFilesTest/testData txt
bazel run FilePacker $(PWD)/packFilesTest/testData/test2.dat $(PWD)/packFilesTest/testData txt,ogg
bazel run FilePacker $(PWD)/packFilesTest/testData/test3.dat $(PWD)/packFilesTest/testData txt recursive
