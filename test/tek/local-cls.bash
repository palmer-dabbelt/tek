#include "tempdir.bash"

cat >>class.cls <<"EOF"
\ProvidesClass{class}
\NeedsTeXFormat{LaTeX2e}[1995/06/01]
\LoadClass{article}
EOF

cat >>main.tex <<"EOF"
\documentclass{class}

\begin{document}
% \includegraphics
Source
\end{document}
EOF

#include "harness.bash"
