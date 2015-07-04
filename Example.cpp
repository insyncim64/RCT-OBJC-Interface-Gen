#include <string>
#include <iostream>
#include <algorithm>
#include <system_error>
#include "stdio.h"

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang::tooling;
using namespace llvm;
using namespace std;
using namespace clang;
using namespace clang::ast_matchers;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("-fsyntax-only -ObjC");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

// Create the ObjCMethodDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCMethodDecl> objCMethodDecl;

// Create the ObjCProtocolDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCProtocolDecl> objCProtocolDecl;

// Create the ObjCImplDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCImplDecl> objCImplDecl;

// Create the ObjCCategoryDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCCategoryDecl> objCCategoryDecl;

// Create the ObjCPropertyImplDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCPropertyImplDecl> objCPropertyImplDecl;

// Create the ObjCImplementationDecl matcher
const internal::VariadicDynCastAllOfMatcher<Decl, ObjCImplementationDecl> objCImplementationDecl;

//DeclarationMatcher MethodMatcher = methodDecl(isPublic()).bind("publicMethod");
DeclarationMatcher MethodMatcher = objCMethodDecl().bind("publicMethod");

std::vector<std::string> AllowTypes = {"Hello", "World"};

class PublicMethodPrinter : public MatchFinder::MatchCallback{
    public:
    PublicMethodPrinter(std::vector<std::string> *sourcePathList, Rewriter* R);
    public :
    
    virtual void run(const MatchFinder::MatchResult &Result) {
        if(init)
        {
            LangOptions langOpts;
            auto& sourceManager = *Result.SourceManager;
            Rewrite->setSourceMgr(sourceManager, langOpts);
            init = false;
        }
        
        if (const ObjCMethodDecl *methodDecl = Result.Nodes.getNodeAs<clang::ObjCMethodDecl>("publicMethod"))
        {
            const auto& sm = *Result.SourceManager;
            const auto fileName = sm.getFilename(methodDecl->getLocation()).str();
            
            if(!isIncluded(fileName))
            {
                    // add method here
                cout <<fileName<<endl;
                addReactNativeMethod(methodDecl);
            }
        }
    }
    private:
    bool init = true;
    Rewriter* Rewrite;
    std::vector<std::string> *sourcePathList;
    bool isIncluded(std::string fileName);
    void addReactNativeMethod(const ObjCMethodDecl *methodDecl);
};

PublicMethodPrinter::PublicMethodPrinter(std::vector<std::string>* list, Rewriter* R)
{
    sourcePathList = list;
    Rewrite = R;
}

bool PublicMethodPrinter::isIncluded(std::string fileName)
{
    if (std::find_if(sourcePathList->begin(), sourcePathList->end(),
                     [fileName](const std::string & n) -> bool
                     {
                         if (fileName.length() >= n.length())
                             return (0 == fileName.compare (fileName.length() - n.length(), n.length(), n));
                         else
                             return false;
                         
                     }
                     ) != sourcePathList->end())
    {
        return false;
    }else
    {
        return true;
    }
}

void PublicMethodPrinter::addReactNativeMethod(const ObjCMethodDecl *methodDecl)
{
    auto end   = methodDecl->getLocEnd();
    
    
}

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    
    auto sourcePathList = OptionsParser.getSourcePathList();
    
    ClangTool Tool(OptionsParser.getCompilations(),
                   sourcePathList);
    
    Rewriter rewriter;
    PublicMethodPrinter Printer(&sourcePathList, &rewriter);
    MatchFinder Finder;
    Finder.addMatcher(MethodMatcher, &Printer);
    
    Tool.run(newFrontendActionFactory(&Finder).get());
    
    
    // Convert <file>.c to <file_out>.c
    std::string outName (argv[1]);
    size_t ext = outName.rfind(".");
    if (ext == std::string::npos)
        ext = outName.length();
    outName.insert(ext, "_out");

    llvm::errs() << "Output to: " << outName << "\n";
    std::error_code OutErrorInfo;
    std::error_code ok;
    llvm::raw_fd_ostream outFile(llvm::StringRef(outName), OutErrorInfo, llvm::sys::fs::F_None);
    
    if (OutErrorInfo == ok)
    {
        // At this point the rewriter's buffer should be full with the rewritten
        // file contents.
        const RewriteBuffer *RewriteBuf =
        rewriter.getRewriteBufferFor(rewriter.getSourceMgr().getMainFileID());
        auto outString = string(RewriteBuf->begin(), RewriteBuf->end());
        llvm::outs() << outString;
        outFile << outString;
    }
    else
    {
        llvm::errs() << "Cannot open " << outName << " for writing\n";
    }
    
    outFile.close();
    return 1;
}
