#include "tempdir.bash"

cat >>main.tex <<"EOF"
\documentclass{article}

\begin{document}
% \includegraphics
Source
\end{document}
EOF

#include "harness.bash"
