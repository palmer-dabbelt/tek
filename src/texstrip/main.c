
/*
 * Copyright (C) 2011 Daniel Dabbelt
 *   <palmem@comcast.net>
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

int main(int argc, char **argv)
{
    int i;
    char *input, *output, *last;
    FILE *inf, *otf;
    char buf[BUF_SIZE];

    input = NULL;
    last = NULL;
    output = NULL;

    for (i = 1; i < argc; i++)
    {
        if (last != NULL)
        {
            if (strcmp(last, "-o") == 0)
            {
                output = argv[i];
                last = NULL;
            }
            else if (strcmp(last, "-i") == 0)
            {
                input = argv[i];
                last = NULL;
            }
        }
        else
            last = argv[i];
    }

    if ((input == NULL) || (output == NULL))
    {
        fprintf(stderr, "Specify both -i and -o\n");
        return 1;
    }

    inf = fopen(input, "r");
    otf = fopen(output, "w");

    while (fgets(buf, BUF_SIZE, inf) != NULL)
    {
        if (strncmp(buf, "\\documentclass{", strlen("\\documentclass{")) == 0)
            continue;
        else if (strncmp(buf, "\\usepackage{", strlen("\\usepackage{")) == 0)
            continue;
        else if (strncmp(buf, "\\begin{docume", strlen("\\begin{docume")) ==
                 0)
            continue;
        else if (strncmp(buf, "\\end{document", strlen("\\end{document")) ==
                 0)
            continue;
        else
            fputs(buf, otf);
    }

    fclose(inf);
    fclose(otf);

    return 0;
}
