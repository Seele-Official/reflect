#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <cstddef>
#include <llvm/Support/raw_ostream.h>
#include <fstream>
using namespace clang;
char **ARGV;



auto codegen(const CXXRecordDecl *decl) {
    std::string code;
    code += "\n//static reflect--------------------------------\n";
    code += "template<>\n";
    code += "constexpr staticReflectVar staticReflect<" + decl->getName().str() + ">(" + decl->getName().str() + " &c, std::string_view name) {\n";
    code += "constexpr auto keynames = std::array<map, ";
    size_t i = 0;
    for (const auto *field : decl->fields())
        i++;
    std::string size = std::to_string(i);
    code += size;
    code += ">{\n";


    for (const auto *field : decl->fields()) {
        code += "map{\"";
        code += field->getName().str();
        code += "\", offsetof(";
        code += decl->getName().str();
        code += ", ";
        code += field->getName().str();
        code += "), sizeof(";
        code += field->getQualifiedNameAsString();
        code += ")},\n";
    }
    code += "};\n";
    code += "for (const auto& keyname : keynames){\n";
    code += "if (keyname.keyname == name){\n";
    code += "return staticReflectVar{&c, keyname.offset, keyname.size};\n";
    code += "}\n";
    code += "}\n";
    code += "return staticReflectVar{};\n";
    code += "}\n";

    code += "//static reflect--------------------------------\n";
    return code;
    
}


class MyTraverser : public RecursiveASTVisitor<MyTraverser> {
public:
    explicit MyTraverser(Rewriter &R) : TheRewriter(R) {}

    bool TraverseCXXRecordDecl(CXXRecordDecl *decl) {
        if (decl) {
            if (decl->hasAttrs()) {                
                if (const auto *annotateAttr = decl->getAttr<AnnotateAttr>()) {
                    llvm::outs() << "Annotation: " << annotateAttr->getAnnotation() << "\n";
                    if (annotateAttr->getAnnotation() == "reflect") {
                        llvm::outs() << "--------------------------------\n";
                        llvm::outs() << "Found reflect class\n";
                        SourceLocation endLoc = decl->getEndLoc();
                        if (endLoc.isValid()) {
                            while (*TheRewriter.getSourceMgr().getCharacterData(endLoc) != ';') {
                                endLoc = endLoc.getLocWithOffset(1);
                            }
                            endLoc = endLoc.getLocWithOffset(1);
                            std::string code = codegen(decl);
                            TheRewriter.InsertTextAfter(endLoc, code);
                            llvm::outs() << "Codegen: " << code << "\n";
                        }
                        llvm::outs() << "--------------------------------\n";
                    }
                }
            }
        }
        return RecursiveASTVisitor::TraverseCXXRecordDecl(decl);
    }
private:
    Rewriter &TheRewriter;
};

class ASTTraverser : public clang::ASTConsumer {
public:
    explicit ASTTraverser(Rewriter &R) : visitor(R) {}
    void HandleTranslationUnit(clang::ASTContext &Context) override {
        llvm::outs() << "Traversing AST\n";
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
        // Context.getTranslationUnitDecl()->dump();
        
    }
private:
    MyTraverser visitor;
};

class ASTAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef file) override {
        Rewriter &R = TheRewriter;
        R.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<ASTTraverser>(R);
    }


    void EndSourceFileAction() override {

        std::fstream file(ARGV[2], std::ios::out);
        if (!file.is_open()) {
            return;
        }
        auto& buffer = TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID());
        for (auto it = buffer.begin(); it != buffer.end(); ++it) {
            file << *it;
        }
        
        file.close();
    }

private:
    Rewriter TheRewriter;
};





int main(int argc, char **argv) {
    ARGV = argv;

    if (argc != 3){
        return 1;
    }



    std::fstream file(ARGV[1], std::ios::in);

    if (!file.is_open()) {
        return 1;
    }

    std::string code;
    while (!file.eof()) {
        std::string line;
        std::getline(file, line);
        code += line + "\n";
    }
    clang::tooling::runToolOnCode(std::make_unique<ASTAction>(), code.c_str());

    return 0;
}