#include "tempdir.bash"

cat >>main.tex <<"EOF"
\documentclass{article}

\begin{document}
Main before
\input{subchapter}
Main end
\end{document}
EOF

cat >>subchapter.tex <<"EOF"
\documentclass{article}

\begin{document}
Sub Chapter
\end{document}
EOF

#include "harness.bash"
