#include "tempdir.bash"

cat >main.tex <<"EOF"
\documentclass{article}
\begin{document}

% the following is a supported include
\input{chapter.stex}

% the following is a not-supported include
\input{chapters/chapter.stex}

\end{document}
EOF

cat >chapter.tex <<"EOF"
\documentclass{article}
\begin{document}
This is a sample chapter
\end{document}
EOF

mkdir chapters
cat >chapters/chapter.tex<<"EOF"
\documentclass{article}
\begin{document}
This is a sample chapter
\end{document}
EOF

#include "harness.bash"
