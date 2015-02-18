
/*
 * Copyright (C) 2011 Palmer Dabbelt
 *   <palmer@dabbelt.com>
 *
 * This file is part of tek.
 * 
 * tek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * tek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with tek.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "latex.h"

#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef HAVE_TALLOC
#include <talloc.h>
#else
#include "extern/talloc.h"
#endif

#include "../global.h"

static bool string_ends_with(const char *string, const char *end);
static int basename_len(const char *string);
static const char *restname(const char *string);
static int string_index(const char *a, const char *b);

static void process(struct processor *p_uncast, const char *filename,
                    struct stack *s, struct makefile *m);

static char *p_name;

void processor_latex_boot(void *context)
{
    p_name = talloc_strdup(context, "LATEX");
}

struct processor *processor_latex_search(void *context, const char *filename)
{
    struct processor_latex *p;

    p = NULL;

    if (string_ends_with(filename, ".tex")) {
        p = talloc(context, struct processor_latex);
        p->nopdf = false;
    }

    if (string_ends_with(filename, ".tex-nopdf")) {
        p = talloc(context, struct processor_latex);
        p->nopdf = true;
    }

    if (p != NULL) {
        p->p.name = talloc_reference(p, p_name);
        p->p.process = &process;
    }

    return (struct processor *)p;
}

bool string_ends_with(const char *string, const char *end)
{
    return strcmp(string + strlen(string) - strlen(end), end) == 0;
}

int basename_len(const char *string)
{
    int i;

    for (i = strlen(string) - 1; i >= 0; i--) {
        if (string[i] == '/')
            return i;
    }

    return 0;
}

const char *restname(const char *string)
{
    if (basename_len(string) == 0)
        return string;

    return string + basename_len(string) + 1;
}

int string_index(const char *a, const char *b)
{
    size_t i;

    for (i = 0; i < strlen(a); i++) {
        if (strncmp(a + i, b, strlen(b)) == 0)
            return i;
    }

    return -1;
}

void process(struct processor *p_uncast, const char *filename_input,
             struct stack *s, struct makefile *m)
{
    void *c;
    int cache_dir_size;
    char *cache_dir;
    char *pp_file;
    char *pp_file_html;
    char *pdf_file;
    char *html_file;
    char *out_file;
    char *out_file_html;
    struct processor_latex *p;
    FILE *inf;
    char *buf;
    int buf_size;
    char *filename;
    char *phonydeps;
    char *biblio;
    bool has_index;
    bool has_fmtcount;
    char *texputs;

    phonydeps = NULL;

    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_latex);

    /* Makes a new context */
    c = talloc_new(p);

    /* By default, there is no bibliography or index file. */
    biblio = NULL;
    has_index = false;
    has_fmtcount = false;

    /* FIXME: This is probably wrong... */
    filename = talloc_strdup(c, filename_input);
    if (p->nopdf == true)
        filename[strlen(filename) - 6] = '\0';

    /* The cache dir */
    cache_dir_size = basename_len(filename) + strlen(CACHE_DIR) + 2;
    cache_dir = talloc_array(c, char, cache_dir_size);
    cache_dir[0] = '\0';
    if (basename_len(filename) != 0) {
        strncat(cache_dir, filename, basename_len(filename));
        strcat(cache_dir, "/");
    }
    strcat(cache_dir, CACHE_DIR);

    /* The directory this file resides in */
    talloc_strndup(c, filename, basename_len(filename));

    /* The preprocessed file */
    pp_file = talloc_array(c, char,
                           cache_dir_size + strlen(restname(filename)) + 2);
    pp_file[0] = '\0';
    strcat(pp_file, cache_dir);
    strcat(pp_file, "/");
    strcat(pp_file, restname(filename));
    pp_file_html = talloc_asprintf(c, "%s-html.tex", pp_file);

    /* The output file from latex */
    pdf_file = talloc_strdup(c, pp_file);
    pdf_file[strlen(pdf_file) - 4] = '\0';
    html_file = talloc_asprintf(c, "%s.html", pdf_file);
    strcat(pdf_file, ".pdf");

    /* The actual output file the user wants */
    out_file = talloc_strdup(c, filename);
    out_file[strlen(out_file) - 4] = '\0';
    out_file_html = talloc_asprintf(c, "%s.html", out_file);
    strcat(out_file, ".pdf");

    /* First, we preprocess the .tex file */
    if (p->nopdf == false) {
        makefile_create_target(m, pp_file);
        makefile_start_deps(m);
        makefile_add_dep(m, filename);
        makefile_end_deps(m);

        makefile_start_cmds(m);
        makefile_nam_cmd(m, "echo -e \"TEXPP\\t%s\"", filename);
        makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                         cache_dir);
        makefile_add_cmd(m, "texpp -i \"%s\" -o \"%s\"", filename, pp_file);
        makefile_end_cmds(m);

        if (global_with_html) {
            makefile_create_target(m, pp_file_html);
            makefile_start_deps(m);
            makefile_add_dep(m, filename);
            makefile_end_deps(m);

            makefile_start_cmds(m);
            makefile_nam_cmd(m, "echo -e \"PPHTML\\t%s\"", filename);
            makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                             cache_dir);
            makefile_add_cmd(m, "texpp-html -i \"%s\" -o \"%s\"", filename,
                             pp_file_html);
            makefile_end_cmds(m);
        }
    }
    /* Then, we use latex to process the PDF.  Here is where we scan the
     * file for dependencies. */
    if (p->nopdf == false) {
        makefile_create_target(m, out_file);
        makefile_start_deps(m);
        makefile_add_dep(m, pp_file);
    } else {
        phonydeps = talloc_array(c, char, strlen(pp_file) + 20);
        phonydeps[0] = '\0';
        strcat(phonydeps, pp_file);
        phonydeps[strlen(phonydeps) - 4] = '\0';
        strcat(phonydeps, ".stex-stexdeps");

        makefile_create_target(m, phonydeps);
        makefile_start_deps(m);
    }

    buf_size = 10240;
    buf = talloc_array(c, char, buf_size);
    inf = fopen(filename, "r");
    if (inf == NULL) {
        fprintf(stderr, "Unable to open '%s'\n", filename);
        abort();
    }
    while (fgets(buf, buf_size, inf) != NULL) {
        if (string_index(buf, "\\input") != -1) {
            int index;
            char *included_name;
            char *full_path;
            int full_path_size;
            char *pdf_path;
            int pdf_path_size;

            /* Checks for comments */
            index = string_index(buf, "\\input");
            if (string_index(buf, "%") != -1) {
                int cindex;

                cindex = string_index(buf, "%");

                if (cindex == 0)
                    continue;

                if ((buf[cindex - 1] != '\\') && (cindex < index))
                    continue;
            }

            /* Removes all the optional agruments */
            index += strlen("\\input");
            while ((buf[index] != '{') && (buf[index] != '\0'))
                index++;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0') {
                fprintf(stderr, "Bad input\n");
                continue;
            }

            /* Stores the buffer and gets the filename */
            included_name = buf + index + 1;
            while ((buf[index] != '}') && (buf[index] != '\0'))
                index++;
            buf[index] = '\0';

            if (strcmp(included_name + strlen(included_name) - 4, ".tex") == 0)
                included_name[strlen(included_name) - 4] = '\0';
            else if (strcmp(included_name + strlen(included_name) - 5, ".stex") == 0)
                included_name[strlen(included_name) - 5] = '\0';
            included_name = talloc_asprintf(c, "%s.stex", included_name);

            /* Creates the full path */
            full_path_size =
                strlen(included_name) + basename_len(filename) + 3;
            full_path = talloc_array(c, char, full_path_size);
            full_path[0] = '\0';
            if (basename_len(filename) != 0) {
                strncat(full_path, filename, basename_len(filename));
                strcat(full_path, "/");
            }
            strcat(full_path, included_name);

            /* We need to convert to PDF so latex will input the file */
            pdf_path_size = strlen(cache_dir) + strlen(included_name) + 10;
            pdf_path = talloc_array(c, char, pdf_path_size);
            pdf_path[0] = '\0';
            strcat(pdf_path, cache_dir);
            strcat(pdf_path, "/");
            strcat(pdf_path, included_name);
            makefile_add_dep(m, pdf_path);

            /* The path to the input file should be processed */
            stack_push(s, pdf_path);
            talloc_unlink(full_path, c);
        }

        if (string_index(buf, "\\includegraphics") != -1) {
            int index;
            char *included_name;
            char *full_path;
            int full_path_size;
            char *pdf_path;
            int pdf_path_size;

            /* Checks for comments */
            index = string_index(buf, "\\includegraphics");
            if (string_index(buf, "%") != -1) {
                int cindex;

                cindex = string_index(buf, "%");

                if (cindex == 0)
                    continue;

                if ((buf[cindex - 1] != '\\') && (cindex < index))
                    continue;
            }

            /* Removes all the optional agruments */
            index += strlen("\\includegraphics");
            while ((buf[index] != '{') && (buf[index] != '\0'))
                index++;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0') {
                fprintf(stderr, "Bad includegraphics\n");
                continue;
            }

            /* Stores the buffer and gets the filename */
            included_name = buf + index + 1;
            while ((buf[index] != '}') && (buf[index] != '\0'))
                index++;
            buf[index] = '\0';

            /* Creates the full path */
            full_path_size =
                strlen(included_name) + basename_len(filename) + 3;
            full_path = talloc_array(c, char, full_path_size);
            full_path[0] = '\0';
            if (basename_len(filename) != 0) {
                strncat(full_path, filename, basename_len(filename));
                strcat(full_path, "/");
            }
            strcat(full_path, included_name);

            /* We need to convert to PDF so latex will input the file */
            pdf_path_size = strlen(cache_dir) + strlen(included_name) + 10;
            pdf_path = talloc_array(c, char, pdf_path_size);
            pdf_path[0] = '\0';
            strcat(pdf_path, cache_dir);
            strcat(pdf_path, "/");
            strcat(pdf_path, included_name);
            strcat(pdf_path, ".pdf");
            makefile_add_dep(m, pdf_path);

            /* The path to the input file should be processed */
            stack_push(s, pdf_path);
            talloc_unlink(full_path, c);
        }

        if (string_index(buf, "\\bibliography{") != -1) {
            int index;
            char *included_name;
            char *full_path;
            int full_path_size;

            /* Checks for comments */
            index = string_index(buf, "\\bibliography");
            if (string_index(buf, "%") != -1) {
                int cindex;

                cindex = string_index(buf, "%");

                if (cindex == 0)
                    continue;

                if ((buf[cindex - 1] != '\\') && (cindex < index))
                    continue;
            }

            /* Removes all the optional agruments */
            index += strlen("\\bibliography");
            while ((buf[index] != '{') && (buf[index] != '\0'))
                index++;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0') {
                fprintf(stderr, "Bad bibliography\n");
                continue;
            }

            /* Stores the buffer and gets the filename */
            included_name = buf + index + 1;
            while ((buf[index] != '}') && (buf[index] != '\0'))
                index++;
            buf[index] = '\0';

            /* Creates the full path */
            full_path_size =
                strlen(included_name) + basename_len(filename) + 9;
            full_path = talloc_array(c, char, full_path_size);
            full_path[0] = '\0';
            if (basename_len(filename) != 0) {
                strncat(full_path, filename, basename_len(filename));
                strcat(full_path, "/");
            }
            strcat(full_path, included_name);
            strcat(full_path, ".bib");

            biblio = full_path;
            makefile_add_dep(m, full_path);
        }

        if (string_index(buf, "\\printindex") != -1)
            has_index = true;

        if (string_index(buf, "\\Numberstringnum{") != -1)
            has_fmtcount = true;

        if (string_index(buf, "\\documentclass{") != -1) {
            char *cls_name;
            char *cls_file, *sty_file;

            cls_name = buf + strlen("\\documentclass{");

            cls_file = talloc_asprintf(c, "%.*s.cls",
                                       string_index(cls_name, "}"),
                                       cls_name
                );
            if (access(cls_file, R_OK) == 0)
                makefile_add_dep(m, cls_file);

            sty_file = talloc_asprintf(c, "%.*s.sty",
                                       string_index(cls_name, "}"),
                                       cls_name
                );
            if (access(sty_file, R_OK) == 0)
                makefile_add_dep(m, sty_file);
        }
    }

    TALLOC_FREE(buf);

    texputs = talloc_strdup(c, ".:..:");
    if (getenv("TEXINPUTS") != NULL)
        texputs = talloc_asprintf(c, ".:..:%s", getenv("TEXINPUTS"));

    if (p->nopdf == false) {
        makefile_end_deps(m);

        makefile_start_cmds(m);
        makefile_nam_cmd(m, "echo -e \"LATEX\\t%s\"", filename);

        if (has_fmtcount == true) {
            makefile_add_cmd(m,
                             "cd \"%s\" ; "
                             "TEXINPUTS=\"%s\" pdflatex -interaction=batchmode \"%s\" "
                             ">& /dev/null"
                             " || true",
                             cache_dir,
                             texputs, restname(pp_file));
        }

        if (biblio != NULL) {
            makefile_add_cmd(m, "cp \"%s\" \"%s\"", biblio, cache_dir);
            makefile_add_cmd(m,
                             "cd \"%s\" ; "
                             "TEXINPUTS=\"%s\" pdflatex -interaction=batchmode \"%s\" "
                             ">& /dev/null"
                             " || TEXINPUTS=\"%s\" pdflatex \"%s\"",
                             cache_dir,
                             texputs, restname(pp_file),
                             texputs, restname(pp_file));
            makefile_add_cmd(m,
                             "cd \"%s\" ; "
                             "bibtex `basename \"%s\" .tex` >& /dev/null || "
                             "bibtex `basename \"%s\" .tex`",
                             cache_dir,
                             restname(pp_file),
                             restname(pp_file));

        }

        if (has_index == true) {
            makefile_add_cmd(m,
                             "cd \"%s\" ; "
                             "TEXINPUTS=\"%s\" pdflatex -interaction=batchmode \"%s\" "
                             ">& /dev/null"
                             " || TEXINPUTS=\"%s\" pdflatex \"%s\"",
                             cache_dir,
                             texputs, restname(pp_file),
                             texputs, restname(pp_file));
            makefile_add_cmd(m,
                             "cd \"%s\" ; "
                             "makeindex `basename \"%s\" .tex`.idx ",
                             cache_dir, restname(pp_file));
        }

        makefile_add_cmd(m,
                         "cd \"%s\" ; "
                         "TEXINPUTS=\"%s\" pdflatex -interaction=batchmode \"%s\" >& /dev/null"
                         " || TEXINPUTS=\"%s\" pdflatex \"%s\"",
                         cache_dir,
                         texputs, restname(pp_file),
                         texputs, restname(pp_file));
        makefile_add_cmd(m,
                         "cd \"%s\" ; "
                         "TEXINPUTS=\"%s\" pdflatex -interaction=batchmode \"%s\" >& /dev/null"
                         " || TEXINPUTS=\"%s\" pdflatex \"%s\"",
                         cache_dir,
                         texputs, restname(pp_file),
                         texputs, restname(pp_file));
        makefile_add_cmd(m, "cp \"%s\" \"%s\".tmp", pdf_file, pdf_file);
        makefile_add_cmd(m, "mv \"%s\".tmp \"%s\"", pdf_file, out_file);
        makefile_end_cmds(m);

        if (global_with_html) {
            /* Uses pandoc to convert this file to HTML. */
            makefile_create_target(m, out_file_html);
            makefile_start_deps(m);
            makefile_add_dep(m, out_file);
            makefile_add_dep(m, pp_file_html);
            makefile_end_deps(m);
            makefile_start_cmds(m);
            makefile_nam_cmd(m, "echo -e \"HTML\\t%s\"", filename);
            makefile_add_cmd(m,
                             "cd \"%s\" && "
                             "yes '' | mk4ht xhmlatex \"%s\" >& /dev/null",
                             cache_dir, restname(pp_file_html));
            makefile_add_cmd(m, "cd \"%s\" && "
                             "cp `basename \"%s\" .tex`.html \"%s\" &&"
                             "echo '<style type=\"text/css\">' >> \"%s\" &&"
                             "cat `basename \"%s\" .tex`.css >> \"%s\" &&"
                             "echo '</style>' >> \"%s\"",
                             cache_dir,
                             restname(pp_file_html), restname(out_file_html),
                             restname(out_file_html),
                             restname(pp_file_html), restname(out_file_html),
                             restname(out_file_html));
            makefile_add_cmd(m, "cp \"%s\" \"%s\"", html_file, out_file_html);
            makefile_end_cmds(m);
        }
    } else {
        if (phonydeps != NULL) {
            makefile_end_deps(m);

            makefile_start_cmds(m);
            makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                             cache_dir);
            makefile_add_cmd(m, "date > \"%s\"", phonydeps);
            makefile_end_cmds(m);
        }
    }

    /* The only real file we added was the output pdf */
    if (p->nopdf == false) {
        makefile_add_all(m, out_file);
        if (global_with_html)
            makefile_add_all(m, out_file_html);
    }

    /* But we did also create a whole bunch of temporary files (potentially, at
     * least. */
    makefile_add_clean(m, out_file);
    makefile_add_clean(m, out_file_html);
    makefile_add_cleancache(m, cache_dir);

    /* Cleans up all the memory allocated by this code. */
    TALLOC_FREE(c);
}
