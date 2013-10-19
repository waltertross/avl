
# cfunx.awk - C functions extractor

# Copyright (c) 2013, Walter Tross <waltertross at gmail dot com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Examples of usage:
# awk -f cfunx.awk ../*.c
# awk -f cfunx.awk dir=xpl 'fun=[a-z][a-z0-9_]*_[xpl]' ../avl.c

BEGIN {
   dir = "cfunx"
   fun = "[a-zA-Z_][a-zA-Z0-9_]*"
}

!init {
   system("if test ! -d " dir "; then mkdir " dir "; else rm -f " dir "/*.c; fi")
   init = 1
}

!out && $0 ~ "^[a-zA-Z][a-zA-Z0-9_* \t]+[* \t]" fun "[ \t]*\(" && $0 !~ /^[^{]+\)[ \t]*;[ \t]*$/ {
   match($0, fun "[ \t]*\(")
   f = substr($0, RSTART, RLENGTH)
   gsub(/[ \t(]/, "", f)
   if (fset[f]) {
      for (i = 1; fset[f "__" i]; i++) {}
      f = f "__" i
   }
   fset[f] = 1
   out = dir "/" f ".c"
   if (/}/) {
      print > out
      close(out)
      out = 0
   }
}

out {
   print > out
   if (/^}/) {
      close(out)
      out = 0
   }
}
