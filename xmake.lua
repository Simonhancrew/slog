target("slog")
	set_kind("static")
	set_languages("cxx17")
	add_files("src/*.cpp")
	
target("test")
	set_kind("binary")
	add_includedirs("src/")
	set_languages("cxx17")
	add_files("test/*.cpp")
	add_deps("slog")
	
