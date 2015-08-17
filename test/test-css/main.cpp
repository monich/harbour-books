/*
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <glib.h>

#define private public
#include "StyleSheetParser.h"
#undef private

#include "ZLInputStream.h"
#include "ZLStringUtil.h"
#include "ZLTextStyle.h"
#include "ZLFile.h"

#include "filesystem/ZLQtFSManager.h"

#include <iostream>
#include <fstream>
#include <sstream>

#define RET_OK          (0)
#define RET_CMD_LINE    (1)
#define RET_ERR_IO      (2)
#define RET_ERR_TEST    (3)

static void
dump_length(
    const ZLTextStyleEntry& entry,
    ZLTextStyleEntry::Length type,
    const char* name,
    std::ostream& out)
{
    if (entry.lengthSupported(type)) {
        const ZLTextStyleEntry::LengthType* length = entry.myLengths + type;
        out << "    " << name << ": ";
        if (!length->Size) {
            out << "0";
        } else {
            switch (entry.myLengths[type].Unit) {
            case ZLTextStyleEntry::SIZE_UNIT_PIXEL:
                out << length->Size << "px";
                break;
            case ZLTextStyleEntry::SIZE_UNIT_EM_100:
                out << length->Size/100. << "em";
                break;
            case ZLTextStyleEntry::SIZE_UNIT_EX_100:
                out << length->Size/100. << "ex";
                break;
            case ZLTextStyleEntry::SIZE_UNIT_PERCENT:
                out << length->Size << "%";
                break;
            case ZLTextStyleEntry::SIZE_UNIT_AUTO:
                out << "auto";
                break;
            }
        }
        out << ";\n";
    }
}

static void
dump_style(
    const StyleSheetTable::Style& style,
    std::ostream& out)
{
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_WIDTH, "width", out);
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_LEFT_INDENT, "margin-left", out);
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_SPACE_AFTER, "margin-bottom", out);
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, "margin-top", out);
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, "margin-right", out);
    dump_length(style.TextStyle, ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA, "text-indent", out);
    if (style.PageBreakBefore != B3_UNDEFINED) {
        out << "    page-break-before: " <<
            ((style.PageBreakBefore == B3_TRUE) ? "always" : "avoid") << ";\n";
    }
    if (style.PageBreakAfter != B3_UNDEFINED) {
        out << "    page-break-before: " <<
            ((style.PageBreakAfter == B3_TRUE) ? "always" : "avoid") << ";\n";
    }
    if (style.WhiteSpace != StyleSheetTable::WS_UNDEFINED) {
        static const char* values[] = {NULL, "normal", "nowrap", "pre", "pre-wrap", "pre-line" };
        out << "    white-space: " << values[style.WhiteSpace] << ";\n";
    }
    if (style.DisplayNone) {
        out << "    display: none;\n";
    }
    if (style.TextStyle.alignmentTypeSupported()) {
        const char* align;
        switch (style.TextStyle.alignmentType()) {
        case ALIGN_LEFT: align = "left"; break;
        case ALIGN_RIGHT: align = "right"; break;
        case ALIGN_CENTER: align = "center"; break;
        case ALIGN_JUSTIFY: align = "justify"; break;
        default: align = NULL; break;
        }
        if (align) {
            out << "    text-align: " << align << ";\n";
        }
    }
    const unsigned char supMod = style.TextStyle.supportedFontModifier();
    const unsigned char mod = style.TextStyle.fontModifier();
    if (supMod & FONT_MODIFIER_BOLD) {
        out << "    font-weight: " <<
            ((mod & FONT_MODIFIER_BOLD) ? "bold" : "normal") << ";\n";
    }
    if (supMod & FONT_MODIFIER_ITALIC) {
        out << "    font-style: " <<
            ((mod & FONT_MODIFIER_ITALIC) ? "italic" : "normal") << ";\n";
    }
    if (supMod & FONT_MODIFIER_SMALLCAPS) {
        out << "    font-variant: " <<
            ((mod & FONT_MODIFIER_SMALLCAPS) ? "small-caps" : "normal") << ";\n";
    }
    if (style.TextStyle.fontFamiliesSupported()) {
        const std::vector<std::string>& fonts = style.TextStyle.fontFamilies();
        if (!fonts.empty()) {
            out << "    font-family: ";
            std::vector<std::string>::const_iterator it;
            for (it = fonts.begin(); it != fonts.end(); ++it) {
                if (it != fonts.begin()) out << ", ";
                out << "\"" << *it << "\"";
            }
            out << ";\n";
        }
    }
    if (style.TextStyle.fontSizeSupported()) {
        out << "    font-size: ";
        const signed char mag = style.TextStyle.fontSizeMag();
        int size = 100;
        if (mag >= 0) {
            for (int i = 0; i < mag; ++i) size *= 6;
            for (int i = 0; i < mag; ++i) size /= 5;
        } else {
            for (int i = 0; i > mag; --i) size *= 5;
            for (int i = 0; i > mag; --i) size /= 6;
        }
        out << size << "%; /* " <<  ((int)mag) << " */\n";
    }
    if (style.TextStyle.colorSupported()) {
        out << "    color: " << ZLTextStyle::colorStyle(style.TextStyle.color()) << ";\n";
    }
}

