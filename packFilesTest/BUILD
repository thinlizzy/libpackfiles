cc_library(
	name = "test_runner",
	srcs = [
		"packFilesTest.cpp",
	],
	deps = [
		"@tut//:tut",
	],
)

cc_test(
	name = "packed_files_mixed",
	srcs = [
		"TestPackedFilesMixed.cpp",
		"test_data_deps.inc",
	],
	deps = [
		":test_runner",
		"//src:packfiles",
		"@tut//:tut",
	],
	data = [
        "//packFilesTest/testData:binary",
        "//packFilesTest/testData:dats",
        "//packFilesTest/testData:txts",
    ],
	copts = select({
		"@bazel_tools//src/conditions:windows": ["-DBAZEL_FOR_WINDOWS"],
		"//conditions:default": [],
	}),
)

cc_test(
	name = "packed_files_text",
	srcs = [
		"TestPackedFilesText.cpp",
		"test_data_deps.inc",
	],
	deps = [
		":test_runner",
		"//src:packfiles",
		"@tut//:tut",
	],
	data = ["//packFilesTest/testData:dats"],
	copts = select({
		"@bazel_tools//src/conditions:windows": ["-DBAZEL_FOR_WINDOWS"],
		"//conditions:default": [],
	}),
)

cc_test(
	name = "recursive_packed_files",
	srcs = [
		"TestRecursivePackedFiles.cpp",
		"test_data_deps.inc",
	],
	deps = [
		":test_runner",
		"//src:packfiles",
		"@tut//:tut",
	],
	data = ["//packFilesTest/testData:dats"],
	copts = select({
		"@bazel_tools//src/conditions:windows": ["-DBAZEL_FOR_WINDOWS"],
		"//conditions:default": [],
	}),
)

cc_test(
	name = "resource",
	srcs = [
		"TestResource.cpp",
		"test_data_deps.inc",
	],
	deps = [
		":test_runner",
		"//src:packfiles",
		"@tut//:tut",
	],
	data = [":resourceFile"],
	copts = select({
		"@bazel_tools//src/conditions:windows": ["-DBAZEL_FOR_WINDOWS"],
		"//conditions:default": [],
	}),
)

filegroup(
    name = "resourceFile",
    srcs = ["packFilesTest.cpp"],
)
