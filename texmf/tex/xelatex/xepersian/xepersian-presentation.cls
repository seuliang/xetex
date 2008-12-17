\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{xepersian-presentation}[2009/03/01 class for plain Persian electronic presentations in XeLaTeX]

\begingroup
 \def\notXeLaTeXformat{%
  \@latex@error{*** this class currently works only with XeLaTeX ***^^J
   You are not using XeLaTeX, so we are exiting cleanly!^^J
   Continuing may lead to 'unavailable font metrics' errors!^^J}\@ehc
  \endgroup\endinput}
\expandafter\ifx\csname XeTeXpicfile\endcsname\relax\expandafter\notXeLaTeXformat\fi
\endgroup


\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions\relax
\LoadClass{article}

\RequirePackage[foot=10.2pt,head=0pt,paperwidth=128mm,paperheight=96mm,left=5mm,top=5mm,right=7mm,bottom=8mm]{geometry}

\RequirePackage{fancyhdr}
\renewcommand{\headrulewidth}{0mm}


\renewcommand{\maketitle}%
  {\ClassError{xepersian-presentation}{Caution: ``maketitle'' command not supported}%
  {Please use ``titlepage'' environment instead}%
}

\fancypagestyle{pres}{%
\fancyhf{}%
\fancyfoot[RO,LE]{\footnotesize \thepage}%
}

\pagestyle{pres}

\newcommand{\distance}[1]{\vspace*{\stretch{#1}}}
\newcommand{\abstand}[1]{\vspace*{\stretch{#1}}}

\setlength{\parskip}{0.6ex}
\setlength{\parindent}{0mm}

\newenvironment{plainslide}[1][]%
   {{\raggedleft \large\bfseries #1\par}\par\vspace*{\stretch{1}}}%
   {\par\vspace*{\stretch{1}}\newpage}


\newenvironment{rawslide}{}{\newpage}%


\def\itemize{%
  \ifnum \@itemdepth >\thr@@\@toodeep\else
    \advance\@itemdepth\@ne
    \edef\@itemitem{labelitem\romannumeral\the\@itemdepth}%
    \expandafter
    \list
      \csname\@itemitem\endcsname
      {\def\makelabel##1{\hss\llap{##1}}}%
  \fi
    \setlength{\itemsep}{0ex}%
    \setlength{\parskip}{0.1ex}%
    \setlength{\parsep}{0ex}%
    }%
\let\enditemize =\endlist


\def\enumerate{%
  \ifnum \@enumdepth >\thr@@\@toodeep\else
    \advance\@enumdepth\@ne
    \edef\@enumctr{enum\romannumeral\the\@enumdepth}%
      \expandafter
      \list
        \csname label\@enumctr\endcsname
        {\usecounter\@enumctr\def\makelabel##1{\hss\llap{##1}}}%
  \fi
    \setlength{\itemsep}{0ex}%
    \setlength{\parskip}{0.1ex}%
    \setlength{\parsep}{0ex}%
    }%
\let\endenumerate =\endlist


\renewenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}
 \setlength{\itemsep}{0ex}%
    \setlength{\parskip}{0.1ex}%
    \setlength{\parsep}{0ex}}
               {\endlist}

\endinput