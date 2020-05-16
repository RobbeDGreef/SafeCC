#include <arch/x86/generator.h>
#include <core.h>
#include <errorhandler.h>
#include <getopt.h>
#include <parser/parser.h>
#include <scanner.h>
#include <symbols.h>
#include <token.h>

#ifdef MODE_DEBUG
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>

void debughandler(int sig)
{
    fprintf(stderr, "Debug handler called for sig: %i\n", sig);

    void *array[30];
    size_t size = backtrace(array, 30);

    backtrace_symbols_fd(array, size, 2);
    exit(1);
}

#endif

// Simple random string generator, takes length parameter
string randomString(int l)
{
    srand(time(NULL));
    
    string s;
    for (int i = 0; i < l; i++)
        s += 'a' + rand() % 26;
    return s;
}

int main(int argc, char *const *argv)
{
#ifdef MODE_DEBUG
    signal(SIGABRT, debughandler);
    signal(SIGSEGV, debughandler);
#endif

    int opt;
    int option_index = 0;
    string outfile = "";
    string linkfile = "/tmp/" + randomString(8) + ".o";
    string asmfile = "/tmp/" + randomString(8) + ".S";
    string ppfile = "/tmp/" + randomString(8) + ".c";
    string assembler = "nasm";
    string linker = "ld";
    string preprocessor = "gcc";
    string arch = "i386";
    
    string ppFlags = " -E -I/home/robbe/Projects/Compiler/includes";
    string linkFlags =
        "-m elf_i386 -dynamic-linker /lib/ld-linux.so.2 "
        "/usr/lib/gcc/x86_64-linux-gnu/6/32/crtbeginS.o "
        "/usr/lib/gcc/x86_64-linux-gnu/6/../../../../lib32/crti.o "
        "/usr/lib/gcc/x86_64-linux-gnu/6/../../../../lib32/Scrt1.o "
        "-L/usr/lib/gcc/x86_64-linux-gnu/6/32 -L/usr/lib/i386-linux-gnu "
        "-L/usr/lib/gcc/x86_64-linux-gnu/6/../../../../lib32 -lgcc --as-needed "
        "-lgcc_s --no-as-needed -lc "
        "/usr/lib/gcc/x86_64-linux-gnu/6/32/crtendS.o "
        "/usr/lib/gcc/x86_64-linux-gnu/6/../../../../lib32/crtn.o";
    
    
    int f_onlyCompile = false;
    int f_noLink = false;
    int f_onlyPreProcess = false;
    
    static struct option long_options[] = 
    {
        /* Flags */
        {"Wconversion", no_argument, &err.f_conversionWarn, 1},
        {"Werror", no_argument, &err.f_warningAsError, 1},
        {"Compile", no_argument, &f_onlyCompile, 'S'},
        {"NoLink", no_argument, &f_noLink, 'c'},
        {"Preprocessor", no_argument, &f_onlyPreProcess, 'E'},
        {"No-Memory-Check", no_argument, &err.f_noMemChecking, 1},
        
        /* Arguments */
        {"output", required_argument, 0, 'o'},
        {"assembler", optional_argument, 0, 'a'},
        {"linker", optional_argument, 0, 'l'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "o:a:l:P:cSE", long_options,
                              &option_index)) != -1)
    {
        switch (opt)
        {
        case 0:
            break; /* set a flag */
        case 'o':
            outfile = string(optarg);
            break;
        case 'c':
            f_noLink = true;
            break;
        case 'S':
            f_onlyCompile = true;
            break;
        case 'E':
            f_onlyPreProcess = true;
            break;
        default:
            err.fatalNL("Usage: Compiler -o <OUTFILE> <INFILES>");
        }
    }
    

    if (optind >= argc)
    {
        err.fatalNL("Error infiles expected\nUsage: Compiler -o <outfile> "
                    "<infiles>");
    }
    
    if (outfile == "")
    {
        outfile = string(argv[optind]);
        outfile = outfile.substr(0, outfile.size() - 2);
    }
    
    if (f_noLink)
        linkfile = outfile;
    
    if (f_onlyCompile)
        asmfile = outfile;
    
    if (f_onlyPreProcess)
        ppfile = outfile;

    DEBUG("ASM: " << asmfile)
    DEBUG("PP: " << ppfile)
    GeneratorX86 generator(asmfile);
    for (; optind < argc; optind++)
    {
        system((preprocessor + ppFlags + " -o " + ppfile + " " + argv[optind]).c_str());
        
        if (f_onlyPreProcess)
            return 0;
        
        Scanner scanner(ppfile.c_str());
        err.setupLinehandler(scanner);
        generator.setupInfileHandler(scanner);

        Parser parser(scanner, generator);

        scanner.scan();

        struct ast_node *t = parser.parserMain();
        generator.generateFromAst(t, -1, 0);
    }

    generator.genDataSection();
    generator.close();

    remove(ppfile.c_str());

    int status = 0;

    if (f_onlyCompile)
        goto end;

    status =
        system((assembler + " -F dwarf -g -felf -o " + linkfile + " " + asmfile).c_str());

    if (status)
        err.fatalNL("Failed to assemble binary");

    remove(asmfile.c_str());

    if (f_noLink)
        goto end;

    status = system(
        (linker + " " + linkfile + " -o " + outfile + " " + linkFlags).c_str());

    if (status)
        err.fatalNL("Failed to link binary");

    remove(linkfile.c_str());

end:;

    return 0;
}