# tut external repo

new_local_repository(
	name = "win32_tut",
	path = "D:/diego/progs/c++/lib/tut/",
	build_file_content = """
cc_library(
	name = "tut",
	srcs = glob([
		"tut/*.hpp",
	]),
	hdrs = glob([
		"*.h",
	]),
	visibility = ["//visibility:public"],
)
""",
)

new_local_repository(
	name = "linux_tut",
	path = "/usr/include/",
	build_file_content = """
cc_library(
	name = "tut",
	srcs = glob([
		"tut/*.hpp",
	]),
	hdrs = glob([
		"tut.h",
	]),
	visibility = ["//visibility:public"],
)
""",
)

new_local_repository(
	name = "tut",
	path = ".",
	build_file_content = """
cc_library(
	name = "tut",
	deps = select({
		"@bazel_tools//src/conditions:windows": ["@win32_tut//:tut"],
		"//conditions:default": ["@linux_tut//:tut"],
	}),
	visibility = ["//visibility:public"],
)
""",
)
