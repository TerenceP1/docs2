abc=[['n',"a(n)","b(n)","c(n)"],[1,1,1]]
for i in range(6):
    abc.append([sum(abc[-1]),sum(abc[-1])-abc[-1][1],sum(abc[-1])-abc[-1][2]])
    print(abc)
for i in range(1,8):
    abc[i]=[i]+abc[i]
def trans(M):
    return [[M[j][i] for j in range(len(M))] for i in range(len(M[0]))]
abc=trans(abc)
print(abc)
res="\\begin{center}\n\\begin{tabular}{ |c| c |c|c|c|c|c|c| }\n"
for i in abc:
    res+="\\hline\n"+" & ".join(str(j) for j in i)+"\\\\\n"
res+="\\\\\n\\hline\n\\end{tabular}\n\\end{center}"
print(res)