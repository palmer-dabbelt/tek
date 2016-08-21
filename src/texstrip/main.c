
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define BUF_SIZE 10240

static int string_index(const char *a, const char *b)
{
    size_t i;

    for (i = 0; i < strlen(a); i++) {
        if (strncmp(a + i, b, strlen(b)) == 0)
            return i;
    }

    return -1;
}

int main(int argc, char **argv)
{
    int i;
    char *input, *output, *last;
    FILE *inf, *otf;
    char buf[BUF_SIZE];
    char *subdir1, *subdir2;

    input = NULL;
    last = NULL;
    output = NULL;
    subdir1 = "";
    subdir2 = "";

    for (i = 1; i < argc; i++) {
        if (last != NULL) {
            if (strcmp(last, "-o") == 0) {
                output = argv[i];
                last = NULL;
            } else if (strcmp(last, "-i") == 0) {
                input = argv[i];
                last = NULL;
            } else if (strcmp(last, "--subdir") == 0) {
	        subdir1 = argv[i];
                subdir2 = "/";
                last = NULL;
            }
        } else
            last = argv[i];
    }

    if ((input == NULL) || (output == NULL)) {
        fprintf(stderr, "Specify both -i and -o\n");
        return 1;
    }

    inf = fopen(input, "r");
    otf = fopen(output, "w");

    while (fgets(buf, BUF_SIZE, inf) != NULL) {
        if (strncmp(buf, "\\documentclass{", strlen("\\documentclass{")) == 0)
            continue;
        if (strncmp(buf, "\\documentclass[", strlen("\\documentclass[")) == 0)
            continue;
        else if (strncmp(buf, "\\usepackage{", strlen("\\usepackage{")) == 0)
            continue;
        else if (strncmp(buf, "\\usepackage[", strlen("\\usepackage[")) == 0)
            continue;
        else if (strncmp(buf, "\\begin{docume", strlen("\\begin{docume")) ==
                 0)
            continue;
        else if (strncmp(buf, "\\end{document", strlen("\\end{document")) ==
                 0)
            continue;
        else if (string_index(buf, "\\includegraphics") != -1) {
            size_t index, iindex, oindex;;

            /* Removes all the optional agruments */
            index = string_index(buf, "\\includegraphics");
            index += strlen("\\includegraphics");
            while ((buf[index] != '{') && (buf[index] != '\0'))
                index++;
            iindex = index + 1;
            while ((buf[index] != '}') && (buf[index] != '\0'))
                index++;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0') {
                fprintf(stderr, "Bad includegraphics\n");
                return 1;
            }
            oindex = index;

            /* Appends ".pdf" to the filename */
       	    if (fwrite(buf, 1, iindex, otf) != iindex) {
                fprintf(stderr, "iindex failed\n");
	        abort();
            }
            fputs(subdir1, otf);
            fputs(subdir2, otf);
            if (fwrite(buf + iindex, 1, oindex - iindex, otf) != oindex - iindex) {
                fprintf(stderr, "oindex - iindex failed\n");
		abort();
            }
	    fputs(".pdf", otf);
            fputs(buf + index, otf);
 } else if (string_index(buf, "\\input") != -1) {
            size_t iindex, index, oindex;

            /* Removes all the optional agruments */
            index = string_index(buf, "\\input");
            index += strlen("\\input");
            while ((buf[index] != '{') && (buf[index] != '\0'))
                index++;
	    iindex = index + 1;
            while ((buf[index] != '}') && (buf[index] != '\0'))
                index++;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0') {
                fprintf(stderr, "Bad input\n");
                return 1;
            }

            /* Find the rest of the stuff to output. */
            oindex = index;
            if (strncmp(buf + index - 4, ".tex", 4) == 0)
                oindex = index - 4;
            if (strncmp(buf + index - 5, ".stex", 5) == 0)
                oindex = index - 5;

	    if (fwrite(buf, 1, iindex, otf) != iindex)
	        abort();
            fputs(subdir1, otf);
            fputs(subdir2, otf);
            if (fwrite(buf + iindex, 1, oindex - iindex, otf) != oindex - iindex)
		abort();
	    fputs(".stex", otf);
            fputs(buf + index, otf);
        } else
            fputs(buf, otf);
    }

    fclose(inf);
    fclose(otf);

    return 0;
}
