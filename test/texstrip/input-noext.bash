#include "tempdir.bash"

cat >>input <<"EOF"
\documentclass{article}

\begin{document}
Main before
\input{subchapter}
Main end
\end{document}
EOF

cat >>gold <<"EOF"

Main before
\input{subchapter}
Main end
EOF

#include "harness.bash"
