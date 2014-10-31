#include "tempdir.bash"

cat >>input <<"EOF"
\\documentclass{article}

\begin{document}
Main before
\input{subchapter}
Main end
\end{document}
EOF

cat >>gold <<"EOF"
\\documentclass{article}

\begin{document}
Main before
\input{subchapter.stex}
Main end
\end{document}
EOF

#include "harness.bash"
