package(default_visibility = ["//visibility:public"])

cc_library(
	name = "pf_types",
	hdrs = [
        "pf_types.h",
    ],
)

cc_library(
	name = "packfiles",
	srcs = [
        "PackedFiles.cpp",
        "Resource.cpp",
    ],
	hdrs = [
        "PackedFiles.h",
        "Resource.h",
    ],
    deps = [":pf_types"],
)
