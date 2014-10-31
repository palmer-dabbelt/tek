#include "tempdir.bash"

cat >input <<"EOF"
\documentclass{article}

\begin{document}
% \includegraphics
\end{document}
EOF

cat >gold <<"EOF"
\documentclass{article}

\begin{document}
\end{document}
EOF

#include "harness.bash"