static void
dump_table(
    const StyleSheetTable& table,
    std::ostream& out)
{
    std::vector<StyleSheetTable::Entry>::const_iterator it;
    for (it = table.myEntries.begin(); it != table.myEntries.end(); ++it) {
        const StyleSheetTable::Entry& entry = *it;
        StyleSheetTable::SelectorList::const_iterator sit;
        for (sit = entry.Selectors.begin(); sit != entry.Selectors.end(); ++sit) {
            const StyleSheetTable::Selector& selector = *sit;
            out << selector.myType;
            if (!selector.myClass.empty()) out << "." << selector.myClass;
            if (!selector.myId.empty()) out << "#" << selector.myId;
            out << " ";
        }
        out << "{\n";
        dump_style(entry.Style, out);
        out << "}\n";
    }
}

static int
process_file(
    std::string fname,
    std::ostream& out)
{
    ZLFile file(fname);
    shared_ptr<ZLInputStream> stream = file.inputStream();
    if (!stream.isNull() && stream->open()) {
        StyleSheetTable table;
        StyleSheetTableParser parser(table);
        parser.parse(*stream);
        dump_table(table, out);
        return RET_OK;
    } else {
        std::cerr << "Failed to open " << fname << std::endl;
        return RET_ERR_IO;
    }
}

static int
process(
    std::string name)
{
    int ret;
    ZLFile file(name);
    if (file.isDirectory()) {
        std::string in(name + "/in.css");
        std::string res(name + "/out.css");
        std::ostringstream out;
        ret = process_file(in, out);
        if (ret == RET_OK) {
            std::ifstream fres(res.c_str());
            if (fres) {
                std::stringstream buf;
                buf << fres.rdbuf();
                fres.close();
                if (buf.str() != out.str()) {
                    std::cerr << "Test output mismatch with " << res << std::endl;
                    //std::cerr << out.str();
                    ret = RET_ERR_TEST;
                }
            } else {
                std::cerr << "Failed to open " << res << std::endl;
                ret = RET_ERR_IO;
            }
        }
        std::cerr << ((ret == RET_OK) ? "OK" : "FAIL") << ": " << name << std::endl;
    } else {
        ret = process_file(name, std::cout);
    }
    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    char* customDataDir = NULL;
    gboolean autoTest = FALSE;

#define DATA_DIR "data"
    GOptionEntry entries[] = {
        { "autotest", 'a', 0, G_OPTION_ARG_NONE, &autoTest,
          "Run auto-tests", NULL },
        { "data", 'd', 0, G_OPTION_ARG_FILENAME, &customDataDir,
          "Data directory for autotest [" DATA_DIR "]", "DIR" },
        { NULL }
    };

    GError* error = NULL;
    GOptionContext* options = g_option_context_new("- CSS parsing test");
    g_option_context_add_main_entries(options, entries, NULL);
    gboolean ok = g_option_context_parse(options, &argc, &argv, &error);

    if (ok) {
        if (argc == 1 && !autoTest) {
            ret = RET_CMD_LINE;
            char* help = g_option_context_get_help(options, FALSE, NULL);
            std::cout << help;
            g_free(help);
        } else {
            ret = RET_OK;
            ZLQtFSManager::createInstance();
            if (argc > 1) {
                for (int i=1; i<argc; i++) {
                    const int ret2 = process(argv[i]);
                    if (ret2 != RET_OK && ret == RET_OK) ret = ret2;
                }
            }
            if (autoTest) {
                static const char* tests[] = {
                    "basic",
                    "test1",
                    "test2"
                };
                std::string dataDir(customDataDir ? customDataDir : DATA_DIR);
                if (!ZLStringUtil::stringEndsWith(dataDir, "/")) dataDir += "/";
                for (unsigned int i=0; i<(sizeof(tests)/sizeof(tests[0])); i++) {
                    const int ret2 = process(dataDir + tests[i]);
                    if (ret2 != RET_OK && ret == RET_OK) ret = ret2;
                }
            }
            ZLFSManager::deleteInstance();
        }
    } else {
        std::cerr << error->message << std::endl;
        g_error_free(error);
        ret = RET_CMD_LINE;
    }

    g_option_context_free(options);
    g_free(customDataDir);
    return ret;
}
