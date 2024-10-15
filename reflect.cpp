#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"
#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/DeclBase.h>
#include <clang/Basic/LLVM.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <fstream>
using namespace clang;
char **ARGV;


struct fieldinfo {
    std::string name;
    std::string qualifiedName;
};

auto codegen(const CXXRecordDecl *decl) {

    auto name = decl->getQualifiedNameAsString();

    auto fieldinfos = std::vector<fieldinfo>();

    for (const auto *field : decl->fields()) {
        fieldinfos.push_back({field->getName().str(), field->getQualifiedNameAsString()});
    }
    std::string size = std::to_string(fieldinfos.size());



    std::string code;
    code += "\n//static reflect--------------------------------\n";
    code += "template<>\n";
    code += "constexpr staticReflectVar staticReflect<" + name + ">(" + name + " &c, std::string_view name) {\n";
    code += "constexpr auto keynames = std::array<map, " + size + ">{\n";
    for (const auto &fieldinfo : fieldinfos) {
        code += "map{\""+ fieldinfo.name + "\", offsetof(" + name + ", " + fieldinfo.name + "), sizeof(" + fieldinfo.qualifiedName + ")},\n";
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
    code += "\n\n";



    code += "//dynamic reflect--------------------------------\n";
    code += "template <>\n";
    code += "auto& typeInfo<" + name + ">() {\n";
    code += "const static std::unordered_map<std::string_view, typeinfo> typeinfos = {\n";
    for (const auto &fieldinfo : fieldinfos) {
        code += "{\"" + fieldinfo.name + "\", {offsetof(" + name + ", " + fieldinfo.name + "), sizeof(" + fieldinfo.qualifiedName + "), std::type_index(typeid(" + fieldinfo.qualifiedName + "))}},\n";
    }
    code += "};\n";
    code += "return typeinfos;\n";
    code += "}\n";



    code += "template <>\n";
    code += "ReflectVar reflect<" + name + ">(" + name + " &c, std::string_view name) {\n";
    code += "auto& typeinfos = typeInfo<" + name + ">();\n";
    code += "auto it = typeinfos.find(name);\n";
    code += "if (it != typeinfos.end()){\n";
    code += "return ReflectVar{&c, it->second};\n";
    code += "}\n";
    code += "return ReflectVar{};\n";
    code += "}\n";
    code += "//dynamic reflect--------------------------------\n";
    return code;
    
}

// int traverseParents(const DynTypedNodeList &Parents) {
//     for (const auto &Parent : Parents) {
//         llvm::outs() << "Parent: " << Parent.getNodeKind().asStringRef() << " ";
//         switch (Parent.get<Decl>()->getKind()) {
//             case Decl::Kind::CXXRecord:
//                 llvm::outs() << llvm::dyn_cast<CXXRecordDecl>(Parent.get<Decl>())->getQualifiedNameAsString() << "\n";
//                 break;
//             case Decl::Kind::Namespace:
//                 llvm::outs() << llvm::dyn_cast<NamespaceDecl>(Parent.get<Decl>())->getQualifiedNameAsString() << "\n";
//                 break;
//             case Decl::Kind::TranslationUnit:
//                 llvm::outs() << "TranslationUnit\n";
//                 break;
//             default:
//                 llvm::outs() << "Unknown\n";
//                 break;
//         }
        
//         const auto &Parents = Parent.get<Decl>()->getASTContext().getParents(*Parent.get<Decl>());
//         if (Parents.empty()) {
//             return 0;
//         }
//         traverseParents(Parents);
//     }
//     return 0;
// }

SourceLocation findParentEndloc(const Decl *decl) {
    const auto &Parents = decl->getASTContext().getParents(*decl);
    for (const auto &Parent : Parents) {
        switch (Parent.get<Decl>()->getKind()) {
            case Decl::Kind::CXXRecord:
                llvm::outs() << llvm::dyn_cast<CXXRecordDecl>(Parent.get<Decl>())->getQualifiedNameAsString() << "\n";
                return findParentEndloc(Parent.get<Decl>());
            case Decl::Kind::Namespace:
                llvm::outs() << llvm::dyn_cast<NamespaceDecl>(Parent.get<Decl>())->getQualifiedNameAsString() << "\n";

                return findParentEndloc(Parent.get<Decl>());
            case Decl::Kind::TranslationUnit:
                llvm::outs() << "TranslationUnit\n";
                return decl->getEndLoc();
            default:
                llvm::outs() << "Unknown\n";
                break;
        }
    }
    return SourceLocation();
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
                        llvm::outs() << "Name: " << decl->getQualifiedNameAsString() << "\n";
                        



                        SourceLocation endLoc = findParentEndloc(decl);
                        if (endLoc.isValid()) {
                            // it it also error, but now i dont care
                            while (*TheRewriter.getSourceMgr().getCharacterData(endLoc) != ';') {
                                endLoc = endLoc.getLocWithOffset(1);
                            }
                            endLoc = endLoc.getLocWithOffset(1);
                            std::string code = codegen(decl);
                            TheRewriter.InsertTextAfter(endLoc, code);
                            // llvm::outs() << "Codegen: " << code << "\n";
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