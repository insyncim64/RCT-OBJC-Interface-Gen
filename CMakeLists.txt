set(LLVM_LINK_CONPONENTS support)
set(LLVM_USED_LIBS clangFrontend clangSerialization clangDriver clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend clangRewriteCore clangEdit clangAST clangLex clangBasic)
add_definitions(-std=c++11)

add_clang_executable(rewrite-example Example.cpp)
target_link_libraries(rewrite-example clangTooling clangBasic clangASTMatchers)

