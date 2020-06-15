#include <yaoosl/parser/yaoosl.tab.h>
#include <yaoosl/parser/yaoosl_compilationunit.h>
#include <yaoosl/parser/yaoosl_cstnode.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>

static struct filereadres {
    size_t len;
    char* contents;
};
static int get_bom_skip(char* ubuff)
{
    if (!ubuff)
        return 0;
    // We are comparing against unsigned
    if (ubuff[0] == 0xEF && ubuff[1] == 0xBB && ubuff[2] == 0xBF)
    {
        //UTF-8
        return 3;
    }
    else if (ubuff[0] == 0xFE && ubuff[1] == 0xFF)
    {
        //UTF-16 (BE)
        return 2;
    }
    else if (ubuff[0] == 0xFE && ubuff[1] == 0xFE)
    {
        //UTF-16 (LE)
        return 2;
    }
    else if (ubuff[0] == 0x00 && ubuff[1] == 0x00 && ubuff[2] == 0xFF && ubuff[3] == 0xFF)
    {
        //UTF-32 (BE)
        return 2;
    }
    else if (ubuff[0] == 0xFF && ubuff[1] == 0xFF && ubuff[2] == 0x00 && ubuff[3] == 0x00)
    {
        //UTF-32 (LE)
        return 2;
    }
    else if (ubuff[0] == 0x2B && ubuff[1] == 0x2F && ubuff[2] == 0x76 &&
        (ubuff[3] == 0x38 || ubuff[3] == 0x39 || ubuff[3] == 0x2B || ubuff[3] == 0x2F))
    {
        //UTF-7
        return 4;
    }
    else if (ubuff[0] == 0xF7 && ubuff[1] == 0x64 && ubuff[2] == 0x4C)
    {
        //UTF-1
        return 3;
    }
    else if (ubuff[0] == 0xDD && ubuff[1] == 0x73 && ubuff[2] == 0x66 && ubuff[3] == 0x73)
    {
        //UTF-EBCDIC
        return 3;
    }
    else if (ubuff[0] == 0x0E && ubuff[1] == 0xFE && ubuff[2] == 0xFF)
    {
        //SCSU
        return 3;
    }
    else if (ubuff[0] == 0xFB && ubuff[1] == 0xEE && ubuff[2] == 0x28)
    {
        //BOCU-1
        if (ubuff[3] == 0xFF)
            return 4;
        return 3;
    }
    else if (ubuff[0] == 0x84 && ubuff[1] == 0x31 && ubuff[2] == 0x95 && ubuff[3] == 0x33)
    {
        //GB 18030
        return 3;
    }
    return 0;
}
static struct filereadres readFile(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    struct filereadres res = { 0 };
    size_t file_size;
    char* out;
    int bom_skip, i;
    if (!file)
    {
        return res;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (!(out = malloc(sizeof(char) * (file_size + 1))))
    {
        fclose(file);
        return res;
    }
    if (file_size > 5)
    {
        fread(out, sizeof(char), 5, file);
        out[5] = '\0';
        bom_skip = get_bom_skip(out);
        for (i = bom_skip; i < 5; i++)
        {
            out[i - bom_skip] = out[i];
        }
        fread(out + 5 - bom_skip, sizeof(char), file_size - 5, file);
    }
    else
    {
        fread(out, sizeof(char), file_size, file);
    }
    out[file_size] = '\0';
    fclose(file);
    res.contents = out;
    res.len = file_size;
    return res;
}

void yaoosl_compile_test(const char* filepath)
{
    yaoosl_compilationunit ycu = { 0 };
    yyscan_t scan = { 0 };
    
    size_t filepath_len = strlen(filepath);
    if (filepath_len > UINT16_MAX)
    {
        return;
    }
    struct filereadres res = readFile(filepath);
    yaoosl_compilation_parse(&ycu, filepath, res.contents, res.len);
    if (ycu.errored)
    {
        return;
    }
    free(res.contents);
    yaoosl_yycstnode_printf(ycu.parse_0);
}